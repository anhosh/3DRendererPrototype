#include <Scene/Scene.hpp>

#include <Graphics/Buffers/InstanceBuffer.hpp>
#include <Graphics/RenderingEngine.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Outline.hpp>
#include <Scene/Components/Spectator.hpp>
#include <Util/Math/Clipping.hpp>
#include <Util/Math/Rectangle.hpp>
#include <Util/Math/Triangle.hpp>
#include <Util/Math/Vectors.hpp>

#include <glm/gtx/compatibility.hpp>

#include <algorithm>
#include <execution>
#include <ranges>

namespace views = std::ranges::views;

Scene::Scene() {
  ecs.on_construct<CompGraphics>().connect<&Scene::onConstructGraphics>(this);
  ecs.on_construct<CompOutline>().connect<&Scene::onConstructOutline>(this);
  ecs.on_construct<CompDirectionalLight>().connect<&Scene::onConstructDirectionalLight>(this);
  ecs.on_construct<CompPointLight>().connect<&Scene::onConstructPointLight>(this);
  ecs.on_construct<CompSpotlight>().connect<&Scene::onConstructSpotlight>(this);
  ecs.on_construct<CompTransform>().connect<&Scene::onConstructTransform>(this);

  ecs.on_destroy<CompOutline>().connect<&Scene::onDestroyOutline>(this);
  ecs.on_destroy<CompGraphics>().connect<&Scene::onDestroyGraphics>(this);

  ecs.on_update<CompCamera>().connect<&Scene::onUpdateCamera>(this);
  ecs.on_update<CompDirectionalLight>().connect<&Scene::onUpdateDirectionalLight>(this);
  ecs.on_update<CompPointLight>().connect<&Scene::onUpdatePointLight>(this);
  ecs.on_update<CompSpotlight>().connect<&Scene::onUpdateSpotlight>(this);
  ecs.on_update<CompTransform>().connect<&Scene::onUpdateTransform>(this);
}

void Scene::destroy() {
  mCachedSortedOpaqueMeshes.clear();
  mCachedSortedOutlines.clear();
  mCachedDraws.clear();
}

void Scene::prepareForRendering() {
  ZoneScoped;
  {
    ZoneScopedN("Collect");
    mCachedSortedOpaqueMeshes.clear();
    mCachedSortedTransparentMeshes.clear();
    mCachedSortedOutlines.clear();
    for (const auto [entity, graphics, transform] : ecs.view<const CompGraphics, const CompTransform>().each()) {
      for (size_t renderDataIndex = 0; renderDataIndex < graphics.renderData.size(); ++renderDataIndex) {
        MeshDataReference mesh(&ecs, entity, renderDataIndex);
        if (mesh.renderData().renderOptions.bTransparent) {
          mCachedSortedTransparentMeshes.push_back(mesh);
        } else {
          mCachedSortedOpaqueMeshes.push_back(mesh);
        }
        if (ecs.all_of<CompOutline>(entity)) {
          mesh.bHasOutline = true;
          mCachedSortedOutlines.push_back(mesh);
        }
      }
    }
  }
  this->sortMeshes();
  this->sortOutlines();
}

DirectionalLightSourceBuffer Scene::createDirectionalLightBufferData() const {
  ZoneScoped;

  const entt::basic_view directionalLights = ecs.view<const CompDirectionalLight, const CompCamera, const CompTransform>();

  DirectionalLightSourceBuffer buffer;
  buffer.sources.reserve(std::distance(directionalLights.begin(), directionalLights.end()));
  for (const auto [entity, light, camera, transform] : directionalLights.each()) {
    buffer.sources.push_back(DirectionalLightShaderData::from(light, camera, transform));
  }
  return buffer;
}

PointLightSourceBuffer Scene::createPointLightBufferData() const {
  ZoneScoped;

  const entt::basic_view pointLights = ecs.view<const CompPointLight, const CompTransform>();

  PointLightSourceBuffer buffer;
  buffer.sources.reserve(static_cast<size_t>(std::distance(pointLights.begin(), pointLights.end())));
  for (const auto [entity, light, transform]: pointLights.each()) {
    buffer.sources.push_back(PointLightShaderData::from(light, transform));
  }
  return buffer;
}

SpotlightSourceBuffer Scene::createSpotlightBufferData() const {
  ZoneScoped;

  const entt::basic_view spotlights = ecs.view<const CompSpotlight, const CompCamera, const CompTransform>();

  SpotlightSourceBuffer buffer;
  buffer.sources.reserve(static_cast<size_t>(std::distance(spotlights.begin(), spotlights.end())));
  for (const auto [entity, light, camera, transform]: spotlights.each()) {
    buffer.sources.push_back(SpotlightShaderData::from(light, camera, transform));
  }
  return buffer;
}

std::span<Draw> Scene::draw(const entt::entity entityCamera, RenderingEngine& renderingEngine) {
  ZoneScoped;

  mCachedDraws.clear();

  if (mCachedSortedOpaqueMeshes.empty() && mCachedSortedTransparentMeshes.empty()) {
    return mCachedDraws;
  }

  this->sortTransparentMeshes(entityCamera);

  // Schedule instanced draws for meshes.
  {
    ZoneScopedN("Schedule draws");
    InstanceBuffer instancesData;
    instancesData.instances.reserve(mCachedSortedOpaqueMeshes.size() + mCachedSortedOutlines.size());

    {
      ZoneScopedN("Opaque meshes");
      this->drawMeshes(mCachedSortedOpaqueMeshes, instancesData);
    }
    {
      ZoneScopedN("Transparent meshes");
      this->drawMeshes(mCachedSortedTransparentMeshes, instancesData);
    }

    if (skybox.has_value()) {
      ZoneScopedN("Skybox");
      mCachedDraws.push_back(Draw {
        .shaderProgramInstance = skybox->shader,
        .mesh = skybox->cubeMesh,
        .environmentMap = skybox->texture,
        .bBackfaceCulling = false,
        .bSkybox = true,
      });
    }

    {
      ZoneScopedN("Outlines");
      const size_t firstOutlineIndex = mCachedSortedOpaqueMeshes.size();
      size_t firstInstanceIndex = 0;
      size_t instanceCount = 0;
      for (size_t meshIndex = 1; meshIndex <= mCachedSortedOutlines.size(); ++meshIndex) {
        ZoneScopedN("Outline");
        MeshDataReference& firstInstance = mCachedSortedOutlines[firstInstanceIndex];
        RenderData& firstInstanceRD = firstInstance.renderData();

        ++instanceCount;
        if (meshIndex < mCachedSortedOutlines.size() &&
            (meshIndex == firstInstanceIndex || firstInstanceRD.eqIgnoreMainShader(mCachedSortedOutlines[meshIndex].renderData())))
        {
          continue;
        }

        {
          ZoneScopedN("Transforms");
          const auto instancesStart = instancesData.instances.end();
          instancesData.instances.resize(instancesData.instances.size() + instanceCount);
          const auto meshesStart = mCachedSortedOutlines.begin() + static_cast<long>(firstInstanceIndex);
          const auto meshesEnd = meshesStart + static_cast<long>(instanceCount);
          std::transform(std::execution::par_unseq, meshesStart, meshesEnd, instancesStart,
            [&](const MeshDataReference& meshRef) {
              CompTransform outlineTransform = ecs.get<const CompTransform>(meshRef.entity);
              outlineTransform.scale *= 1.1f;
              const glm::mat4 model  = outlineTransform.modelMatrix();
              const glm::mat3 normal = glm::transpose(glm::inverse(model));
              return InstanceData(model, normal);
            });
        }

        const CompOutline& outline = firstInstance.ecs->get<CompOutline>(firstInstance.entity);
        mCachedDraws.push_back(Draw {
          .shaderProgramInstance = outline.outlineShader,
          .mesh = firstInstanceRD.mesh,
          .instanceOffset = firstOutlineIndex + firstInstanceIndex,
          .instanceCount = instanceCount,
          .bBackfaceCulling = true,
          .bStencilTest = true,
        });

        firstInstanceIndex = meshIndex;
        instanceCount = 0;
      }
    }
    renderingEngine.updateInstances(0, instancesData);
  }

  return mCachedDraws;
}

void Scene::drawMeshes(const std::span<const MeshDataReference> meshes, InstanceBuffer& instanceBuffer) {
  ZoneScoped;

  size_t firstInstanceIndex = 0;
  size_t instanceCount = 0;
  for (size_t meshIndex = 1; meshIndex <= meshes.size(); ++meshIndex) {
    ZoneScopedN("Mesh");
    const MeshDataReference& firstInstance = meshes[firstInstanceIndex];
    const RenderData& firstInstanceRD = firstInstance.renderData();

    ++instanceCount;
    if (meshIndex < meshes.size() &&
        (meshIndex == firstInstanceIndex ||
         (firstInstance.bHasOutline == meshes[meshIndex].bHasOutline &&
          firstInstanceRD == meshes[meshIndex].renderData())))
    {
      continue;
    }

    {
      ZoneScopedN("Create instance data");
      const auto instancesStart = instanceBuffer.instances.end();
      instanceBuffer.instances.resize(instanceBuffer.instances.size() + instanceCount);
      const auto meshesStart = meshes.begin() + static_cast<long>(firstInstanceIndex);
      const auto meshesEnd = meshesStart + static_cast<long>(instanceCount);
      {
        ZoneScopedN("Transforms");
        std::transform(std::execution::par_unseq, meshesStart, meshesEnd, instancesStart,
          [&](const MeshDataReference& meshRef) {
            const CompTransform& transform = ecs.get<const CompTransform>(meshRef.entity);
            const glm::mat4 model  = transform.modelMatrix();
            const glm::mat3 normal = glm::transpose(glm::inverse(model));
            return InstanceData(model, normal);
          });
      }
    }

    mCachedDraws.push_back(Draw {
      .shaderProgramInstance = firstInstanceRD.shader,
      .mesh = firstInstanceRD.mesh,
      .instanceOffset = firstInstanceIndex,
      .instanceCount = instanceCount,
      .diffuseMap = firstInstanceRD.diffuseMap,
      .diffuseOverlayMap = firstInstanceRD.diffuseOverlayMap,
      .specularMap = firstInstanceRD.specularMap,
      .emissionMap = firstInstanceRD.emissionMap,
      .normalMap = firstInstanceRD.normalMap,
      .environmentMap = firstInstanceRD.environmentMap,
      .bBackfaceCulling = firstInstanceRD.renderOptions.bBackfaceCulling,
      .bWriteToStencil = meshes[firstInstanceIndex].bHasOutline,
    });

    firstInstanceIndex = meshIndex;
    instanceCount = 0;
  }
}

void Scene::sortMeshes() {
  ZoneScoped;

  std::sort(std::execution::par_unseq, mCachedSortedOpaqueMeshes.begin(), mCachedSortedOpaqueMeshes.end(),
    [&](const MeshDataReference& a, const MeshDataReference& b) {
      ZoneScopedN("Compare");
      const RenderData& rdA = a.renderData();
      const RenderData& rdB = b.renderData();
      return rdA.shader.itemID() < rdB.shader.itemID() || rdA.mesh.itemID() < rdB.mesh.itemID();
    });
}

void Scene::sortTransparentMeshes(const entt::entity entityCamera) {
  ZoneScoped;

  std::sort(std::execution::par_unseq, mCachedSortedTransparentMeshes.begin(), mCachedSortedTransparentMeshes.end(),
    [&](const MeshDataReference& a, const MeshDataReference& b) {
      ZoneScopedN("Compare");
      const CompTransform& cameraTransform = ecs.get<const CompTransform>(entityCamera);
      const float distanceA = glm::distance(cameraTransform.translation, ecs.get<const CompTransform>(a.entity).translation);
      const float distanceB = glm::distance(cameraTransform.translation, ecs.get<const CompTransform>(b.entity).translation);
      return distanceA > distanceB; // Transparent objects further away should be rendered before those closer to the camera.
    });
}

void Scene::sortOutlines() {
  ZoneScoped;

  std::sort(std::execution::par_unseq, mCachedSortedOutlines.begin(), mCachedSortedOutlines.end(),
    [](const MeshDataReference& a, const MeshDataReference& b) {
      ZoneScopedN("Compare");
      const RenderData& rdA = a.renderData();
      const RenderData& rdB = b.renderData();
      return rdA.mesh.itemID() < rdB.mesh.itemID();
    });
}

void Scene::onConstructGraphics(entt::registry&, const entt::entity enttOutline) {
  const CompGraphics& graphics = ecs.get<const CompGraphics>(enttOutline);
  if (const CompTransform* transform = ecs.try_get<const CompTransform>(enttOutline)) {
    for (const RenderData& renderData : graphics.renderData) {
      mSceneBounds.includeAABB(renderData.mesh->boundingBox().transformed(transform->modelMatrix()));
    }
  }
}

void Scene::onConstructOutline(entt::registry&, const entt::entity enttOutline) {
  ZoneScoped;

  const auto addOutline = [this](MeshDataReference& mesh) {
    mesh.bHasOutline = true;
    mCachedSortedOutlines.push_back(mesh);
    this->sortOutlines();
  };
  const auto meshHasSameEntity = [=](const MeshDataReference& mesh) { return mesh.entity == enttOutline; };
  if (const auto mesh = std::ranges::find_if(mCachedSortedOpaqueMeshes, meshHasSameEntity);
      mesh != mCachedSortedOpaqueMeshes.end())
  {
    addOutline(*mesh);
  } else if (const auto mesh = std::ranges::find_if(mCachedSortedTransparentMeshes, meshHasSameEntity);
             mesh != mCachedSortedTransparentMeshes.end())
  {
    addOutline(*mesh);
  }}

void Scene::onConstructDirectionalLight(entt::registry&, const entt::entity enttLight) {
  ZoneScoped;

  if (!ecs.all_of<CompCamera>(enttLight)) {
    ecs.emplace<CompCamera>(enttLight);
  }
  Scene::onUpdateDirectionalLight(ecs, enttLight);
}

void Scene::onConstructPointLight(entt::registry&, const entt::entity enttLight) {
  ZoneScoped;

  Scene::onUpdatePointLight(ecs, enttLight);
}

void Scene::onConstructSpotlight(entt::registry&, const entt::entity enttLight) {
  ZoneScoped;

  if (!ecs.all_of<CompCamera>(enttLight)) {
    ecs.emplace<CompCamera>(enttLight);
  }
  Scene::onUpdateSpotlight(ecs, enttLight);
}

void Scene::onConstructTransform(entt::registry&, entt::entity enttTransform) {
  ZoneScoped;

  this->onUpdateTransform(ecs, enttTransform);
}

void Scene::onDestroyGraphics(entt::registry&, const entt::entity enttGraphics) {
  ZoneScoped;

  const auto meshHasSameEntity = [=](const MeshDataReference& mesh) { return mesh.entity == enttGraphics; };

  if (auto removed = std::ranges::remove_if(mCachedSortedOutlines, meshHasSameEntity); !removed.empty()) {
    mCachedSortedOutlines.erase(removed.begin(), removed.end());
  }

  if (auto removed = std::ranges::remove_if(mCachedSortedOpaqueMeshes, meshHasSameEntity); !removed.empty()) {
    mCachedSortedOpaqueMeshes.erase(removed.begin(), removed.end());
    return; // If an opaque mesh has the entity, then a transparent mesh can't have it.
  }
  if (auto removed = std::ranges::remove_if(mCachedSortedTransparentMeshes, meshHasSameEntity); !removed.empty()) {
    mCachedSortedTransparentMeshes.erase(removed.begin(), removed.end());
  }
}

void Scene::onDestroyOutline(entt::registry&, const entt::entity enttOutline) {
  ZoneScoped;

  const auto meshHasSameEntity = [=](const MeshDataReference& mesh) { return mesh.entity == enttOutline; };
  if (const auto mesh = std::ranges::find_if(mCachedSortedOpaqueMeshes, meshHasSameEntity);
      mesh != mCachedSortedOpaqueMeshes.end())
  {
    mesh->bHasOutline = false;
  } else if (const auto mesh = std::ranges::find_if(mCachedSortedTransparentMeshes, meshHasSameEntity);
             mesh != mCachedSortedTransparentMeshes.end())
  {
    mesh->bHasOutline = false;
  }
  auto removedOutlines = std::ranges::remove_if(mCachedSortedOutlines, meshHasSameEntity);
  mCachedSortedOutlines.erase(removedOutlines.begin(), removedOutlines.end());
}

void Scene::onUpdateCamera(entt::registry&, const entt::entity enttCamera) {
  ZoneScoped;

  const CompCamera& camera = ecs.get<const CompCamera>(enttCamera);

  if (CompSpotlight* light = ecs.try_get<CompSpotlight>(enttCamera)) {
    light->outerCutOff = camera.fov * 0.5f;
  }
}

void Scene::onUpdateDirectionalLight(entt::registry&, const entt::entity enttLight) {
  ZoneScoped;

  const CompDirectionalLight& light = ecs.get<const CompDirectionalLight>(enttLight);

  const Rotation lightRotation = Rotation::fromDirection(light.direction);
  if (CompTransform* transform = ecs.try_get<CompTransform>(enttLight)) {
    transform->rotation = lightRotation;
  } else {
    ecs.emplace<CompTransform>(enttLight, CompTransform { .rotation = lightRotation });
  }

  if (CompGraphics* graphics = ecs.try_get<CompGraphics>(enttLight)) {
    for (RenderData& renderData : graphics->renderData) {
      if (renderData.shader->type() == ShaderProgramType::Light) {
        renderData.shader->uniforms["uLightColor"] = light.colors.diffuse;
      }
    }
  }
}

void Scene::onUpdatePointLight(entt::registry&, const entt::entity enttLight) {
  ZoneScoped;

  const CompPointLight& light = ecs.get<const CompPointLight>(enttLight);

  if (CompGraphics* graphics = ecs.try_get<CompGraphics>(enttLight)) {
    for (RenderData& renderData : graphics->renderData) {
      if (renderData.shader->type() == ShaderProgramType::Light) {
        renderData.shader->uniforms["uLightColor"] = light.colors.diffuse;
      }
    }
  }
}

void Scene::onUpdateSpotlight(entt::registry&, const entt::entity enttLight) {
  ZoneScoped;

  const CompSpotlight& light = ecs.get<const CompSpotlight>(enttLight);

  if (CompCamera* camera = ecs.try_get<CompCamera>(enttLight)) {
    camera->fov = light.outerCutOff * 2.0f;
  }

  if (CompGraphics* graphics = ecs.try_get<CompGraphics>(enttLight)) {
    for (RenderData& renderData : graphics->renderData) {
      if (renderData.shader->type() == ShaderProgramType::Light) {
        renderData.shader->uniforms["uLightColor"] = light.colors.diffuse;
      }
    }
  }
}

void Scene::onUpdateTransform(entt::registry&, const entt::entity enttTransform) {
  ZoneScoped;

  const CompTransform& transform = ecs.get<const CompTransform>(enttTransform);

  if (CompDirectionalLight* light = ecs.try_get<CompDirectionalLight>(enttTransform)) {
    light->direction = transform.forward();
  }

  if (CompSpotlight* light = ecs.try_get<CompSpotlight>(enttTransform)) {
    light->direction = transform.forward();
  }

  if (CompGraphics* graphics = ecs.try_get<CompGraphics>(enttTransform)) {
    for (const RenderData& renderData : graphics->renderData) {
      mSceneBounds.includeAABB(renderData.mesh->boundingBox().transformed(transform.modelMatrix()));
    }
  }

  if (const auto [spectator, camera] = ecs.try_get<const CompSpectator, const CompCamera>(enttTransform);
      spectator && camera && spectator->bDirectionalLightsFollowSpectator)
  {
    const Frustum frustum = camera->viewFrustumPerspective(transform);
    for (auto [enttLight, light, lightTransform, lightCamera] : ecs.view<const CompDirectionalLight, CompTransform, CompCamera>().each()) {
      const glm::mat4 lightView = lightTransform.viewMatrix();
      const std::array<glm::vec3, 8> viewFrustumPointsLightSpace = frustum.transform(lightView).asArray();
      const std::array<Triangle, 12> sceneBoundsTrianglesLightSpace = mSceneBounds.triangulated(lightView);

      const auto [minX, maxX] = std::ranges::minmax(viewFrustumPointsLightSpace, vecXLess<3, float>);
      const auto [minY, maxY] = std::ranges::minmax(viewFrustumPointsLightSpace, vecYLess<3, float>);
      lightCamera.clipBox = { { minX.x, minY.y, 0.0f }, { maxX.x, maxY.y, glm::epsilon<float>() } };

      std::vector<float> clippedSceneBoundsDepths;
      clippedSceneBoundsDepths.reserve(16);
      for (const Triangle& triangle : sceneBoundsTrianglesLightSpace) {
        const std::vector<Triangle> clippedTriangles = clipTriangleToRectanglePlanes(triangle, lightCamera.screenBounds());
        for (const Triangle& clippedTriangle : clippedTriangles) {
          for (const glm::vec3 point : clippedTriangle.points) {
            clippedSceneBoundsDepths.push_back(point.z);
          }
        }
      }

      if (!clippedSceneBoundsDepths.empty()) {
        const auto [minZ, maxZ] = std::ranges::minmax(clippedSceneBoundsDepths);
        lightCamera.clipBox.min.z = minZ;
        lightCamera.clipBox.max.z = maxZ;
      }

      // Reduce shimmering in shadow edges
      const glm::vec2 unitsPerTexel = lightCamera.clipBox.size().xy() / static_cast<float>(SHADOW_MAP_SIZE);
      lightCamera.clipBox.min /= glm::vec3(unitsPerTexel, 1.0f);
      lightCamera.clipBox.max /= glm::vec3(unitsPerTexel, 1.0f);
      lightCamera.clipBox.min = glm::floor(lightCamera.clipBox.min);
      lightCamera.clipBox.max = glm::floor(lightCamera.clipBox.max);
      lightCamera.clipBox.min *= glm::vec3(unitsPerTexel, 1.0f);
      lightCamera.clipBox.max *= glm::vec3(unitsPerTexel, 1.0f);
    }
  }
}

RenderData& Scene::MeshDataReference::renderData() {
  return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
}

const RenderData& Scene::MeshDataReference::renderData() const {
  return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
}

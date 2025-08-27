#include <Scene/Scene.hpp>

#include <Graphics/Buffers/InstanceBuffer.hpp>
#include <Graphics/RenderingEngine.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Outline.hpp>
#include <Scene/Components/Spectator.hpp>

#include <glm/gtx/compatibility.hpp>

#include <algorithm>
#include <execution>
#include <ranges>

namespace views = std::ranges::views;

Scene::Scene() {
  ecs.on_construct<CompOutline>().connect<&Scene::onConstructOutline>(this);
  ecs.on_construct<CompDirectionalLight>().connect<&Scene::onConstructDirectionalLight>();
  ecs.on_construct<CompPointLight>().connect<&Scene::onConstructPointLight>();
  ecs.on_construct<CompSpotlight>().connect<&Scene::onConstructSpotlight>();

  ecs.on_destroy<CompOutline>().connect<&Scene::onDestroyOutline>(this);
  ecs.on_destroy<CompGraphics>().connect<&Scene::onDestroyGraphics>(this);

  ecs.on_update<CompCamera>().connect<&Scene::onUpdateCamera>();
  ecs.on_update<CompDirectionalLight>().connect<&Scene::onUpdateDirectionalLight>();
  ecs.on_update<CompPointLight>().connect<&Scene::onUpdatePointLight>();
  ecs.on_update<CompSpotlight>().connect<&Scene::onUpdateSpotlight>();
  ecs.on_update<CompTransform>().connect<&Scene::onUpdateTransform>();
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
        mSceneBounds.includeAABB(mesh.renderData().mesh->boundingBox().transformed(transform.modelMatrix()));
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

std::span<const Draw> Scene::draw(const entt::entity entityCamera, RenderingEngine& renderingEngine) {
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
         (firstInstance.bHasOutline == meshes[meshIndex].bHasOutline && firstInstanceRD == meshes[meshIndex].renderData())))
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
      .specularMap = firstInstanceRD.specularMap,
      .emissionMap = firstInstanceRD.emissionMap,
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

void Scene::onConstructOutline(entt::registry&, const entt::entity entity) {
  const auto meshHasSameEntity = [=](const MeshDataReference& mesh) { return mesh.entity == entity; };
  auto meshesRange = std::views::concat(mCachedSortedOpaqueMeshes, mCachedSortedTransparentMeshes);
  if (const auto mesh = std::find_if(meshesRange.begin(), meshesRange.end(), meshHasSameEntity); mesh != meshesRange.end()) {
    (*mesh).bHasOutline = true;
    mCachedSortedOutlines.push_back(*mesh);
    this->sortOutlines();
  }
}

void Scene::onConstructDirectionalLight(entt::registry& registry, const entt::entity entity) {
  if (!registry.all_of<CompCamera>(entity)) {
    registry.emplace<CompCamera>(entity);
  }
  Scene::onUpdateDirectionalLight(registry, entity);
}

void Scene::onConstructPointLight(entt::registry& registry, const entt::entity entity) {
  Scene::onUpdatePointLight(registry, entity);
}

void Scene::onConstructSpotlight(entt::registry& registry, const entt::entity entity) {
  if (!registry.all_of<CompCamera>(entity)) {
    registry.emplace<CompCamera>(entity);
  }
  Scene::onUpdateSpotlight(registry, entity);
}

void Scene::onDestroyOutline(entt::registry&, const entt::entity entity) {
  const auto meshHasSameEntity = [=](const MeshDataReference& mesh) { return mesh.entity == entity; };
  auto meshesRange = std::views::concat(mCachedSortedOpaqueMeshes, mCachedSortedTransparentMeshes);
  if (const auto mesh = std::find_if(meshesRange.begin(), meshesRange.end(), meshHasSameEntity); mesh != meshesRange.end()) {
    (*mesh).bHasOutline = false;
  }
  auto removedOutlines = std::ranges::remove_if(mCachedSortedOutlines, meshHasSameEntity);
  mCachedSortedOutlines.erase(removedOutlines.begin(), removedOutlines.end());
}

void Scene::onDestroyGraphics(entt::registry&, const entt::entity entity) {
  const auto meshHasSameEntity = [=](const MeshDataReference& mesh) { return mesh.entity == entity; };

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

void Scene::onUpdateCamera(entt::registry& ecs, const entt::entity entity) {
  const CompCamera& camera = ecs.get<const CompCamera>(entity);

  if (CompSpotlight* light = ecs.try_get<CompSpotlight>(entity)) {
    light->outerCutOff = camera.fov * 0.5f;
  }
}

void Scene::onUpdateDirectionalLight(entt::registry& ecs, const entt::entity enttLight) {
  const CompDirectionalLight& light = ecs.get<const CompDirectionalLight>(enttLight);

  const glm::vec3 lightDirection = glm::normalize(light.direction);
  const glm::vec3 lookAt = [&] {
    for (auto [_, transform, camera] : ecs.view<CompSpectator, const CompTransform, const CompCamera>().each()) {
      return transform.translation + transform.forward() * (camera.near + camera.far) * 0.5f;
    }
    return glm::vec3(0.0f);
  }();
  ecs.emplace_or_replace<CompTransform>(enttLight, CompTransform {
    .translation = lookAt - lightDirection * 50.0f,
    .rotation = Rotation::fromDirection(lightDirection),
  });

  if (CompGraphics* graphics = ecs.try_get<CompGraphics>(enttLight)) {
    for (RenderData& renderData : graphics->renderData) {
      if (renderData.shader->type() == ShaderProgramType::Light) {
        renderData.shader->uniforms["uLightColor"] = light.colors.diffuse;
      }
    }
  }
}

void Scene::onUpdatePointLight(entt::registry& ecs, const entt::entity entity) {
  const CompPointLight& light = ecs.get<const CompPointLight>(entity);

  if (CompGraphics* graphics = ecs.try_get<CompGraphics>(entity)) {
    for (RenderData& renderData : graphics->renderData) {
      if (renderData.shader->type() == ShaderProgramType::Light) {
        renderData.shader->uniforms["uLightColor"] = light.colors.diffuse;
      }
    }
  }
}

void Scene::onUpdateSpotlight(entt::registry& ecs, const entt::entity entity) {
  const CompSpotlight& light = ecs.get<const CompSpotlight>(entity);

  if (CompCamera* camera = ecs.try_get<CompCamera>(entity)) {
    camera->fov = light.outerCutOff * 2.0f;
  }

  if (CompGraphics* graphics = ecs.try_get<CompGraphics>(entity)) {
    for (RenderData& renderData : graphics->renderData) {
      if (renderData.shader->type() == ShaderProgramType::Light) {
        renderData.shader->uniforms["uLightColor"] = light.colors.diffuse;
      }
    }
  }
}

void Scene::onUpdateTransform(entt::registry& ecs, const entt::entity enttTransform) {
  const CompTransform& transform = ecs.get<const CompTransform>(enttTransform);

  if (ecs.all_of<const CompSpectator, const CompCamera>(enttTransform)) {
    const CompCamera& camera = ecs.get<const CompCamera>(enttTransform);
    const Frustum frustum = camera.viewFrustumPerspective(transform);

    const glm::vec3 specPos = transform.translation;
    const glm::vec3 specDir = transform.forward();
    const glm::vec3 midpoint = specPos + specDir * (camera.near + camera.far) * 0.5f;

    for (auto [_, light, lightTransform, lightCamera] : ecs.view<const CompDirectionalLight, CompTransform, CompCamera>().each()) {
      const glm::vec3 lightDirection = glm::normalize(light.direction);
      lightTransform.translation = midpoint - lightDirection * 50.0f;

      const std::array<glm::vec3, 8> pointsLightSpace = frustum.transform(lightTransform.viewMatrix()).asArray();
      const auto [minX, maxX] = std::ranges::minmax(pointsLightSpace, [](const glm::vec3 a, const glm::vec3 b) { return a.x > b.x; });
      const auto [minY, maxY] = std::ranges::minmax(pointsLightSpace, [](const glm::vec3 a, const glm::vec3 b) { return a.y > b.y; });
      lightCamera.screenSize = { maxX.x - minX.x, maxY.y - minY.y };
    }
  }

  if (CompDirectionalLight* light = ecs.try_get<CompDirectionalLight>(enttTransform)) {
    light->direction = transform.forward();
  }

  if (CompSpotlight* light = ecs.try_get<CompSpotlight>(enttTransform)) {
    light->direction = transform.forward();
  }
}

RenderData& Scene::MeshDataReference::renderData() {
  return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
}

const RenderData& Scene::MeshDataReference::renderData() const {
  return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
}

#include <Scene/Scene.hpp>

#include <Graphics/Buffers/InstanceBuffer.hpp>
#include <Graphics/RenderingEngine.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Outline.hpp>

#include <glm/gtx/compatibility.hpp>

#include <algorithm>
#include <execution>
#include <ranges>

namespace views = std::ranges::views;

Scene::Scene() {
  ecs.on_construct<CompOutline>().connect<&Scene::onConstructOutlineComponent>(this);
  ecs.on_destroy<CompOutline>().connect<&Scene::onDestroyOutlineComponent>(this);
  ecs.on_destroy<CompGraphics>().connect<&Scene::onDestroyGraphicsComponent>(this);
  ecs.on_update<CompDirectionalLight>().connect<&Scene::onUpdateDirectionalLight>();
  ecs.on_update<CompPointLight>().connect<&Scene::onUpdatePointLight>();
  ecs.on_update<CompSpotlight>().connect<&Scene::onUpdateSpotlight>();
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
    for (const auto [entity, graphics] : ecs.view<CompGraphics>().each()) {
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

DirectionalLightSourceBuffer Scene::createDirectionalLightUniforms() const {
  ZoneScoped;

  const entt::basic_view directionalLights = ecs.view<const CompDirectionalLight>();

  DirectionalLightSourceBuffer buffer;
  buffer.sources.reserve(directionalLights.size());
  for (const auto& [entity, directionalLight]: directionalLights.each()) {
    buffer.sources.push_back(DirectionalLightUniforms::from(directionalLight));
  }
  return buffer;
}

PointLightSourceBuffer Scene::createPointLightUniforms() const {
  ZoneScoped;

  const entt::basic_view pointLights = ecs.view<const CompPointLight, const CompTransform>();

  PointLightSourceBuffer buffer;
  buffer.sources.reserve(static_cast<size_t>(std::distance(pointLights.begin(), pointLights.end())));
  for (const auto& [entity, directionalLight, transform]: pointLights.each()) {
    buffer.sources.push_back(PointLightUniforms::from(directionalLight, transform));
  }
  return buffer;
}

SpotlightSourceBuffer Scene::createSpotlightUniforms() const {
  ZoneScoped;

  const entt::basic_view spotlights = ecs.view<const CompSpotlight, const CompTransform>();

  SpotlightSourceBuffer buffer;
  buffer.sources.reserve(static_cast<size_t>(std::distance(spotlights.begin(), spotlights.end())));
  for (const auto& [entity, directionalLight, transform]: spotlights.each()) {
    buffer.sources.push_back(SpotlightUniforms::from(directionalLight, transform));
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
      .shaderProgramInstance = firstInstanceRD.shaderProgramInstance,
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
      return rdA.shaderProgramInstance.itemID() < rdB.shaderProgramInstance.itemID() ||
             rdA.mesh.itemID() < rdB.mesh.itemID();
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

void Scene::onConstructOutlineComponent(entt::registry&, const entt::entity entity) {
  const auto meshHasSameEntity = [=](const MeshDataReference& mesh) { return mesh.entity == entity; };
  auto meshesRange = std::views::concat(mCachedSortedOpaqueMeshes, mCachedSortedTransparentMeshes);
  if (const auto mesh = std::find_if(meshesRange.begin(), meshesRange.end(), meshHasSameEntity); mesh != meshesRange.end()) {
    (*mesh).bHasOutline = true;
    mCachedSortedOutlines.push_back(*mesh);
    this->sortOutlines();
  }
}

void Scene::onDestroyOutlineComponent(entt::registry&, const entt::entity entity) {
  const auto meshHasSameEntity = [=](const MeshDataReference& mesh) { return mesh.entity == entity; };
  auto meshesRange = std::views::concat(mCachedSortedOpaqueMeshes, mCachedSortedTransparentMeshes);
  if (const auto mesh = std::find_if(meshesRange.begin(), meshesRange.end(), meshHasSameEntity); mesh != meshesRange.end()) {
    (*mesh).bHasOutline = false;
  }
  auto removedOutlines = std::ranges::remove_if(mCachedSortedOutlines, meshHasSameEntity);
  mCachedSortedOutlines.erase(removedOutlines.begin(), removedOutlines.end());
}

void Scene::onDestroyGraphicsComponent(entt::registry&, const entt::entity entity) {
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

void Scene::onUpdateDirectionalLight(entt::registry& ecs, const entt::entity entity) {
  const auto& light = ecs.get<const CompDirectionalLight>(entity);
  const glm::vec3 lightDirection = glm::normalize(light.direction);
  ecs.emplace_or_replace<CompTransform>(entity, CompTransform {
    .translation = -lightDirection * 50.0f,
    .rotation = glm::vec3 {
      glm::degrees(glm::atan2(lightDirection.z, lightDirection.x)),
      glm::degrees(glm::asin(lightDirection.y)),
      0.0f,
    },
  });
  if (CompGraphics* graphics = ecs.try_get<CompGraphics>(entity)) {
    for (RenderData& renderData : graphics->renderData) {
      if (renderData.shaderProgramInstance->type() == ShaderProgramType::Light) {
        renderData.shaderProgramInstance->uniforms["uLightColor"] = light.colors.diffuse;
      }
    }
  }
}

void Scene::onUpdatePointLight(entt::registry& ecs, const entt::entity entity) {
  if (auto [graphics, light] = ecs.try_get<CompGraphics, const CompPointLight>(entity); graphics != nullptr) {
    for (RenderData& renderData : graphics->renderData) {
      if (renderData.shaderProgramInstance->type() == ShaderProgramType::Light) {
        renderData.shaderProgramInstance->uniforms["uLightColor"] = NotNull(light)->colors.diffuse;
      }
    }
  }
}

void Scene::onUpdateSpotlight(entt::registry& ecs, const entt::entity entity) {
  if (auto [graphics, light] = ecs.try_get<CompGraphics, const CompSpotlight>(entity); graphics != nullptr) {
    for (RenderData& renderData : graphics->renderData) {
      if (renderData.shaderProgramInstance->type() == ShaderProgramType::Light) {
        renderData.shaderProgramInstance->uniforms["uLightColor"] = NotNull(light)->colors.diffuse;
      }
    }
  }
}

RenderData& Scene::MeshDataReference::renderData() {
  return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
}

const RenderData& Scene::MeshDataReference::renderData() const {
  return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
}

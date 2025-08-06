#include <Scene/Scene.hpp>

#include <Graphics/Buffers/InstanceBuffer.hpp>
#include <Graphics/RenderingEngine.hpp>
#include <Scene/Components/Dirty.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Outline.hpp>

#include <algorithm>
#include <execution>
#include <ranges>

namespace views = std::ranges::views;

Scene::Scene() {
  ecs.on_construct<CompOutline>().connect<&Scene::onOutlineComponentAdded>(this);
  ecs.on_destroy<CompOutline>().connect<&Scene::onOutlineComponentDestroyed>(this);
  ecs.on_destroy<CompGraphics>().connect<&Scene::onGraphicsComponentDestroyed>(this);
}

void Scene::destroy() {
  mCachedSortedMeshes.clear();
  mCachedSortedOutlines.clear();
  mCachedDraws.clear();
}

std::span<const Draw> Scene::draw(entt::entity entityCamera, RenderingEngine& renderingEngine) {
  ZoneScoped;

  mCachedDraws.clear();
  const entt::basic_view dirtyActors = ecs.view<const CompDirty, const CompGraphics,
                                                entt::exclude_t<CompDirectionalLight, CompPointLight, CompSpotlight>>();
  ecs.erase<CompDirty>(dirtyActors.begin(), dirtyActors.end());

  if (ecs.view<const CompGraphics>().empty()) {
    return mCachedDraws;
  }

  // Sort transparent objects
  {
    ZoneScopedN("Sort transparent meshes");
    auto firstTransparent = std::ranges::find_if(mCachedSortedMeshes, [](const MeshDataReference& mesh) {
      return mesh.renderData().renderOptions.bTransparent;
    });
    std::sort(std::execution::par_unseq, firstTransparent, mCachedSortedMeshes.end(), [&](const MeshDataReference& a, const MeshDataReference& b) {
      assert(a.renderData().renderOptions.bTransparent);
      assert(b.renderData().renderOptions.bTransparent);
      ZoneScopedN("Compare transparent meshes");

      const CompTransform& cameraTransform = ecs.get<const CompTransform>(entityCamera);
      const float distanceA = glm::distance(cameraTransform.translation, ecs.get<const CompTransform>(a.entity).translation);
      const float distanceB = glm::distance(cameraTransform.translation, ecs.get<const CompTransform>(b.entity).translation);
      return distanceA > distanceB; // Transparent objects further away should be rendered before those closer to the camera.
    });
  }

  // Schedule instanced draws for meshes.
  {
    ZoneScopedN("Schedule draws");
    size_t firstInstanceIndex = 0;
    size_t instanceCount = 0;
    {
      ZoneScopedN("Meshes");
      for (size_t meshIndex = 1; meshIndex <= mCachedSortedMeshes.size(); ++meshIndex) {
        ZoneScopedN("Mesh");
        MeshDataReference& firstInstance = mCachedSortedMeshes[firstInstanceIndex];
        RenderData& firstInstanceRD = firstInstance.renderData();

        ++instanceCount;
        if (meshIndex < mCachedSortedMeshes.size() &&
            (meshIndex == firstInstanceIndex || (firstInstanceRD == mCachedSortedMeshes[meshIndex].renderData() &&
                                                 firstInstance.bHasOutline == mCachedSortedMeshes[meshIndex].bHasOutline)))
        {
          continue;
        }

        {
          ZoneScopedN("Transforms");
          InstanceBuffer instancesData;
          instancesData.instances.resize(instanceCount);
          const auto rangeStart = mCachedSortedMeshes.begin() + static_cast<long>(firstInstanceIndex);
          const auto rangeEnd = rangeStart + static_cast<long>(instanceCount);
          {
            ZoneScopedN("Create instance data");
            std::transform(std::execution::par_unseq, rangeStart, rangeEnd, instancesData.instances.begin(),
              [&](const MeshDataReference& meshRef) {
                const CompTransform& transform = ecs.get<const CompTransform>(meshRef.entity);
                const glm::mat4 model  = transform.modelMatrix();
                const glm::mat3 normal = glm::transpose(glm::inverse(model));
                return InstanceData(model, normal);
              });
          }
          renderingEngine.updateInstances(firstInstanceIndex, instancesData);
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
          .bWriteToStencil = mCachedSortedMeshes[firstInstanceIndex].bHasOutline,
        });

        firstInstanceIndex = meshIndex;
        instanceCount = 0;
      }
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
      const size_t firstOutlineIndex = mCachedSortedMeshes.size();
      firstInstanceIndex = 0;
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
          InstanceBuffer instancesData;
          instancesData.instances.resize(instanceCount);
          const auto rangeStart = mCachedSortedOutlines.begin() + static_cast<long>(firstInstanceIndex);
          const auto rangeEnd = rangeStart + static_cast<long>(instanceCount);
          std::transform(std::execution::par_unseq, rangeStart, rangeEnd, instancesData.instances.begin(),
            [&](const MeshDataReference& meshRef) {
              CompTransform outlineTransform = ecs.get<const CompTransform>(meshRef.entity);
              outlineTransform.scale *= 1.1f;
              const glm::mat4 model  = outlineTransform.modelMatrix();
              const glm::mat3 normal = glm::transpose(glm::inverse(model));
              return InstanceData(model, normal);
            });
          renderingEngine.updateInstances(firstOutlineIndex + firstInstanceIndex, instancesData);
        }

        const CompOutline& outline = firstInstance.ecs->get<CompOutline>(firstInstance.entity);
        mCachedDraws.push_back(Draw {
          .shaderProgramInstance = outline.outlineShader,
          .mesh = firstInstanceRD.mesh,
          .instanceOffset = firstOutlineIndex + firstInstanceIndex,
          .instanceCount = instanceCount,
          .bBackfaceCulling = true,
          .bStencilTest = true,
          .bDepthTest = false,
        });

        firstInstanceIndex = meshIndex;
        instanceCount = 0;
      }
    }
  }

  return mCachedDraws;
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

void Scene::onOutlineComponentAdded(entt::registry&, const entt::entity entity) {
  const auto mesh = std::ranges::find_if(mCachedSortedMeshes, [=](const MeshDataReference& mesh) {
    return mesh.entity == entity;
  });
  if (mesh != mCachedSortedMeshes.end())
  {
    mesh->bHasOutline = true;
    mCachedSortedOutlines.push_back(*mesh);
    this->sortOutlines();
  }
}

void Scene::onOutlineComponentDestroyed(entt::registry&, const entt::entity entity) {
  const auto mesh = std::ranges::find_if(mCachedSortedMeshes, [=](const MeshDataReference& mesh) {
    return mesh.entity == entity;
  });
  if (mesh != mCachedSortedMeshes.end())
  {
    mesh->bHasOutline = false;
  }
  auto removedOutlines = std::ranges::remove_if(mCachedSortedOutlines, [=](const MeshDataReference& mesh) {
    return mesh.entity == entity;
  });
  mCachedSortedOutlines.erase(removedOutlines.begin(), removedOutlines.end());
}

void Scene::onGraphicsComponentDestroyed(entt::registry&, const entt::entity entity) {
  auto removedMeshes = std::ranges::remove_if(mCachedSortedMeshes, [=](const MeshDataReference& mesh) {
    return mesh.entity == entity;
  });
  auto removedOutlines = std::ranges::remove_if(mCachedSortedOutlines, [=](const MeshDataReference& mesh) {
    return mesh.entity == entity;
  });
  mCachedSortedMeshes.erase(removedMeshes.begin(), removedMeshes.end());
  mCachedSortedOutlines.erase(removedOutlines.begin(), removedOutlines.end());
}

void Scene::sortMeshes() {
  ZoneScopedN("Sort meshes");
  std::sort(std::execution::par_unseq, mCachedSortedMeshes.begin(), mCachedSortedMeshes.end(), [&](const MeshDataReference& a, const MeshDataReference& b) {
    ZoneScopedN("Compare meshes");
    const RenderData& rdA = a.renderData();
    const RenderData& rdB = b.renderData();

    if (rdA.renderOptions.bTransparent != rdB.renderOptions.bTransparent) {
      return !rdA.renderOptions.bTransparent; // Opaque objects should be rendered before transparent objects.
    }

    return rdA.shaderProgramInstance.itemID() < rdB.shaderProgramInstance.itemID() ||
           rdA.mesh.itemID() < rdB.mesh.itemID();
  });
}

void Scene::sortOutlines() {
  ZoneScopedN("Sort outlines");
  std::sort(std::execution::par_unseq, mCachedSortedOutlines.begin(), mCachedSortedOutlines.end(), [](const MeshDataReference& a, const MeshDataReference& b) {
    ZoneScopedN("Compare outlines");
    const RenderData& rdA = a.renderData();
    const RenderData& rdB = b.renderData();

    return rdA.mesh.itemID() < rdB.mesh.itemID();
  });
}

void Scene::prepareForRendering() {
  ZoneScopedN("Segregate meshes");
  {
    ZoneScopedN("Collect");
    mCachedSortedMeshes.clear();
    mCachedSortedOutlines.clear();
    for (const auto [entity, graphics] : ecs.view<CompGraphics>().each()) {
      for (size_t renderDataIndex = 0; renderDataIndex < graphics.renderData.size(); ++renderDataIndex) {
        MeshDataReference& mesh = mCachedSortedMeshes.emplace_back(&ecs, entity, renderDataIndex);
        if (ecs.all_of<CompOutline>(entity)) {
          mesh.bHasOutline = true;
          mCachedSortedOutlines.push_back(mesh);
        }
      }
    }
  }
  this->sortOutlines();
  this->sortOutlines();
}

RenderData& Scene::MeshDataReference::renderData() {
  return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
}

const RenderData& Scene::MeshDataReference::renderData() const {
  return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
}


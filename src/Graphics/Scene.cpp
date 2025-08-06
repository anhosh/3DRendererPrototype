#include <Graphics/Scene.hpp>

#include <Graphics/InstanceData.hpp>
#include <Graphics/Components/Dirty.hpp>
#include <Graphics/Components/Graphics.hpp>
#include <Graphics/Components/Outline.hpp>

#include <algorithm>
#include <execution>
#include <ranges>

namespace views = std::ranges::views;

Scene::Scene() {
  ecs.on_construct<CompDirty>().connect<&Scene::onEntityMarkedDirty>(this);
  ecs.on_destroy<CompGraphics>().connect<&Scene::onGraphicsComponentDestroyed>(this);
}

void Scene::destroy() {
  mCachedSortedMeshes.clear();
  mCachedOutlinedMeshes.clear();
  mCachedDraws.clear();
}

std::span<const Draw> Scene::draw(entt::entity entityCamera) {
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
    size_t instanceBufferIndex = 0;
    size_t firstInstanceIndex = 0;
    size_t instanceCount = 0;
    {
      ZoneScopedN("Meshes");
      for (size_t meshIndex = 1; meshIndex <= mCachedSortedMeshes.size(); ++meshIndex) {
        ZoneScopedN("Mesh");
        RenderData& firstInstanceRD = mCachedSortedMeshes[firstInstanceIndex].renderData();

        ++instanceCount;
        if (meshIndex < mCachedSortedMeshes.size() &&
            (meshIndex == firstInstanceIndex || firstInstanceRD == mCachedSortedMeshes[meshIndex].renderData()))
        {
          continue;
        }

        {
          ZoneScopedN("Transforms");
          std::vector<InstanceData> instances(instanceCount);
          const auto rangeStart = mCachedSortedMeshes.begin() + static_cast<long>(firstInstanceIndex);
          const auto rangeEnd = rangeStart + static_cast<long>(instanceCount);
          {
            ZoneScopedN("Create instance data");
            std::transform(std::execution::par_unseq, rangeStart, rangeEnd, instances.begin(),
              [&](const MeshDataReference& meshRef) {
                const CompTransform& transform = ecs.get<const CompTransform>(meshRef.entity);
                const glm::mat4 model  = transform.modelMatrix();
                const glm::mat3 normal = glm::transpose(glm::inverse(model));
                return InstanceData(model, normal);
              });
          }
          firstInstanceRD.mesh->instanceData.write(instances);
        }

        mCachedDraws.push_back(Draw {
          .shaderProgramInstance = firstInstanceRD.shaderProgramInstance,
          .mesh = firstInstanceRD.mesh,
          .instanceCount = instanceCount,
          .diffuseMap = firstInstanceRD.diffuseMap,
          .specularMap = firstInstanceRD.specularMap,
          .emissionMap = firstInstanceRD.emissionMap,
          .environmentMap = firstInstanceRD.environmentMap,
          .bBackfaceCulling = firstInstanceRD.renderOptions.bBackfaceCulling,
          .bWriteToStencil = mCachedSortedMeshes[firstInstanceIndex].bHasOutline,
        });

        ++instanceBufferIndex;
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
      firstInstanceIndex = 0;
      for (size_t meshIndex = 1; meshIndex <= mCachedOutlinedMeshes.size(); ++meshIndex) {
        ZoneScopedN("Outline");
        RenderData& firstInstanceRD = mCachedOutlinedMeshes[firstInstanceIndex].renderData();

        ++instanceCount;
        if (meshIndex < mCachedSortedMeshes.size() &&
            (meshIndex == firstInstanceIndex || firstInstanceRD.eqIgnoreMainShader(mCachedSortedMeshes[meshIndex].renderData())))
        {
          continue;
        }

        {
          ZoneScopedN("Transforms");
          std::vector<InstanceData> instances(instanceCount);
          const auto rangeStart = mCachedOutlinedMeshes.begin() + static_cast<long>(firstInstanceIndex);
          const auto rangeEnd = rangeStart + static_cast<long>(instanceCount);
          std::transform(std::execution::par_unseq, rangeStart, rangeEnd, instances.begin(),
            [&](const MeshDataReference& meshRef) {
              CompTransform outlineTransform = ecs.get<const CompTransform>(meshRef.entity);
              outlineTransform.scale *= 1.05f;
              const glm::mat4 model  = outlineTransform.modelMatrix();
              const glm::mat3 normal = glm::transpose(glm::inverse(model));
              return InstanceData(model, normal);
            });
          firstInstanceRD.mesh->instanceData.write(instances);
        }

        // TODO: fix outlines
        // mCachedDraws.push_back(Draw {
        //   .shaderProgramInstance = firstInstanceRD.outlineShaderInstance.value(),
        //   .vertexArray = firstInstanceRD.vertexArray,
        //   .instanceCount = instanceCount,
        //   .bBackfaceCulling = true,
        //   .bStencilTest = true,
        //   .bDepthTest = false,
        // });

        ++instanceBufferIndex;
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

void Scene::onEntityMarkedDirty(entt::registry&, const entt::entity) {

}

void Scene::onGraphicsComponentDestroyed(entt::registry&, const entt::entity entity) {
  std::ranges::remove_if(mCachedSortedMeshes, [=](const MeshDataReference& mesh) { return mesh.entity == entity; });
  std::ranges::remove_if(mCachedOutlinedMeshes, [=](const MeshDataReference& mesh) { return mesh.entity == entity; });
}

void Scene::prepareForRendering() {
  ZoneScopedN("Segregate meshes");
  {
    ZoneScopedN("Collect");
    for (const auto [entity, graphics] : ecs.view<CompGraphics>().each()) {
      for (size_t renderDataIndex = 0; renderDataIndex < graphics.renderData.size(); ++renderDataIndex) {
        MeshDataReference& mesh = mCachedSortedMeshes.emplace_back(&ecs, entity, renderDataIndex);
        if (ecs.all_of<CompOutline>(entity)) {
          mesh.bHasOutline = true;
          mCachedOutlinedMeshes.push_back(mesh);
        }
      }
    }
  }
  {
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
  {
    ZoneScopedN("Sort outlines");
    std::sort(std::execution::par_unseq, mCachedOutlinedMeshes.begin(), mCachedOutlinedMeshes.end(), [](const MeshDataReference& a, const MeshDataReference& b) {
      ZoneScopedN("Compare outlines");
      const RenderData& rdA = a.renderData();
      const RenderData& rdB = b.renderData();

      return rdA.mesh.itemID() < rdB.mesh.itemID();
    });
  }
}

RenderData& Scene::MeshDataReference::renderData() {
  return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
}

const RenderData& Scene::MeshDataReference::renderData() const {
  return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
}


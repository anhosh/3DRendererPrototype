#include <Graphics/Scene.hpp>

#include <Graphics/Actor.hpp>
#include <Graphics/Buffers/InstanceBufferData.hpp>
#include <Graphics/Components/Dirty.hpp>
#include <Graphics/Components/Graphics.hpp>

#include <algorithm>
#include <execution>
#include <ranges>

namespace views = std::ranges::views;

void Scene::destroy() {
  ecs.clear();
}

std::span<const Draw> Scene::draw(const Camera& camera, Registry<Buffer>& buffers) {
  ZoneScoped;

  struct MeshDataReference {
    const entt::registry* ecs;
    entt::entity entity;
    size_t renderDataIndex = SIZE_MAX;

    [[nodiscard]] const RenderData& renderData() const {
      return ecs->get<CompGraphics>(entity).renderData[renderDataIndex];
    }
  };

  mCachedDraws.clear();
  entt::basic_view dirtyActors = ecs.view<const CompDirty, entt::exclude_t<CompDirectionalLight, CompPointLight, CompSpotlight>>();
  ecs.erase<CompDirty>(dirtyActors.begin(), dirtyActors.end());

  if (ecs.view<const CompGraphics>().empty()) {
    return mCachedDraws;
  }

  // Sort meshes so they are easier to group into instanced calls.
  std::vector<MeshDataReference> sortedMeshes;
  std::vector<MeshDataReference> outlinedMeshes;
  {
    ZoneScopedN("Segregate meshes");
    {
      ZoneScopedN("Collect");
      entt::basic_view actors = ecs.view<const CompGraphics>();
      for (const auto [entity, graphics] : actors.each()) {
        for (size_t renderDataIndex = 0; renderDataIndex < graphics.renderData.size(); ++renderDataIndex) {
          const MeshDataReference& mesh = sortedMeshes.emplace_back(&ecs, entity, renderDataIndex);
          if (mesh.renderData().outlineShaderInstance.has_value()) {
            assert(mesh.renderData().outlineShaderInstance.value()->type() == ShaderProgramType::Outline);
            outlinedMeshes.push_back(sortedMeshes.back());
          }
        }
      }
    }
    {
      ZoneScopedN("Sort meshes");
      std::sort(std::execution::par_unseq, sortedMeshes.begin(), sortedMeshes.end(), [&](const MeshDataReference& a, const MeshDataReference& b) {
        ZoneScopedN("Compare meshes");
        const RenderData& rdA = a.renderData();
        const RenderData& rdB = b.renderData();

        if (rdA.renderOptions.bTransparent != rdB.renderOptions.bTransparent) {
          return !rdA.renderOptions.bTransparent; // Opaque objects should be rendered before transparent objects.
        }

        if (rdA.renderOptions.bTransparent && rdB.renderOptions.bTransparent) {
          const float distanceA = glm::length(camera.position - ecs.get<const CompTransform>(a.entity).translation);
          const float distanceB = glm::length(camera.position - ecs.get<const CompTransform>(b.entity).translation);
          return distanceA > distanceB; // Transparent objects further away should be rendered before those closer to the camera.
        }

        return rdA.shaderProgramInstance.itemID() < rdB.shaderProgramInstance.itemID() ||
               rdA.vertexArray.itemID() < rdB.vertexArray.itemID();
      });
    }
    {
      ZoneScopedN("Sort outlines");
      std::sort(std::execution::par_unseq, outlinedMeshes.begin(), outlinedMeshes.end(), [&](const MeshDataReference& a, const MeshDataReference& b) {
        ZoneScopedN("Compare outlines");
        const RenderData& rdA = a.renderData();
        const RenderData& rdB = b.renderData();

        return rdA.outlineShaderInstance->itemID() < rdB.outlineShaderInstance->itemID() ||
               rdA.vertexArray.itemID() < rdB.vertexArray.itemID();
      });
    }
  }

  // Schedule instanced draws for meshes.
  {
    ZoneScopedN("Schedule draws");
    size_t instanceBufferIndex = 0;
    size_t firstInstance = 0;
    size_t instanceCount = 0;
    {
      ZoneScopedN("Meshes");
      for (size_t meshIndex = 1; meshIndex <= sortedMeshes.size(); ++meshIndex) {
        ZoneScoped;
        ZoneNamedN(Mesh, "Mesh", true);
        const RenderData& firstInstanceRD = sortedMeshes[firstInstance].renderData();

        ++instanceCount;
        if (meshIndex < sortedMeshes.size() &&
            (meshIndex == firstInstance || firstInstanceRD.eqIgnoreOutline(sortedMeshes[meshIndex].renderData())))
        {
          continue;
        }

        {
          ZoneScopedN("Transforms");
          InstanceBufferData instanceBufferData;
          instanceBufferData.instances.resize(instanceCount);
          const auto rangeStart = sortedMeshes.begin() + static_cast<long>(firstInstance);
          const auto rangeEnd = rangeStart + static_cast<long>(instanceCount);
          std::transform(std::execution::par_unseq, rangeStart, rangeEnd, instanceBufferData.instances.begin(),
            [&](const MeshDataReference& meshRef) {
              const CompTransform& transform = ecs.get<const CompTransform>(meshRef.entity);
              const glm::mat4 model  = transform.matrix();
              const glm::mat3 normal = glm::transpose(glm::inverse(model));
              return InstanceData(model, normal);
            });

          BufferHandle instanceBuffer = this->obtainInstanceBuffer(instanceBufferIndex, buffers);
          instanceBuffer->write(instanceBufferData);
          instanceBufferData.setupInstanceVertexAttributes(firstInstanceRD.vertexArray, instanceBuffer);
        }

        mCachedDraws.push_back(Draw {
          .shaderProgramInstance = firstInstanceRD.shaderProgramInstance,
          .vertexArray = firstInstanceRD.vertexArray,
          .instanceCount = instanceCount,
          .diffuseMap = firstInstanceRD.diffuseMap,
          .specularMap = firstInstanceRD.specularMap,
          .emissionMap = firstInstanceRD.emissionMap,
          .environmentMap = firstInstanceRD.environmentMap,
          .bBackfaceCulling = firstInstanceRD.renderOptions.bBackfaceCulling,
          .bWriteToStencil = firstInstanceRD.outlineShaderInstance.has_value(),
        });

        ++instanceBufferIndex;
        firstInstance = meshIndex;
        instanceCount = 0;
      }
    }

    if (skybox.has_value()) {
      ZoneScopedN("Skybox");
      mCachedDraws.push_back(Draw {
        .shaderProgramInstance = skybox->shader,
        .vertexArray = skybox->cubeMesh,
        .environmentMap = skybox->texture,
        .bBackfaceCulling = false,
        .bSkybox = true,
      });
    }

    {
      ZoneScopedN("Outlines");
      firstInstance = 0;
      for (size_t meshIndex = 1; meshIndex <= outlinedMeshes.size(); ++meshIndex) {
        ZoneScopedN("Outline");
        const RenderData& firstInstanceRD = outlinedMeshes[firstInstance].renderData();

        ++instanceCount;
        if (meshIndex < sortedMeshes.size() &&
            (meshIndex == firstInstance || firstInstanceRD.eqIgnoreOutline(sortedMeshes[meshIndex].renderData())))
        {
          continue;
        }

        {
          ZoneScopedN("Transforms");
          InstanceBufferData instanceBufferData;
          instanceBufferData.instances.resize(instanceCount);
          const auto rangeStart = outlinedMeshes.begin() + static_cast<long>(firstInstance);
          const auto rangeEnd = rangeStart + static_cast<long>(instanceCount);
          std::transform(std::execution::par_unseq, rangeStart, rangeEnd, instanceBufferData.instances.begin(),
            [&](const MeshDataReference& meshRef) {
              CompTransform outlineTransform = ecs.get<const CompTransform>(meshRef.entity);
              outlineTransform.scale *= 1.05f;
              const glm::mat4 model  = outlineTransform.matrix();
              const glm::mat3 normal = glm::transpose(glm::inverse(model));
              return InstanceData(model, normal);
            });

          BufferHandle instanceBuffer = this->obtainInstanceBuffer(instanceBufferIndex, buffers);
          instanceBuffer->write(instanceBufferData);
          instanceBufferData.setupInstanceVertexAttributes(firstInstanceRD.vertexArray, instanceBuffer);
        }

        mCachedDraws.push_back(Draw {
          .shaderProgramInstance = firstInstanceRD.outlineShaderInstance.value(),
          .vertexArray = firstInstanceRD.vertexArray,
          .instanceCount = instanceCount,
          .bBackfaceCulling = true,
          .bStencilTest = true,
          .bDepthTest = false,
        });

        ++instanceBufferIndex;
        firstInstance = meshIndex;
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

BufferHandle Scene::obtainInstanceBuffer(const size_t bufferIndex, Registry<Buffer>& buffers) const {
  ZoneScoped;

  assert(bufferIndex < mCachedInstanceBuffers.size() + 1);

  if (mCachedInstanceBuffers.size() > bufferIndex) {
    return mCachedInstanceBuffers[bufferIndex];
  }

  const BufferHandle newBuffer = buffers.add(Buffer(GL_ARRAY_BUFFER));
  mCachedInstanceBuffers.push_back(newBuffer);
  return newBuffer;
}

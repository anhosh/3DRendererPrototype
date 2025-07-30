#include <Graphics/Scene.hpp>

#include <Graphics/Actor.hpp>
#include <Graphics/Buffers/InstanceBufferData.hpp>

#include <algorithm>
#include <execution>
#include <ranges>

namespace views = std::ranges::views;

void Scene::destroy() {
  actors.clear();
}

std::span<const Draw> Scene::draw(const Camera& camera, Registry<Buffer>& buffers) const {
  ZoneScoped;
  //
  // if (actors.empty()) {
  //   return {};
  // }

  if (!actors.hasDirtyItems()) {
    return mCachedDraws;
  }
  actors.clearDirtyItems();

  // Sort meshes so they are easier to group into instanced calls.
  std::vector<MeshDataReference> sortedMeshes;
  std::vector<MeshDataReference> outlinedMeshes;
  {
    ZoneScopedN("Segregate meshes");
    {
      ZoneScopedN("Collect");
      for (const Actor& actor : actors | views::values) {
        for (size_t renderDataIndex = 0; renderDataIndex < actor.renderData.size(); ++renderDataIndex) {
          const MeshDataReference& mesh = sortedMeshes.emplace_back(&actor, renderDataIndex);
          if (mesh.renderData().outlineShaderInstance.has_value()) {
            assert(mesh.renderData().outlineShaderInstance.value()->type == ShaderProgramType::Outline);
            outlinedMeshes.push_back(sortedMeshes.back());
          }
        }
      }
    }
    {
      ZoneScopedN("Sort all");
      std::sort(std::execution::par_unseq, sortedMeshes.begin(), sortedMeshes.end(), [&](const MeshDataReference& a, const MeshDataReference& b) {
        const RenderData& rdA = a.renderData();
        const RenderData& rdB = b.renderData();

        if (rdA.renderOptions.bTransparent != rdB.renderOptions.bTransparent) {
          return !rdA.renderOptions.bTransparent; // Opaque objects should be rendered before transparent objects.
        }

        if (rdA.renderOptions.bTransparent && rdB.renderOptions.bTransparent) {
          const float distanceA = glm::length(camera.position - a.actor->transform.translation);
          const float distanceB = glm::length(camera.position - b.actor->transform.translation);
          return distanceA > distanceB; // Transparent objects further away should be rendered before those closer to the camera.
        }

        return rdA.shaderProgramInstance.itemID() < rdB.shaderProgramInstance.itemID() ||
               rdA.vertexArray.itemID() < rdB.vertexArray.itemID();
      });
    }
    {
      ZoneScopedN("Sort outlined");
      std::sort(std::execution::par_unseq, outlinedMeshes.begin(), outlinedMeshes.end(), [&](const MeshDataReference& a, const MeshDataReference& b) {
        const RenderData& rdA = a.renderData();
        const RenderData& rdB = b.renderData();

        return rdA.outlineShaderInstance->itemID() < rdB.outlineShaderInstance->itemID() ||
               rdA.vertexArray.itemID() < rdB.vertexArray.itemID();
      });
    }
  }

  // Schedule instanced draws for meshes.
  std::vector<Draw> draws;
  draws.reserve(sortedMeshes.size() + outlinedMeshes.size());
  {
    ZoneScopedN("Schedule draws");
    size_t instanceBufferIndex = 0;
    size_t firstInstance = 0;
    size_t instanceCount = 0;
    {
      ZoneScopedN("Meshes");
      for (const MeshDataReference& mesh : sortedMeshes) {
        ZoneScoped;
        ZoneNameF("Mesh: %s", mesh.actor->name.c_str());
        const RenderData& firstInstanceRD = sortedMeshes[firstInstance].renderData();

        if (&mesh != &sortedMeshes.back() && (instanceCount == 0 || firstInstanceRD.eqIgnoreOutline(mesh.renderData()))) {
          ++instanceCount;
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
              const glm::mat4 modelTransform  = meshRef.actor->transform.matrix();
              const glm::mat3 normalTransform = glm::transpose(glm::inverse(modelTransform));
              return InstanceData(modelTransform, normalTransform);
            });

          BufferHandle instanceBuffer = this->obtainInstanceBuffer(instanceBufferIndex, buffers);
          instanceBuffer->write(instanceBufferData);
          instanceBufferData.setupInstanceVertexAttributes(firstInstanceRD.vertexArray, instanceBuffer);
        }

        draws.push_back(Draw {
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
        firstInstance += instanceCount;
        instanceCount = 0;
      }
    }

    if (skybox.has_value()) {
      ZoneScopedN("Skybox");
      draws.push_back(Draw {
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
      for (const MeshDataReference& mesh: outlinedMeshes) {
        ZoneScopedN("Outline");
        const RenderData& firstInstanceRD = outlinedMeshes[firstInstance].renderData();

        if (&mesh != &outlinedMeshes.back() && (instanceCount == 0 || firstInstanceRD.eqIgnoreOutline(mesh.renderData()))) {
          ++instanceCount;
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
              Transform outlineTransform = meshRef.actor->transform;
              outlineTransform.scale *= 1.05f;
              const glm::mat4 modelTransform  = outlineTransform.matrix();
              const glm::mat3 normalTransform = glm::transpose(glm::inverse(modelTransform));
              return InstanceData(modelTransform, normalTransform);
            });

          BufferHandle instanceBuffer = this->obtainInstanceBuffer(instanceBufferIndex, buffers);
          instanceBuffer->write(instanceBufferData);
          instanceBufferData.setupInstanceVertexAttributes(firstInstanceRD.vertexArray, instanceBuffer);
        }

        draws.push_back(Draw {
          .shaderProgramInstance = firstInstanceRD.outlineShaderInstance.value(),
          .vertexArray = firstInstanceRD.vertexArray,
          .instanceCount = instanceCount,
          .bBackfaceCulling = true,
          .bStencilTest = true,
          .bDepthTest = false,
        });

        ++instanceBufferIndex;
        firstInstance = instanceCount;
        instanceCount = 0;
      }
    }
  }

  mCachedDraws = draws;
  return mCachedDraws;
}

DirectionalLightSourceBuffer Scene::createDirectionalLightUniforms() const {
  ZoneScoped;

  DirectionalLightSourceBuffer buffer;
  buffer.sources.reserve(directionalLights.size());
  for (const DirectionalLight& directionalLight: directionalLights | views::values) {
    buffer.sources.push_back(DirectionalLightUniforms::from(directionalLight));
  }
  return buffer;
}

PointLightSourceBuffer Scene::createPointLightUniforms() const {
  ZoneScoped;

  PointLightSourceBuffer buffer;
  buffer.sources.reserve(pointLights.size());
  for (const PointLight& directionalLight: pointLights | views::values) {
    buffer.sources.push_back(PointLightUniforms::from(directionalLight));
  }
  return buffer;
}

SpotlightSourceBuffer Scene::createSpotlightUniforms() const {
  ZoneScoped;

  SpotlightSourceBuffer buffer;
  buffer.sources.reserve(spotlights.size());
  for (const Spotlight& directionalLight: spotlights | views::values) {
    buffer.sources.push_back(SpotlightUniforms::from(directionalLight));
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

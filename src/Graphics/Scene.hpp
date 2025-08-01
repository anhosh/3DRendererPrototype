#pragma once

#include <Graphics/Buffer.hpp>
#include <Graphics/Buffers/LightSourceUniforms.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/Skybox.hpp>
#include <Util/Registry.hpp>

#include <entt/entity/registry.hpp>

#include <optional>
#include <span>
#include <vector>

struct RenderData;

class Scene {
public:
  Scene();
  ~Scene() { this->destroy(); }

  void prepareForRendering();
  void destroy();

  [[nodiscard]] std::span<const Draw> draw(entt::entity entityCamera, Registry<Buffer>& buffers);
  [[nodiscard]] DirectionalLightSourceBuffer createDirectionalLightUniforms() const;
  [[nodiscard]] PointLightSourceBuffer createPointLightUniforms() const;
  [[nodiscard]] SpotlightSourceBuffer createSpotlightUniforms() const;

public:
  entt::registry ecs;

  std::optional<Skybox> skybox = std::nullopt;

private:
  void onEntityMarkedDirty(entt::registry& registry, entt::entity entity);
  void onGraphicsComponentDestroyed(entt::registry& registry, entt::entity entity);

  BufferHandle obtainInstanceBuffer(size_t bufferIndex, Registry<Buffer>& buffers);

private:
  struct MeshDataReference {
    const entt::registry* ecs;
    entt::entity entity;
    size_t renderDataIndex = SIZE_MAX;
    bool bHasOutline = false;

    [[nodiscard]] const RenderData& renderData() const;
  };

  std::vector<BufferHandle> mCachedInstanceBuffers;
  std::vector<MeshDataReference> mCachedSortedMeshes;
  std::vector<MeshDataReference> mCachedOutlinedMeshes;
  std::vector<Draw> mCachedDraws;
};

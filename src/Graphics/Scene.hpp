#pragma once

#include <Graphics/Buffers/LightSourceUniforms.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/Skybox.hpp>

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

  [[nodiscard]] std::span<const Draw> draw(entt::entity entityCamera);
  [[nodiscard]] DirectionalLightSourceBuffer createDirectionalLightUniforms() const;
  [[nodiscard]] PointLightSourceBuffer createPointLightUniforms() const;
  [[nodiscard]] SpotlightSourceBuffer createSpotlightUniforms() const;

public:
  entt::registry ecs;

  std::optional<Skybox> skybox = std::nullopt;

private:
  void onEntityMarkedDirty(entt::registry& registry, entt::entity entity);
  void onGraphicsComponentDestroyed(entt::registry& registry, entt::entity entity);

private:
  struct MeshDataReference {
    entt::registry* ecs;
    entt::entity entity;
    size_t renderDataIndex = SIZE_MAX;
    bool bHasOutline = false;

    [[nodiscard]] RenderData& renderData();
    [[nodiscard]] const RenderData& renderData() const;
  };

  std::vector<MeshDataReference> mCachedSortedMeshes;
  std::vector<MeshDataReference> mCachedOutlinedMeshes;
  std::vector<Draw> mCachedDraws;
};

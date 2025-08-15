#pragma once

#include <Graphics/Buffers/LightSourceBuffer.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Skybox.hpp>
#include <Scene/Scene.hpp>

#include <entt/entity/registry.hpp>

#include <optional>
#include <span>
#include <vector>

struct InstanceBuffer;
struct RenderData;
class RenderingEngine;

class Scene {
public:
  Scene();
  ~Scene() { this->destroy(); }

  void destroy();
  void prepareForRendering();

  [[nodiscard]] DirectionalLightSourceBuffer createDirectionalLightUniforms() const;
  [[nodiscard]] PointLightSourceBuffer createPointLightUniforms() const;
  [[nodiscard]] SpotlightSourceBuffer createSpotlightUniforms() const;

  [[nodiscard]] std::span<const Draw> draw(entt::entity entityCamera, RenderingEngine& renderingEngine);

public:
  entt::registry ecs;
  std::optional<Skybox> skybox = std::nullopt;

private:
  struct MeshDataReference;

  void drawMeshes(std::span<const MeshDataReference> meshes, InstanceBuffer& instanceBuffer);

  void sortMeshes();
  void sortTransparentMeshes(entt::entity entityCamera);
  void sortOutlines();

  void onOutlineComponentAdded(entt::registry& registry, entt::entity entity);
  void onOutlineComponentDestroyed(entt::registry& registry, entt::entity entity);
  void onGraphicsComponentDestroyed(entt::registry& registry, entt::entity entity);

private:
  struct MeshDataReference {
    entt::registry* ecs = nullptr;
    entt::entity entity = entt::null;
    size_t renderDataIndex = SIZE_MAX;
    bool bHasOutline = false;

    [[nodiscard]] RenderData& renderData();
    [[nodiscard]] const RenderData& renderData() const;
  };

  std::vector<MeshDataReference> mCachedSortedMeshes;
  std::vector<MeshDataReference> mCachedSortedTransparentMeshes;
  std::vector<MeshDataReference> mCachedSortedOutlines;
  std::vector<Draw> mCachedDraws;
};

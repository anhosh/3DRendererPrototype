#pragma once

#include <GraphicsOpenGL/Buffers/LightSourceBuffer.hpp>
#include <GraphicsOpenGL/Draw.hpp>
#include <GraphicsOpenGL/Skybox.hpp>
#include <Scene/Scene.hpp>

#include <Util/Math/AABB.hpp>

#include <entt/entity/registry.hpp>

#include <optional>
#include <span>
#include <vector>

namespace GraphicsOpenGL {
  struct InstanceBuffer;
  struct RenderData;
  class RenderingEngine;
}

class Scene {
public:
  Scene();
  ~Scene() { this->destroy(); }

  void destroy();
  void prepareForRendering();

  [[nodiscard]] AABB bounds() const { return mSceneBounds; }

  [[nodiscard]] GraphicsOpenGL::DirectionalLightSourceBuffer createDirectionalLightBufferData() const;
  [[nodiscard]] GraphicsOpenGL::PointLightSourceBuffer createPointLightBufferData() const;
  [[nodiscard]] GraphicsOpenGL::SpotlightSourceBuffer createSpotlightBufferData() const;

  [[nodiscard]] std::span<GraphicsOpenGL::Draw> draw(entt::entity enttCamera, GraphicsOpenGL::RenderingEngine& renderingEngine);

public:
  entt::registry ecs;
  std::optional<GraphicsOpenGL::Skybox> skybox = std::nullopt;

private:
  struct MeshDataReference;

  void drawMeshes(std::span<const MeshDataReference> meshes, GraphicsOpenGL::InstanceBuffer& instanceBuffer);

  void sortMeshes();
  void sortTransparentMeshes(entt::entity enttCamera);
  void sortOutlines();

  void onConstructGraphics(entt::registry&, entt::entity enttOutline);
  void onConstructOutline(entt::registry&, entt::entity enttOutline);
  void onConstructDirectionalLight(entt::registry&, entt::entity enttLight);
  void onConstructPointLight(entt::registry&, entt::entity enttLight);
  void onConstructSpotlight(entt::registry&, entt::entity enttLight);
  void onConstructTransform(entt::registry&, entt::entity enttTransform);

  void onDestroyGraphics(entt::registry&, entt::entity enttGraphics);
  void onDestroyOutline(entt::registry&, entt::entity enttOutline);

  void onUpdateCamera(entt::registry&, entt::entity enttCamera);
  void onUpdateDirectionalLight(entt::registry&, entt::entity enttLight);
  void onUpdatePointLight(entt::registry&, entt::entity enttLight);
  void onUpdateSpotlight(entt::registry&, entt::entity enttLight);
  void onUpdateTransform(entt::registry&, entt::entity enttTransform);

private:
  struct MeshDataReference {
    entt::registry* ecs = nullptr;
    entt::entity entity = entt::null;
    size_t renderDataIndex = SIZE_MAX;
    bool bHasOutline = false;

    [[nodiscard]] GraphicsOpenGL::RenderData& renderData();
    [[nodiscard]] const GraphicsOpenGL::RenderData& renderData() const;
  };

  std::vector<MeshDataReference> mCachedSortedOpaqueMeshes;
  std::vector<MeshDataReference> mCachedSortedTransparentMeshes;
  std::vector<MeshDataReference> mCachedSortedOutlines;
  std::vector<GraphicsOpenGL::Draw> mCachedDraws;

  AABB mSceneBounds;
};

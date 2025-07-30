#pragma once

#include <Graphics/Actor.hpp>
#include <Graphics/Buffers/LightSourceUniforms.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Buffer.hpp>
#include <Graphics/Light.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/Skybox.hpp>
#include <Util/Registry.hpp>

#include <optional>
#include <span>
#include <vector>

class Scene {
public:
  ~Scene() { this->destroy(); }

  void destroy();

  [[nodiscard]] std::span<const Draw> draw(const Camera& camera, Registry<Buffer>& buffers) const;
  [[nodiscard]] DirectionalLightSourceBuffer createDirectionalLightUniforms() const;
  [[nodiscard]] PointLightSourceBuffer createPointLightUniforms() const;
  [[nodiscard]] SpotlightSourceBuffer createSpotlightUniforms() const;

public:
  Registry<Actor> actors;
  Registry<DirectionalLight> directionalLights;
  Registry<PointLight> pointLights;
  Registry<Spotlight> spotlights;

  std::optional<Skybox> skybox = std::nullopt;

private:
  BufferHandle obtainInstanceBuffer(size_t bufferIndex, Registry<Buffer>& buffers) const;

private:
  struct MeshDataReference {
    NotNull<const Actor> actor;
    size_t renderDataIndex = SIZE_MAX;

    [[nodiscard]] const RenderData& renderData() const {
      return actor->renderData[renderDataIndex];
    }
  };

  mutable std::vector<BufferHandle> mCachedInstanceBuffers;
  mutable std::vector<Draw> mCachedDraws;
};

template <typename T> requires
  std::same_as<T, Actor> ||
  std::same_as<T, DirectionalLight> ||
  std::same_as<T, PointLight> ||
  std::same_as<T, Spotlight>
using SceneHandle = typename Registry<T>::Handle;

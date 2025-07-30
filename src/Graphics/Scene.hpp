#pragma once

#include <Graphics/Actor.hpp>
#include <Graphics/Buffer.hpp>
#include <Graphics/Buffers/LightSourceUniforms.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Components/Light.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/Skybox.hpp>
#include <Util/Registry.hpp>

#include <entt/entity/registry.hpp>

#include <optional>
#include <span>
#include <vector>

class Scene {
public:
  ~Scene() { this->destroy(); }

  void destroy();

  [[nodiscard]] std::span<const Draw> draw(const Camera& camera, Registry<Buffer>& buffers);
  [[nodiscard]] DirectionalLightSourceBuffer createDirectionalLightUniforms() const;
  [[nodiscard]] PointLightSourceBuffer createPointLightUniforms() const;
  [[nodiscard]] SpotlightSourceBuffer createSpotlightUniforms() const;

public:
  entt::registry ecs;

  std::optional<Skybox> skybox = std::nullopt;

private:
  BufferHandle obtainInstanceBuffer(size_t bufferIndex, Registry<Buffer>& buffers) const;

private:
  mutable std::vector<BufferHandle> mCachedInstanceBuffers;
  mutable std::vector<Draw> mCachedDraws;
};

template <typename T> requires
  std::same_as<T, Actor> ||
  std::same_as<T, CompDirectionalLight> ||
  std::same_as<T, CompPointLight> ||
  std::same_as<T, CompSpotlight>
using SceneHandle = typename Registry<T>::Handle;

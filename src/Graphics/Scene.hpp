#pragma once

#include <Graphics/Actor.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Light.hpp>
#include <Graphics/Scene.hpp>
#include <Util/Registry.hpp>

#include <vector>

using ActorHandle = Registry<Actor>::Handle;

class Scene {
public:
  ~Scene() { this->destroy(); }

  void destroy();

  ActorHandle addActor(Actor&& actor);

  [[nodiscard]] std::vector<Draw> draw(const Camera& camera) const;

public:
  Registry<Actor> actors;

  DirectionalLight directionalLight;
  PointLight pointLight;
  Spotlight spotlight;
};

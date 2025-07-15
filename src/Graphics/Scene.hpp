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

  [[nodiscard]] std::vector<Draw> draw() const;

  ActorHandle addActor(Actor&& actor);

public:
  Registry<Actor> actors;

  Camera camera;

  DirectionalLight directionalLight;
  PointLight pointLight;
  Spotlight spotlight;
};

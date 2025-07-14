#pragma once

#include <Graphics/Actor.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Light.hpp>
#include <Graphics/Scene.hpp>
#include <Util/Macros/Classes.hpp>

#include <vector>

class Scene {
public:
  DECLARE_ITEM_HANDLE(Scene)

public:
  void destroy();

  [[nodiscard]] std::vector<Draw> draw() const;

  Handle<Actor> addActor(Actor actor);

public:
  std::vector<Actor> actors;

  Camera camera;

  DirectionalLight directionalLight;
  PointLight pointLight;
  Spotlight spotlight;
};

using ActorHandle = Scene::Handle<Actor>;

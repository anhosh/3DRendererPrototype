#pragma once

#include <Graphics/Actor.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Light.hpp>
#include <Graphics/Scene.hpp>
#include <Util/Macros/Classes.hpp>

#include <unordered_map>
#include <vector>

class Scene {
public:
  DECLARE_ITEM_HANDLE(Scene)

public:
  ~Scene() { this->destroy(); }

  void destroy();

  [[nodiscard]] std::vector<Draw> draw() const;

  Handle<Actor> addActor(Actor actor);

public:
  std::unordered_map<size_t, Actor> actors;

  Camera camera;

  DirectionalLight directionalLight;
  PointLight pointLight;
  Spotlight spotlight;

private:
  size_t mNextActorID = 0;
};

using ActorHandle = Scene::Handle<Actor>;

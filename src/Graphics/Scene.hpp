#pragma once

#include <Graphics/Actor.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Light.hpp>
#include <Graphics/Scene.hpp>
#include <Util/PtrAndIndex.hpp>

#include <vector>

class Scene {
public:
  class ActorHandle {
    friend class Scene;

    ActorHandle(size_t index, Scene* scene)
      : index(index)
      , scene(scene)
    {}

  public:
    ActorHandle(const ActorHandle& other) = default;
    ActorHandle& operator=(const ActorHandle& other) = default;

    Actor& get() { return scene->actors[index]; }

    size_t index = SIZE_MAX;

  private:
    NotNull<Scene> scene;
  };

public:
  void destroy();

  [[nodiscard]] std::vector<Draw> draw() const;

  ActorHandle addActor(Actor actor);

public:
  std::vector<Actor> actors;

  Camera camera;

  DirectionalLight directionalLight;
  PointLight pointLight;
  Spotlight spotlight;
};

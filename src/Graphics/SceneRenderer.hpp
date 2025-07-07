#pragma once

#include <unordered_set>

class Scene;

class SceneRenderer {
public:
  void render(const Scene& scene, glm::uvec2 windowSize);

private:
  std::unordered_set<GLuint> mBoundTextureSlots;
};

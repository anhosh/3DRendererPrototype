#pragma once

#include <unordered_set>

class Scene;

class SceneRenderer {
public:
  void render(const Scene& scene, glm::uvec2 windowSize) const;

private:
  mutable std::unordered_set<GLuint> mBoundTextureSlots;
};

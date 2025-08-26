#pragma once

#include <Demos/DemoBase.hpp>

class FlyCamDemoBase : public DemoBase {
public:
  virtual Expected<void> init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) override;
  virtual void processKeyboard(GLFWwindow* window) override;
  virtual void processMouse(glm::vec2 mousePosition) override;
  virtual void update(double dt) override;
  [[nodiscard]] virtual CommandBuffer render() override;

  virtual void onWindowResize(GLFWwindow* window, glm::uvec2 newSize) override;

  virtual void gui(AppState& state) override;

private:
  float mCameraSpeed = 5.0f;
  glm::vec3 mCameraVelocity = glm::vec3(0.0f);
  glm::vec2 mLastMousePosition = glm::vec2(0);

  bool mbFreeCursorPressed = false;
  bool mbFreeCursor = true;
  bool mbFirstMouse = true;

  entt::entity mViewFrustum = entt::null;
  bool mbDrawViewFrustum = false;
  bool mbViewFrustumFollowsMainView = true;
};

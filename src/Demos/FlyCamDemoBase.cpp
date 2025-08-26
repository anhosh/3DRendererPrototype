#include <Demos/FlyCamDemoBase.hpp>

#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Name.hpp>
#include <Scene/Components/Transform.hpp>
#include <Util/Macros/Errors.hpp>

#include <imgui.h>

Expected<void> FlyCamDemoBase::init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) {
  mViewFrustum = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(mViewFrustum, "View frustum");
  mScene.ecs.emplace<CompTransform>(mViewFrustum);

  return DemoBase::init(assets, renderer);
}

void FlyCamDemoBase::processKeyboard(GLFWwindow* window) {
  ZoneScoped;

  if (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) {
    return;
  }

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
    if (!mbFreeCursorPressed) {
      mbFreeCursor = !mbFreeCursor;
      mbFirstMouse = !mbFreeCursor;
      glfwSetInputMode(window, GLFW_CURSOR, mbFreeCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
      mbFreeCursorPressed = true;
    }
  } else {
    mbFreeCursorPressed = false;
  }

  if (!mbFreeCursor) {
    auto [camera, cameraTransform] = mScene.ecs.get<CompCamera, CompTransform>(mMainCamera);
    mCameraVelocity = glm::vec3(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      mCameraVelocity += mCameraSpeed * cameraTransform.forward();
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      mCameraVelocity -= mCameraSpeed * cameraTransform.forward();
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      mCameraVelocity -= mCameraSpeed * cameraTransform.right();
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      mCameraVelocity += mCameraSpeed * cameraTransform.right();
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
      mCameraVelocity += mCameraSpeed * cameraTransform.up();
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
      mCameraVelocity -= mCameraSpeed * cameraTransform.up();
    }
  }
}

void FlyCamDemoBase::processMouse(const glm::vec2 mousePosition) {
  ZoneScoped;

  if (mbFreeCursor) {
    return;
  }

  if (mbFirstMouse) {
    mLastMousePosition = mousePosition;
    mbFirstMouse = false;
  }

  mScene.ecs.patch<CompTransform>(mMainCamera, [&](CompTransform& cameraTransform) {
    constexpr float sensitivity = 0.1f;
    const glm::vec2 offset = {
      (mousePosition.x - mLastMousePosition.x) * sensitivity,
      (mousePosition.y - mLastMousePosition.y) * sensitivity,
    };
    cameraTransform.rotation.yaw += offset.x;
    cameraTransform.rotation.pitch = glm::clamp(cameraTransform.rotation.pitch + offset.y, -89.0f, 89.0f);
  });

  mLastMousePosition = mousePosition;
}

void FlyCamDemoBase::update(const double dt) {
  if (mCameraVelocity != glm::vec3(0.0f)) {
    mScene.ecs.patch<CompTransform>(mMainCamera, [&](CompTransform& cameraTransform) {
      cameraTransform.translation += mCameraVelocity * static_cast<float>(dt);
    });
  }

  if (mbViewFrustumFollowsMainView) {
    mScene.ecs.patch<CompTransform>(mViewFrustum, [&](CompTransform& frustumTransform) {
      const CompTransform& cameraTransform = mScene.ecs.get<CompTransform>(mMainCamera);
      frustumTransform = cameraTransform;
    });
  }

  DemoBase::update(dt);
}

CommandBuffer FlyCamDemoBase::render() {
  CommandBuffer commandBuffer = DemoBase::render();
  if (mbDrawViewFrustum) {
    const CompCamera& camera = mScene.ecs.get<const CompCamera>(mMainCamera);
    const CompTransform& frustumTransform = mScene.ecs.get<const CompTransform>(mViewFrustum);
    const auto command = CmdDrawDebugFrustum(camera.viewFrustum(frustumTransform, mWindowSize));
    commandBuffer.commands.push_back(command);
  }
  return commandBuffer;
}

void FlyCamDemoBase::onWindowResize(GLFWwindow* window, const glm::uvec2 newSize) {
  DemoBase::onWindowResize(window, newSize);

  mLastMousePosition = glm::vec2(newSize) * 0.5f;
  mbFirstMouse = true;
}

void FlyCamDemoBase::gui(AppState& state) {
  ZoneScoped;
  DemoBase::gui(state);

  if (ImGui::CollapsingHeader("Fly cam")) {
    ImGui::Indent();

    ImGui::DragFloat("Movement speed", &mCameraSpeed, 0.001f, 0.0f, 5.0f);
    ImGui::Checkbox("Draw view frustum", &mbDrawViewFrustum);
    ImGui::Checkbox("View frustum follows camera", &mbViewFrustumFollowsMainView);

    ImGui::Unindent();
  }
}

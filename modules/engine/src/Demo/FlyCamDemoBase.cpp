#include <Demo/FlyCamDemoBase.hpp>

#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Transform.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/Math/Vectors.hpp>

#include <imgui.h>

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

  mCameraVelocity = glm::vec3(0.0f);
  if (!mbFreeCursor) {
    auto [camera, cameraTransform] = mScene.ecs.get<CompCamera, CompTransform>(mMainCamera);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      mCameraVelocity += mCameraSpeed * cameraTransform.forward();
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      mCameraVelocity -= mCameraSpeed * cameraTransform.forward();
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      mCameraVelocity += mCameraSpeed * cameraTransform.right();
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      mCameraVelocity -= mCameraSpeed * cameraTransform.right();
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
      mCameraVelocity += mCameraSpeed * DIRECTION_UP;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
      mCameraVelocity -= mCameraSpeed * DIRECTION_UP;
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
      (mLastMousePosition.x - mousePosition.x) * sensitivity,
      (mousePosition.y - mLastMousePosition.y) * sensitivity,
    };
    cameraTransform.rotation.yaw += offset.x;
    cameraTransform.rotation.pitch = glm::clamp(cameraTransform.rotation.pitch + offset.y, -89.0f, 89.0f);
  });

  mLastMousePosition = mousePosition;
}

void FlyCamDemoBase::update(const double dt) {
  ZoneScopedN("FlyCamDemoBase::update");

  if (mCameraVelocity != glm::vec3(0.0f)) {
    mScene.ecs.patch<CompTransform>(mMainCamera, [&](CompTransform& cameraTransform) {
      cameraTransform.translation += mCameraVelocity * static_cast<float>(dt);
    });
  }

  DemoBase::update(dt);
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

    ImGui::DragFloat("Movement speed", &mCameraSpeed, 0.001f, 0.0f, 50.0f);

    ImGui::Unindent();
  }
}

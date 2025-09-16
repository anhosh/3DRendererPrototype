#pragma once

#include <Assets/AssetManager.hpp>
#include <Demo/DemoBase.hpp>
#include <GraphicsOpenGL/RenderingEngine.hpp>

#include <memory>

struct AppState {
  GLFWwindow* window = nullptr;
  glm::uvec2 windowSize = glm::uvec2(0);

  double currentFrameTime = 0.0f;
  double lastFrameTime = 0.0f;
  double lastSceneRenderDuration = 0.0f;
  double lastGuiRenderDuration = 0.0f;

  std::shared_ptr<AssetManager> assetManager;
  std::shared_ptr<GraphicsOpenGL::RenderingEngine> renderingEngine;

  std::unique_ptr<DemoBase> currentDemo;
};

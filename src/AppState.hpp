#pragma once

#include <Assets/AssetManager.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/RenderingEngine.hpp>
#include <Graphics/RenderPass.hpp>
#include <Graphics/Scene.hpp>

#include <memory>
#include <optional>

struct AppState {
  GLFWwindow* window = nullptr;
  glm::uvec2 windowSize = glm::uvec2(0);
  glm::vec2 lastMousePosition = glm::vec2(0);

  double currentFrameTime = 0.0f;
  double lastFrameTime = 0.0f;
  double lastSceneRenderTime = 0.0f;
  double lastGuiRenderTime = 0.0f;

  bool bFreeCursorPressed = false;
  bool bFreeCursor = true;
  bool bFirstMouse = true;
  bool bFlashlightFollowCamera = true;
  bool bDrawBackpackOutline = false;
  bool bDrawLightOutline = false;
  bool bBackMirror = false;

  std::unique_ptr<AssetManager> assetManager;
  std::unique_ptr<RenderingEngine> renderingEngine;
  std::unique_ptr<Scene> scene;
  Camera mainCamera;

  std::optional<ActorHandle> backpackActor = std::nullopt;
  std::optional<ActorHandle> lightActor = std::nullopt;
  std::optional<ActorHandle> grassActor = std::nullopt;

  ShaderProgramType backpackShaderProgramType = ShaderProgramType::LitSurface;
  std::vector<ShaderProgramType> postProcessingShaderProgramTypes;
  std::vector<ShaderProgramInstanceHandle> postProcessingShaderProgramInstances;

  std::optional<FramebufferHandle> mainSceneFramebuffer;
  std::optional<FramebufferHandle> backCameraSceneFramebuffer;
  std::vector<FramebufferHandle> postProcessingFramebuffers;
  std::vector<RenderPass> renderPasses;

  std::optional<ShaderProgramInstanceHandle> litSurfaceShaderProgram = std::nullopt;
  std::optional<ShaderProgramInstanceHandle> lightShaderProgram = std::nullopt;
  std::optional<ShaderProgramInstanceHandle> visualiseDepthShaderProgram = std::nullopt;
  std::optional<ShaderProgramInstanceHandle> visualiseNormalShaderProgram = std::nullopt;

  std::optional<ShaderProgramInstanceHandle> backpackOutlineShaderProgram = std::nullopt;
  std::optional<ShaderProgramInstanceHandle> lightCubeOutlineShaderProgram = std::nullopt;

  std::optional<ShaderProgramInstanceHandle> postProcessingCopyShaderProgramInstance = std::nullopt;
};

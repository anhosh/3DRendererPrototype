#pragma once

#include <Assets/AssetManager.hpp>
#include <Graphics/RenderingEngine.hpp>
#include <Graphics/Scene.hpp>
#include <Util/Expected.hpp>

#include <string_view>

class OutlineShaderProgram;
class VisualiseNormalShaderProgram;
class LitSurfaceShaderProgram;
class LightSourceShaderProgram;
class VisualiseDepthShaderProgram;

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

  std::unique_ptr<AssetManager> assetManager;
  std::unique_ptr<Scene> scene;
  std::unique_ptr<RenderingEngine> renderingEngine;

  std::optional<ActorHandle> backpackActor = std::nullopt;
  std::optional<ActorHandle> lightActor = std::nullopt;
  std::optional<ActorHandle> grassActor = std::nullopt;

  ShaderProgramType backpackShaderProgramType = ShaderProgramType::LitSurface;

  std::optional<RenderingEngine::Handle<ShaderProgramInstance>> litSurfaceShaderProgram = std::nullopt;
  std::optional<RenderingEngine::Handle<ShaderProgramInstance>> lightShaderProgram = std::nullopt;
  std::optional<RenderingEngine::Handle<ShaderProgramInstance>> visualiseDepthShaderProgram = std::nullopt;
  std::optional<RenderingEngine::Handle<ShaderProgramInstance>> visualiseNormalShaderProgram = std::nullopt;

  std::optional<RenderingEngine::Handle<ShaderProgramInstance>> backpackOutlineShaderProgram = std::nullopt;
  std::optional<RenderingEngine::Handle<ShaderProgramInstance>> lightCubeOutlineShaderProgram = std::nullopt;
};

class Engine {
public:
  Engine(const Engine&) = delete;
  Engine(Engine&&) noexcept;

  static Expected<Engine> create(std::string_view title, glm::uvec2 initialWindowSize);

  void run();
  void shutDown();

private:
  Engine();

  [[nodiscard]] Expected<void> createContext(std::string_view title, glm::uvec2 initialWindowSize);
  void initialiseImGui() const;
  [[nodiscard]] Expected<void> createScene();

  void processKeyboard();
  void processMousePosition(glm::vec2 mousePosition);
  void runImGui();
  void gui();
  void updateScene();
  void drawFrame();

private:
  AppState mState;
};

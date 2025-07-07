#pragma once

#include <Graphics/Scene.hpp>
#include <Graphics/SceneRenderer.hpp>

#include <expected>
#include <string_view>

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

  bool freeCursorPressed = false;
  bool freeCursor = true;
  bool firstMouse = true;
  bool flashlightFollowCamera = true;

  Scene scene;
  SceneRenderer sceneRenderer;

  size_t cubeMeshIndex = 0;
  size_t lightModelIndex = 0;
  size_t backpackModelIndex = 0;

  enum class FragmentShader : int32_t {
    LitSurface,
    VisualiseDepth,
    VisualiseNormal,
  } fsType = FragmentShader::LitSurface;

  size_t litSurfaceShaderProgramIndex = 0;
  size_t lightShaderProgramIndex = 0;
  size_t visualiseDepthShaderProgramIndex = 0;
  size_t visualiseNormalShaderProgramIndex = 0;
  LitSurfaceShaderProgram* litSurfaceShaderProgram = nullptr;
  LightSourceShaderProgram* lightShaderProgram = nullptr;
  VisualiseDepthShaderProgram* visualiseDepthShaderProgram = nullptr;
  VisualiseNormalShaderProgram* visualiseNormalShaderProgram = nullptr;
};

class Engine {
public:
  Engine(const Engine&) = delete;
  Engine(Engine&&) noexcept;

  static std::expected<Engine, std::string> create(std::string_view title, glm::uvec2 initialWindowSize);

  void run();
  void shutDown();

private:
  Engine();

  std::expected<void, std::string> createContext(std::string_view title, glm::uvec2 initialWindowSize);
  void initialiseImGui() const;
  std::expected<void, std::string> loadShaders();
  std::expected<void, std::string> loadModels();

  void processKeyboard();
  void processMousePosition(glm::vec2 mousePosition);
  void runImGui();
  void gui();
  void updateScene();
  void drawFrame();

private:
  AppState mState;
};

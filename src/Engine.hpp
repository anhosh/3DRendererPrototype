#pragma once

#include <RenderData.hpp>
#include <SceneRenderer.hpp>

#include <expected>
#include <Light.hpp>
#include <string_view>

class LitSurfaceShaderProgram;
class LightSourceShaderProgram;

struct AppState {
  GLFWwindow* window = nullptr;
  glm::uvec2 windowSize = glm::uvec2(0);
  glm::vec2 lastMousePosition = glm::vec2(0);

  double currentFrameTime = 0.0f;
  double lastFrameTime = 0.0f;

  bool freeCursorPressed = false;
  bool freeCursor = true;
  bool firstMouse = true;

  RenderData renderData;
  SceneRenderer sceneRenderer;

  size_t cubeMaterialIndex = 0;
  size_t lightMaterialIndex = 0;

  LitSurfaceShaderProgram* cubeShaderProgram = nullptr;
  LightSourceShaderProgram* lightShaderProgram = nullptr;
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
  std::expected<void, std::string> createMaterials();
  std::expected<void, std::string> createTextures();
  void createMeshes();

  void processKeyboard();
  void processMousePosition(glm::vec2 mousePosition);
  void runImGui();
  void gui();
  void updateScene();
  void drawFrame();

private:
  AppState mState;
};

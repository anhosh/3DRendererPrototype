#pragma once

#include <AppState.hpp>
#include <Util/Expected.hpp>

#include <string_view>

class Application {
  Application();

public:
  Application(const Application&) = delete;
  Application(Application&&) noexcept;

  static Expected<Application> create(std::string_view title, glm::uvec2 initialWindowSize);

private:
  [[nodiscard]] Expected<void> createContext(std::string_view title, glm::uvec2 initialWindowSize);
#ifdef DEBUG_ENABLED
  static void APIENTRY openGlDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                           GLsizei length, const GLchar* logMessage, const void* userParam);
#endif
  [[nodiscard]] Expected<void> createScene();

public:
  void run();
  void shutDown();

private:
  void processKeyboard();
  void processMousePosition(glm::vec2 mousePosition);
  void updateScene();
  void drawFrame();

private:
  AppState mState;
};

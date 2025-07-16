#pragma once

#include <AppState.hpp>
#include <Util/Expected.hpp>

#include <string_view>

class Application {
public:
  Application(const Application&) = delete;
  Application(Application&&) noexcept;

  static Expected<Application> create(std::string_view title, glm::uvec2 initialWindowSize);

  void run();
  void shutDown();

private:
  Application();

  [[nodiscard]] Expected<void> createContext(std::string_view title, glm::uvec2 initialWindowSize);
  [[nodiscard]] Expected<void> createScene();

  void processKeyboard();
  void processMousePosition(glm::vec2 mousePosition);
  void updateScene();
  void drawFrame();

private:
  AppState mState;
};

#pragma once

#include <AppState.hpp>
#include <Util/Expected.hpp>

#include <string_view>

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
  [[nodiscard]] Expected<void> createScene();

  void processKeyboard();
  void processMousePosition(glm::vec2 mousePosition);
  void updateScene();
  void drawFrame();

private:
  AppState mState;
};

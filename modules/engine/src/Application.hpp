#pragma once

#include <AppState.hpp>
#include <Util/Expected.hpp>

#include <string_view>

class Application {
  Application();

public:
  Application(const Application&) = delete;
  Application(Application&&) noexcept;

  static Expected<Application> create(std::string_view title, glm::uvec2 initialWindowSize, std::unique_ptr<DemoBase> demo);

private:
  [[nodiscard]] Expected<void> createContext(std::string_view title, glm::uvec2 initialWindowSize);

public:
  void run();
  void shutDown();

private:
  void drawFrame();

private:
  AppState mState;
};

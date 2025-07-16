#include <Application.hpp>

#include <Util/Log.hpp>

int32_t main() {
  Expected<Application> engine = Application::create("LearnOpenGL", glm::uvec2(1920, 1080));
  if (engine.has_value()) {
    engine->run();
    engine->shutDown();
    return 0;
  }

  LOG_ERROR("{}", engine.error());
  return 1;
}

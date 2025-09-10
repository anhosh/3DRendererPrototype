#include <Application.hpp>
#include <SpaceDemo.hpp>

#include <Util/Log.hpp>

int32_t main() {
  Expected engine = Application::create("3D Renderer Prototype - Space Demo", glm::uvec2(1920, 1080), std::make_unique<SpaceDemo>());
  if (engine.has_value()) {
    engine->run();
    engine->shutDown();
    return 0;
  }

  LOG_ERROR("{}", engine.error());
  return 1;
}

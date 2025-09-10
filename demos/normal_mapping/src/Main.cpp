#include <NormalMappingDemo.hpp>

#include <Application.hpp>
#include <Util/Log.hpp>

int32_t main() {
  Expected engine = Application::create("3D Renderer Prototype - Normal Mapping Demo", glm::uvec2(1920, 1080), std::make_unique<NormalMappingDemo>());
  if (engine.has_value()) {
    engine->run();
    engine->shutDown();
    return 0;
  }

  LOG_ERROR("{}", engine.error());
  return 1;
}

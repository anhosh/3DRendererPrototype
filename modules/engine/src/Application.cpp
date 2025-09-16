#include <Application.hpp>

#include <Assets/AssetManager.hpp>
#include <Assets/MeshData.hpp>
#include <GUI.hpp>
#include <GraphicsOpenGL/RenderingEngine.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/NotNull.hpp>
#include <Util/Timers/TimedBlock.hpp>

static Application* gApp = nullptr;

Application::Application() {
  assert(gApp == nullptr);
  gApp = this;
}

Application::Application(Application&& other) noexcept {
  gApp = this;

  mState = std::move(other.mState);
}

Expected<Application> Application::create(const std::string_view title, const glm::uvec2 initialWindowSize, std::unique_ptr<DemoBase> demo) {
  ZoneScoped;
  static constexpr std::string_view markerName [[maybe_unused]] = "Application init";
  FrameMarkStart(markerName.data());

  Application app;

  RETURN_ERROR_IF_UNEXPECTED(app.createContext(title, initialWindowSize));
  // initialiseImGui(app.mState.window->);

  app.mState.assetManager = std::make_shared<AssetManager>();
  // app.mState.renderingEngine = std::make_shared<GraphicsOpenGL::RenderingEngine>();
  // RETURN_ERROR_IF_UNEXPECTED(app.mState.renderingEngine->init(*app.mState.assetManager));

  // app.mState.currentDemo = std::move(demo);
  // RETURN_ERROR_IF_UNEXPECTED(app.mState.currentDemo->init(app.mState.assetManager, app.mState.renderingEngine));
  FrameMarkEnd(markerName.data());
  return app;
}

Expected<void> Application::createContext(const std::string_view title, const glm::uvec2 initialWindowSize) {
  ZoneScoped;

  mState.vkfwInstance = vkfw::initUnique();
  mState.windowSize = initialWindowSize;

  const vkfw::WindowHints windowHints = {
    .clientAPI = vkfw::ClientAPI::eNone,
    .resizable = false,
  };
  mState.window = vkfw::createWindowUnique(mState.windowSize.x, mState.windowSize.y, title.data(), windowHints);

  mState.window->callbacks()->on_framebuffer_resize = [](const vkfw::Window& window, const size_t width, const size_t height) {
    const glm::uvec2 newSize = {width, height};
    NotNull(gApp)->mState.windowSize = newSize;
    // NotNull(gApp)->mState.currentDemo->onWindowResize(window, newSize);
  };

  mState.window->callbacks()->on_cursor_move = [](const vkfw::Window&, const double xPos, const double yPos) {
    // NotNull(gApp)->mState.currentDemo->processMouse(glm::vec2(xPos, yPos));
  };

  if (!vkfw::vulkanSupported()) {
    return std::unexpected(std::format("Vulkan is not supported"));
  }

  // vkfw::initVulkanLoader(&vkGetInstanceProcAddr);
  std::span<const char*> extensions = vkfw::getRequiredInstanceExtensions();
  const auto applicationInfo = vk::ApplicationInfo{}
    .setPApplicationName(title.data())
    .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
    .setApiVersion(vk::ApiVersion14);
  const auto instanceInfo = vk::InstanceCreateInfo{}
    .setPEnabledExtensionNames(extensions)
    .setPApplicationInfo(&applicationInfo);
  mState.vkInstance = vk::createInstanceUnique(instanceInfo);
  mState.surface = vkfw::createWindowSurfaceUnique(*mState.vkInstance, *mState.window);

  // TracyGpuContext;

  return {};
}

void Application::run() {
  FrameMark;

  while (!mState.window->shouldClose()) {
    ZoneScopedN("Frame");

    mState.lastFrameTime = mState.currentFrameTime;
    mState.currentFrameTime = vkfw::getTime();

    // mState.currentDemo->processKeyboard(mState.window);
    vkfw::pollEvents();

    // if (glfwGetWindowAttrib(, GLFW_ICONIFIED) != 0) {
    //   ImGui_ImplGlfw_Sleep(10);
    //   continue;
    // }

    // runImGui(mState);
    // mState.currentDemo->update(mState.currentFrameTime - mState.lastFrameTime);
    this->drawFrame();

    // mState.window->swapBuffers();
    // mState.currentDemo->onFrameEnd();

    FrameMark;
    // TracyGpuCollect;
  }
}

void Application::shutDown() {
  // shutdownImGui();

  // mState.renderingEngine->destroy();

  mState.window->destroy();
  mState.window.reset();
  vkfw::terminate();

  assert(gApp != nullptr);
  gApp = nullptr;
}

void Application::drawFrame() {
  ZoneScoped;

  // GraphicsOpenGL::CommandBuffer commandBuffer = mState.currentDemo->render();
  // mState.lastSceneRenderDuration = timedBlock([&, this] {
  //   const GraphicsOpenGL::FramebufferHandle lastFramebuffer = mState.renderingEngine->submitCommands(std::move(commandBuffer));
  //   mState.renderingEngine->present(mState.windowSize, lastFramebuffer);
  // });
  //
  // mState.lastGuiRenderDuration = timedBlock(renderImGui);
}

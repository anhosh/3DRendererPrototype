#include <Application.hpp>

#include <Assets/AssetManager.hpp>
#include <Assets/MeshData.hpp>
#include <GUI.hpp>
#include <GraphicsOpenGL/RenderingEngine.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/NotNull.hpp>
#include <Util/Timers/TimedBlock.hpp>

#include <backends/imgui_impl_glfw.h>

static Application* gApp = nullptr;

Application::Application() {
  assert(gApp == nullptr);
  gApp = this;
}

Application::Application(Application&& other) noexcept {
  gApp = this;

  mState = std::move(other.mState);

  other.mState.window = nullptr;
}

Expected<Application> Application::create(const std::string_view title, const glm::uvec2 initialWindowSize, std::unique_ptr<DemoBase> demo) {
  ZoneScoped;
  static constexpr std::string_view markerName [[maybe_unused]] = "Application init";
  FrameMarkStart(markerName.data());

  Application app;

  RETURN_ERROR_IF_UNEXPECTED(app.createContext(title, initialWindowSize));
  initialiseImGui(app.mState.window);

  app.mState.assetManager = std::make_shared<AssetManager>();
  app.mState.renderingEngine = std::make_shared<GraphicsOpenGL::RenderingEngine>();
  RETURN_ERROR_IF_UNEXPECTED(app.mState.renderingEngine->init(*app.mState.assetManager));

  app.mState.currentDemo = std::move(demo);
  RETURN_ERROR_IF_UNEXPECTED(app.mState.currentDemo->init(app.mState.assetManager, app.mState.renderingEngine));
  app.mState.currentDemo->onWindowResize(app.mState.window, app.mState.windowSize);
  FrameMarkEnd(markerName.data());
  return app;
}

Expected<void> Application::createContext(const std::string_view title, const glm::uvec2 initialWindowSize) {
  ZoneScoped;

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef DEBUG_ENABLED
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

  mState.windowSize = initialWindowSize;
  mState.window = glfwCreateWindow(static_cast<int32_t>(mState.windowSize.x),
                                   static_cast<int32_t>(mState.windowSize.y),
                                   title.data(), nullptr, nullptr);
  if (mState.window == nullptr) {
    glfwTerminate();
    return std::unexpected("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(mState.window);
  // glfwSwapInterval(0);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    return std::unexpected("Failed to initialize GLAD");
  }

#ifdef DEBUG_ENABLED
  if (GLint debugContextFlags; glGetIntegerv(GL_CONTEXT_FLAGS, &debugContextFlags),
      (debugContextFlags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0)
  {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&Application::openGlDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  } else {
    LOG_ERROR("Failed to created debug context");
  }
#endif

  glEnable(GL_MULTISAMPLE);

  glViewport(0, 0, static_cast<int32_t>(mState.windowSize.x), static_cast<int32_t>(mState.windowSize.y));
  glfwSetFramebufferSizeCallback(mState.window, [](GLFWwindow* window, const int32_t width, const int32_t height) {
    const glm::uvec2 newSize = {width, height};
    NotNull(gApp)->mState.windowSize = newSize;
    NotNull(gApp)->mState.currentDemo->onWindowResize(window, newSize);
  });

  glfwSetCursorPosCallback(mState.window, [](GLFWwindow*, const double xpos, const double ypos) {
    NotNull(gApp)->mState.currentDemo->processMouse(glm::vec2(xpos, ypos));
  });

  TracyGpuContext;

  return {};
}

#ifdef DEBUG_ENABLED
void APIENTRY Application::openGlDebugCallback(const GLenum source, const GLenum type, const GLuint id, const GLenum severity,
                                               const GLsizei length [[maybe_unused]], const GLchar* logMessage,
                                               const void* userParam [[maybe_unused]])
{
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
    return;
  }

  std::string message = "---------------\n";
  message += std::format("Debug message ({}): {}\n", id, logMessage);

  switch (source) {
    case GL_DEBUG_SOURCE_API:             message += "Source: API\n";             break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   message += "Source: Window System\n";   break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: message += "Source: Shader Compiler\n"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     message += "Source: Third Party\n";     break;
    case GL_DEBUG_SOURCE_APPLICATION:     message += "Source: Application\n";     break;
    case GL_DEBUG_SOURCE_OTHER:           message += "Source: Other\n";           break;
    default:                              UNREACHABLE();
  }

  switch (type) {
    case GL_DEBUG_TYPE_ERROR:               message += "Type: Error\n";                break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: message += "Type: Deprecated Behaviour\n"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  message += "Type: Undefined Behaviour\n";  break;
    case GL_DEBUG_TYPE_PORTABILITY:         message += "Type: Portability\n";          break;
    case GL_DEBUG_TYPE_PERFORMANCE:         message += "Type: Performance\n";          break;
    case GL_DEBUG_TYPE_MARKER:              message += "Type: Marker\n";               break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          message += "Type: Push Group\n";           break;
    case GL_DEBUG_TYPE_POP_GROUP:           message += "Type: Pop Group\n";            break;
    case GL_DEBUG_TYPE_OTHER:               message += "Type: Other\n";                break;
    default:                                UNREACHABLE();
  }

  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:         message += "Severity: high\n";         break;
    case GL_DEBUG_SEVERITY_MEDIUM:       message += "Severity: medium\n";       break;
    case GL_DEBUG_SEVERITY_LOW:          message += "Severity: low\n";          break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: message += "Severity: notification\n"; break;
    default:                             UNREACHABLE();
  }

  if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR) {
    LOG_ERROR("{}", message);
  } else {
    LOG_INFO("{}", message);
  }
}
#endif

void Application::run() {
  FrameMark;

  while (!glfwWindowShouldClose(mState.window)) {
    ZoneScopedN("Frame");

    mState.lastFrameTime = mState.currentFrameTime;
    mState.currentFrameTime = glfwGetTime();

    mState.currentDemo->processKeyboard(mState.window);
    glfwPollEvents();

    if (glfwGetWindowAttrib(mState.window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    runImGui(mState);
    mState.currentDemo->update(mState.currentFrameTime - mState.lastFrameTime);
    this->drawFrame();

    glfwSwapBuffers(mState.window);
    mState.currentDemo->onFrameEnd();

    FrameMark;
    TracyGpuCollect;
  }
}

void Application::shutDown() {
  shutdownImGui();

  mState.renderingEngine->destroy();

  glfwDestroyWindow(mState.window);
  mState.window = nullptr;
  glfwTerminate();

  assert(gApp != nullptr);
  gApp = nullptr;
}

void Application::drawFrame() {
  ZoneScoped;

  GraphicsOpenGL::CommandBuffer commandBuffer = mState.currentDemo->render();
  mState.lastSceneRenderDuration = timedBlock([&, this] {
    const GraphicsOpenGL::FramebufferHandle lastFramebuffer = mState.renderingEngine->submitCommands(std::move(commandBuffer));
    mState.renderingEngine->present(mState.windowSize, lastFramebuffer);
  });

  mState.lastGuiRenderDuration = timedBlock(renderImGui);
}

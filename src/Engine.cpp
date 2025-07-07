#include <Engine.hpp>

#include <Graphics/Bitmap.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Meshes.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/ShaderPrograms/LightSourceShaderProgram.hpp>
#include <Graphics/ShaderPrograms/LitSurfaceShaderProgram.hpp>
#include <Graphics/ShaderPrograms/ShaderProgram.hpp>
#include <Util/Macros.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include <Util/Timers/TimedBlock.hpp>

static Engine* gApp = nullptr;

Engine::Engine(Engine&& other) noexcept {
  gApp = this;

  mState = std::move(other.mState);

  other.mState.window = nullptr;
}

std::expected<Engine, std::string> Engine::create(std::string_view title, glm::uvec2 initialWindowSize) {
  Engine app;
  RETURN_ERROR_IF_UNEXPECTED(app.createContext(title, initialWindowSize));
  app.initialiseImGui();
  RETURN_ERROR_IF_UNEXPECTED(app.loadShaders());
  RETURN_ERROR_IF_UNEXPECTED(app.loadModels());
  return app;
}

void Engine::run() {
  while (!glfwWindowShouldClose(mState.window)) {
    mState.lastFrameTime = mState.currentFrameTime;
    mState.currentFrameTime = glfwGetTime();

    this->processKeyboard();
    glfwPollEvents();

    if (glfwGetWindowAttrib(mState.window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    this->runImGui();
    this->updateScene();
    this->drawFrame();

    glfwSwapBuffers(mState.window);
  }
}

void Engine::shutDown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  mState.scene.destroy();

  glfwDestroyWindow(mState.window);
  mState.window = nullptr;
  glfwTerminate();

  assert(gApp != nullptr);
  gApp = nullptr;
}

Engine::Engine() {
  assert(gApp == nullptr);
  gApp = this;
}

std::expected<void, std::string> Engine::createContext(std::string_view title, glm::uvec2 initialWindowSize) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

  glEnable(GL_DEPTH_TEST);

  glViewport(0, 0, static_cast<int32_t>(mState.windowSize.x), static_cast<int32_t>(mState.windowSize.y));
  glfwSetFramebufferSizeCallback(mState.window, [](GLFWwindow*, int32_t width, int32_t height) {
    glViewport(0, 0, width, height);
    assert(gApp != nullptr);
    gApp->mState.windowSize = glm::uvec2(width, height);
    gApp->mState.lastMousePosition = glm::vec2(gApp->mState.windowSize) * 0.5f;
    gApp->mState.firstMouse = true;
  });

  glfwSetCursorPosCallback(mState.window, [](GLFWwindow*, double xpos, double ypos) {
    assert(gApp != nullptr);
    gApp->processMousePosition(glm::vec2(xpos, ypos));
  });

  return {};
}

void Engine::initialiseImGui() const {
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(mState.window, true);
  ImGui_ImplOpenGL3_Init("#version 460");
}

std::expected<void, std::string> Engine::loadShaders() {
  if (!locateShaders()) {
    return std::unexpected("Could not locate shader directory");
  }

  auto cubeShaderProgram = ShaderPrograms::fromShaders<LitSurfaceShaderProgram>({ .vertex = "simple.vert", .fragment = "litSurface.frag" });
  auto lightShaderProgram = ShaderPrograms::fromShaders<LightSourceShaderProgram>({ .vertex = "simple.vert", .fragment = "light.frag" });

  RETURN_ERROR_IF_UNEXPECTED(cubeShaderProgram);
  RETURN_ERROR_IF_UNEXPECTED(lightShaderProgram);

  mState.cubeShaderProgram = dynamic_cast<LitSurfaceShaderProgram*>(cubeShaderProgram.value().get());
  mState.lightShaderProgram = dynamic_cast<LightSourceShaderProgram*>(lightShaderProgram.value().get());

  mState.cubeShaderProgramIndex = mState.scene.addShaderProgram(std::move(cubeShaderProgram.value()));
  mState.lightShaderProgramIndex = mState.scene.addShaderProgram(std::move(lightShaderProgram.value()));

  return {};
}

std::expected<void, std::string> Engine::loadModels() {
  if (!locateModels()) {
    return std::unexpected("Could not locate model directory");
  }
  if (!locateTextures()) {
    return std::unexpected("Could not locate texture directory");
  }
  RETURN_ERROR_IF_UNEXPECTED(mState.scene.loadModel("backpack/backpack.obj"));
  mState.scene.models.back().shaderProgramIndex = mState.cubeShaderProgramIndex;

  mState.cubeMeshIndex = mState.scene.addMesh(createCubeMesh());
  mState.lightModelIndex = mState.scene.addModel(Model {
    .meshIndices = { mState.cubeMeshIndex },
    .shaderProgramIndex = mState.lightShaderProgramIndex,
    .transform = ModelTransform {
      .scale = glm::vec3(0.1f),
    },
  });

  return {};
}

void Engine::processKeyboard() {
  if (glfwGetKey(mState.window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) {
    return;
  }

  if (glfwGetKey(mState.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(mState.window, true);
  }

  if (glfwGetKey(mState.window, GLFW_KEY_G) == GLFW_PRESS) {
    if (!mState.freeCursorPressed) {
      mState.freeCursor = !mState.freeCursor;
      mState.firstMouse = !mState.freeCursor;
      glfwSetInputMode(mState.window, GLFW_CURSOR, mState.freeCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
      mState.freeCursorPressed = true;
    }
  } else {
    mState.freeCursorPressed = false;
  }

  const auto deltaTime = static_cast<float>(mState.currentFrameTime - mState.lastFrameTime);
  Camera& camera = mState.scene.camera;
  if (glfwGetKey(mState.window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.position += deltaTime * camera.speed * camera.forward();
  }
  if (glfwGetKey(mState.window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.position -= deltaTime * camera.speed * camera.forward();
  }
  if (glfwGetKey(mState.window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.position -= deltaTime * camera.speed * glm::normalize(glm::cross(camera.forward(), camera.up()));
  }
  if (glfwGetKey(mState.window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.position += deltaTime * camera.speed * glm::normalize(glm::cross(camera.forward(), camera.up()));
  }
  if (glfwGetKey(mState.window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    camera.position += deltaTime * camera.speed * camera.up();
  }
  if (glfwGetKey(mState.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    camera.position -= deltaTime * camera.speed * camera.up();
  }
}

void Engine::processMousePosition(glm::vec2 mousePosition) {
  if (mState.freeCursor) {
    return;
  }

  if (mState.firstMouse) {
    mState.lastMousePosition = mousePosition;
    mState.firstMouse = false;
  }

  constexpr float sensitivity = 0.1f;
  const glm::vec2 offset = {
    (mousePosition.x - mState.lastMousePosition.x) * sensitivity,
    (mState.lastMousePosition.y - mousePosition.y) * sensitivity,
  };
  Camera& camera = mState.scene.camera;
  camera.rotation.x += offset.x;
  camera.rotation.y = glm::clamp(camera.rotation.y + offset.y, -89.0f, 89.0f);

  mState.lastMousePosition = mousePosition;
}

void Engine::runImGui() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  this->gui();
  ImGui::Render();
}

void Engine::gui() {
  constexpr ImGuiColorEditFlags colorEditFlags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR;
  if (ImGui::Begin("Test")) {
    ImGui::Text("FPS: %.03f", 1.0 / (mState.currentFrameTime - mState.lastFrameTime));
    ImGui::Text("Frame duration: %.03f ms", (mState.currentFrameTime - mState.lastFrameTime) * 1000.0);
    ImGui::Text("Scene draw: %.03f ms", mState.lastSceneRenderTime * 1000.0);
    ImGui::Text("GUI draw: %.03f ms", mState.lastGuiRenderTime * 1000.0);
    ImGui::Text("Window size: %ux%u", mState.windowSize.x, mState.windowSize.y);

    if (ImGui::CollapsingHeader("Camera")) {
      ImGui::Indent();
      Camera& camera = mState.scene.camera;

      ImGui::DragFloat("Movement speed", &camera.speed, 0.001f, 0.0f, 5.0f);
      ImGui::DragFloat("FOV", &camera.fov, 0.1f, 10.0f, 120.0f);
      ImGui::DragFloat("Near", &camera.near, 0.01f, 0.01f, 10.0f);
      ImGui::DragFloat("Far", &camera.far, 0.01f, 10.0f, 1000.0f);

      ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Cube")) {
      ImGui::Indent();
      assert(mState.cubeShaderProgram != nullptr);
      auto& cubeShaderProgram = *mState.cubeShaderProgram;

      ImGui::Text("Lit surface material");
      ImGui::DragFloat("Shininess", &cubeShaderProgram.material.shininess, 1.0f, 1.0f, 256.0f);

      ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Light")) {
      ImGui::Indent();

      if (ImGui::CollapsingHeader("Directional light")) {
        ImGui::Indent();
        DirectionalLight& directionalLight = mState.scene.directionalLight;

        ImGui::ColorPicker3("Ambient##dl", glm::value_ptr(directionalLight.colors.ambient), colorEditFlags);
        ImGui::ColorPicker3("Diffuse##dl", glm::value_ptr(directionalLight.colors.diffuse), colorEditFlags);
        ImGui::ColorPicker3("Specular##dl", glm::value_ptr(directionalLight.colors.specular), colorEditFlags);

        ImGui::Unindent();
      }

      if (ImGui::CollapsingHeader("Point light")) {
        ImGui::Indent();
        PointLight& pointLight = mState.scene.pointLight;

        ImGui::DragFloat("Constant", &pointLight.constant, 0.1f, 1.0f, 100.0f);
        ImGui::DragFloat("Linear", &pointLight.linear, 0.01f, 0.01f, 10.0f);
        ImGui::DragFloat("Quadratic", &pointLight.quadratic, 0.001f, 0.01f, 1.0f);
        ImGui::Spacing();

        ImGui::ColorPicker3("Ambient##pl", glm::value_ptr(pointLight.colors.ambient), colorEditFlags);
        ImGui::ColorPicker3("Diffuse##pl", glm::value_ptr(pointLight.colors.diffuse), colorEditFlags);
        ImGui::ColorPicker3("Specular##pl", glm::value_ptr(pointLight.colors.specular), colorEditFlags);

        ImGui::Unindent();
      }

      if (ImGui::CollapsingHeader("Spotlight")) {
        ImGui::Indent();
        Spotlight& spotlight = mState.scene.spotlight;

        ImGui::DragFloat("Cut off", &spotlight.cutOff, 0.01f, 1.0f, spotlight.outerCutOff);
        ImGui::DragFloat("Outer cut off", &spotlight.outerCutOff, 0.01f, spotlight.cutOff, 120.0f);
        ImGui::Spacing();

        ImGui::ColorPicker3("Ambient##sl", glm::value_ptr(spotlight.colors.ambient), colorEditFlags);
        ImGui::ColorPicker3("Diffuse##sl", glm::value_ptr(spotlight.colors.diffuse), colorEditFlags);
        ImGui::ColorPicker3("Specular##sl", glm::value_ptr(spotlight.colors.specular), colorEditFlags);

        ImGui::Unindent();
      }

      ImGui::Unindent();
    }

    ImGui::End();
  }
}

void Engine::updateScene() {
  // Flying light cube
  mState.scene.pointLight.position = {
    glm::cos(mState.currentFrameTime * 0.05f),
    glm::cos(mState.currentFrameTime * 0.075f),
    glm::sin(mState.currentFrameTime * 0.05f),
  };
  mState.scene.models[mState.lightModelIndex].transform.translation = mState.scene.pointLight.position;

  // Flashlight
  mState.scene.spotlight.position = mState.scene.camera.position;
  mState.scene.spotlight.direction = mState.scene.camera.forward();

  // Shaders
  assert(mState.cubeShaderProgram != nullptr);
  assert(mState.lightShaderProgram != nullptr);
  mState.cubeShaderProgram->directionalLight = mState.scene.directionalLight;
  mState.cubeShaderProgram->pointLight = mState.scene.pointLight;
  mState.cubeShaderProgram->spotlight = mState.scene.spotlight;

  mState.lightShaderProgram->emittedColor = mState.scene.pointLight.colors.specular;

  const glm::mat4 view = mState.scene.camera.view();
  const glm::vec3 pointLightDirectionView = glm::mat3(glm::transpose(glm::inverse(view))) *
                                            mState.scene.directionalLight.direction;
  const glm::vec4 pointLightPosView = view * glm::vec4(mState.scene.pointLight.position, 1.0f);

  mState.cubeShaderProgram->directionalLight.direction = pointLightDirectionView;
  mState.cubeShaderProgram->pointLight.position = pointLightPosView;
  mState.cubeShaderProgram->spotlight.position = view * glm::vec4(mState.scene.spotlight.position, 1.0f);
  mState.cubeShaderProgram->spotlight.direction = glm::mat3(glm::transpose(glm::inverse(view))) *
                                                  mState.scene.spotlight.direction;
  mState.cubeShaderProgram->spotlight.cutOff = glm::cos(glm::radians(mState.scene.spotlight.cutOff));
  mState.cubeShaderProgram->spotlight.outerCutOff = glm::cos(glm::radians(mState.scene.spotlight.outerCutOff));
}

void Engine::drawFrame() {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mState.lastSceneRenderTime = timedBlock([this] {
    mState.sceneRenderer.render(mState.scene, mState.windowSize);
  });
  mState.lastGuiRenderTime = timedBlock([] {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  });
}

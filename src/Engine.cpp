#include <Engine.hpp>

#include <Graphics/Bitmap.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Meshes.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/ShaderPrograms/LightSourceShaderProgram.hpp>
#include <Graphics/ShaderPrograms/LitSurfaceShaderProgram.hpp>
#include <Graphics/ShaderPrograms/OutlineShaderProgram.hpp>
#include <Graphics/ShaderPrograms/VisualiseDepthShaderProgram.hpp>
#include <Graphics/ShaderPrograms/VisualiseNormalShaderProgram.hpp>
#include <Util/Macros.hpp>
#include <Util/NotNull.hpp>
#include <Util/Timers/TimedBlock.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

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
  RETURN_ERROR_IF_UNEXPECTED(app.createScene());
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

  glEnable(GL_STENCIL_TEST);

  glViewport(0, 0, static_cast<int32_t>(mState.windowSize.x), static_cast<int32_t>(mState.windowSize.y));
  glfwSetFramebufferSizeCallback(mState.window, [](GLFWwindow*, int32_t width, int32_t height) {
    glViewport(0, 0, width, height);
    NotNull(gApp)->mState.windowSize = glm::uvec2(width, height);
    NotNull(gApp)->mState.lastMousePosition = glm::vec2(NotNull(gApp)->mState.windowSize) * 0.5f;
    NotNull(gApp)->mState.bFirstMouse = true;
  });

  glfwSetCursorPosCallback(mState.window, [](GLFWwindow*, double xpos, double ypos) {
    NotNull(gApp)->processMousePosition(glm::vec2(xpos, ypos));
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

  const auto addShaderProgram = [this]<typename TShaderProgram>(const ShaderStages& stages,
                                                                TShaderProgram*& outShader,
                                                                size_t& outShaderProgramIndex) -> std::expected<void, std::string>
  {
    auto program = ShaderPrograms::fromShaders<TShaderProgram>(stages);
    RETURN_ERROR_IF_UNEXPECTED(program);
    outShader = NotNull(dynamic_cast<TShaderProgram*>(program.value().get()));
    outShaderProgramIndex = mState.scene.addShaderProgram(std::move(program.value()));
    return {};
  };

  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "simple.vert", .fragment = "litSurface.frag"},
                                              mState.litSurfaceShaderProgram,
                                              mState.litSurfaceShaderProgramIndex));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "simple.vert", .fragment = "light.frag"},
                                              mState.lightShaderProgram,
                                              mState.lightShaderProgramIndex));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "simple.vert", .fragment = "visualiseDepth.frag"},
                                              mState.visualiseDepthShaderProgram,
                                              mState.visualiseDepthShaderProgramIndex));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "simple.vert", .fragment = "visualiseNormal.frag"},
                                              mState.visualiseNormalShaderProgram,
                                              mState.visualiseNormalShaderProgramIndex));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "simple.vert", .fragment = "outline.frag"},
                                              mState.outlineShaderProgram,
                                              mState.outlineShaderProgramIndex));

  return {};
}

std::expected<void, std::string> Engine::createScene() {
  RETURN_ERROR_IF_UNEXPECTED(locateModels());
  RETURN_ERROR_IF_UNEXPECTED(locateTextures());

  RETURN_ERROR_IF_UNEXPECTED(mState.scene.loadModel("backpack/backpack.obj"));
  mState.scene.models.back().shaderProgramIndex = mState.litSurfaceShaderProgramIndex;

  mState.cubeMeshIndex = mState.scene.addMesh(createCubeMesh());
  mState.lightModelIndex = mState.scene.addModel(Model {
    .meshIndices = { mState.cubeMeshIndex },
    .transform = ModelTransform {
      .scale = glm::vec3(0.1f),
    },
    .shaderProgramIndex = mState.lightShaderProgramIndex,
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
    if (!mState.bFreeCursorPressed) {
      mState.bFreeCursor = !mState.bFreeCursor;
      mState.bFirstMouse = !mState.bFreeCursor;
      glfwSetInputMode(mState.window, GLFW_CURSOR, mState.bFreeCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
      mState.bFreeCursorPressed = true;
    }
  } else {
    mState.bFreeCursorPressed = false;
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
  if (mState.bFreeCursor) {
    return;
  }

  if (mState.bFirstMouse) {
    mState.lastMousePosition = mousePosition;
    mState.bFirstMouse = false;
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
  constexpr ImGuiColorEditFlags lightColorEditFlags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR;
  if (ImGui::Begin("Test")) {
    Model& backpackModel = mState.scene.models[mState.backpackModelIndex];
    Model& lightModel = mState.scene.models[mState.lightModelIndex];

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

    if (ImGui::CollapsingHeader("Backpack")) {
      ImGui::Indent();

      static constexpr const char* fsTypeNames[] = {
        "Lit surface",
        "Visualise depth",
        "Visualise normal",
      };
      if (ImGui::Combo("Fragment shader", reinterpret_cast<int32_t*>(&mState.fsType), fsTypeNames, std::size(fsTypeNames))) {
        switch (mState.fsType) {
          case AppState::FragmentShader::LitSurface:
            backpackModel.shaderProgramIndex = mState.litSurfaceShaderProgramIndex;
            break;
          case AppState::FragmentShader::VisualiseDepth:
            backpackModel.shaderProgramIndex = mState.visualiseDepthShaderProgramIndex;
            break;
          case AppState::FragmentShader::VisualiseNormal:
            backpackModel.shaderProgramIndex = mState.visualiseNormalShaderProgramIndex;
            break;
        }
      }

      ImGui::DragFloat("Shininess", &NotNull(mState.litSurfaceShaderProgram)->material.shininess, 1.0f, 1.0f, 256.0f);

      ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Light")) {
      ImGui::Indent();

      if (ImGui::CollapsingHeader("Directional light")) {
        ImGui::Indent();
        DirectionalLight& directionalLight = mState.scene.directionalLight;

        ImGui::ColorPicker3("Ambient##dl", glm::value_ptr(directionalLight.colors.ambient), lightColorEditFlags);
        ImGui::ColorPicker3("Diffuse##dl", glm::value_ptr(directionalLight.colors.diffuse), lightColorEditFlags);
        ImGui::ColorPicker3("Specular##dl", glm::value_ptr(directionalLight.colors.specular), lightColorEditFlags);

        ImGui::Unindent();
      }

      if (ImGui::CollapsingHeader("Point light")) {
        ImGui::Indent();
        PointLight& pointLight = mState.scene.pointLight;

        ImGui::DragFloat("Constant", &pointLight.constant, 0.1f, 1.0f, 100.0f);
        ImGui::DragFloat("Linear", &pointLight.linear, 0.01f, 0.01f, 10.0f);
        ImGui::DragFloat("Quadratic", &pointLight.quadratic, 0.001f, 0.01f, 1.0f);
        ImGui::Spacing();

        ImGui::ColorPicker3("Ambient##pl", glm::value_ptr(pointLight.colors.ambient), lightColorEditFlags);
        ImGui::ColorPicker3("Diffuse##pl", glm::value_ptr(pointLight.colors.diffuse), lightColorEditFlags);
        ImGui::ColorPicker3("Specular##pl", glm::value_ptr(pointLight.colors.specular), lightColorEditFlags);

        ImGui::Unindent();
      }

      if (ImGui::CollapsingHeader("Spotlight")) {
        ImGui::Indent();
        Spotlight& spotlight = mState.scene.spotlight;

        ImGui::Checkbox("Follow camera", &mState.bFlashlightFollowCamera);
        ImGui::DragFloat("Cut off", &spotlight.cutOff, 0.01f, 1.0f, spotlight.outerCutOff);
        ImGui::DragFloat("Outer cut off", &spotlight.outerCutOff, 0.01f, spotlight.cutOff, 120.0f);
        ImGui::Spacing();

        ImGui::ColorPicker3("Ambient##sl", glm::value_ptr(spotlight.colors.ambient), lightColorEditFlags);
        ImGui::ColorPicker3("Diffuse##sl", glm::value_ptr(spotlight.colors.diffuse), lightColorEditFlags);
        ImGui::ColorPicker3("Specular##sl", glm::value_ptr(spotlight.colors.specular), lightColorEditFlags);

        ImGui::Unindent();
      }

      ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Object outlines")) {
      ImGui::Indent();

      ImGui::ColorPicker3("Outline color", glm::value_ptr(NotNull(mState.outlineShaderProgram)->outlineColor), ImGuiColorEditFlags_Float);

      if (ImGui::Checkbox("Backpack outline", &mState.bDrawBackpackOutline)) {
        if (mState.bDrawBackpackOutline) {
          backpackModel.outlineShaderProgramIndex = mState.outlineShaderProgramIndex;
        } else {
          backpackModel.outlineShaderProgramIndex = std::nullopt;
        }
      }

      if (ImGui::Checkbox("Light outline", &mState.bDrawLightOutline)) {
        if (mState.bDrawLightOutline) {
          lightModel.outlineShaderProgramIndex = mState.outlineShaderProgramIndex;
        } else {
          lightModel.outlineShaderProgramIndex = std::nullopt;
        }
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
  if (mState.bFlashlightFollowCamera) {
    mState.scene.spotlight.position = mState.scene.camera.position;
    mState.scene.spotlight.direction = mState.scene.camera.forward();
  }

  // Shaders
  NotNull(mState.lightShaderProgram)->emittedColor = mState.scene.pointLight.colors.specular;

  NotNull(mState.visualiseDepthShaderProgram)->frustumNear = mState.scene.camera.near;
  NotNull(mState.visualiseDepthShaderProgram)->frustumFar = mState.scene.camera.far;

  NotNull(mState.litSurfaceShaderProgram)->viewPos = mState.scene.camera.position;
  NotNull(mState.litSurfaceShaderProgram)->directionalLight = mState.scene.directionalLight;
  NotNull(mState.litSurfaceShaderProgram)->pointLight = mState.scene.pointLight;
  NotNull(mState.litSurfaceShaderProgram)->spotlight = mState.scene.spotlight;
  NotNull(mState.litSurfaceShaderProgram)->spotlight.cutOff = glm::cos(glm::radians(mState.scene.spotlight.cutOff));
  NotNull(mState.litSurfaceShaderProgram)->spotlight.outerCutOff = glm::cos(glm::radians(mState.scene.spotlight.outerCutOff));
}

void Engine::drawFrame() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glStencilMask(0xff);
  glStencilFunc(GL_ALWAYS, 1, 0xff);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  mState.lastSceneRenderTime = timedBlock([this] {
    mState.sceneRenderer.render(mState.scene, mState.windowSize);
  });
  mState.lastGuiRenderTime = timedBlock([] {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  });
}

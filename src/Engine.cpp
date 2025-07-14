#include <Engine.hpp>

#include <Assets/Bitmap.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Meshes.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/NotNull.hpp>
#include <Util/Timers/TimedBlock.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

static Engine* gApp = nullptr;

Engine::Engine(Engine&& other) noexcept {
  gApp = this;

  mState = std::move(other.mState);

  other.mState.window = nullptr;
}

Expected<Engine> Engine::create(std::string_view title, glm::uvec2 initialWindowSize) {
  Engine app;
  RETURN_ERROR_IF_UNEXPECTED(app.createContext(title, initialWindowSize));
  app.mState.assetManager = std::make_unique<AssetManager>();
  app.mState.scene = std::make_unique<Scene>();
  app.mState.renderingEngine = std::make_unique<RenderingEngine>();
  app.initialiseImGui();
  RETURN_ERROR_IF_UNEXPECTED(app.mState.renderingEngine->init());
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

  mState.scene->destroy();

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

Expected<void> Engine::createContext(std::string_view title, glm::uvec2 initialWindowSize) {
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
  ImGui_ImplOpenGL3_Init("#version 460 core");
}

Expected<void> Engine::createScene() {
  // Load assets
  const AssetHandle<Mesh> quadMesh = mState.assetManager->addMesh(createQuadMesh());
  const AssetHandle<Mesh> cubeMesh = mState.assetManager->addMesh(createCubeMesh());

  const Expected<AssetHandle<Model>> backpackModel = mState.assetManager->loadModel("backpack/backpack.obj");
  const Expected<AssetHandle<Bitmap>> grassBitmap = mState.assetManager->loadBitmap("grass/diffuse.png");
  const Expected<AssetHandle<Bitmap>> windowBitmap = mState.assetManager->loadBitmap("window/diffuse.png");

  RETURN_ERROR_IF_UNEXPECTED(backpackModel);
  RETURN_ERROR_IF_UNEXPECTED(grassBitmap);
  RETURN_ERROR_IF_UNEXPECTED(windowBitmap);

  // Get shader instances
  mState.litSurfaceShaderProgram = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::LitSurface);
  mState.lightShaderProgram = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::Light);
  mState.visualiseDepthShaderProgram = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::VisualiseDepth);
  mState.visualiseNormalShaderProgram = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::VisualiseNormal);
  mState.backpackOutlineShaderProgram = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::Outline);
  mState.lightCubeOutlineShaderProgram = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::Outline);

  // Upload assets to GPU
  const RenderingEngine::Handle<VertexArray> quadVA = mState.renderingEngine->addMesh(quadMesh);
  const RenderingEngine::Handle<VertexArray> cubeVA = mState.renderingEngine->addMesh(cubeMesh);

  constexpr SamplerOptions samplerOptionsClampToEdge = { .wrapS = GL_CLAMP_TO_EDGE, .wrapT = GL_CLAMP_TO_EDGE };
  const RenderingEngine::Handle<Texture> grassTexture = mState.renderingEngine->addTexture(grassBitmap.value(), samplerOptionsClampToEdge);
  const RenderingEngine::Handle<Texture> windowTexture = mState.renderingEngine->addTexture(windowBitmap.value(), samplerOptionsClampToEdge);

  constexpr RenderingEngine::RenderOptions transparentQuadOptions = { .bBackfaceCulling = false, .bTransparent = true };
  std::vector<RenderingEngine::RenderData> backpackResources = mState.renderingEngine->addModel(backpackModel.value(), mState.litSurfaceShaderProgram.value());
  const std::vector lightResources = {
    RenderingEngine::RenderData {
      .vertexArray = cubeVA,
      .shaderProgramInstance = mState.lightShaderProgram.value(),
    }
  };
  const std::vector grassResources = {
    RenderingEngine::RenderData {
      .vertexArray = quadVA,
      .shaderProgramInstance = mState.litSurfaceShaderProgram.value(),
      .diffuseMap = grassTexture,
      .renderOptions = transparentQuadOptions,
    }
  };
  const std::vector windowResources = {
    RenderingEngine::RenderData {
      .vertexArray = quadVA,
      .shaderProgramInstance = mState.litSurfaceShaderProgram.value(),
      .diffuseMap = windowTexture,
      .renderOptions = transparentQuadOptions,
    }
  };

  // Create actors
  mState.backpackActor = mState.scene->addActor({
    .name = "Backpack",
    .transform = Transform {
      .translation = glm::vec3(0.0f),
    },
    .renderData = std::move(backpackResources),
  });

  mState.lightActor = mState.scene->addActor({
    .name = "Light cube",
    .transform = Transform {
      .scale = glm::vec3(0.1f),
    },
    .renderData = lightResources,
  });

  mState.grassActor = mState.scene->addActor({
    .name = "Grass",
    .transform = Transform {
      .translation = glm::vec3(0.0f, 0.0f, 1.0f),
    },
    .renderData = grassResources,
  });

  Actor windowActor = { .renderData = windowResources, };
  windowActor.name = "Window 0";
  windowActor.transform.translation = glm::vec3(-0.25f, 0.0f, 1.5f);
  mState.scene->addActor(windowActor);
  windowActor.name = "Window 1";
  windowActor.transform.translation = glm::vec3(0.25f, 0.0f, 1.75f);
  mState.scene->addActor(windowActor);
  windowActor.name = "Window 2";
  windowActor.transform.translation = glm::vec3(0.0f, 0.0f, 2.0f);
  mState.scene->addActor(std::move(windowActor));

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
  Camera& camera = mState.scene->camera;
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
  Camera& camera = mState.scene->camera;
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
    ImGui::Text("FPS: %.03f", 1.0 / (mState.currentFrameTime - mState.lastFrameTime));
    ImGui::Text("Frame duration: %.03f ms", (mState.currentFrameTime - mState.lastFrameTime) * 1000.0);
    ImGui::Text("Scene draw: %.03f ms", mState.lastSceneRenderTime * 1000.0);
    ImGui::Text("GUI draw: %.03f ms", mState.lastGuiRenderTime * 1000.0);
    ImGui::Text("Window size: %ux%u", mState.windowSize.x, mState.windowSize.y);

    if (ImGui::CollapsingHeader("Camera")) {
      ImGui::Indent();
      Camera& camera = mState.scene->camera;

      ImGui::DragFloat("Movement speed", &camera.speed, 0.001f, 0.0f, 5.0f);
      ImGui::DragFloat("FOV", &camera.fov, 0.1f, 10.0f, 120.0f);
      ImGui::DragFloat("Near", &camera.near, 0.01f, 0.01f, 10.0f);
      ImGui::DragFloat("Far", &camera.far, 0.01f, 10.0f, 1000.0f);

      ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Light")) {
      ImGui::Indent();

      if (ImGui::CollapsingHeader("Directional light")) {
        ImGui::Indent();
        DirectionalLight& directionalLight = mState.scene->directionalLight;

        ImGui::ColorPicker3("Ambient##dl", glm::value_ptr(directionalLight.colors.ambient), lightColorEditFlags);
        ImGui::ColorPicker3("Diffuse##dl", glm::value_ptr(directionalLight.colors.diffuse), lightColorEditFlags);
        ImGui::ColorPicker3("Specular##dl", glm::value_ptr(directionalLight.colors.specular), lightColorEditFlags);

        ImGui::Unindent();
      }

      if (ImGui::CollapsingHeader("Point light")) {
        ImGui::Indent();
        PointLight& pointLight = mState.scene->pointLight;

        ImGui::DragFloat("Constant", &pointLight.constant, 0.1f, 1.0f, 100.0f);
        ImGui::DragFloat("Linear", &pointLight.linear, 0.01f, 0.01f, 10.0f);
        ImGui::DragFloat("Quadratic", &pointLight.quadratic, 0.001f, 0.01f, 1.0f);
        ImGui::Spacing();

        ImGui::ColorPicker3("Ambient##pl", glm::value_ptr(pointLight.colors.ambient), lightColorEditFlags);
        ImGui::ColorPicker3("Diffuse##pl", glm::value_ptr(pointLight.colors.diffuse), lightColorEditFlags);
        if (ImGui::ColorPicker3("Specular##pl", glm::value_ptr(pointLight.colors.specular), lightColorEditFlags)) {
          mState.lightShaderProgram->get().uniforms["uLightColor"] = pointLight.colors.specular;
        }

        ImGui::Unindent();
      }

      if (ImGui::CollapsingHeader("Spotlight")) {
        ImGui::Indent();
        Spotlight& spotlight = mState.scene->spotlight;

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

    if (ImGui::CollapsingHeader("Actors")) {
      ImGui::Indent();

      for (Actor& actor : mState.scene->actors) {
        if (ImGui::CollapsingHeader(actor.name.c_str())) {
          ImGui::Indent();

          ImGui::Text("Transform");
          Transform& grassTransform = actor.transform;
          ImGui::DragFloat3(("Translation##" + actor.name).c_str(), glm::value_ptr(grassTransform.translation), 0.01f);
          ImGui::DragFloat3(("Rotation##" + actor.name).c_str(), glm::value_ptr(grassTransform.rotation), 0.01f);
          ImGui::DragFloat3(("Scale##" + actor.name).c_str(), glm::value_ptr(grassTransform.scale), 0.01f);

          if (actor.name.contains("Backpack")) {
            static constexpr const char* fsTypeNames[] = {
              "Light",
              "Lit surface",
              "Outline",
              "Visualise depth",
              "Visualise normal",
            };
            if (ImGui::Combo("Fragment shader",
                             reinterpret_cast<int32_t*>(&mState.backpackShaderProgramType),
                             fsTypeNames,
                             std::size(fsTypeNames)))
            {
              switch (mState.backpackShaderProgramType) {
                case ShaderProgramType::Light:
                  actor.setShaderProgramInstance(mState.lightShaderProgram.value());
                  break;
                case ShaderProgramType::LitSurface:
                  actor.setShaderProgramInstance(mState.litSurfaceShaderProgram.value());
                  break;
                case ShaderProgramType::Outline:
                  actor.setShaderProgramInstance(mState.backpackOutlineShaderProgram.value());
                  break;
                case ShaderProgramType::VisualiseDepth:
                  actor.setShaderProgramInstance(mState.visualiseDepthShaderProgram.value());
                  break;
                case ShaderProgramType::VisualiseNormal:
                  actor.setShaderProgramInstance(mState.visualiseNormalShaderProgram.value());
                  break;
                default:
                  PANIC("Unexpected shader program type");
              }
            }

            if (ImGui::Checkbox("Draw outline##backpack", &mState.bDrawBackpackOutline)) {
              actor.setOutlineShaderInstance(mState.bDrawBackpackOutline ? mState.backpackOutlineShaderProgram : std::nullopt);
            }

            ImGui::BeginDisabled(!mState.bDrawBackpackOutline);
            std::unordered_map<std::string, ShaderUniform>& outlineUniforms = mState.backpackOutlineShaderProgram->get().uniforms;
            ImGui::ColorPicker3("Outline color##backpack", outlineUniforms["uOutlineColor"].getValuePtr<glm::vec3>(), ImGuiColorEditFlags_Float);
            ImGui::EndDisabled();
          } else if (actor.name.contains("Light cube")) {
            if (ImGui::Checkbox("Draw outline##lightCube", &mState.bDrawLightOutline)) {
              actor.setOutlineShaderInstance(mState.bDrawLightOutline ? mState.lightCubeOutlineShaderProgram : std::nullopt);
            }

            ImGui::BeginDisabled(!mState.bDrawLightOutline);
            std::unordered_map<std::string, ShaderUniform>& outlineUniforms = mState.lightCubeOutlineShaderProgram->get().uniforms;
            ImGui::ColorPicker3("Outline color##lightCube", outlineUniforms["uOutlineColor"].getValuePtr<glm::vec3>(), ImGuiColorEditFlags_Float);
            ImGui::EndDisabled();
          }

          ImGui::Unindent();
        }
      }

      ImGui::Unindent();
    }

    ImGui::End();
  }
}

void Engine::updateScene() {
  // Flying light cube
  mState.scene->pointLight.position = {
    2.0f * glm::cos(mState.currentFrameTime * 0.05f),
    2.0f * glm::cos(mState.currentFrameTime * 0.075f),
    2.0f * glm::sin(mState.currentFrameTime * 0.05f),
  };
  mState.lightActor->get().transform.translation = mState.scene->pointLight.position;

  // Flashlight
  if (mState.bFlashlightFollowCamera) {
    mState.scene->spotlight.position = mState.scene->camera.position;
    mState.scene->spotlight.direction = mState.scene->camera.forward();
  }

  // Shaders
  mState.litSurfaceShaderProgram->get().uniforms["uViewPos"] = mState.scene->camera.position;
  mState.litSurfaceShaderProgram->get().uniforms["uDirectionalLight.colors.ambient"] = mState.scene->directionalLight.colors.ambient;
  mState.litSurfaceShaderProgram->get().uniforms["uDirectionalLight.colors.diffuse"] = mState.scene->directionalLight.colors.diffuse;
  mState.litSurfaceShaderProgram->get().uniforms["uDirectionalLight.colors.specular"] = mState.scene->directionalLight.colors.specular;
  mState.litSurfaceShaderProgram->get().uniforms["uDirectionalLight.direction"] = mState.scene->directionalLight.direction;
  mState.litSurfaceShaderProgram->get().uniforms["uPointLight.colors.ambient"] = mState.scene->pointLight.colors.ambient;
  mState.litSurfaceShaderProgram->get().uniforms["uPointLight.colors.diffuse"] = mState.scene->pointLight.colors.diffuse;
  mState.litSurfaceShaderProgram->get().uniforms["uPointLight.position"] = mState.scene->pointLight.position;
  mState.litSurfaceShaderProgram->get().uniforms["uPointLight.position"] = mState.scene->pointLight.position;
  mState.litSurfaceShaderProgram->get().uniforms["uPointLight.constant"] = mState.scene->pointLight.constant;
  mState.litSurfaceShaderProgram->get().uniforms["uPointLight.linear"] = mState.scene->pointLight.linear;
  mState.litSurfaceShaderProgram->get().uniforms["uPointLight.quadratic"] = mState.scene->pointLight.quadratic;
  mState.litSurfaceShaderProgram->get().uniforms["uSpotlight.colors.ambient"] = mState.scene->spotlight.colors.ambient;
  mState.litSurfaceShaderProgram->get().uniforms["uSpotlight.colors.diffuse"] = mState.scene->spotlight.colors.diffuse;
  mState.litSurfaceShaderProgram->get().uniforms["uSpotlight.colors.specular"] = mState.scene->spotlight.colors.specular;
  mState.litSurfaceShaderProgram->get().uniforms["uSpotlight.position"] = mState.scene->spotlight.position;
  mState.litSurfaceShaderProgram->get().uniforms["uSpotlight.direction"] = mState.scene->spotlight.direction;
  mState.litSurfaceShaderProgram->get().uniforms["uSpotlight.cutOff"] = glm::cos(glm::radians(mState.scene->spotlight.cutOff));
  mState.litSurfaceShaderProgram->get().uniforms["uSpotlight.outerCutOff"] = glm::cos(glm::radians(mState.scene->spotlight.outerCutOff));

  mState.visualiseDepthShaderProgram->get().uniforms["uCamera.near"] = mState.scene->camera.near;
  mState.visualiseDepthShaderProgram->get().uniforms["uCamera.far"] = mState.scene->camera.far;
}

void Engine::drawFrame() {
  mState.lastSceneRenderTime = timedBlock([this] {
    mState.renderingEngine->renderScene(*mState.scene, mState.scene->camera, mState.windowSize);
    mState.renderingEngine->postProcess();
    mState.renderingEngine->present(mState.windowSize);
  });

  mState.lastGuiRenderTime = timedBlock([] {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  });
}

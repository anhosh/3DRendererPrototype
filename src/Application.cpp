#include <Application.hpp>

#include <Graphics/Components/Camera.hpp>
#include <Graphics/RenderingEngine.hpp>
#include <Graphics/Scene.hpp>
#include <GUI.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/NotNull.hpp>
#include <Util/Timers/TimedBlock.hpp>

#include <backends/imgui_impl_glfw.h>

#include <random>
#include <Graphics/Components/Dirty.hpp>
#include <Graphics/Components/Graphics.hpp>
#include <Graphics/Components/Name.hpp>
#include <tracy/TracyOpenGL.hpp>

static Application* gApp = nullptr;

Application::Application(Application&& other) noexcept {
  gApp = this;

  mState = std::move(other.mState);

  other.mState.window = nullptr;
}

Expected<Application> Application::create(std::string_view title, glm::uvec2 initialWindowSize) {
  ZoneScoped;
  static constexpr std::string_view markerName = "Application init";
  FrameMarkStart(markerName.data());

  Application app;

  RETURN_ERROR_IF_UNEXPECTED(app.createContext(title, initialWindowSize));
  initialiseImGui(app.mState.window);

  app.mState.assetManager = std::make_unique<AssetManager>();
  app.mState.scene = std::make_unique<Scene>();

  app.mState.renderingEngine = std::make_unique<RenderingEngine>();
  RETURN_ERROR_IF_UNEXPECTED(app.mState.renderingEngine->init());

  app.mState.mainSceneFramebuffer = app.mState.renderingEngine->addFramebuffer({
    .size = initialWindowSize,
  });
  app.mState.backCameraSceneFramebuffer = app.mState.renderingEngine->addFramebuffer({
    .size = glm::vec2(initialWindowSize) * glm::vec2(0.4f, 0.2f),
  });

  RETURN_ERROR_IF_UNEXPECTED(app.createScene());
  FrameMarkEnd(markerName.data());
  return app;
}

void Application::run() {
  FrameMark;
  while (!glfwWindowShouldClose(mState.window)) {
    ZoneScopedN("Frame");

    mState.lastFrameTime = mState.currentFrameTime;
    mState.currentFrameTime = glfwGetTime();

    this->processKeyboard();
    glfwPollEvents();

    if (glfwGetWindowAttrib(mState.window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    runImGui(mState);
    this->updateScene();
    this->drawFrame();

    glfwSwapBuffers(mState.window);

    FrameMark;
    TracyGpuCollect;
  }
}

void Application::shutDown() {
  shutdownImGui();

  mState.scene->destroy();
  mState.renderingEngine->destroy();

  glfwDestroyWindow(mState.window);
  mState.window = nullptr;
  glfwTerminate();

  assert(gApp != nullptr);
  gApp = nullptr;
}

Application::Application() {
  assert(gApp == nullptr);
  gApp = this;
}

Expected<void> Application::createContext(std::string_view title, glm::uvec2 initialWindowSize) {
  ZoneScoped;

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
    const glm::uvec2 newSize = {width, height};
    NotNull(gApp)->mState.windowSize = newSize;
    NotNull(gApp)->mState.mainSceneFramebuffer.value()->resize(newSize);
    NotNull(gApp)->mState.backCameraSceneFramebuffer.value()->resize(glm::vec2(newSize) * glm::vec2(0.4f, 0.2f));
    NotNull(gApp)->mState.lastMousePosition = glm::vec2(newSize) * 0.5f;
    NotNull(gApp)->mState.bFirstMouse = true;
  });

  glfwSetCursorPosCallback(mState.window, [](GLFWwindow*, double xpos, double ypos) {
    NotNull(gApp)->processMousePosition(glm::vec2(xpos, ypos));
  });

  TracyGpuContext;

  return {};
}

Expected<void> Application::createScene() {
  ZoneScoped;

  // Load assets
  const Expected modelPlanet = mState.assetManager->loadModel("planet/planet.obj");
  const Expected modelRock = mState.assetManager->loadModel("rock/rock.obj");

  const Expected bitmapSkyboxRight  = mState.assetManager->loadBitmap("skybox/space/right.png", false);
  const Expected bitmapSkyboxLeft   = mState.assetManager->loadBitmap("skybox/space/left.png", false);
  const Expected bitmapSkyboxTop    = mState.assetManager->loadBitmap("skybox/space/top.png", false);
  const Expected bitmapSkyboxBottom = mState.assetManager->loadBitmap("skybox/space/bottom.png", false);
  const Expected bitmapSkyboxBack   = mState.assetManager->loadBitmap("skybox/space/back.png", false);
  const Expected bitmapSkyboxFront  = mState.assetManager->loadBitmap("skybox/space/front.png", false);

  const AssetHandle<Mesh> skyboxCubeMesh = mState.assetManager->addMesh(Mesh::createCube(glm::vec3(2.0f)));
  const AssetHandle<Mesh> lightCubeMesh = mState.assetManager->addMesh(Mesh::createCube(glm::vec3(1.0f)));

  RETURN_ERROR_IF_UNEXPECTED(modelPlanet);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxRight);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxLeft);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxTop);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBottom);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBack);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxFront);

  // Get shader instances
  mState.litSurfaceShader         = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::LitSurface);
  mState.postProcessingCopyShader = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::PostProcessCopy);

  // Upload assets to GPU
  std::vector<RenderData> planetMeshes = mState.renderingEngine->addModel(modelPlanet.value(), mState.litSurfaceShader.value());
  std::vector<RenderData> rockMeshes   = mState.renderingEngine->addModel(modelRock.value(), mState.litSurfaceShader.value());

  TextureCubeMapHandle skyboxTexture = mState.renderingEngine->addTextureCubeMap(
    TextureCubeMapBitmaps {
      bitmapSkyboxRight.value(),
      bitmapSkyboxLeft.value(),
      bitmapSkyboxTop.value(),
      bitmapSkyboxBottom.value(),
      bitmapSkyboxBack.value(),
      bitmapSkyboxFront.value(),
    },
    SamplerOptions { .minFilter = GL_LINEAR }
  );

  // Create scene
  mState.mainCamera = mState.scene->ecs.create();
  mState.scene->ecs.emplace<CompName>(mState.mainCamera, "Main camera");
  mState.scene->ecs.emplace<CompTransform>(mState.mainCamera, CompTransform {
    .translation = glm::vec3(0.0f, 0.0f, 10.0f),
    .rotation = glm::vec3(-90.0f, 0.0f, 0.0f),
  });
  CompCamera& mainCamera = mState.scene->ecs.emplace<CompCamera>(mState.mainCamera);
  mainCamera.speed = 10.0f;
  mState.scene->ecs.emplace<CompDirty>(mState.mainCamera);

  mState.backCamera = mState.scene->ecs.create();
  mState.scene->ecs.emplace<CompName>(mState.backCamera, "Main camera");
  mState.scene->ecs.emplace<CompTransform>(mState.backCamera);
  mState.scene->ecs.emplace<CompCamera>(mState.backCamera);

  mState.scene->skybox = Skybox {
    .cubeMesh = mState.renderingEngine->addMesh(skyboxCubeMesh),
    .texture = skyboxTexture,
    .shader = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::Skybox),
  };

  const entt::entity entitySun = mState.scene->ecs.create();
  mState.scene->ecs.emplace<CompName>(entitySun, "Sun");
  mState.scene->ecs.emplace<CompDirectionalLight>(entitySun, CompDirectionalLight {
    .colors =  LightColors {
      .ambient = glm::vec3(0.05f),
      .diffuse = glm::vec3(1.2f),
      .specular = glm::vec3(3.0f),
    },
    .direction = glm::vec3(0.3f, -1.0f, -0.3f),
  });
  mState.scene->ecs.emplace<CompDirty>(entitySun);

  constexpr auto lightPositions = std::array {
    glm::vec3(-3.0f, -3.0f, -3.0f),
    glm::vec3( 3.0f, -3.0f, -3.0f),
    glm::vec3(-3.0f, -3.0f,  3.0f),
    glm::vec3( 3.0f, -3.0f,  3.0f),
  };
  constexpr auto lightColors = std::array {
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
  };
  for (size_t lightIndex = 0; const glm::vec3& position : lightPositions) {
    const entt::entity entityLight = mState.scene->ecs.create();
    mState.scene->ecs.emplace<CompName>(entityLight, ("Light " + std::to_string(lightIndex)).c_str());
    mState.scene->ecs.emplace<CompTransform>(entityLight, CompTransform {
      .translation = position,
    });
    mState.scene->ecs.emplace<CompPointLight>(entityLight, CompPointLight {
      .colors = LightColors {
        .ambient = 0.05f * lightColors[lightIndex],
        .diffuse = 1.2f * lightColors[lightIndex],
        .specular = 3.0f * lightColors[lightIndex],
      },
    });
    mState.scene->ecs.emplace<CompGraphics>(entityLight, CompGraphics {
      .renderData = {
        RenderData {
          .vertexArray = mState.renderingEngine->addMesh(lightCubeMesh),
          .shaderProgramInstance = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::Light),
        },
      },
    });
    mState.scene->ecs.emplace<CompDirty>(entityLight);
    ++lightIndex;
  }

  const entt::entity entityPlanet = mState.scene->ecs.create();
  mState.scene->ecs.emplace<CompName>(entityPlanet, "Planet Mars");
  mState.scene->ecs.emplace<CompTransform>(entityPlanet);
  mState.scene->ecs.emplace<CompGraphics>(entityPlanet, planetMeshes);
  mState.scene->ecs.emplace<CompDirty>(entityPlanet);

  std::random_device rd;
  std::mt19937 gen(rd());
  constexpr float offset = 25.0f;
  std::uniform_real_distribution displacementDistribution(-offset, offset);
  std::uniform_real_distribution rotationAngleDistribution(0.0f, 360.0f);
  std::uniform_real_distribution scaleDistribution(0.05f, 0.25f);
  constexpr uint32_t numAsteroids = 10000;
  for (uint32_t i = 0; i < numAsteroids; ++i) {
    constexpr float radius = 50.0f;
    const float angle = static_cast<float>(i) / static_cast<float>(numAsteroids) * 360.0f;
    const entt::entity entityAsteroid = mState.scene->ecs.create();
    mState.scene->ecs.emplace<CompName>(entityAsteroid, std::format("Asteroid {}", i));
    mState.scene->ecs.emplace<CompTransform>(entityAsteroid, CompTransform {
      .translation = {
        glm::sin(glm::radians(angle)) * radius + displacementDistribution(gen),
        0.4f * displacementDistribution(gen),
        glm::cos(glm::radians(angle)) * radius + displacementDistribution(gen),
      },
      .rotation = rotationAngleDistribution(gen) * glm::vec3(0.4f, 0.6f, 0.8f),
      .scale = glm::vec3(scaleDistribution(gen)),
    });
    mState.scene->ecs.emplace<CompGraphics>(entityAsteroid, rockMeshes);
    mState.scene->ecs.emplace<CompDirty>(entityAsteroid);
  }

  mState.scene->prepareForRendering();
  return {};
}

void Application::processKeyboard() {
  ZoneScoped;

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
  auto [camera, cameraTransform] = mState.scene->ecs.get<CompCamera, CompTransform>(mState.mainCamera);

  if (glfwGetKey(mState.window, GLFW_KEY_W) == GLFW_PRESS) {
    cameraTransform.translation += deltaTime * camera.speed * cameraTransform.forward();
  }
  if (glfwGetKey(mState.window, GLFW_KEY_S) == GLFW_PRESS) {
    cameraTransform.translation -= deltaTime * camera.speed * cameraTransform.forward();
  }
  if (glfwGetKey(mState.window, GLFW_KEY_A) == GLFW_PRESS) {
    cameraTransform.translation -= deltaTime * camera.speed * glm::normalize(glm::cross(cameraTransform.forward(), cameraTransform.up()));
  }
  if (glfwGetKey(mState.window, GLFW_KEY_D) == GLFW_PRESS) {
    cameraTransform.translation += deltaTime * camera.speed * glm::normalize(glm::cross(cameraTransform.forward(), cameraTransform.up()));
  }
  if (glfwGetKey(mState.window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    cameraTransform.translation += deltaTime * camera.speed * cameraTransform.up();
  }
  if (glfwGetKey(mState.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    cameraTransform.translation -= deltaTime * camera.speed * cameraTransform.up();
  }
}

void Application::processMousePosition(glm::vec2 mousePosition) {
  ZoneScoped;

  if (mState.bFreeCursor) {
    return;
  }

  if (mState.bFirstMouse) {
    mState.lastMousePosition = mousePosition;
    mState.bFirstMouse = false;
  }

  mState.scene->ecs.patch<CompTransform>(mState.mainCamera, [&](CompTransform& cameraTransform) {
    constexpr float sensitivity = 0.1f;
    const glm::vec2 offset = {
      (mousePosition.x - mState.lastMousePosition.x) * sensitivity,
      (mState.lastMousePosition.y - mousePosition.y) * sensitivity,
    };
    cameraTransform.rotation.x += offset.x;
    cameraTransform.rotation.y = glm::clamp(cameraTransform.rotation.y + offset.y, -89.0f, 89.0f);
  });

  mState.lastMousePosition = mousePosition;
}

void Application::updateScene() {
  ZoneScoped;

  // Back mirror camera
  if (mState.bBackMirror) {
    const CompTransform mainCameraTransform = mState.scene->ecs.get<CompTransform>(mState.mainCamera);
    mState.scene->ecs.patch<CompTransform>(mState.backCamera, [&](CompTransform& backCameraTransform) {
      backCameraTransform = mainCameraTransform;
      backCameraTransform.rotation.x += 180.0f;
      backCameraTransform.rotation.y *= -1.0f;
    });
  }
}

void Application::drawFrame() {
  ZoneScoped;

  mState.renderPasses.clear();
  if (mState.bBackMirror) {
    mState.renderPasses.emplace_back(Viewport {}, mState.backCameraSceneFramebuffer.value(),
                                     RenderScenePass { mState.scene.get(), mState.backCamera });
  }
  mState.renderPasses.emplace_back(Viewport {}, mState.mainSceneFramebuffer.value(),
                                   RenderScenePass { mState.scene.get(), mState.mainCamera });

  FramebufferHandle lastFramebuffer = mState.mainSceneFramebuffer.value();
  size_t shaderIndex = 0;
  for (FramebufferHandle framebuffer : mState.postProcessingFramebuffers) {
    const PostProcessingPass pass = {
      .srcFramebuffer = lastFramebuffer,
      .postProcessingShader = mState.postProcessingShaderProgramInstances[shaderIndex++],
    };
    mState.renderPasses.emplace_back(Viewport {}, framebuffer, pass);
    lastFramebuffer = framebuffer;
  }

  if (mState.bBackMirror) {
    mState.renderPasses.push_back({
      .viewport = {
        .position = { 0.3f, 0.8f },
        .size = { 0.4f, 0.2f },
      },
      .dstFramebuffer = mState.renderPasses.back().dstFramebuffer,
      .pass = PostProcessingPass {
        .srcFramebuffer = mState.backCameraSceneFramebuffer.value(),
        .postProcessingShader = mState.postProcessingCopyShader.value(),
      },
    });
  }

  mState.lastSceneRenderDuration = timedBlock([&, this] {
    mState.renderingEngine->submitRenderPasses(mState.renderPasses);
    mState.renderingEngine->present(mState.windowSize, mState.renderPasses.back().dstFramebuffer);
  });

  mState.lastGuiRenderDuration = timedBlock(renderImGui);
}

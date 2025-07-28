#include <Application.hpp>

#include <Assets/Bitmap.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/RenderingEngine.hpp>
#include <Graphics/Scene.hpp>
#include <GUI.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/NotNull.hpp>
#include <Util/Timers/TimedBlock.hpp>

#include <backends/imgui_impl_glfw.h>

static Application* gApp = nullptr;

Application::Application(Application&& other) noexcept {
  gApp = this;

  mState = std::move(other.mState);

  other.mState.window = nullptr;
}

Expected<Application> Application::create(std::string_view title, glm::uvec2 initialWindowSize) {
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
  return app;
}

void Application::run() {
  while (!glfwWindowShouldClose(mState.window)) {
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

  return {};
}

Expected<void> Application::createScene() {
  // Load assets
  const Expected modelBackpack = mState.assetManager->loadModel("backpack/backpack.obj");

  const Expected bitmapSkyboxRight  = mState.assetManager->loadBitmap("skybox/sea_afternoon/right.jpg", false);
  const Expected bitmapSkyboxLeft   = mState.assetManager->loadBitmap("skybox/sea_afternoon/left.jpg", false);
  const Expected bitmapSkyboxTop    = mState.assetManager->loadBitmap("skybox/sea_afternoon/top.jpg", false);
  const Expected bitmapSkyboxBottom = mState.assetManager->loadBitmap("skybox/sea_afternoon/bottom.jpg", false);
  const Expected bitmapSkyboxBack   = mState.assetManager->loadBitmap("skybox/sea_afternoon/back.jpg", false);
  const Expected bitmapSkyboxFront  = mState.assetManager->loadBitmap("skybox/sea_afternoon/front.jpg", false);

  const AssetHandle<Mesh> skyboxCubeMesh = mState.assetManager->addMesh(Mesh::createCube(glm::vec3(2.0f)));
  const AssetHandle<Mesh> lightCubeMesh = mState.assetManager->addMesh(Mesh::createCube(glm::vec3(0.2f)));

  RETURN_ERROR_IF_UNEXPECTED(modelBackpack);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxRight);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxLeft);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxTop);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBottom);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBack);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxFront);

  // Get shader instances
  mState.litSurfaceShader         = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::LitSurface);
  mState.litExplodedShader        = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::LitExploded);
  mState.lightShader              = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::Light);
  mState.reflectiveSurfaceShader  = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::ReflectiveSurface);
  mState.refractiveSurfaceShader  = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::RefractiveSurface);
  mState.backpackOutlineShader    = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::Outline);
  mState.postProcessingCopyShader = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::PostProcessCopy);

  // Upload assets to GPU
  std::vector<RenderData> backpackMeshes = mState.renderingEngine->addModel(modelBackpack.value(), mState.litSurfaceShader.value());
  VertexArrayHandle lightCubeVA = mState.renderingEngine->addMesh(lightCubeMesh);

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

  for (RenderData& mesh : backpackMeshes) {
    mesh.environmentMap = skyboxTexture;
    mesh.renderOptions.bBackfaceCulling = false;
  }

  // Create scene
  mState.mainCamera.position = glm::vec3(0.0f, 0.0f, 3.0f);
  mState.mainCamera.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);

  mState.scene->skybox = Skybox {
    .cubeMesh = mState.renderingEngine->addMesh(skyboxCubeMesh),
    .texture = skyboxTexture,
    .shader = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::Skybox),
  };

  mState.scene->directionalLights.add(DirectionalLight {
    .name = "Sun",
    .colors = LightColors {
      // .ambient = glm::vec3(0.587f),
      .ambient = glm::vec3(0.1f),
      .diffuse = glm::vec3(0.808f),
      .specular = glm::vec3(1.0f),
    },
    .direction = glm::vec3(0.3f, -1.0f, 0.3f),
  });

  mState.flashlight = mState.scene->spotlights.add(Spotlight {
    .name = "Flashlight",
    .colors = LightColors {
      .ambient = glm::vec3(0.1f),
      .diffuse = glm::vec3(0.5f),
      .specular = glm::vec3(1.0f),
    },
  });

  constexpr auto pointLightPositions = std::array {
    glm::vec3(2.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 2.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 2.0f),
    glm::vec3(2.0f, 2.0f, 0.0f),
  };
  size_t lightCubeIndex = 0;
  for (glm::vec3 position : pointLightPositions) {
    ShaderProgramInstanceHandle instance = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::Light);
    instance->uniforms["uLightColor"] = position * 0.5f;
    mState.scene->actors.add(Actor {
      .name = "Light cube " + std::to_string(lightCubeIndex),
      .transform = Transform {
        .translation = position,
      },
      .renderData = {
        RenderData {
          .vertexArray = lightCubeVA,
          .shaderProgramInstance = instance,
        },
      },
    });
    mState.scene->pointLights.add(PointLight {
      .name = "Point light " + std::to_string(lightCubeIndex),
      .colors = LightColors {
        .ambient = glm::vec3(0.1f),
        .diffuse = position * 0.25f,
        .specular = position * 0.5f,
      },
      .position = position,
    });
    ++lightCubeIndex;
  }

  mState.scene->actors.add(Actor {
    .name = "Backpack",
    .transform = Transform {
      .translation = glm::vec3(0.0f),
    },
    .renderData = std::move(backpackMeshes),
  });

  return {};
}

void Application::processKeyboard() {
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
  Camera& camera = mState.mainCamera;

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

void Application::processMousePosition(glm::vec2 mousePosition) {
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
  Camera& camera = mState.mainCamera;
  camera.rotation.x += offset.x;
  camera.rotation.y = glm::clamp(camera.rotation.y + offset.y, -89.0f, 89.0f);

  mState.lastMousePosition = mousePosition;
}

void Application::updateScene() {
  // Back mirror camera
  mState.backCamera = mState.mainCamera;
  mState.backCamera.rotation.x += 180.0f;
  mState.backCamera.rotation.y *= -1.0f;

  // Flashlight
  if (mState.bFlashlightFollowCamera) {
    mState.flashlight.value()->position = mState.mainCamera.position;
    mState.flashlight.value()->direction = mState.mainCamera.forward();
  }
}

void Application::drawFrame() {
  mState.renderPasses.clear();
  if (mState.bBackMirror) {
    mState.renderPasses.emplace_back(Viewport {}, mState.backCameraSceneFramebuffer.value(),
                                     RenderScenePass { mState.scene.get(), &mState.backCamera });
  }
  mState.renderPasses.emplace_back(Viewport {}, mState.mainSceneFramebuffer.value(),
                                   RenderScenePass { mState.scene.get(), &mState.mainCamera });

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

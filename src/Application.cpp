#include <Application.hpp>

#include <GUI.hpp>

#include <Assets/Bitmap.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Meshes.hpp>
#include <Graphics/RenderingEngine.hpp>
#include <Graphics/Scene.hpp>
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
  app.mState.assetManager = std::make_unique<AssetManager>();
  app.mState.scene = std::make_unique<Scene>();
  app.mState.renderingEngine = std::make_unique<RenderingEngine>();
  app.mState.mainSceneFramebuffer = app.mState.renderingEngine->addFramebuffer({
    .size = initialWindowSize,
  });
  app.mState.backCameraSceneFramebuffer = app.mState.renderingEngine->addFramebuffer({
    .size = glm::vec2(initialWindowSize) * glm::vec2(0.4f, 0.2f),
  });
  initialiseImGui(app.mState.window);
  RETURN_ERROR_IF_UNEXPECTED(app.mState.renderingEngine->init());
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

  mState.postProcessingCopyShaderProgramInstance = mState.renderingEngine->createShaderProgramInstance(ShaderProgramType::PostProcessCopy);

  // Upload assets to GPU
  const VertexArrayHandle quadVA = mState.renderingEngine->addMesh(quadMesh);
  const VertexArrayHandle cubeVA = mState.renderingEngine->addMesh(cubeMesh);

  constexpr SamplerOptions samplerOptionsClampToEdge = { .wrapS = GL_CLAMP_TO_EDGE, .wrapT = GL_CLAMP_TO_EDGE };
  const TextureHandle grassTexture = mState.renderingEngine->addTexture(grassBitmap.value(), samplerOptionsClampToEdge);
  const TextureHandle windowTexture = mState.renderingEngine->addTexture(windowBitmap.value(), samplerOptionsClampToEdge);

  constexpr RenderOptions transparentQuadOptions = { .bBackfaceCulling = false, .bTransparent = true };
  std::vector<RenderData> backpackResources = mState.renderingEngine->addModel(backpackModel.value(), mState.litSurfaceShaderProgram.value());
  std::vector lightResources = {
    RenderData {
      .vertexArray = cubeVA,
      .shaderProgramInstance = mState.lightShaderProgram.value(),
    }
  };
  std::vector grassResources = {
    RenderData {
      .vertexArray = quadVA,
      .shaderProgramInstance = mState.litSurfaceShaderProgram.value(),
      .diffuseMap = grassTexture,
      .renderOptions = transparentQuadOptions,
    }
  };
  std::vector windowResources = {
    RenderData {
      .vertexArray = quadVA,
      .shaderProgramInstance = mState.litSurfaceShaderProgram.value(),
      .diffuseMap = windowTexture,
      .renderOptions = transparentQuadOptions,
    }
  };

  // Create actors
  mState.backpackActor = mState.scene->addActor(Actor {
    .name = "Backpack",
    .transform = Transform {
      .translation = glm::vec3(0.0f),
    },
    .renderData = std::move(backpackResources),
  });

  mState.lightActor = mState.scene->addActor(Actor {
    .name = "Light cube",
    .transform = Transform {
      .scale = glm::vec3(0.1f),
    },
    .renderData = std::move(lightResources),
  });

  mState.grassActor = mState.scene->addActor(Actor {
    .name = "Grass",
    .transform = Transform {
      .translation = glm::vec3(0.0f, 0.0f, 1.0f),
    },
    .renderData = std::move(grassResources),
  });

  Actor windowActor = { .renderData = windowResources };
  windowActor.name = "Window 0";
  windowActor.transform.translation = glm::vec3(-0.25f, 0.0f, 1.5f);
  mState.scene->addActor(Actor(windowActor));
  windowActor.name = "Window 1";
  windowActor.transform.translation = glm::vec3(0.25f, 0.0f, 1.75f);
  mState.scene->addActor(Actor(windowActor));
  windowActor.name = "Window 2";
  windowActor.transform.translation = glm::vec3(0.0f, 0.0f, 2.0f);
  mState.scene->addActor(std::move(windowActor));

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
  // Flying light cube
  mState.scene->pointLight.position = {
    2.0f * glm::cos(mState.currentFrameTime * 0.05f),
    2.0f * glm::cos(mState.currentFrameTime * 0.075f),
    2.0f * glm::sin(mState.currentFrameTime * 0.05f),
  };
  (*mState.lightActor)->transform.translation = mState.scene->pointLight.position;

  // Flashlight
  if (mState.bFlashlightFollowCamera) {
    mState.scene->spotlight.position = mState.mainCamera.position;
    mState.scene->spotlight.direction = mState.mainCamera.forward();
  }

  // Shaders
  (*mState.litSurfaceShaderProgram)->uniforms["uViewPos"] = mState.mainCamera.position;
  (*mState.litSurfaceShaderProgram)->uniforms["uDirectionalLight.colors.ambient"] = mState.scene->directionalLight.colors.ambient;
  (*mState.litSurfaceShaderProgram)->uniforms["uDirectionalLight.colors.diffuse"] = mState.scene->directionalLight.colors.diffuse;
  (*mState.litSurfaceShaderProgram)->uniforms["uDirectionalLight.colors.specular"] = mState.scene->directionalLight.colors.specular;
  (*mState.litSurfaceShaderProgram)->uniforms["uDirectionalLight.direction"] = mState.scene->directionalLight.direction;
  (*mState.litSurfaceShaderProgram)->uniforms["uPointLight.colors.ambient"] = mState.scene->pointLight.colors.ambient;
  (*mState.litSurfaceShaderProgram)->uniforms["uPointLight.colors.diffuse"] = mState.scene->pointLight.colors.diffuse;
  (*mState.litSurfaceShaderProgram)->uniforms["uPointLight.position"] = mState.scene->pointLight.position;
  (*mState.litSurfaceShaderProgram)->uniforms["uPointLight.position"] = mState.scene->pointLight.position;
  (*mState.litSurfaceShaderProgram)->uniforms["uPointLight.constant"] = mState.scene->pointLight.constant;
  (*mState.litSurfaceShaderProgram)->uniforms["uPointLight.linear"] = mState.scene->pointLight.linear;
  (*mState.litSurfaceShaderProgram)->uniforms["uPointLight.quadratic"] = mState.scene->pointLight.quadratic;
  (*mState.litSurfaceShaderProgram)->uniforms["uSpotlight.colors.ambient"] = mState.scene->spotlight.colors.ambient;
  (*mState.litSurfaceShaderProgram)->uniforms["uSpotlight.colors.diffuse"] = mState.scene->spotlight.colors.diffuse;
  (*mState.litSurfaceShaderProgram)->uniforms["uSpotlight.colors.specular"] = mState.scene->spotlight.colors.specular;
  (*mState.litSurfaceShaderProgram)->uniforms["uSpotlight.position"] = mState.scene->spotlight.position;
  (*mState.litSurfaceShaderProgram)->uniforms["uSpotlight.direction"] = mState.scene->spotlight.direction;
  (*mState.litSurfaceShaderProgram)->uniforms["uSpotlight.cutOff"] = glm::cos(glm::radians(mState.scene->spotlight.cutOff));
  (*mState.litSurfaceShaderProgram)->uniforms["uSpotlight.outerCutOff"] = glm::cos(glm::radians(mState.scene->spotlight.outerCutOff));

  (*mState.visualiseDepthShaderProgram)->uniforms["uCamera.near"] = mState.mainCamera.near;
  (*mState.visualiseDepthShaderProgram)->uniforms["uCamera.far"] = mState.mainCamera.far;
}

void Application::drawFrame() {
  Camera backCamera = mState.mainCamera;
  backCamera.rotation.x += 180.0f;
  backCamera.rotation.y *= -1.0f;

  mState.renderPasses.clear();
  if (mState.bBackMirror) {
    mState.renderPasses.emplace_back(Viewport {}, mState.backCameraSceneFramebuffer.value(),
                                     RenderScenePass { mState.scene.get(), &backCamera });
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
        .postProcessingShader = mState.postProcessingCopyShaderProgramInstance.value(),
      },
    });
  }

  mState.lastSceneRenderTime = timedBlock([&, this] {
    mState.renderingEngine->submitRenderPasses(mState.renderPasses);
    mState.renderingEngine->present(mState.windowSize, mState.renderPasses.back().dstFramebuffer);
  });

  mState.lastGuiRenderTime = timedBlock(renderImGui);
}

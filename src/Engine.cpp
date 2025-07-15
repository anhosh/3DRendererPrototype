#include <Engine.hpp>

#include <GUI.hpp>

#include <Assets/Bitmap.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Meshes.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/NotNull.hpp>
#include <Util/Timers/TimedBlock.hpp>

#include <backends/imgui_impl_glfw.h>

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
  initialiseImGui(app.mState.window);
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

    runImGui(mState);
    this->updateScene();
    this->drawFrame();

    glfwSwapBuffers(mState.window);
  }
}

void Engine::shutDown() {
  shutdownImGui();

  mState.scene->destroy();
  mState.renderingEngine->destroy();

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
  const VertexArrayHandle quadVA = mState.renderingEngine->addMesh(quadMesh);
  const VertexArrayHandle cubeVA = mState.renderingEngine->addMesh(cubeMesh);

  constexpr SamplerOptions samplerOptionsClampToEdge = { .wrapS = GL_CLAMP_TO_EDGE, .wrapT = GL_CLAMP_TO_EDGE };
  const TextureHandle grassTexture = mState.renderingEngine->addTexture(grassBitmap.value(), samplerOptionsClampToEdge);
  const TextureHandle windowTexture = mState.renderingEngine->addTexture(windowBitmap.value(), samplerOptionsClampToEdge);

  constexpr RenderingEngine::RenderOptions transparentQuadOptions = { .bBackfaceCulling = false, .bTransparent = true };
  std::vector<RenderingEngine::RenderData> backpackResources = mState.renderingEngine->addModel(backpackModel.value(), mState.litSurfaceShaderProgram.value());
  std::vector lightResources = {
    RenderingEngine::RenderData {
      .vertexArray = cubeVA,
      .shaderProgramInstance = mState.lightShaderProgram.value(),
    }
  };
  std::vector grassResources = {
    RenderingEngine::RenderData {
      .vertexArray = quadVA,
      .shaderProgramInstance = mState.litSurfaceShaderProgram.value(),
      .diffuseMap = grassTexture,
      .renderOptions = transparentQuadOptions,
    }
  };
  std::vector windowResources = {
    RenderingEngine::RenderData {
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
    mState.renderingEngine->postProcess(mState.postProcessingShaderProgramInstances);
    mState.renderingEngine->present(mState.windowSize);
  });

  mState.lastGuiRenderTime = timedBlock(renderImGui);
}

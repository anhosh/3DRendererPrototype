#include <Demos/SpaceDemo.hpp>

#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Name.hpp>
#include <Util/Macros/Errors.hpp>

#include <imgui.h>

#include <random>

Expected<void> SpaceDemo::init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) {
  ZoneScoped;
  
  RETURN_ERROR_IF_UNEXPECTED(FlyCamDemoBase::init(assets, renderer));

  mMainSceneFramebuffer = mRenderingEngine->addFramebuffer({
    .size = glm::uvec2(1),
    .samples = 4,
  });
  
  const ShaderProgramInstanceHandle gammaCorrectionShader = mRenderingEngine->createShaderProgramInstance(ShaderProgramType::PostProcessGammaCorrection);
  mPostProcessingShaderProgramInstances.push_back(gammaCorrectionShader);
  mPostProcessingFramebuffers.push_back(mRenderingEngine->addFramebuffer({
    .size = glm::uvec2(1),
    .bDepthStencil = false,
  }));

  // Load assets
  const Expected modelBackpack = assets->loadModel("backpack/backpack.obj");
  const Expected modelPlanet = assets->loadModel("planet/planet.obj");
  const Expected modelRock = assets->loadModel("rock/rock.obj");

  Expected bitmapSkyboxRight  = assets->loadBitmap("skybox/space/right.png", false);
  Expected bitmapSkyboxLeft   = assets->loadBitmap("skybox/space/left.png", false);
  Expected bitmapSkyboxTop    = assets->loadBitmap("skybox/space/top.png", false);
  Expected bitmapSkyboxBottom = assets->loadBitmap("skybox/space/bottom.png", false);
  Expected bitmapSkyboxBack   = assets->loadBitmap("skybox/space/back.png", false);
  Expected bitmapSkyboxFront  = assets->loadBitmap("skybox/space/front.png", false);

  const AssetHandle<MeshData> skyboxCubeMesh = assets->addMesh(MeshData::createCube(glm::vec3(2.0f)));
  const AssetHandle<MeshData> lightCubeMesh = assets->addMesh(MeshData::createCube(glm::vec3(1.0f)));

  RETURN_ERROR_IF_UNEXPECTED(modelBackpack);
  RETURN_ERROR_IF_UNEXPECTED(modelPlanet);
  RETURN_ERROR_IF_UNEXPECTED(modelRock);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxRight);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxLeft);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxTop);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBottom);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBack);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxFront);

  // Get shader instances
  const ShaderProgramInstanceHandle litSurfaceShader = mRenderingEngine->createShaderProgramInstance(ShaderProgramType::LitSurface);

  // Upload assets to GPU
  std::vector<RenderData> backpackMeshes = mRenderingEngine->addModel(modelBackpack.value(), litSurfaceShader);
  std::vector<RenderData> planetMeshes = mRenderingEngine->addModel(modelPlanet.value(), litSurfaceShader);
  std::vector<RenderData> rockMeshes   = mRenderingEngine->addModel(modelRock.value(), litSurfaceShader);

  TextureCubeMapHandle skyboxTexture = mRenderingEngine->addTextureCubeMap(
    TextureCubeMapBitmaps {
      bitmapSkyboxRight.value(),
      bitmapSkyboxLeft.value(),
      bitmapSkyboxTop.value(),
      bitmapSkyboxBottom.value(),
      bitmapSkyboxBack.value(),
      bitmapSkyboxFront.value(),
      .bSRGB = true,
    }
  );

  // Create scene
  mScene.skybox = Skybox {
    .cubeMesh = mRenderingEngine->addMesh(skyboxCubeMesh),
    .texture = skyboxTexture,
    .shader = mRenderingEngine->createShaderProgramInstance(ShaderProgramType::Skybox),
  };

  const entt::entity entitySun = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(entitySun, "Sun");
  mScene.ecs.emplace<CompDirectionalLight>(entitySun, CompDirectionalLight {
    .colors =  LightColors {
      .ambient = glm::vec3(0.01f),
      .diffuse = glm::vec3(1.0f),
      .specular = glm::vec3(2.0f),
    },
    .direction = glm::vec3(0.3f, -1.0f, -0.3f),
  });

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
    const entt::entity entityLight = mScene.ecs.create();
    mScene.ecs.emplace<CompName>(entityLight, ("Light " + std::to_string(lightIndex)).c_str());
    mScene.ecs.emplace<CompTransform>(entityLight, CompTransform {
      .translation = position,
    });
    mScene.ecs.emplace<CompPointLight>(entityLight, CompPointLight {
      .colors = LightColors {
        .ambient = 0.01f * lightColors[lightIndex],
        .diffuse = 1.0f * lightColors[lightIndex],
        .specular = 2.0f * lightColors[lightIndex],
      },
    });
    mScene.ecs.emplace<CompGraphics>(entityLight, CompGraphics {
      .renderData = {
        RenderData {
          .mesh = mRenderingEngine->addMesh(lightCubeMesh),
          .shaderProgramInstance = mRenderingEngine->createShaderProgramInstance(ShaderProgramType::Light),
        },
      },
    });
    ++lightIndex;
  }

  const entt::entity entityPlanet = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(entityPlanet, "Planet Mars");
  mScene.ecs.emplace<CompTransform>(entityPlanet);
  mScene.ecs.emplace<CompGraphics>(entityPlanet, planetMeshes);

  const entt::entity entityBackpack = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(entityBackpack, "Backpack");
  mScene.ecs.emplace<CompTransform>(entityBackpack, CompTransform {
    .translation = glm::vec3(0.0f, 6.0f, 0.0f),
  });
  mScene.ecs.emplace<CompGraphics>(entityBackpack, backpackMeshes);

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
    const entt::entity entityAsteroid = mScene.ecs.create();
    mScene.ecs.emplace<CompName>(entityAsteroid, std::format("Asteroid {}", i));
    mScene.ecs.emplace<CompTransform>(entityAsteroid, CompTransform {
      .translation = {
        glm::sin(glm::radians(angle)) * radius + displacementDistribution(gen),
        0.4f * displacementDistribution(gen),
        glm::cos(glm::radians(angle)) * radius + displacementDistribution(gen),
      },
      .rotation = rotationAngleDistribution(gen) * glm::vec3(0.4f, 0.6f, 0.8f),
      .scale = glm::vec3(scaleDistribution(gen)),
    });
    mScene.ecs.emplace<CompGraphics>(entityAsteroid, rockMeshes);
  }

  mScene.prepareForRendering();
  return {};
}

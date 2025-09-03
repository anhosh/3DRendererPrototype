#include <Demos/SpaceDemo.hpp>

#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Name.hpp>
#include <Util/Macros/Errors.hpp>

#include <imgui.h>

#include <random>

Expected<void> SpaceDemo::init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) {
  ZoneScoped;

  // Load assets
  const Expected modelPlanet = assets->loadModel("planet/planet.obj");
  const Expected modelRock = assets->loadModel("rock/rock.obj");

  Expected bitmapSkyboxRight  = assets->loadBitmap("skybox/space/right.png", false);
  Expected bitmapSkyboxLeft   = assets->loadBitmap("skybox/space/left.png", false);
  Expected bitmapSkyboxTop    = assets->loadBitmap("skybox/space/top.png", false);
  Expected bitmapSkyboxBottom = assets->loadBitmap("skybox/space/bottom.png", false);
  Expected bitmapSkyboxBack   = assets->loadBitmap("skybox/space/back.png", false);
  Expected bitmapSkyboxFront  = assets->loadBitmap("skybox/space/front.png", false);

  const AssetHandle<MeshData> skyboxCubeMesh = assets->addMesh(MeshData::createCube(glm::vec3(2.0f)));

  RETURN_ERROR_IF_UNEXPECTED(modelPlanet);
  RETURN_ERROR_IF_UNEXPECTED(modelRock);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxRight);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxLeft);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxTop);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBottom);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBack);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxFront);

  // Get shader instances
  const ShaderProgramInstanceHandle litSurfaceShader = renderer->createShaderProgramInstance(ShaderProgramType::LitSurface);

  // Upload assets to GPU
  std::vector<RenderData> planetMeshes = renderer->addModel(modelPlanet.value(), litSurfaceShader);
  std::vector<RenderData> rockMeshes   = renderer->addModel(modelRock.value(), litSurfaceShader);

  TextureCubeMapHandle skyboxTexture = renderer->addTextureCubeMap(
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
    .cubeMesh = renderer->addMesh(skyboxCubeMesh),
    .texture = skyboxTexture,
    .shader = renderer->createShaderProgramInstance(ShaderProgramType::Skybox),
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

  const entt::entity entityPlanet = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(entityPlanet, "Planet Mars");
  mScene.ecs.emplace<CompTransform>(entityPlanet);
  mScene.ecs.emplace<CompGraphics>(entityPlanet, planetMeshes);

  const entt::entity entityPlanet2 = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(entityPlanet2, "Planet Mars 2");
  mScene.ecs.emplace<CompTransform>(entityPlanet2, CompTransform {
    .translation = glm::vec3(0.0f, -7.0f, 0.0f),
  });
  mScene.ecs.emplace<CompGraphics>(entityPlanet2, planetMeshes);

  std::random_device rd;
  std::mt19937 gen(rd());
  constexpr float offset = 25.0f;
  std::uniform_real_distribution displacementDistribution(-offset, offset);
  std::uniform_real_distribution rotationAngleDistribution(0.0f, 360.0f);
  std::uniform_real_distribution scaleDistribution(0.05f, 0.25f);
  constexpr uint32_t numAsteroids = 5000;
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
      .rotation = rotationAngleDistribution(gen) * Rotation(0.4f, 0.6f, 0.8f),
      .scale = glm::vec3(scaleDistribution(gen)),
    });
    mScene.ecs.emplace<CompGraphics>(entityAsteroid, rockMeshes);
  }

  return FlyCamDemoBase::init(assets, renderer);
}

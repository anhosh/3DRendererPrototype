#include <SpaceDemo.hpp>

#include <Components/Orbit.hpp>

#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Name.hpp>
#include <Util/Macros/Errors.hpp>

#include <glm/gtx/optimum_pow.hpp>

#include <imgui.h>

#include <algorithm>
#include <execution>
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
  mMainCamera = mScene.ecs.create();
  mScene.ecs.emplace<CompCamera>(mMainCamera, CompCamera {
    .clipBox.min.z = 0.001f,
    .clipBox.max.z = 200.0f,
  });

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

  mEnttMars = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(mEnttMars, "Planet Mars");
  mScene.ecs.emplace<CompTransform>(mEnttMars, CompTransform { .rotation.pitch = 90.0f });
  mScene.ecs.emplace<CompGraphics>(mEnttMars, planetMeshes);

  // Generate asteroids
  std::random_device rd;
  std::mt19937 gen(rd());

  constexpr float offset = 25.0f;
  std::uniform_real_distribution displacementDistribution(-offset, offset);
  std::uniform_real_distribution rotationAngleDistribution(0.0f, 360.0f);
  std::uniform_real_distribution scaleDistribution(0.05f, 0.25f);
  std::uniform_real_distribution orbitSpeedDistribution(40.0f, 80.0f);

  constexpr uint32_t numAsteroids = 3000;
  mAsteroidAngles.reserve(numAsteroids);
  for (uint32_t i = 0; i < numAsteroids; ++i) {
    const entt::entity enttAsteroid = mScene.ecs.create();
    mScene.ecs.emplace<CompName>(enttAsteroid, std::format("Asteroid {}", i));
    mScene.ecs.emplace<CompGraphics>(enttAsteroid, rockMeshes);

    constexpr float radius = 50.0f;
    const float angle = static_cast<float>(i) / static_cast<float>(numAsteroids) * 360.0f;
    const float distanceFromCentre = radius + displacementDistribution(gen);

    mScene.ecs.emplace<CompOrbit>(enttAsteroid, glm::vec3(0.0f), distanceFromCentre, 100.0f * orbitSpeedDistribution(gen) / glm::pow2(distanceFromCentre));
    mScene.ecs.emplace<CompTransform>(enttAsteroid, CompTransform {
      .translation = {
        glm::sin(glm::radians(angle)) * distanceFromCentre,
        0.4f * displacementDistribution(gen),
        glm::cos(glm::radians(angle)) * distanceFromCentre,
      },
      .rotation = rotationAngleDistribution(gen) * Rotation(0.4f, 0.6f, 0.8f),
      .scale = glm::vec3(scaleDistribution(gen)),
    });

    mAsteroidAngles.push_back(std::make_pair(enttAsteroid, angle));
  }

  return FlyCamDemoBase::init(assets, renderer);
}

void SpaceDemo::update(const double deltaTime) {
  FlyCamDemoBase::update(deltaTime);

  std::for_each(std::execution::par_unseq, mAsteroidAngles.begin(), mAsteroidAngles.end(),
    [=, this](std::pair<entt::entity, float>& asteroidAngle) {
      const CompOrbit& orbit = mScene.ecs.get<CompOrbit>(asteroidAngle.first);
      asteroidAngle.second += orbit.angularVelocity * static_cast<float>(deltaTime);
      mScene.ecs.patch<CompTransform>(asteroidAngle.first, [&](CompTransform& transform) {
        transform.translation.x = glm::sin(glm::radians(asteroidAngle.second)) * orbit.radius;
        transform.translation.z = glm::cos(glm::radians(asteroidAngle.second)) * orbit.radius;
      });
    });

  mScene.ecs.patch<CompTransform>(mEnttMars, [=, this](CompTransform& transform) {
    transform.rotation.yaw = glm::mod(transform.rotation.yaw + mMarsRotationSpeed * static_cast<float>(deltaTime), 360.0f);
  });
}

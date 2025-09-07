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
  const Expected modelPlanet = assets->loadModel("sphere/sphere.obj");
  const Expected modelRock = assets->loadModel("rock/rock.obj");

  Expected bitmapSkyboxRight  = assets->loadBitmap("skybox/px.png", false);
  Expected bitmapSkyboxLeft   = assets->loadBitmap("skybox/nx.png", false);
  Expected bitmapSkyboxTop    = assets->loadBitmap("skybox/py.png", false);
  Expected bitmapSkyboxBottom = assets->loadBitmap("skybox/ny.png", false);
  Expected bitmapSkyboxFront  = assets->loadBitmap("skybox/pz.png", false);
  Expected bitmapSkyboxBack   = assets->loadBitmap("skybox/nz.png", false);

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
      bitmapSkyboxFront.value(),
      bitmapSkyboxBack.value(),
      .bSRGB = true,
    }
  );

  // Create scene
  mMainCamera = mScene.ecs.create();
  mScene.ecs.emplace<CompCamera>(mMainCamera, CompCamera {
    .clipBox.min.z = 0.01f,
    .clipBox.max.z = 400.0f,
  });

  mScene.skybox = Skybox {
    .cubeMesh = renderer->addMesh(skyboxCubeMesh),
    .texture = skyboxTexture,
    .shader = renderer->createShaderProgramInstance(ShaderProgramType::Skybox),
  };

  mEnttSun = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(mEnttSun, "Sun");
  mScene.ecs.emplace<CompTransform>(mEnttSun, CompTransform { .rotation.pitch = 90.0f, .scale = glm::vec3(0.01f) });
  mScene.ecs.emplace<CompGraphics>(mEnttSun, planetMeshes);
  mScene.ecs.emplace<CompPointLight>(mEnttSun, CompPointLight {
    .colors.ambient = glm::vec3(0.01f),
    .colors.diffuse = glm::vec3(1000.0f),
    .colors.specular = glm::vec3(1500.0f),
    .constant = 1.0f,
    .linear = 6.35f,
    .quadratic = 0.01f,
  });

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

  mScene.ecs.patch<CompTransform>(mEnttSun, [=, this](CompTransform& transform) {
    transform.rotation.yaw = glm::mod(transform.rotation.yaw + mMarsRotationSpeed * static_cast<float>(deltaTime), 360.0f);
  });
}

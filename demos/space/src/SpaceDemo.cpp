#include <SpaceDemo.hpp>

#include <Components/Orbit.hpp>

#include <Assets/MeshData.hpp>
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

  Expected bitmapSkyboxRight   = assets->loadBitmap("skybox/px.png", false);
  Expected bitmapSkyboxLeft    = assets->loadBitmap("skybox/nx.png", false);
  Expected bitmapSkyboxTop     = assets->loadBitmap("skybox/py.png", false);
  Expected bitmapSkyboxBottom  = assets->loadBitmap("skybox/ny.png", false);
  Expected bitmapSkyboxFront   = assets->loadBitmap("skybox/pz.png", false);
  Expected bitmapSkyboxBack    = assets->loadBitmap("skybox/nz.png", false);
  Expected bitmapMercury       = assets->loadBitmap("spheres/2k_mercury.jpg", false);
  Expected bitmapVenus         = assets->loadBitmap("spheres/2k_venus_atmosphere.jpg", false);
  Expected bitmapEarthClouds   = assets->loadBitmap("spheres/2k_earth_clouds.png", false);
  Expected bitmapEarthDayMap   = assets->loadBitmap("spheres/2k_earth_daymap.jpg", false);
  Expected bitmapEarthSpecular = assets->loadBitmap("spheres/2k_earth_specular_map_clouds.png", false);
  Expected bitmapMoon          = assets->loadBitmap("spheres/2k_moon.jpg", false);
  Expected bitmapMars          = assets->loadBitmap("spheres/2k_mars.jpg", false);

  const AssetHandle<MeshData> skyboxCubeMesh = assets->addMesh(MeshData::createCube(glm::vec3(2.0f)));

  RETURN_ERROR_IF_UNEXPECTED(modelPlanet);
  RETURN_ERROR_IF_UNEXPECTED(modelRock);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxRight);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxLeft);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxTop);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBottom);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBack);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxFront);
  RETURN_ERROR_IF_UNEXPECTED(bitmapMercury);
  RETURN_ERROR_IF_UNEXPECTED(bitmapVenus);
  RETURN_ERROR_IF_UNEXPECTED(bitmapEarthClouds);
  RETURN_ERROR_IF_UNEXPECTED(bitmapEarthDayMap);
  RETURN_ERROR_IF_UNEXPECTED(bitmapEarthSpecular);
  RETURN_ERROR_IF_UNEXPECTED(bitmapMoon);
  RETURN_ERROR_IF_UNEXPECTED(bitmapMars);

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

  Texture2DHandle textureMercury = renderer->addTexture2D(bitmapMercury.value());
  Texture2DHandle textureVenus = renderer->addTexture2D(bitmapVenus.value());
  Texture2DHandle textureEarthClouds = renderer->addTexture2D(bitmapEarthClouds.value());
  Texture2DHandle textureEarthDayMap = renderer->addTexture2D(bitmapEarthDayMap.value());
  Texture2DHandle textureEarthSpecular = renderer->addTexture2D(bitmapEarthSpecular.value());
  Texture2DHandle textureMoon = renderer->addTexture2D(bitmapMoon.value());
  Texture2DHandle textureMars = renderer->addTexture2D(bitmapMars.value());

  // Create scene
  mMainCamera = mScene.ecs.create();
  mScene.ecs.emplace<CompCamera>(mMainCamera, CompCamera {
    .clipBox.min.z = 0.01f,
    .clipBox.max.z = 400.0f,
  });
  mScene.ecs.emplace<CompTransform>(mMainCamera, CompTransform {
    .translation = glm::vec3(70.0f, 75.0f, -58.0f),
    .rotation.pitch = 38.0f,
    .rotation.yaw = -42.0f,
  });
  mCameraSpeed = 30.0f;

  mScene.skybox = Skybox {
    .cubeMesh = renderer->addMesh(skyboxCubeMesh),
    .texture = skyboxTexture,
    .shader = renderer->createShaderProgramInstance(ShaderProgramType::Skybox),
  };

  mEnttSun = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(mEnttSun, "Sun");
  mScene.ecs.emplace<CompTransform>(mEnttSun, CompTransform { .rotation.pitch = 90.0f, .scale = glm::vec3(0.02f) });
  planetMeshes[0].emissionMap = planetMeshes[0].diffuseMap;
  mScene.ecs.emplace<CompGraphics>(mEnttSun, planetMeshes);
  mScene.ecs.emplace<CompPointLight>(mEnttSun, CompPointLight {
    .colors.ambient = glm::vec3(0.14f),
    .colors.diffuse = glm::vec3(50.0f),
    .colors.specular = glm::vec3(75.0f),
    .constant = 1.0f,
    .linear = 1.01f,
    .quadratic = 0.01f,
  });

  // Generate planets and asteroids
  std::random_device rd;
  std::mt19937 gen(rd());

  constexpr float offset = 25.0f;
  std::uniform_real_distribution displacementDistribution(-offset, offset);
  std::uniform_real_distribution rotationAngleDistribution(0.0f, 360.0f);
  std::uniform_real_distribution scaleDistribution(0.05f, 0.25f);
  std::uniform_real_distribution orbitSpeedDistribution(40.0f, 80.0f);

  constexpr uint32_t numAsteroids = 8000;
  mCelestialBodyAngles.reserve(numAsteroids + 5);

  const entt::entity enttMercury = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttMercury, "Mercury");
  planetMeshes[0].diffuseMap = textureMercury;
  planetMeshes[0].emissionMap = Texture2DHandle::null();
  mScene.ecs.emplace<CompGraphics>(enttMercury, planetMeshes);
  mScene.ecs.emplace<CompOrbit>(enttMercury, glm::vec3(0.0f), 20.0f, 4000.0f / 200.0f);
  mScene.ecs.emplace<CompTransform>(enttMercury, CompTransform { .rotation.pitch = -90.0f, .scale = glm::vec3(0.001f) });
  mCelestialBodyAngles.emplace_back(enttMercury, 0.0f);

  const entt::entity enttVenus = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttVenus, "Venus");
  planetMeshes[0].diffuseMap = textureVenus;
  mScene.ecs.emplace<CompGraphics>(enttVenus, planetMeshes);
  mScene.ecs.emplace<CompOrbit>(enttVenus, glm::vec3(0.0f), 40.0f, 4000.0f / 160.0f);
  mScene.ecs.emplace<CompTransform>(enttVenus, CompTransform { .rotation.pitch = -90.0f, .scale = glm::vec3(0.002f) });
  mCelestialBodyAngles.emplace_back(enttVenus, 0.0f);

  mEnttEarth = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(mEnttEarth, "Earth");
  planetMeshes[0].diffuseMap = textureEarthDayMap;
  planetMeshes[0].diffuseOverlayMap = textureEarthClouds;
  planetMeshes[0].specularMap = textureEarthSpecular;
  mScene.ecs.emplace<CompGraphics>(mEnttEarth, planetMeshes);
  mScene.ecs.emplace<CompOrbit>(mEnttEarth, glm::vec3(0.0f), 60.0f, 4000.0f / 360.0f);
  const CompTransform& earthTransform = mScene.ecs.emplace<CompTransform>(mEnttEarth, CompTransform { .rotation.pitch = -90.0f, .scale = glm::vec3(0.004f) });
  mCelestialBodyAngles.emplace_back(mEnttEarth, 0.0f);

  mEnttMoon = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(mEnttMoon, "Moon");
  planetMeshes[0].diffuseMap = textureMoon;
  planetMeshes[0].diffuseOverlayMap = Texture2DHandle::null();
  planetMeshes[0].specularMap = Texture2DHandle::null();
  mScene.ecs.emplace<CompGraphics>(mEnttMoon, planetMeshes);
  mScene.ecs.emplace<CompOrbit>(mEnttMoon, earthTransform.translation, 4.0f, 4000.0f / 360.0f * 12.0f);
  mScene.ecs.emplace<CompTransform>(mEnttMoon, CompTransform {
    .rotation.pitch = -90.0f,
    .rotation.yaw = 180.0f,
    .scale = glm::vec3(0.0005f),
  });
  mCelestialBodyAngles.emplace_back(mEnttMoon, 0.0f);

  const entt::entity enttMars = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttMars, "Mars");
  planetMeshes[0].diffuseMap = textureMars;
  mScene.ecs.emplace<CompGraphics>(enttMars, planetMeshes);
  mScene.ecs.emplace<CompOrbit>(enttMars, glm::vec3(0.0f), 80.0f, 4000.0f / 640.0f);
  mScene.ecs.emplace<CompTransform>(enttMars, CompTransform { .rotation.pitch = -90.0f, .scale = glm::vec3(0.003f) });
  mCelestialBodyAngles.emplace_back(enttMars, 0.0f);

  for (uint32_t i = 0; i < numAsteroids; ++i) {
    const entt::entity enttAsteroid = mScene.ecs.create();
    mScene.ecs.emplace<CompName>(enttAsteroid, std::format("Asteroid {}", i));
    mScene.ecs.emplace<CompGraphics>(enttAsteroid, rockMeshes);

    constexpr float radius = 120.0f;
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

    mCelestialBodyAngles.emplace_back(enttAsteroid, angle);
  }

  mScene.ecs.on_update<CompTransform>().connect<&SpaceDemo::onTransformUpdate>(this);

  return FlyCamDemoBase::init(assets, renderer);
}

void SpaceDemo::update(const double deltaTime) {
  ZoneScopedN("SpaceDemo::update");

  FlyCamDemoBase::update(deltaTime);

  std::for_each(std::execution::par_unseq, mCelestialBodyAngles.begin(), mCelestialBodyAngles.end(),
    [=, this](std::pair<entt::entity, float>& celestialBodyAngle) {
      auto& [enttBody, angle] = celestialBodyAngle;
      const CompOrbit& orbit = mScene.ecs.get<CompOrbit>(enttBody);
      const float deltaAngle = orbit.angularVelocity * mOrbitSpeedMultiplier * static_cast<float>(deltaTime);
      angle = glm::mod(angle + deltaAngle, 360.0f);
      const float angleRadians = glm::radians(angle);
      mScene.ecs.patch<CompTransform>(enttBody, [&](CompTransform& transform) {
        transform.translation.x = glm::sin(angleRadians) * orbit.radius + orbit.center.x;
        transform.translation.z = glm::cos(angleRadians) * orbit.radius + orbit.center.z;
        transform.rotation.yaw += deltaAngle * (enttBody == mEnttMoon ? 1.0f : -10.0f);
        transform.rotation.yaw = glm::mod(transform.rotation.yaw, 360.0f);
      });
    });

  mScene.ecs.patch<CompTransform>(mEnttSun, [=, this](CompTransform& transform) {
    transform.rotation.yaw += mSunRotationSpeed * static_cast<float>(deltaTime);
    transform.rotation.yaw = glm::mod(transform.rotation.yaw, 360.0f);
  });
}

void SpaceDemo::gui(AppState& state) {
  FlyCamDemoBase::gui(state);

  if (ImGui::CollapsingHeader("Space demo")) {
    ImGui::DragFloat("Orbit speed multiplier", &mOrbitSpeedMultiplier, 0.01f, 0.0f, 5.0f);
  }
}

void SpaceDemo::onTransformUpdate(entt::registry&, const entt::entity entity) {
  if (entity == mEnttEarth) {
    const CompTransform& earthTransform = mScene.ecs.get<const CompTransform>(mEnttEarth);
    mScene.ecs.patch<CompOrbit>(mEnttMoon, [&](CompOrbit& orbit) {
      orbit.center = earthTransform.translation;
    });
  }
}

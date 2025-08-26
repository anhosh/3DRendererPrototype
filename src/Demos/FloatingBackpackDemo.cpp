#include <Demos/FloatingBackpackDemo.hpp>

#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Name.hpp>
#include <Util/Macros/Errors.hpp>

Expected<void> FloatingBackpackDemo::init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) {
  ZoneScoped;

  // Load assets
  const Expected modelBackpack = assets->loadModel("backpack/backpack.obj");

  Expected bitmapSkyboxRight  = assets->loadBitmap("skybox/sea_afternoon/right.jpg", false);
  Expected bitmapSkyboxLeft   = assets->loadBitmap("skybox/sea_afternoon/left.jpg", false);
  Expected bitmapSkyboxTop    = assets->loadBitmap("skybox/sea_afternoon/top.jpg", false);
  Expected bitmapSkyboxBottom = assets->loadBitmap("skybox/sea_afternoon/bottom.jpg", false);
  Expected bitmapSkyboxBack   = assets->loadBitmap("skybox/sea_afternoon/back.jpg", false);
  Expected bitmapSkyboxFront  = assets->loadBitmap("skybox/sea_afternoon/front.jpg", false);

  AssetHandle<Bitmap> floorBitmap = assets->addBitmap(Bitmap::fromMemory(asBytes("\xFF\xFF\xFF"), glm::uvec2(1), 3).value());

  RETURN_ERROR_IF_UNEXPECTED(modelBackpack);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxRight);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxLeft);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxTop);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBottom);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBack);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxFront);

  const AssetHandle<MeshData> skyboxCubeMesh = assets->addMesh(MeshData::createCube(glm::vec3(2.0f)));
  const AssetHandle<MeshData> floorMeshData = assets->addMesh(MeshData::createQuad(glm::vec2(15.0f)));

  // Get shader instances
  const ShaderProgramInstanceHandle litSurfaceShader = renderer->createShaderProgramInstance(ShaderProgramType::LitSurface);

  // Upload assets to GPU
  std::vector<RenderData> backpackMeshes = renderer->addModel(modelBackpack.value(), litSurfaceShader);

  MeshHandle floorMesh = renderer->addMesh(floorMeshData);
  Texture2DHandle floorTexture = renderer->addTexture2D(floorBitmap);
  RenderData floorRenderData = {
    .mesh = floorMesh,
    .shader = litSurfaceShader,
    .diffuseMap = floorTexture,
    .renderOptions = { .bBackfaceCulling = false, },
  };

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
  mScene.ecs.emplace<CompCamera>(mMainCamera, CompCamera { .far = 50.0f });

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
    .direction = glm::vec3(0.166f, -0.2f, 0.161f),
  });

  const entt::entity entityBackpack = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(entityBackpack, "Backpack");
  mScene.ecs.emplace<CompTransform>(entityBackpack);
  mScene.ecs.emplace<CompGraphics>(entityBackpack, std::move(backpackMeshes));

  const entt::entity entityFloor = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(entityFloor, "Floor");
  mScene.ecs.emplace<CompTransform>(entityFloor, CompTransform {
    .translation = glm::vec3(0.0f, -1.75f, 0.0f),
    .rotation = Rotation(90.0f, 0.0f, 0.0f),
  });
  mScene.ecs.emplace<CompGraphics>(entityFloor, std::vector { floorRenderData });

  return FlyCamDemoBase::init(assets, renderer);
}

#include <FloatingBackpackDemo.hpp>

#include <Assets/MeshData.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Name.hpp>
#include <Util/Macros/Errors.hpp>

#include <imgui.h>

Expected<void> FloatingBackpackDemo::init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) {
  ZoneScoped;

  // Load assets
  const Expected modelBackpack = assets->loadModel("backpack/backpack.obj");

  Expected bitmapSkyboxRight  = assets->loadBitmap("skybox/sea_afternoon/right.jpg", false);
  Expected bitmapSkyboxLeft   = assets->loadBitmap("skybox/sea_afternoon/left.jpg", false);
  Expected bitmapSkyboxTop    = assets->loadBitmap("skybox/sea_afternoon/top.jpg", false);
  Expected bitmapSkyboxBottom = assets->loadBitmap("skybox/sea_afternoon/bottom.jpg", false);
  Expected bitmapSkyboxFront  = assets->loadBitmap("skybox/sea_afternoon/front.jpg", false);
  Expected bitmapSkyboxBack   = assets->loadBitmap("skybox/sea_afternoon/back.jpg", false);

  AssetHandle<Bitmap> floorBitmap = assets->addBitmap(Bitmap::fromMemory(asBytes("\xFF\xFF\xFF"), glm::uvec2(1), 3).value());

  RETURN_ERROR_IF_UNEXPECTED(modelBackpack);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxRight);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxLeft);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxTop);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBottom);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxFront);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBack);

  const AssetHandle<MeshData> skyboxCubeMesh = assets->addMesh(MeshData::createCube(glm::vec3(2.0f)));
  const AssetHandle<MeshData> floorMeshData = assets->addMesh(MeshData::createCube(glm::vec3(15.0f, 0.05f, 15.0f)));

  // Get shader instances
  const ShaderProgramInstanceHandle litSurfaceShader = renderer->createShaderProgramInstance(ShaderProgramType::LitSurface);
  mLitSurfaceShader = litSurfaceShader;
  mLitSurfaceShader->setUniform("uDebugBiasMultiplier", 0.0f);

  // Upload assets to GPU
  std::vector<RenderData> backpackMeshes = renderer->addModel(modelBackpack.value(), litSurfaceShader);

  MeshHandle floorMesh = renderer->addMesh(floorMeshData);
  Texture2DHandle floorTexture = renderer->addTexture2D(floorBitmap);
  RenderData floorRenderData = {
    .mesh = floorMesh,
    .shader = litSurfaceShader,
    .diffuseMap = floorTexture,
    .renderOptions = { .bBackfaceCulling = false },
  };

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
  mScene.skybox = Skybox {
    .cubeMesh = renderer->addMesh(skyboxCubeMesh),
    .texture = skyboxTexture,
    .shader = renderer->createShaderProgramInstance(ShaderProgramType::Skybox),
  };

  const entt::entity enttSun = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttSun, "Sun");
  mScene.ecs.emplace<CompDirectionalLight>(enttSun, CompDirectionalLight {
    .colors =  LightColors {
      .ambient = glm::vec3(0.01f),
      .diffuse = glm::vec3(1.0f),
      .specular = glm::vec3(2.0f),
    },
    .direction = glm::vec3(0.166f, -0.2f, 0.161f),
  });

  mEnttFlashlight = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(mEnttFlashlight, "Flashlight");
  mScene.ecs.emplace<CompSpotlight>(mEnttFlashlight);
  mScene.ecs.emplace<CompTransform>(mEnttFlashlight);

  const entt::entity enttBackpack = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttBackpack, "Backpack");
  mScene.ecs.emplace<CompTransform>(enttBackpack);
  mScene.ecs.emplace<CompGraphics>(enttBackpack, backpackMeshes);

  const entt::entity enttBackpack2 = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttBackpack2, "Backpack 2");
  mScene.ecs.emplace<CompTransform>(enttBackpack2, CompTransform { .translation = { 0.0f, -5.0f, 0.0f} });
  mScene.ecs.emplace<CompGraphics>(enttBackpack2, std::move(backpackMeshes));

  const entt::entity enttFloor = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttFloor, "Floor");
  mScene.ecs.emplace<CompTransform>(enttFloor, CompTransform {
    .translation = glm::vec3(0.0f, -1.75f, 0.0f),
  });
  mScene.ecs.emplace<CompGraphics>(enttFloor, std::vector { floorRenderData });

  return FlyCamDemoBase::init(assets, renderer);
}

void FloatingBackpackDemo::update(const double deltaTime) {
  FlyCamDemoBase::update(deltaTime);

  if (mbFlashlightFollowsCamera) {
    mScene.ecs.patch<CompTransform>(mEnttFlashlight, [this](CompTransform& lightTransform) {
      lightTransform = mScene.ecs.get<CompTransform>(mMainCamera);
    });
  }
}

void FloatingBackpackDemo::gui(AppState& state) {
  FlyCamDemoBase::gui(state);

  if (ImGui::CollapsingHeader("Floating Backpack Demo")) {
    ImGui::Checkbox("Flashlight follows camera", &mbFlashlightFollowsCamera);
    if (ImGui::DragFloat("[Debug] Flashlight Z Offset", &mScene.ecs.get<CompSpotlight>(mEnttFlashlight).debugZOffset, 0.01f)) {
      mScene.ecs.patch<CompSpotlight>(mEnttFlashlight);
    }
    ImGui::DragFloat("[Debug] uDebugBiasMultiplier", mLitSurfaceShader->uniforms["uDebugBiasMultiplier"].getPtr<GLfloat>(), 0.001f);
  }
}

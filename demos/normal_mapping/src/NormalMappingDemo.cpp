#include <NormalMappingDemo.hpp>

#include <Assets/MeshData.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Scene/Components/Name.hpp>
#include <Util/Macros/Errors.hpp>

#include <imgui.h>

Expected<void> NormalMappingDemo::init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<GraphicsOpenGL::RenderingEngine>& renderer) {
  ZoneScoped;

  // Load assets
  const Expected modelBackpack = assets->loadModel("backpack/backpack.obj");

  Expected bitmapBrickwall       = assets->loadBitmap("brickwall/brickwall.jpg", true, false);
  Expected bitmapBrickwallNormal = assets->loadBitmap("brickwall/brickwall_normal.jpg", false, false);
  Expected bitmapSkyboxRight     = assets->loadBitmap("skybox/right.jpg", true, false);
  Expected bitmapSkyboxLeft      = assets->loadBitmap("skybox/left.jpg", true, false);
  Expected bitmapSkyboxTop       = assets->loadBitmap("skybox/top.jpg", true, false);
  Expected bitmapSkyboxBottom    = assets->loadBitmap("skybox/bottom.jpg", true, false);
  Expected bitmapSkyboxFront     = assets->loadBitmap("skybox/front.jpg", true, false);
  Expected bitmapSkyboxBack      = assets->loadBitmap("skybox/back.jpg", true, false);

  AssetHandle<Bitmap> floorBitmap = assets->addBitmap(Bitmap::fromMemory(asBytes("\xFF\xFF\xFF"), glm::uvec2(1), 3).value());

  RETURN_ERROR_IF_UNEXPECTED(modelBackpack);
  RETURN_ERROR_IF_UNEXPECTED(bitmapBrickwall);
  RETURN_ERROR_IF_UNEXPECTED(bitmapBrickwallNormal);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxRight);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxLeft);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxTop);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBottom);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxFront);
  RETURN_ERROR_IF_UNEXPECTED(bitmapSkyboxBack);

  bitmapBrickwall.value()->bSRGB = true;

  const AssetHandle<MeshData> skyboxCubeMesh = assets->addMesh(MeshData::createCube(glm::vec3(2.0f)));
  const AssetHandle<MeshData> floorMeshData = assets->addMesh(MeshData::createCube(glm::vec3(30.0f, 0.05f, 30.0f)));
  const AssetHandle<MeshData> wallMeshData = assets->addMesh(MeshData::createCube(glm::vec3(5.0f, 5.0f, 5.0f)));

  // Get shader instances
  const GraphicsOpenGL::ShaderProgramInstanceHandle litSurfaceShader = renderer->createShaderProgramInstance(GraphicsOpenGL::ShaderProgramType::LitSurface);

  // Upload assets to GPU
  std::vector<GraphicsOpenGL::RenderData> backpackMeshes = renderer->addModel(modelBackpack.value(), litSurfaceShader);

  GraphicsOpenGL::RenderData floorRenderData = {
    .mesh = renderer->addMesh(floorMeshData),
    .shader = litSurfaceShader,
    .diffuseMap = renderer->addTexture2D(floorBitmap),
  };

  GraphicsOpenGL::RenderData wallRenderData = {
    .mesh = renderer->addMesh(wallMeshData),
    .shader = litSurfaceShader,
    .diffuseMap = renderer->addTexture2D(bitmapBrickwall.value()),
    .normalMap = renderer->addTexture2D(bitmapBrickwallNormal.value()),
  };

  // Create scene
  mScene.skybox = GraphicsOpenGL::Skybox {
    .cubeMesh = renderer->addMesh(skyboxCubeMesh),
    .texture = renderer->addTextureCubeMap(
      GraphicsOpenGL::TextureCubeMapBitmaps {
        .right = bitmapSkyboxRight.value(),
        .left = bitmapSkyboxLeft.value(),
        .top = bitmapSkyboxTop.value(),
        .bottom = bitmapSkyboxBottom.value(),
        .front = bitmapSkyboxFront.value(),
        .back = bitmapSkyboxBack.value(),
        .bSRGB = true,
      }
    ),
    .shader = renderer->createShaderProgramInstance(GraphicsOpenGL::ShaderProgramType::Skybox),
  };

  const entt::entity enttSun = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttSun, "Sun");
  mScene.ecs.emplace<CompDirectionalLight>(enttSun, CompDirectionalLight {
    .colors =  LightColors {
      .ambient = glm::vec3(0.01f),
      .diffuse = glm::vec3(1.0f),
      .specular = glm::vec3(2.0f),
    },
    .direction = glm::vec3(-0.345f, -0.674f, -0.653f),
  });

  const entt::entity enttFloor = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttFloor, "Floor");
  mScene.ecs.emplace<CompTransform>(enttFloor, CompTransform {
    .translation = glm::vec3(0.0f, -1.75f, 0.0f),
  });
  mScene.ecs.emplace<CompGraphics>(enttFloor, std::vector { floorRenderData });

  const entt::entity enttBackpack = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttBackpack, "Backpack");
  mScene.ecs.emplace<CompTransform>(enttBackpack, CompTransform {
    .translation = glm::vec3(3.0f, 1.0f, 0.0f),
  });
  mScene.ecs.emplace<CompGraphics>(enttBackpack, backpackMeshes);

  const entt::entity enttWall = mScene.ecs.create();
  mScene.ecs.emplace<CompName>(enttWall, "Wall");
  mScene.ecs.emplace<CompTransform>(enttWall, CompTransform {
    .translation = glm::vec3(-3.0f, 2.0f, 0.0f),
  });
  mScene.ecs.emplace<CompGraphics>(enttWall, std::vector { wallRenderData });

  return FlyCamDemoBase::init(assets, renderer);
}

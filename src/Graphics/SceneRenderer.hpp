#pragma once

#include <Assets/AssetManager.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Mesh.hpp>
#include <Graphics/Model.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/Shaders.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/VertexArray.hpp>

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class SceneRenderer {
public:
  template <typename Resource>
  class Handle {
    friend class SceneRenderer;

    Handle(size_t index, SceneRenderer* assetManager)
      : index(index)
      , renderer(assetManager)
    {}

  public:
    Handle(const Handle& other) = default;
    Handle& operator=(const Handle& other) = default;

    Resource& get();

    size_t index = SIZE_MAX;

  private:
    NotNull<SceneRenderer> renderer;
  };

  struct RenderOptions {
    bool bBackfaceCulling = true;
    bool bTransparent = false;
    std::optional<Handle<ShaderProgramInstance>> outlineShaderInstance = std::nullopt;
  };

  struct RenderData {
    Handle<VertexArray> vertexArray;
    Handle<ShaderProgramInstance> shaderProgramInstance;
    std::optional<Handle<Texture>> diffuseMap = std::nullopt;
    std::optional<Handle<Texture>> specularMap = std::nullopt;
    std::optional<Handle<Texture>> emissionMap = std::nullopt;
    RenderOptions renderOptions = {};
  };

  Expected<void> loadShaders();

  const std::vector<RenderData>& addModel(AssetManager::Handle<Model> model,
                                          Handle<ShaderProgramInstance> initialShaderProgramInstance);

  Handle<VertexArray> addMesh(AssetManager::Handle<Mesh> mesh);
  Handle<ShaderProgramInstance> createShaderProgramInstance(ShaderProgramType type);
  Handle<Texture> addTexture(AssetManager::Handle<Bitmap> bitmap, const SamplerOptions& options = {});

  std::vector<Handle<VertexArray>> addMeshes(std::span<const AssetManager::Handle<Mesh>> meshes);
  std::vector<std::optional<Handle<Texture>>> addTextures(std::span<const std::optional<AssetManager::Handle<Bitmap>>> bitmaps,
                                                          std::span<const SamplerOptions> options = {});

  void render(std::span<const Draw> draws, TransformMatrices transforms);

private:
  std::vector<VertexArray> mVertexArrays;
  std::vector<ShaderProgramInstance> mShaderProgramInstances;
  std::vector<Texture> mTextures;

  std::unique_ptr<ShaderProgram> mLitSurfaceShaderProgram;
  std::unique_ptr<ShaderProgram> mLightShaderProgram;
  std::unique_ptr<ShaderProgram> mOutlineShaderProgram;
  std::unique_ptr<ShaderProgram> mVisualiseDepthShaderProgram;
  std::unique_ptr<ShaderProgram> mVisualiseNormalShaderProgram;

  std::unordered_map<size_t, Handle<Texture>> mUploadedBitmaps;
  std::unordered_map<size_t, std::vector<RenderData>> mUploadedModels;

  std::unordered_set<GLuint> mBoundTextureSlots;
};

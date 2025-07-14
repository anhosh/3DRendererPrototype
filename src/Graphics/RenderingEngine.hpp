#pragma once

#include <Assets/AssetManager.hpp>
#include <Assets/Mesh.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/VertexArray.hpp>
#include <Util/Macros/Classes.hpp>

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define USE_SCREEN_QUAD_MESH 1

class Camera;
class Scene;

class RenderingEngine {
public:
  DECLARE_ITEM_HANDLE(RenderingEngine)

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

  Expected<void> init();

  Handle<ShaderProgramInstance> createShaderProgramInstance(ShaderProgramType type);
  Handle<ShaderProgramInstance> addShaderProgramInstance(ShaderProgramInstance instance);

  const std::vector<RenderData>& addModel(AssetHandle<Model> model, Handle<ShaderProgramInstance> initialShaderProgramInstance);
  Handle<VertexArray> addMesh(AssetHandle<Mesh> mesh);
  Handle<Texture> addTexture(AssetHandle<Bitmap> bitmap, const SamplerOptions& options = {});

  std::vector<Handle<VertexArray>> addMeshes(std::span<const AssetHandle<Mesh>> meshes);
  std::vector<std::optional<Handle<Texture>>> addTextures(std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps,
                                                          std::span<const SamplerOptions> options = {});

  void renderScene(const Scene& scene, const Camera& camera, glm::uvec2 windowSize);
  void postProcess();
  void present(glm::uvec2 windowSize);

private:
  std::unique_ptr<ShaderProgram> mLitSurfaceShaderProgram;
  std::unique_ptr<ShaderProgram> mLightShaderProgram;
  std::unique_ptr<ShaderProgram> mOutlineShaderProgram;
  std::unique_ptr<ShaderProgram> mVisualiseDepthShaderProgram;
  std::unique_ptr<ShaderProgram> mVisualiseNormalShaderProgram;
  std::unique_ptr<ShaderProgram> mPostProcessCopyShaderProgram;
  std::unique_ptr<ShaderProgram> mPostProcessInvertShaderProgram;

  std::vector<ShaderProgramInstance> mShaderProgramInstances;
  std::vector<VertexArray> mVertexArrays;
  std::vector<Texture> mTextures;

#if USE_SCREEN_QUAD_MESH
  VertexArray mScreenQuadVA;
#endif
  std::vector<Framebuffer> mFramebuffers;
  glm::uvec2 mLastFramebufferSize = glm::uvec2(0);
  size_t mLastFramebufferIndex = 0;

  std::unordered_map<size_t, Handle<Texture>> mUploadedBitmaps;
  std::unordered_map<size_t, std::vector<RenderData>> mUploadedModels;

  std::unordered_set<GLuint> mBoundTextureSlots;
};

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

  ~RenderingEngine() { this->destroy(); }

  Expected<void> init();
  void destroy();

  Handle<ShaderProgramInstance> createShaderProgramInstance(ShaderProgramType type);
  Handle<ShaderProgramInstance> addShaderProgramInstance(ShaderProgramInstance instance);

  const std::vector<RenderData>& addModel(AssetHandle<Model> model, Handle<ShaderProgramInstance> initialShaderProgramInstance);
  Handle<VertexArray> addMesh(AssetHandle<Mesh> mesh);
  Handle<Texture> addTexture(AssetHandle<Bitmap> bitmap, const SamplerOptions& options = {});

  std::vector<Handle<VertexArray>> addMeshes(std::span<const AssetHandle<Mesh>> meshes);
  std::vector<std::optional<Handle<Texture>>> addTextures(std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps,
                                                          std::span<const SamplerOptions> options = {});

  void renderScene(const Scene& scene, const Camera& camera, glm::uvec2 windowSize);
  void postProcess(std::span<Handle<ShaderProgramInstance>> postProcessingShaders);
  void present(glm::uvec2 windowSize);

private:
  std::unique_ptr<ShaderProgram> mLitSurfaceShaderProgram;
  std::unique_ptr<ShaderProgram> mLightShaderProgram;
  std::unique_ptr<ShaderProgram> mOutlineShaderProgram;
  std::unique_ptr<ShaderProgram> mVisualiseDepthShaderProgram;
  std::unique_ptr<ShaderProgram> mVisualiseNormalShaderProgram;
  std::unique_ptr<ShaderProgram> mPostProcessCopyShaderProgram;
  std::unique_ptr<ShaderProgram> mPostProcessGrayscaleShaderProgram;
  std::unique_ptr<ShaderProgram> mPostProcessInvertShaderProgram;

  std::unordered_map<size_t, ShaderProgramInstance> mShaderProgramInstances;
  std::unordered_map<size_t, VertexArray> mVertexArrays;
  std::unordered_map<size_t, Texture> mTextures;

  size_t mNextShaderProgramInstanceID = 0;
  size_t mNextVertexArrayID = 0;
  size_t mNextTextureID = 0;

  GLuint mScreenQuadVAO = GL_NONE;
  std::vector<Framebuffer> mFramebuffers;
  glm::uvec2 mLastFramebufferSize = glm::uvec2(0);
  size_t mLastFramebufferIndex = 0;

  std::unordered_map<size_t, Handle<Texture>> mUploadedTextures;
  std::unordered_map<size_t, std::vector<RenderData>> mUploadedModels;

  std::unordered_set<GLuint> mBoundTextureSlots;
};

using ShaderProgramInstanceHandle = RenderingEngine::Handle<ShaderProgramInstance>;
using VertexArrayHandle = RenderingEngine::Handle<VertexArray>;
using TextureHandle = RenderingEngine::Handle<Texture>;

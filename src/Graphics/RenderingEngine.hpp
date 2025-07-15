#pragma once

#include <Assets/AssetManager.hpp>
#include <Assets/Mesh.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/VertexArray.hpp>
#include <Util/Registry.hpp>

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Camera;
class Scene;

using ShaderProgramInstanceHandle = Registry<ShaderProgramInstance>::Handle;
using VertexArrayHandle = Registry<VertexArray>::Handle;
using TextureHandle = Registry<Texture>::Handle;

class RenderingEngine {
public:
  struct RenderOptions {
    bool bBackfaceCulling = true;
    bool bTransparent = false;
    std::optional<ShaderProgramInstanceHandle> outlineShaderInstance = std::nullopt;
  };

  struct RenderData {
    VertexArrayHandle vertexArray;
    ShaderProgramInstanceHandle shaderProgramInstance;
    std::optional<TextureHandle> diffuseMap = std::nullopt;
    std::optional<TextureHandle> specularMap = std::nullopt;
    std::optional<TextureHandle> emissionMap = std::nullopt;
    RenderOptions renderOptions = {};
  };

  ~RenderingEngine() { this->destroy(); }

  Expected<void> init();
  void destroy();

  Registry<ShaderProgramInstance>::Handle createShaderProgramInstance(ShaderProgramType type);
  Registry<ShaderProgramInstance>::Handle addShaderProgramInstance(ShaderProgramInstance instance);

  const std::vector<RenderData>& addModel(AssetHandle<Model> model, ShaderProgramInstanceHandle initialShaderProgramInstance);
  Registry<VertexArray>::Handle addMesh(AssetHandle<Mesh> mesh);
  TextureHandle addTexture(AssetHandle<Bitmap> bitmap, const SamplerOptions& options = {});

  std::vector<VertexArrayHandle> addMeshes(std::span<const AssetHandle<Mesh>> meshes);
  std::vector<std::optional<TextureHandle>> addTextures(std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps,
                                                        std::span<const SamplerOptions> options = {});

  void renderScene(const Scene& scene, const Camera& camera, glm::uvec2 windowSize);
  void postProcess(std::span<ShaderProgramInstanceHandle> postProcessingShaders);
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
  std::unique_ptr<ShaderProgram> mPostProcessKernel3x3ShaderProgram;

  Registry<ShaderProgramInstance> mShaderProgramInstances;
  Registry<VertexArray> mVertexArrays;
  Registry<Texture> mTextures;

  GLuint mScreenQuadVAO = GL_NONE;
  std::vector<Framebuffer> mFramebuffers;
  glm::uvec2 mLastFramebufferSize = glm::uvec2(0);
  size_t mLastFramebufferIndex = 0;

  std::unordered_map<RegItemID, TextureHandle> mUploadedTextures;
  std::unordered_map<RegItemID, std::vector<RenderData>> mUploadedModels;

  std::unordered_set<GLuint> mBoundTextureSlots;
};

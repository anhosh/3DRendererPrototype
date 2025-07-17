#pragma once

#include <Assets/AssetManager.hpp>
#include <Assets/Mesh.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/RenderData.hpp>
#include <Graphics/RenderPass.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/VertexArray.hpp>
#include <Graphics/Viewport.hpp>
#include <Util/Registry.hpp>

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Camera;
class Scene;

class RenderingEngine {
public:
  ~RenderingEngine() { this->destroy(); }

  Expected<void> init();
  void destroy();

  ShaderProgramInstanceHandle createShaderProgramInstance(ShaderProgramType type);
  ShaderProgramInstanceHandle addShaderProgramInstance(ShaderProgramInstance&& instance);

  const std::vector<RenderData>& addModel(AssetHandle<Model> model, ShaderProgramInstanceHandle initialShaderProgramInstance);
  Registry<VertexArray>::Handle addMesh(AssetHandle<Mesh> mesh);
  TextureHandle addTexture(AssetHandle<Bitmap> bitmap, const SamplerOptions& options = {});
  FramebufferHandle addFramebuffer(const FramebufferCreateInfo& info);

  std::vector<VertexArrayHandle> addMeshes(std::span<const AssetHandle<Mesh>> meshes);
  std::vector<std::optional<TextureHandle>> addTextures(std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps,
                                                        std::span<const SamplerOptions> options = {});

  void submitRenderPasses(std::span<const RenderPass> renderPasses);
  void renderScene(const Scene& scene, const Camera& camera, const Viewport& viewport, FramebufferHandle dstFramebuffer);
  void postProcess(const Viewport& viewport, ShaderProgramInstanceHandle postProcessingShader,
                   FramebufferHandle srcFramebuffer, FramebufferHandle dstFramebuffer) const;
  void present(glm::uvec2 windowSize, FramebufferHandle srcFramebuffer) const;

private:
  std::unique_ptr<ShaderProgram> mLitSurfaceShaderProgram;
  std::unique_ptr<ShaderProgram> mLightShaderProgram;
  std::unique_ptr<ShaderProgram> mOutlineShaderProgram;
  std::unique_ptr<ShaderProgram> mVisualiseDepthShaderProgram;
  std::unique_ptr<ShaderProgram> mVisualiseNormalShaderProgram;
  std::unique_ptr<ShaderProgram> mPostProcessCopyShaderProgram;
  std::unique_ptr<ShaderProgram> mPostProcessFlipHorizontally;
  std::unique_ptr<ShaderProgram> mPostProcessFlipVertically;
  std::unique_ptr<ShaderProgram> mPostProcessGrayscaleShaderProgram;
  std::unique_ptr<ShaderProgram> mPostProcessInvertShaderProgram;
  std::unique_ptr<ShaderProgram> mPostProcessKernel3x3ShaderProgram;

  Registry<ShaderProgramInstance> mShaderProgramInstances;
  Registry<VertexArray> mVertexArrays;
  Registry<Texture> mTextures;

  GLuint mScreenQuadVAO = GL_NONE;
  Registry<Framebuffer> mFramebuffers;

  std::unordered_map<RegItemID, TextureHandle> mUploadedTextures;
  std::unordered_map<RegItemID, std::vector<RenderData>> mUploadedModels;

  std::unordered_set<GLuint> mBoundTextureSlots;
};

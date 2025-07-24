#pragma once

#include <Assets/AssetManager.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/RenderData.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture2D.hpp>
#include <Graphics/TextureCubeMap.hpp>
#include <Graphics/UniformBuffer.hpp>
#include <Graphics/VertexArray.hpp>
#include <Util/Registry.hpp>

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Camera;
class Scene;
struct RenderData;
struct RenderPass;
struct Viewport;

class RenderingEngine {
public:
  ~RenderingEngine() { this->destroy(); }

  Expected<void> init();
  void destroy();

  Expected<ShaderProgramHandle> createShaderProgram(const ShaderProgramPaths& shaderPaths);
  ShaderProgramHandle addShaderProgram(ShaderProgram&& shaderProgram);

  ShaderProgramInstanceHandle createShaderProgramInstance(ShaderProgramType type);
  ShaderProgramInstanceHandle addShaderProgramInstance(ShaderProgramInstance&& instance);

  const std::vector<RenderData>& addModel(AssetHandle<Model> model, ShaderProgramInstanceHandle initialShader);

  std::vector<VertexArrayHandle> addMeshes(std::span<const AssetHandle<Mesh>> meshes);
  VertexArrayHandle addMesh(AssetHandle<Mesh> mesh);

  std::vector<std::optional<Texture2DHandle>> addTexture2Ds(std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps,
                                                            std::span<const SamplerOptions> options = {});
  Texture2DHandle addTexture2D(AssetHandle<Bitmap> bitmap, const SamplerOptions& options = {});
  TextureCubeMapHandle addTextureCubeMap(const TextureCubeMapBitmaps& bitmaps, const SamplerOptions& options = {});

  FramebufferHandle addFramebuffer(const FramebufferCreateInfo& info);

  void submitRenderPasses(std::span<const RenderPass> renderPasses);
  void renderScene(const Scene& scene, const Camera& camera, const Viewport& viewport, FramebufferHandle dstFramebuffer);
  void postProcess(const Viewport& viewport, ShaderProgramInstanceHandle postProcessingShader,
                   FramebufferHandle srcFramebuffer, FramebufferHandle dstFramebuffer) const;
  void present(glm::uvec2 windowSize, FramebufferHandle srcFramebuffer) const;

private:
  bool bInitialised = false;

  Registry<ShaderProgram> mShaderPrograms;
  Registry<ShaderProgramInstance> mShaderProgramInstances;
  Registry<VertexArray> mVertexArrays;
  Registry<Texture2D> mTexture2Ds;
  Registry<TextureCubeMap> mTextureCubeMaps;
  Registry<Framebuffer> mFramebuffers;
  Registry<UniformBuffer> mUniformBuffers;

  GLuint mScreenQuadVAO = GL_NONE;

  std::optional<ShaderProgramHandle> mLitSurfaceShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mLightShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mOutlineShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mReflectiveSurfaceShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mRefractiveSurfaceShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mVisualiseDepthShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mVisualiseNormalShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessCopyShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessFlipHorizontallyShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessFlipVerticallyShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessGrayscaleShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessInvertShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessKernel3x3ShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mSkyboxShaderProgram = std::nullopt;

  std::optional<UniformBufferHandle> mCameraUniformBuffer = std::nullopt;

  std::unordered_map<RegItemID, Texture2DHandle> mUploadedTextures;
  std::unordered_map<RegItemID, std::vector<RenderData>> mUploadedModels;

  std::unordered_set<GLuint> mBoundTextureSlots;
};

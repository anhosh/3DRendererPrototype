#pragma once

#include <Assets/AssetManager.hpp>
#include <Graphics/Buffer.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/Mesh.hpp>
#include <Graphics/RenderData.hpp>
#include <Graphics/RenderPass.hpp>
#include <Graphics/Sampler.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture2D.hpp>
#include <Graphics/TextureCubeMap.hpp>
#include <Util/Registry.hpp>

#include <entt/entity/entity.hpp>

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct CompCamera;
struct InstanceBuffer;
struct RenderData;
struct RenderPass;
class Scene;
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

private:
  struct TexturePack {
    std::optional<AssetHandle<Bitmap>> diffuseMap = std::nullopt;
    std::optional<AssetHandle<Bitmap>> specularMap = std::nullopt;
    std::optional<AssetHandle<Bitmap>> emissionMap = std::nullopt;

    auto operator<=>(const TexturePack&) const = default;
  };

  static auto groupMeshesByTextures(AssetHandle<Model> model);
  // -> std::unordered_map<TexturePack, std::vector<NotNull<const MeshData>>, TexturePackHash>

  static MeshData mergeMeshes(std::span<const NotNull<const MeshData>> meshes);

public:
  const std::vector<RenderData>& addModel(AssetHandle<Model> model, ShaderProgramInstanceHandle initialShader);

  std::vector<MeshHandle> addMeshes(std::span<const AssetHandle<MeshData>> meshesData);
  MeshHandle addMesh(AssetHandle<MeshData> meshData);
  MeshHandle addMesh(const MeshData& meshData);

  SamplerHandle addSampler(const SamplerOptions& options);

  std::vector<std::optional<Texture2DHandle>> addTexture2Ds(std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps);
  Texture2DHandle addTexture2D(AssetHandle<Bitmap> bitmap);
  TextureCubeMapHandle addTextureCubeMap(const TextureCubeMapBitmaps& bitmaps);

  FramebufferHandle addFramebuffer(const FramebufferCreateInfo& info);

  BufferHandle createBuffer(GLenum type);

  void updateInstances(size_t first, const InstanceBuffer& instances);

  void submitRenderPasses(std::span<RenderPass> renderPasses);
  void renderSceneFull(RenderScenePass& pass, const Viewport& viewport, FramebufferHandle dstFramebuffer);
  void renderSceneSimple(RenderScenePass& pass, const Viewport& viewport, FramebufferHandle dstFramebuffer);
  void postProcess(const Viewport& viewport, ShaderProgramInstanceHandle postProcessingShader,
                   FramebufferHandle srcFramebuffer, FramebufferHandle dstFramebuffer) const;
  void present(glm::uvec2 windowSize, FramebufferHandle srcFramebuffer) const;

private:
  void updateStorageAndUniformBuffers(RenderScenePass& pass, glm::uvec2 framebufferSize);

private:
  bool mInitialised = false;

  Registry<ShaderProgram> mShaderPrograms;
  Registry<ShaderProgramInstance> mShaderProgramInstances;
  Registry<Mesh> mMeshes;
  Registry<Sampler> mSamplers;
  Registry<Texture2D> mTexture2Ds;
  Registry<TextureCubeMap> mTextureCubeMaps;
  Registry<Framebuffer> mFramebuffers;
  Registry<Buffer> mBuffers;

  GLuint mMeshesVAO = GL_NONE;
  GLuint mScreenQuadVAO = GL_NONE;

  std::optional<ShaderProgramHandle> mLitSurfaceShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mLitExplodedShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mLightShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mOutlineShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mReflectiveSurfaceShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mRefractiveSurfaceShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mSurfaceDepthShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mSurfaceNormalShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mVertexNormalShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessCopyShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessFlipHorizontallyShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessFlipVerticallyShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessGammaCorrectionShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessGrayscaleShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessInvertShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mPostProcessKernel3x3ShaderProgram = std::nullopt;
  std::optional<ShaderProgramHandle> mSkyboxShaderProgram = std::nullopt;

  std::optional<SamplerHandle> mColorAttachmentSampler = std::nullopt;
  std::optional<SamplerHandle> mDiffuseTextureSampler = std::nullopt;
  std::optional<SamplerHandle> mSpecularTextureSampler = std::nullopt;
  std::optional<SamplerHandle> mEmissionTextureSampler = std::nullopt;
  std::optional<SamplerHandle> mEnvironmentTextureSampler = std::nullopt;

  std::optional<BufferHandle> mCameraUniformBuffer = std::nullopt;
  std::optional<BufferHandle> mDirectionalLightsStorageBuffer = std::nullopt;
  std::optional<BufferHandle> mPointLightsStorageBuffer = std::nullopt;
  std::optional<BufferHandle> mSpotlightsStorageBuffer = std::nullopt;
  std::optional<BufferHandle> mInstanceBuffer = std::nullopt;

  std::unordered_map<RegItemID, Texture2DHandle> mUploadedTextures;
  std::unordered_map<RegItemID, std::vector<RenderData>> mUploadedModels;

  std::unordered_set<GLuint> mBoundTextureSlots;
};

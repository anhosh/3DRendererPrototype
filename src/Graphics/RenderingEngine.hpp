#pragma once

#include <Assets/AssetManager.hpp>
#include <Graphics/Buffers/Buffer.hpp>
#include <Graphics/Buffers/MultiBuffer.hpp>
#include <Graphics/Command.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/Mesh.hpp>
#include <Graphics/RenderData.hpp>
#include <Graphics/RenderPass.hpp>
#include <Graphics/Sampler.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Textures/Texture2D.hpp>
#include <Graphics/Textures/Texture2DArray.hpp>
#include <Graphics/Textures/TextureCubeMap.hpp>
#include <Util/Registry.hpp>

#include <unordered_map>
#include <vector>

struct InstanceBuffer;
struct RenderData;
struct RenderPass;
class Scene;
struct Viewport;

class RenderingEngine {
public:
  ~RenderingEngine() { this->destroy(); }

  Expected<void> init(AssetManager& assets);
  void destroy();

  Expected<ShaderProgramHandle> createShaderProgram(const ShaderProgramPaths& shaderPaths);
  ShaderProgramHandle addShaderProgram(ShaderProgram&& shaderProgram);

  ShaderProgramInstanceHandle createShaderProgramInstance(ShaderProgramType type);
  ShaderProgramInstanceHandle addShaderProgramInstance(ShaderProgramInstance&& instance);

  const std::vector<RenderData>& addModel(AssetHandle<Model> model, ShaderProgramInstanceHandle initialShader);

  std::vector<MeshHandle> addMeshes(std::span<const AssetHandle<MeshData>> meshesData);
  MeshHandle addMesh(AssetHandle<MeshData> meshData);
  MeshHandle addMesh(const MeshData& meshData);

  SamplerHandle addSampler(const SamplerOptions& options);

  Texture2DHandle createEmptyTexture2D();
  Texture2DArrayHandle createEmptyTexture2DArray();
  TextureCubeMapHandle createEmptyTextureCubeMap();

  std::vector<Texture2DHandle> addTexture2Ds(std::span<const AssetHandle<Bitmap>> bitmaps);
  Texture2DHandle addTexture2D(AssetHandle<Bitmap> bitmap);
  Texture2DArrayHandle addTexture2DArray(std::span<const AssetHandle<Bitmap>> bitmaps);
  TextureCubeMapHandle addTextureCubeMap(const TextureCubeMapBitmaps& bitmaps);

  FramebufferHandle addFramebuffer(const FramebufferCreateInfo& info);

  BufferHandle createBuffer(GLenum type);

  void updateInstances(size_t first, const InstanceBuffer& instances);
  FramebufferHandle submitCommands(CommandBuffer&& commandBuffer);
  void present(glm::uvec2 windowSize, FramebufferHandle srcFramebuffer) const;
  void swapBuffers();

private:
  struct TexturePack {
    AssetHandle<Bitmap> diffuseMap = AssetHandle<Bitmap>::null();
    AssetHandle<Bitmap> specularMap = AssetHandle<Bitmap>::null();
    AssetHandle<Bitmap> emissionMap = AssetHandle<Bitmap>::null();

    bool operator==(const TexturePack&) const = default;
  };

  static auto groupMeshesByTextures(AssetHandle<Model> model);
  // -> std::unordered_map<TexturePack, std::vector<NotNull<const MeshData>>, TexturePackHash>

  static MeshData mergeMeshes(std::span<const NotNull<const MeshData>> meshes);

  static GLint textureInternalFormat(bool bSRGB, uint32_t channels);

  void cmdRenderPass(CmdRenderPass& cmd, entt::entity& lastCamera, Scene const*& lastScene, std::span<const Draw>& draws);
  void cmdDrawDebugFrustum(const CmdDrawDebugFrustum& cmd);

  void renderPassScene(RenderPassScene& renderScenePass, const Viewport& viewport, FramebufferHandle dstFramebuffer, entt::entity& lastCamera, Scene const*& lastScene, std::span<const Draw>& draws);
  void renderSceneFull(std::span<const Draw> draws, const Viewport& viewport, FramebufferHandle dstFramebuffer, const ShadowMaps& shadowMaps, bool bClearFramebuffer = true);
  void renderSceneSimple(std::span<const Draw> draws, const Viewport& viewport, FramebufferHandle dstFramebuffer, SceneRenderMode mode, bool bClearFramebuffer = true);
  void postProcess(const Viewport& viewport, FramebufferHandle dstFramebuffer, FramebufferHandle srcFramebuffer, ShaderProgramInstanceHandle postProcessingShader) const;

  void updateCameraData(const Scene& scene, entt::entity enttCamera);
  void updateLightSourceData(const Scene& scene);

private:
  bool mInitialised = false;

  Registry<ShaderProgram> mShaderPrograms;
  Registry<ShaderProgramInstance> mShaderProgramInstances;
  Registry<Mesh> mMeshes;
  Registry<Sampler> mSamplers;
  Registry<Texture2D> mTexture2Ds;
  Registry<Texture2DArray> mTexture2DArrays;
  Registry<TextureCubeMap> mTextureCubeMaps;
  Registry<Framebuffer> mFramebuffers;
  Registry<Buffer> mBuffers;

  GLuint mMeshesVAO = GL_NONE;
  GLuint mScreenQuadVAO = GL_NONE;
  GLuint mDebugShapesVAO = GL_NONE;

  GLsync mBuffersFence = GL_NONE;

  ShaderProgramHandle mShadowMapShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mLitSurfaceShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mLitExplodedShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mLightShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mOutlineShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mReflectiveSurfaceShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mRefractiveSurfaceShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mSurfaceDepthShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mSurfaceNormalShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mVertexNormalShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mPostProcessCopyShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mPostProcessFlipHorizontallyShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mPostProcessFlipVerticallyShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mPostProcessGammaCorrectionShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mPostProcessGrayscaleShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mPostProcessInvertShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mPostProcessKernel3x3ShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mSkyboxShaderProgram = ShaderProgramHandle::null();
  ShaderProgramHandle mDebugFrustum = ShaderProgramHandle::null();

  SamplerHandle mDiffuseTextureSampler = SamplerHandle::null();
  SamplerHandle mSpecularTextureSampler = SamplerHandle::null();
  SamplerHandle mEmissionTextureSampler = SamplerHandle::null();
  SamplerHandle mEnvironmentTextureSampler = SamplerHandle::null();
  SamplerHandle mDirectionalShadowMapsSampler = SamplerHandle::null();
  SamplerHandle mPointShadowMapsSampler = SamplerHandle::null();
  SamplerHandle mSpotlightShadowMapsSampler = SamplerHandle::null();
  SamplerHandle mColorAttachmentSampler = SamplerHandle::null();

  Texture2DHandle mWhiteTexture = Texture2DHandle::null();
  Texture2DHandle mBlackTexture = Texture2DHandle::null();
  Texture2DHandle mFlatNormalMap = Texture2DHandle::null();
  Texture2DArrayHandle mEmptyDepthMaps = Texture2DArrayHandle::null();
  TextureCubeMapHandle mBlackCubeMap = TextureCubeMapHandle::null();

  MultiBuffer mCameraUniformBuffer;
  MultiBuffer mDirectionalLightsStorageBuffer;
  MultiBuffer mPointLightsStorageBuffer;
  MultiBuffer mSpotlightsStorageBuffer;
  MultiBuffer mInstanceBuffer;

  std::unordered_map<RegItemID, Texture2DHandle> mUploadedTextures;
  std::unordered_map<RegItemID, std::vector<RenderData>> mUploadedModels;
  std::unordered_map<entt::entity, size_t> mCameraToBufferIndex;
};

#include <Graphics/RenderingEngine.hpp>

#include <Assets/MeshData.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/RenderData.hpp>
#include <Graphics/RenderPass.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/Buffers/BindPoints.hpp>
#include <Graphics/Buffers/CameraUniforms.hpp>
#include <Graphics/Buffers/InstanceBuffer.hpp>
#include <Graphics/Viewport.hpp>
#include <Scene/Components/Camera.hpp>
#include <Scene/Scene.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/Visitor.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <tracy/TracyOpenGL.hpp>

#include <ranges>
#include <unordered_set>

Expected<void> RenderingEngine::init(AssetManager& assets) {
  ZoneScoped;

  // Shader programs
  Expected shadowMap         = this->createShaderProgram({.vertex = "positionOnly.vert", .fragment = "depthMap.frag"});
  Expected litSurface        = this->createShaderProgram({.vertex = "clipSpace.vert",    .fragment = "litSurface.frag"});
  Expected litExploded       = this->createShaderProgram({.vertex = "worldSpace.vert",   .geometry = "explode.geom", .fragment = "litSurface.frag"});
  Expected light             = this->createShaderProgram({.vertex = "clipSpace.vert",    .fragment = "light.frag"});
  Expected surfaceDepth      = this->createShaderProgram({.vertex = "clipSpace.vert",    .fragment = "surfaceDepth.frag"});
  Expected surfaceNormal     = this->createShaderProgram({.vertex = "clipSpace.vert",    .fragment = "surfaceNormal.frag"});
  Expected vertexNormal      = this->createShaderProgram({.vertex = "worldSpace.vert",   .geometry = "vertexNormals.geom", .fragment = "vector.frag"});
  Expected outline           = this->createShaderProgram({.vertex = "clipSpace.vert",    .fragment = "solidColor.frag"});
  Expected reflectiveSurface = this->createShaderProgram({.vertex = "clipSpace.vert",    .fragment = "reflectiveSurface.frag"});
  Expected refractiveSurface = this->createShaderProgram({.vertex = "clipSpace.vert",    .fragment = "refractiveSurface.frag"});
  Expected copy              = this->createShaderProgram({.vertex = "screenQuad.vert",   .fragment = "postProcessing/copy.frag"});
  Expected flipHorizontally  = this->createShaderProgram({.vertex = "screenQuad.vert",   .fragment = "postProcessing/flipHorizontally.frag"});
  Expected flipVertically    = this->createShaderProgram({.vertex = "screenQuad.vert",   .fragment = "postProcessing/flipVertically.frag"});
  Expected gammaCorrection   = this->createShaderProgram({.vertex = "screenQuad.vert",   .fragment = "postProcessing/gammaCorrection.frag"});
  Expected grayscale         = this->createShaderProgram({.vertex = "screenQuad.vert",   .fragment = "postProcessing/grayscale.frag"});
  Expected invert            = this->createShaderProgram({.vertex = "screenQuad.vert",   .fragment = "postProcessing/invert.frag"});
  Expected kernel3x3         = this->createShaderProgram({.vertex = "screenQuad.vert",   .fragment = "postProcessing/kernel3x3.frag"});
  Expected skybox            = this->createShaderProgram({.vertex = "skybox.vert",       .fragment = "skybox.frag"});
  Expected debugFrustum      = this->createShaderProgram({.vertex = "positionOnly.vert", .geometry = "frustum.geom", .fragment = "solidColor.frag"});

  ASSIGN_EXPECTED_OR_RETURN(mShadowMapShaderProgram, shadowMap);
  ASSIGN_EXPECTED_OR_RETURN(mLitSurfaceShaderProgram, litSurface);
  ASSIGN_EXPECTED_OR_RETURN(mLitExplodedShaderProgram, litExploded);
  ASSIGN_EXPECTED_OR_RETURN(mLightShaderProgram, light);
  ASSIGN_EXPECTED_OR_RETURN(mSurfaceDepthShaderProgram, surfaceDepth);
  ASSIGN_EXPECTED_OR_RETURN(mSurfaceNormalShaderProgram, surfaceNormal);
  ASSIGN_EXPECTED_OR_RETURN(mVertexNormalShaderProgram, vertexNormal);
  ASSIGN_EXPECTED_OR_RETURN(mOutlineShaderProgram, outline);
  ASSIGN_EXPECTED_OR_RETURN(mReflectiveSurfaceShaderProgram, reflectiveSurface);
  ASSIGN_EXPECTED_OR_RETURN(mRefractiveSurfaceShaderProgram, refractiveSurface);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessCopyShaderProgram, copy);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessFlipHorizontallyShaderProgram, flipHorizontally);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessFlipVerticallyShaderProgram, flipVertically);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessGammaCorrectionShaderProgram, gammaCorrection);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessGrayscaleShaderProgram, grayscale);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessInvertShaderProgram, invert);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessKernel3x3ShaderProgram, kernel3x3);
  ASSIGN_EXPECTED_OR_RETURN(mSkyboxShaderProgram, skybox);
  ASSIGN_EXPECTED_OR_RETURN(mDebugFrustum, debugFrustum);

  // Vertex arrays
  glCreateVertexArrays(1, &mMeshesVAO);
  glCreateVertexArrays(1, &mScreenQuadVAO);
  glCreateVertexArrays(1, &mDebugShapesVAO);
  Vertex::setupVertexAttributes(mMeshesVAO);

  // Samplers
  mDiffuseTextureSampler = this->addSampler({});
  mSpecularTextureSampler = this->addSampler({});
  mEmissionTextureSampler = this->addSampler({});
  mEnvironmentTextureSampler = this->addSampler({});

  constexpr SamplerOptions shadowSamplerOptions = {
    .minFilter = GL_LINEAR,
    .magFilter = GL_LINEAR,
    .wrapS = GL_CLAMP_TO_BORDER,
    .wrapT = GL_CLAMP_TO_BORDER,
    .borderColor = glm::vec4(1.0f),
    .compareMode = GL_COMPARE_REF_TO_TEXTURE,
    .compareFunc = GL_GREATER,
  };
  mDirectionalShadowMapsSampler = this->addSampler(shadowSamplerOptions);
  mPointShadowMapsSampler = this->addSampler(shadowSamplerOptions);
  mSpotlightShadowMapsSampler = this->addSampler(shadowSamplerOptions);

  mColorAttachmentSampler = this->addSampler(SamplerOptions { .minFilter = GL_LINEAR });

  // Textures
  const AssetHandle<Bitmap> whiteBitmap = assets.addBitmap(Bitmap::fromMemory(asBytes('\xFF'), glm::uvec2(1), 1).value());
  const AssetHandle<Bitmap> blackBitmap = assets.addBitmap(Bitmap::fromMemory(asBytes('\x00'), glm::uvec2(1), 1).value());
  const AssetHandle<Bitmap> flatNormalBitmap = assets.addBitmap(Bitmap::fromMemory(asBytes("\x88\x88\xFF"), glm::uvec2(1), 3).value());

  mWhiteTexture = this->addTexture2D(whiteBitmap);
  mBlackTexture = this->addTexture2D(blackBitmap);
  mFlatNormalMap = this->addTexture2D(flatNormalBitmap);
  mEmptyDepthMaps = this->createEmptyTexture2DArray();
  mBlackCubeMap = this->addTextureCubeMap({blackBitmap, blackBitmap, blackBitmap, blackBitmap, blackBitmap, blackBitmap});

  mEmptyDepthMaps->generate(std::array { whiteBitmap }, GL_DEPTH_COMPONENT24);

  // Buffers
  mCameraUniformBuffer = MultiBuffer(this->createBuffer(GL_UNIFORM_BUFFER), 3);
  mDirectionalLightsStorageBuffer = MultiBuffer(this->createBuffer(GL_SHADER_STORAGE_BUFFER), 3);
  mPointLightsStorageBuffer = MultiBuffer(this->createBuffer(GL_SHADER_STORAGE_BUFFER), 3);
  mSpotlightsStorageBuffer = MultiBuffer(this->createBuffer(GL_SHADER_STORAGE_BUFFER), 3);
  mInstanceBuffer = MultiBuffer(this->createBuffer(GL_SHADER_STORAGE_BUFFER), 3);

  mCameraUniformBuffer.allocate(sizeof(CameraUniforms));

  mInitialised = true;
  return {};
}

void RenderingEngine::destroy() {
  ZoneScoped;

  for (ShaderProgram& shaderProgram : std::views::values(mShaderPrograms)) {
    shaderProgram.destroy();
  }
  for (Mesh& mesh : std::views::values(mMeshes)) {
    mesh.destroy();
  }
  for (Sampler& sampler : std::views::values(mSamplers)) {
    sampler.destroy();
  }
  for (Texture2D& texture2D : std::views::values(mTexture2Ds)) {
    texture2D.destroy();
  }
  for (TextureCubeMap& textureCubeMap : std::views::values(mTextureCubeMaps)) {
    textureCubeMap.destroy();
  }
  for (Framebuffer& framebuffer : std::views::values(mFramebuffers)) {
    framebuffer.destroy();
  }
  for (Buffer& buffer : std::views::values(mBuffers)) {
    buffer.destroy();
  }

  mShaderPrograms.clear();
  mShaderProgramInstances.clear();
  mMeshes.clear();
  mSamplers.clear();
  mTexture2Ds.clear();
  mTextureCubeMaps.clear();
  mFramebuffers.clear();
  mBuffers.clear();

  glDeleteVertexArrays(1, &mMeshesVAO);
  glDeleteVertexArrays(1, &mScreenQuadVAO);
  mMeshesVAO = GL_NONE;
  mScreenQuadVAO = GL_NONE;

  mLitSurfaceShaderProgram = ShaderProgramHandle::null();
  mLightShaderProgram = ShaderProgramHandle::null();
  mOutlineShaderProgram = ShaderProgramHandle::null();
  mSurfaceDepthShaderProgram = ShaderProgramHandle::null();
  mSurfaceNormalShaderProgram = ShaderProgramHandle::null();
  mVertexNormalShaderProgram = ShaderProgramHandle::null();
  mPostProcessCopyShaderProgram = ShaderProgramHandle::null();
  mPostProcessFlipHorizontallyShaderProgram = ShaderProgramHandle::null();
  mPostProcessFlipVerticallyShaderProgram = ShaderProgramHandle::null();
  mPostProcessGrayscaleShaderProgram = ShaderProgramHandle::null();
  mPostProcessInvertShaderProgram = ShaderProgramHandle::null();
  mPostProcessKernel3x3ShaderProgram = ShaderProgramHandle::null();
  mSkyboxShaderProgram = ShaderProgramHandle::null();

  mColorAttachmentSampler = SamplerHandle::null();
  mDiffuseTextureSampler = SamplerHandle::null();
  mSpecularTextureSampler = SamplerHandle::null();
  mEmissionTextureSampler = SamplerHandle::null();
  mEnvironmentTextureSampler = SamplerHandle::null();
  mDirectionalShadowMapsSampler = SamplerHandle::null();
  mPointShadowMapsSampler = SamplerHandle::null();
  mSpotlightShadowMapsSampler = SamplerHandle::null();

  mCameraUniformBuffer = MultiBuffer();
  mDirectionalLightsStorageBuffer = MultiBuffer();
  mPointLightsStorageBuffer = MultiBuffer();
  mSpotlightsStorageBuffer = MultiBuffer();

  mUploadedTextures.clear();
  mUploadedModels.clear();

  mInitialised = false;
}

Expected<ShaderProgramHandle> RenderingEngine::createShaderProgram(const ShaderProgramPaths& shaderPaths) {
  ZoneScoped;

  Expected shaderProgram = ShaderProgram::fromShaders(shaderPaths);
  RETURN_ERROR_IF_UNEXPECTED(shaderProgram);
  return mShaderPrograms.add(std::move(shaderProgram.value()));
}

ShaderProgramHandle RenderingEngine::addShaderProgram(ShaderProgram&& shaderProgram) {
  ZoneScoped;

  return mShaderPrograms.add(std::forward<ShaderProgram>(shaderProgram));
}

ShaderProgramInstanceHandle RenderingEngine::createShaderProgramInstance(const ShaderProgramType type) {
  ZoneScoped;

#define CASE_RETURN(ShaderType, shaderProgramOption) \
  case ShaderType: \
    TO_STATEMENT(return this->addShaderProgramInstance(ShaderProgramInstance::create<ShaderType>(shaderProgramOption));)

  switch (type) {
    CASE_RETURN(ShaderProgramType::LitSurface, mLitSurfaceShaderProgram);
    CASE_RETURN(ShaderProgramType::LitExploded, mLitExplodedShaderProgram);
    CASE_RETURN(ShaderProgramType::Light, mLightShaderProgram);
    CASE_RETURN(ShaderProgramType::Outline, mOutlineShaderProgram);
    CASE_RETURN(ShaderProgramType::ReflectiveSurface, mReflectiveSurfaceShaderProgram);
    CASE_RETURN(ShaderProgramType::RefractiveSurface, mRefractiveSurfaceShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessCopy, mPostProcessCopyShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessBlur, mPostProcessKernel3x3ShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessEdgeDetection, mPostProcessKernel3x3ShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessEmboss, mPostProcessKernel3x3ShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessFlipHorizontally, mPostProcessFlipHorizontallyShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessFlipVertically, mPostProcessFlipVerticallyShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessGammaCorrection, mPostProcessGammaCorrectionShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessGrayscale, mPostProcessGrayscaleShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessInvert, mPostProcessInvertShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessSharpen, mPostProcessKernel3x3ShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessSobelBottom, mPostProcessKernel3x3ShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessSobelLeft, mPostProcessKernel3x3ShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessSobelRight, mPostProcessKernel3x3ShaderProgram);
    CASE_RETURN(ShaderProgramType::PostProcessSobelTop, mPostProcessKernel3x3ShaderProgram);
    CASE_RETURN(ShaderProgramType::Skybox, mSkyboxShaderProgram);
    CASE_RETURN(ShaderProgramType::SurfaceDepth, mSurfaceDepthShaderProgram);
    CASE_RETURN(ShaderProgramType::SurfaceNormal, mSurfaceNormalShaderProgram);
    default:
      UNREACHABLE();
  }
#undef CASE_RETURN
}

ShaderProgramInstanceHandle RenderingEngine::addShaderProgramInstance(ShaderProgramInstance&& instance) {
  ZoneScoped;

  return mShaderProgramInstances.add(std::forward<ShaderProgramInstance>(instance));
}

auto RenderingEngine::groupMeshesByTextures(AssetHandle<Model> model) {
  ZoneScoped;

  struct TexturePackHash {
    static size_t textureHash(const AssetHandle<Bitmap>& bitmap) {
      const auto path = bitmap.isNull() ? std::filesystem::path() : bitmap->filePath();
      return std::hash<std::filesystem::path>{}(path);
    }

    size_t operator()(const TexturePack& texturePack) const {
      const size_t hashDiffuse = textureHash(texturePack.diffuseMap);
      const size_t hashSpecular = textureHash(texturePack.specularMap);
      const size_t hashEmission = textureHash(texturePack.emissionMap);
      return hashDiffuse ^ ((hashSpecular ^ (hashEmission << 1)) << 1);
    }
  };

  std::unordered_map<TexturePack, std::vector<NotNull<const MeshData>>, TexturePackHash> texturesToMesh;
  for (const auto [meshIndex, mesh] : model->meshes | std::views::enumerate) {
    const auto pack = TexturePack {
      .diffuseMap = model->diffuseMaps[meshIndex],
      .specularMap = model->specularMaps[meshIndex],
      .emissionMap = model->emissionMaps[meshIndex],
    };
    texturesToMesh[pack].emplace_back(&mesh.get());
  }
  return texturesToMesh;
}

MeshData RenderingEngine::mergeMeshes(const std::span<const NotNull<const MeshData>> meshes) {
  ZoneScoped;

  MeshData mergedMeshData;
  for (size_t accumulatedVertices = 0; NotNull mesh : meshes) {
    mergedMeshData.vertices.append_range(mesh->vertices);
    mergedMeshData.indices.append_range(mesh->indices |
      std::views::transform([=](const uint32_t index) { return index + accumulatedVertices; }));
    accumulatedVertices += mesh->vertices.size();
  }
  return mergedMeshData;
}

const std::vector<RenderData>& RenderingEngine::addModel(AssetHandle<Model> model, ShaderProgramInstanceHandle initialShader) {
  ZoneScoped;

  if (mUploadedModels.contains(model.itemID())) {
    return mUploadedModels.at(model.itemID());
  }

  const auto texturesToMeshes = RenderingEngine::groupMeshesByTextures(model);
  std::vector<RenderData> modelRenderData;
  modelRenderData.reserve(texturesToMeshes.size());
  for (const auto& [textures, meshes] : texturesToMeshes) {
    const MeshData mergedMesh = RenderingEngine::mergeMeshes(meshes);
    RenderData renderData = {
      .mesh = this->addMesh(mergedMesh),
      .shader = initialShader,
      .diffuseMap = textures.diffuseMap.isNull() ? Texture2DHandle::null() : this->addTexture2D(textures.diffuseMap),
      .specularMap = textures.specularMap.isNull() ? Texture2DHandle::null() : this->addTexture2D(textures.specularMap),
      .emissionMap = textures.emissionMap.isNull() ? Texture2DHandle::null() : this->addTexture2D(textures.emissionMap),
    };
    modelRenderData.push_back(renderData);
  }

  const auto [it, inserted] = mUploadedModels.emplace(model.itemID(), std::move(modelRenderData));
  const auto& [index, resources] = *it;
  return resources;
}

std::vector<MeshHandle> RenderingEngine::addMeshes(std::span<const AssetHandle<MeshData>> meshesData) {
  ZoneScoped;

  std::vector<MeshHandle> handles;
  handles.reserve(meshesData.size());
  for (const AssetHandle<MeshData>& meshData : meshesData) {
    const MeshHandle handle = this->addMesh(meshData);
    handles.push_back(handle);
  }
  return handles;
}

MeshHandle RenderingEngine::addMesh(AssetHandle<MeshData> meshData) {
  ZoneScoped;

  return this->addMesh(meshData.get());
}

MeshHandle RenderingEngine::addMesh(const MeshData& meshData) {
  ZoneScoped;

  return mMeshes.add(Mesh(meshData));
}

SamplerHandle RenderingEngine::addSampler(const SamplerOptions& options) {
  ZoneScoped;

  return mSamplers.add(Sampler(options));
}

Texture2DHandle RenderingEngine::createEmptyTexture2D() {
  ZoneScoped;

  return mTexture2Ds.add(Texture2D());
}

Texture2DArrayHandle RenderingEngine::createEmptyTexture2DArray() {
  ZoneScoped;

  return mTexture2DArrays.add(Texture2DArray());
}

TextureCubeMapHandle RenderingEngine::createEmptyTextureCubeMap() {
  ZoneScoped;

  return mTextureCubeMaps.add(TextureCubeMap());
}

std::vector<Texture2DHandle> RenderingEngine::addTexture2Ds(const std::span<const AssetHandle<Bitmap>> bitmaps) {
  ZoneScoped;

  std::vector<Texture2DHandle> handles;
  handles.reserve(bitmaps.size());
  for (const AssetHandle<Bitmap> bitmap : bitmaps) {
    if (bitmap.isNull()) {
      handles.emplace_back(Texture2DHandle::null());
    } else {
      const Texture2DHandle ref = this->addTexture2D(bitmap);
      handles.emplace_back(ref);
    }
  }
  return handles;
}

Texture2DHandle RenderingEngine::addTexture2D(const AssetHandle<Bitmap> bitmap) {
  ZoneScoped;

  if (mUploadedTextures.contains(bitmap.itemID())) {
    return mUploadedTextures.at(bitmap.itemID());
  }

  const Texture2DHandle handle = mTexture2Ds.add(Texture2D(bitmap, textureInternalFormat(bitmap->bSRGB, bitmap->channels())));
  mUploadedTextures.emplace(bitmap.itemID(), handle);
  return handle;
}

Texture2DArrayHandle RenderingEngine::addTexture2DArray(const std::span<const AssetHandle<Bitmap>> bitmaps) {
  ZoneScoped;
  assert(!bitmaps.empty());

  return mTexture2DArrays.add(Texture2DArray(bitmaps, textureInternalFormat(bitmaps.front()->bSRGB, bitmaps.front()->channels())));
}

TextureCubeMapHandle RenderingEngine::addTextureCubeMap(const TextureCubeMapBitmaps& bitmaps) {
  ZoneScoped;

  const uint32_t maxChannels = std::max({ bitmaps.right->channels(), bitmaps.left->channels(),
                                          bitmaps.top->channels(), bitmaps.bottom->channels(),
                                          bitmaps.front->channels(), bitmaps.back->channels() });

  return mTextureCubeMaps.add(TextureCubeMap(bitmaps, textureInternalFormat(bitmaps.bSRGB, maxChannels)));
}

GLint RenderingEngine::textureInternalFormat(const bool bSRGB, const uint32_t channels) {
  return bSRGB && channels == 4 ? GL_SRGB8_ALPHA8 :
         bSRGB && channels < 4  ? GL_SRGB8 :
        !bSRGB && channels == 4 ? GL_RGBA8 :
        !bSRGB && channels == 3 ? GL_RGBA8 :
        !bSRGB && channels == 2 ? GL_RG8 : GL_R8;
}

FramebufferHandle RenderingEngine::addFramebuffer(const FramebufferCreateInfo& info) {
  ZoneScoped;

  return mFramebuffers.add(Framebuffer(info));
}

BufferHandle RenderingEngine::createBuffer(const GLenum type) {
  ZoneScoped;

  return mBuffers.add(Buffer(type));
}

void RenderingEngine::updateInstances(const size_t first, const InstanceBuffer& instances) {
  ZoneScoped;

  assert(mInitialised);

  mInstanceBuffer.write(instances, first * sizeof(InstanceData));
}

FramebufferHandle RenderingEngine::submitCommands(CommandBuffer&& commandBuffer) {
  ZoneScoped;

  assert(mInitialised);

  entt::entity lastCamera = entt::null;
  const Scene* lastScene = nullptr;
  std::span<const Draw> draws;
  FramebufferHandle lastFramebuffer = FramebufferHandle::null();

  for (CommandBuffer::Command& command : commandBuffer.commands) {
    command.visit(Visitor {
      [&](CmdRenderPass& cmd) {
        lastFramebuffer = cmd.renderPass.dstFramebuffer;
        this->cmdRenderPass(cmd, lastCamera, lastScene, draws);
      },
      [&](const CmdDrawDebugFrustum& cmd) { this->cmdDrawDebugFrustum(cmd); },
    });
  }

  this->swapBuffers();

  return lastFramebuffer;
}

void RenderingEngine::cmdRenderPass(CmdRenderPass& cmd, entt::entity& lastCamera, Scene const*& lastScene, std::span<const Draw>& draws) {
  using namespace std::placeholders;
  auto& [viewport, dstFramebuffer, pass] = cmd.renderPass;
  pass.visit(Visitor {
    [&](RenderPassScene& p) { this->renderPassScene(p, viewport, dstFramebuffer, lastCamera, lastScene, draws); },
    [&](const PostProcessingPass& p) { this->postProcess(viewport, dstFramebuffer, p.srcFramebuffer, p.postProcessingShader); },
  });
}

void RenderingEngine::cmdDrawDebugFrustum(const CmdDrawDebugFrustum& cmd) {
  ZoneScoped;
  TracyGpuZone("Draw debug frustum");

  glUseProgram(mDebugFrustum->id());
  glUniform3fv(glGetUniformLocation(mDebugFrustum->id(), "uFrustum.nearBottomLeft"), 1, glm::value_ptr(cmd.frustum.nearBottomLeft));
  glUniform3fv(glGetUniformLocation(mDebugFrustum->id(), "uFrustum.nearBottomRight"), 1, glm::value_ptr(cmd.frustum.nearBottomRight));
  glUniform3fv(glGetUniformLocation(mDebugFrustum->id(), "uFrustum.nearTopLeft"), 1, glm::value_ptr(cmd.frustum.nearTopLeft));
  glUniform3fv(glGetUniformLocation(mDebugFrustum->id(), "uFrustum.nearTopRight"), 1, glm::value_ptr(cmd.frustum.nearTopRight));
  glUniform3fv(glGetUniformLocation(mDebugFrustum->id(), "uFrustum.farBottomLeft"), 1, glm::value_ptr(cmd.frustum.farBottomLeft));
  glUniform3fv(glGetUniformLocation(mDebugFrustum->id(), "uFrustum.farBottomRight"), 1, glm::value_ptr(cmd.frustum.farBottomRight));
  glUniform3fv(glGetUniformLocation(mDebugFrustum->id(), "uFrustum.farTopLeft"), 1, glm::value_ptr(cmd.frustum.farTopLeft));
  glUniform3fv(glGetUniformLocation(mDebugFrustum->id(), "uFrustum.farTopRight"), 1, glm::value_ptr(cmd.frustum.farTopRight));
  glUniform3f(4, cmd.color.x, cmd.color.y, cmd.color.z);

  glBindVertexArray(mDebugShapesVAO);
  glDrawArrays(GL_POINTS, 0, 1);
  glBindVertexArray(GL_NONE);
}

void RenderingEngine::renderPassScene(RenderPassScene& renderScenePass, const Viewport& viewport, FramebufferHandle dstFramebuffer,
                                      entt::entity& lastCamera, Scene const*& lastScene, std::span<const Draw>& draws)
{
  const bool bSceneChanged = lastScene != renderScenePass.scene;
  const bool bCameraChanged = lastCamera != renderScenePass.entityCamera;

  if (bSceneChanged) {
    lastScene = renderScenePass.scene;
    draws = renderScenePass.scene->draw(renderScenePass.entityCamera, *this);

    const auto camerasView = lastScene->ecs.view<const CompCamera, const CompTransform>();
    if (const size_t numCameras = std::distance(camerasView.begin(), camerasView.end());
        numCameras * 3 > mCameraUniformBuffer.numBuffers())
    {
      mCameraUniformBuffer.setNumBuffers(numCameras * 3);
    }

    for (const auto [i, pack] : camerasView.each() | std::views::enumerate) {
      const auto [entity, camera, transform] = pack;
      mCameraToBufferIndex.insert_or_assign(entity, i);
    }
  }

  if (bCameraChanged && renderScenePass.entityCamera != entt::null) {
    lastCamera = renderScenePass.entityCamera;
    this->updateCameraData(*renderScenePass.scene, renderScenePass.entityCamera);
  }

  if (renderScenePass.mode == SceneRenderMode::Full) {
    if (bSceneChanged || bCameraChanged) {
      this->updateLightSourceData(*renderScenePass.scene);
    }
    this->renderSceneFull(draws, viewport, dstFramebuffer, renderScenePass.shadowMaps, renderScenePass.bClearFramebuffer);
  } else {
    this->renderSceneSimple(draws, viewport, dstFramebuffer, renderScenePass.mode, renderScenePass.bClearFramebuffer);
  }
}

void RenderingEngine::renderSceneFull(const std::span<const Draw> draws, const Viewport& viewport, FramebufferHandle dstFramebuffer,
                                      const ShadowMaps& shadowMaps, const bool bClearFramebuffer) {
  ZoneScoped;
  TracyGpuZone("Render scene (full)");

  assert(mInitialised);

  dstFramebuffer->bind();

  const glm::ivec2 viewportPositionPx = glm::round(viewport.position * glm::vec2(dstFramebuffer->size()));
  const glm::ivec2 viewportSizePx = glm::round(viewport.size * glm::vec2(dstFramebuffer->size()));
  glViewport(viewportPositionPx.x, viewportPositionPx.y, viewportSizePx.x, viewportSizePx.y);

  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glDepthFunc(GL_LEQUAL);

  glEnable(GL_STENCIL_TEST);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glStencilMask(0xff);

  if (bClearFramebuffer) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }

  if (draws.empty()) {
    return;
  }

  glBindVertexArray(mMeshesVAO);

  mDiffuseTextureSampler->bind(BINDING_SAMPLER_DIFFUSE);
  mSpecularTextureSampler->bind(BINDING_SAMPLER_SPECULAR);
  mEmissionTextureSampler->bind(BINDING_SAMPLER_EMISSION);
  mEnvironmentTextureSampler->bind(BINDING_SAMPLER_ENVIRONMENT);
  mDirectionalShadowMapsSampler->bind(BINDING_SAMPLER_DIRECTIONAL_SHADOWS);
  mPointShadowMapsSampler->bind(BINDING_SAMPLER_POINT_SHADOWS);
  mSpotlightShadowMapsSampler->bind(BINDING_SAMPLER_SPOTLIGHT_SHADOWS);

  std::unordered_set<GLuint> boundTextureSlots;

  shadowMaps.directionalShadowMaps.getOrDefault(mEmptyDepthMaps).bind(BINDING_SAMPLER_DIRECTIONAL_SHADOWS);
  shadowMaps.pointShadowMaps.getOrDefault(mEmptyDepthMaps).bind(BINDING_SAMPLER_POINT_SHADOWS);
  shadowMaps.spotlightShadowMaps.getOrDefault(mEmptyDepthMaps).bind(BINDING_SAMPLER_SPOTLIGHT_SHADOWS);
  boundTextureSlots.emplace(BINDING_SAMPLER_DIRECTIONAL_SHADOWS);
  boundTextureSlots.emplace(BINDING_SAMPLER_POINT_SHADOWS);
  boundTextureSlots.emplace(BINDING_SAMPLER_SPOTLIGHT_SHADOWS);

  const Draw* lastDraw = &draws.front();
  for (const auto [drawIdx, currDraw] : draws | std::views::enumerate) {
    mInstanceBuffer.bindRange(BINDING_SSBO_INSTANCES,
                              currDraw.instanceOffset * sizeof(InstanceData),
                              currDraw.instanceCount * sizeof(InstanceData));

    if (drawIdx == 0 || currDraw.bBackfaceCulling != lastDraw->bBackfaceCulling) [[unlikely]] {
      if (currDraw.bBackfaceCulling) {
        glEnable(GL_CULL_FACE);
      } else {
        glDisable(GL_CULL_FACE);
      }
    }
    if (drawIdx == 0 || currDraw.bDepthTest != lastDraw->bDepthTest) [[unlikely]] {
      if (currDraw.bDepthTest) {
        glEnable(GL_DEPTH_TEST);
      } else {
        glDisable(GL_DEPTH_TEST);
      }
    }
    if (drawIdx == 0 || currDraw.bWriteToDepth != lastDraw->bWriteToDepth) [[unlikely]] {
      glDepthMask(currDraw.bWriteToDepth ? GL_TRUE : GL_FALSE);
    }
    if (drawIdx == 0 || currDraw.bStencilTest != lastDraw->bStencilTest) [[unlikely]] {
      glStencilFunc(currDraw.bStencilTest ? GL_NOTEQUAL : GL_ALWAYS, 1, 0xff);
    }
    if (drawIdx == 0 || currDraw.bWriteToStencil != lastDraw->bWriteToStencil) [[unlikely]] {
      glStencilMask(currDraw.bWriteToStencil ? 0xff : 0x00);
    }
    if (drawIdx == 0 || currDraw.bTransparent != lastDraw->bTransparent) [[unlikely]] {
      if (currDraw.bTransparent) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      } else {
        glDisable(GL_BLEND);
      }
    }

    const ShaderProgramInstance& currShader = currDraw.shaderProgramInstance.get();
    if (drawIdx == 0 || currDraw.shaderProgramInstance != lastDraw->shaderProgramInstance) {
      if (drawIdx == 0 || currShader.shaderProgram() != lastDraw->shaderProgramInstance->shaderProgram()) {
        currShader.use();
      }
      currShader.bindUniforms();
    }

    const auto bindTexture = [&](const auto currTexture, const auto lastTexture, const auto defaultTexture, const GLuint unit) {
      if (drawIdx == 0 || currTexture != lastTexture) {
        currTexture.getOrDefault(defaultTexture).bind(unit);
        boundTextureSlots.insert(unit);
      }
    };
    bindTexture(currDraw.diffuseMap, lastDraw->diffuseMap, mBlackTexture, BINDING_SAMPLER_DIFFUSE);
    bindTexture(currDraw.specularMap, lastDraw->specularMap, mBlackTexture, BINDING_SAMPLER_SPECULAR);
    bindTexture(currDraw.emissionMap, lastDraw->emissionMap, mBlackTexture, BINDING_SAMPLER_EMISSION);
    bindTexture(currDraw.environmentMap, lastDraw->environmentMap, mBlackCubeMap, BINDING_SAMPLER_ENVIRONMENT);

    currDraw.mesh->bind();
    {
      TracyGpuZone("Draw elements instanced");
      glDrawElementsInstanced(GL_TRIANGLES, currDraw.mesh->indexCount(), GL_UNSIGNED_INT,
                              reinterpret_cast<void*>(currDraw.mesh->indicesOffset()),
                              static_cast<GLsizei>(currDraw.instanceCount));
    }

    lastDraw = &currDraw;
  }

  dstFramebuffer->resolveMultisample();

  glBindVertexArray(GL_NONE);
  glUseProgram(GL_NONE);
  for (const GLuint slot : boundTextureSlots) {
    glBindTextureUnit(slot, GL_NONE);
  }
}

void RenderingEngine::renderSceneSimple(const std::span<const Draw> draws, const Viewport& viewport,
                                        const FramebufferHandle dstFramebuffer, const SceneRenderMode mode,
                                        const bool bClearFramebuffer)
{
  ZoneScoped;
  TracyGpuZone("Render scene (simple)");

  assert(mInitialised);

  dstFramebuffer->bind();

  const glm::ivec2 viewportPositionPx = glm::round(viewport.position * glm::vec2(dstFramebuffer->size()));
  const glm::ivec2 viewportSizePx = glm::round(viewport.size * glm::vec2(dstFramebuffer->size()));
  glViewport(viewportPositionPx.x, viewportPositionPx.y, viewportSizePx.x, viewportSizePx.y);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);

  if (bClearFramebuffer) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  if (draws.empty()) {
    return;
  }

  glBindVertexArray(mMeshesVAO);

  switch (mode) {
    case SceneRenderMode::Full:
      UNREACHABLE();

    case SceneRenderMode::Wireframe:
      glUseProgram(mLightShaderProgram->id());
      glUniform3f(4, 1.0f, 1.0f, 1.0f);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;

    case SceneRenderMode::SurfaceDepth:
      glUseProgram(mSurfaceDepthShaderProgram->id());
      break;

    case SceneRenderMode::SurfaceNormal:
      glUseProgram(mSurfaceNormalShaderProgram->id());
      break;

    case SceneRenderMode::DepthMap:
      glUseProgram(mShadowMapShaderProgram->id());
      break;

    case SceneRenderMode::VertexNormals:
      glUseProgram(mVertexNormalShaderProgram->id());
      break;
  }

  for (const Draw& currDraw : draws) {
    if (!currDraw.bSkybox) {
      mInstanceBuffer.bindRange(BINDING_SSBO_INSTANCES,
                                currDraw.instanceOffset * sizeof(InstanceData),
                                currDraw.instanceCount * sizeof(InstanceData));
      currDraw.mesh->bind();
      {
        TracyGpuZone("Draw elements instanced");
        glDrawElementsInstanced(GL_TRIANGLES, currDraw.mesh->indexCount(), GL_UNSIGNED_INT,
                                reinterpret_cast<void*>(currDraw.mesh->indicesOffset()),
                                static_cast<GLsizei>(currDraw.instanceCount));
      }
    }
  }

  dstFramebuffer->resolveMultisample();

  glBindVertexArray(GL_NONE);
  glUseProgram(GL_NONE);

  if (mode == SceneRenderMode::Wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}

void RenderingEngine::postProcess(const Viewport& viewport, FramebufferHandle dstFramebuffer, FramebufferHandle srcFramebuffer,
                                  ShaderProgramInstanceHandle postProcessingShader) const
{
  ZoneScoped;
  TracyGpuZone("Postprocess");

  assert(mInitialised);
  assert(!srcFramebuffer->colorAttachments().empty());

  const glm::ivec2 viewportPositionPx = glm::round(viewport.position * glm::vec2(dstFramebuffer->size()));
  const glm::ivec2 viewportSizePx = glm::round(viewport.size * glm::vec2(dstFramebuffer->size()));
  glViewport(viewportPositionPx.x, viewportPositionPx.y, viewportSizePx.x, viewportSizePx.y);

  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);

  dstFramebuffer->bind();

  postProcessingShader->use();
  postProcessingShader->bindUniforms();
  srcFramebuffer->colorAttachments().front().texture->bind(BINDING_SAMPLER_SCREEN);
  mColorAttachmentSampler->bind(BINDING_SAMPLER_SCREEN);

  glBindVertexArray(mScreenQuadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  dstFramebuffer->resolveMultisample();

  glUseProgram(GL_NONE);
  glBindVertexArray(GL_NONE);
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void RenderingEngine::present(const glm::uvec2 windowSize, FramebufferHandle srcFramebuffer) const {
  ZoneScoped;
  TracyGpuZone("Present");

  assert(mInitialised);
  assert(!srcFramebuffer->colorAttachments().empty());

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(0, 0, static_cast<GLint>(windowSize.x), static_cast<GLint>(windowSize.y));

  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(mPostProcessCopyShaderProgram->id());
  glUniform1i(glGetUniformLocation(mPostProcessCopyShaderProgram->id(), "uScreenTexture"), BINDING_SAMPLER_SCREEN);
  srcFramebuffer->colorAttachments().front().texture->bind(BINDING_SAMPLER_SCREEN);
  mColorAttachmentSampler->bind(BINDING_SAMPLER_SCREEN);

  glBindVertexArray(mScreenQuadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(GL_NONE);
  glBindVertexArray(GL_NONE);
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void RenderingEngine::swapBuffers() {
  ZoneScoped;

  assert(mInitialised);

  mCameraUniformBuffer.switchToNext();
  mDirectionalLightsStorageBuffer.switchToNext();
  mPointLightsStorageBuffer.switchToNext();
  mSpotlightsStorageBuffer.switchToNext();
  mInstanceBuffer.switchToNext();
}

void RenderingEngine::updateCameraData(const Scene& scene, const entt::entity enttCamera) {
  ZoneScoped;

  assert(mInitialised);

  const auto [camera, cameraTransform] = scene.ecs.get<const CompCamera, const CompTransform>(enttCamera);
  CameraUniforms cameraUniforms;
  if (scene.ecs.all_of<CompDirectionalLight>(enttCamera)) {
    cameraUniforms = CameraUniforms::fromOrthographic(camera, cameraTransform);
  } else {
    cameraUniforms = CameraUniforms::fromPerspective(camera, cameraTransform);
  }

  mCameraUniformBuffer.setCurrent(mCameraToBufferIndex[enttCamera]);
  mCameraUniformBuffer.write(cameraUniforms);
  mCameraUniformBuffer.bindWhole(BINDING_UBO_CAMERA);
}

void RenderingEngine::updateLightSourceData(const Scene& scene) {
  ZoneScoped;

  assert(mInitialised);

  const DirectionalLightSourceBuffer directionalLightsData = scene.createDirectionalLightBufferData();
  mDirectionalLightsStorageBuffer.write(directionalLightsData);
  mDirectionalLightsStorageBuffer.bindWhole(BINDING_SSBO_DIRECTIONAL_LIGHTS);

  const PointLightSourceBuffer pointLightsData = scene.createPointLightBufferData();
  mPointLightsStorageBuffer.write(pointLightsData);
  mPointLightsStorageBuffer.bindWhole(BINDING_SSBO_POINT_LIGHTS);

  const SpotlightSourceBuffer spotlightsData = scene.createSpotlightBufferData();
  mSpotlightsStorageBuffer.write(spotlightsData);
  mSpotlightsStorageBuffer.bindWhole(BINDING_SSBO_SPOTLIGHTS);
}

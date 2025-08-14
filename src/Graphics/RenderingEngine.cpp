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
#include <Scene/Components/Graphics.hpp>
#include <Scene/Scene.hpp>
#include <Util/Macros/Errors.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <tracy/TracyOpenGL.hpp>

#include <ranges>

Expected<void> RenderingEngine::init() {
  ZoneScoped;

  Expected litSurface        = this->createShaderProgram({.vertex = "clipSpace.vert",  .fragment = "litSurface.frag"});
  Expected litExploded       = this->createShaderProgram({.vertex = "worldSpace.vert", .geometry = "explode.geom", .fragment = "litSurface.frag"});
  Expected light             = this->createShaderProgram({.vertex = "clipSpace.vert",  .fragment = "light.frag"});
  Expected surfaceDepth      = this->createShaderProgram({.vertex = "clipSpace.vert",  .fragment = "surfaceDepth.frag"});
  Expected surfaceNormal     = this->createShaderProgram({.vertex = "clipSpace.vert",  .fragment = "surfaceNormal.frag"});
  Expected vertexNormal      = this->createShaderProgram({.vertex = "worldSpace.vert", .geometry = "vertexNormals.geom", .fragment = "vector.frag"});
  Expected outline           = this->createShaderProgram({.vertex = "clipSpace.vert",  .fragment = "outline.frag"});
  Expected reflectiveSurface = this->createShaderProgram({.vertex = "clipSpace.vert",  .fragment = "reflectiveSurface.frag"});
  Expected refractiveSurface = this->createShaderProgram({.vertex = "clipSpace.vert",  .fragment = "refractiveSurface.frag"});
  Expected copy              = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/copy.frag"});
  Expected flipHorizontally  = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/flipHorizontally.frag"});
  Expected flipVertically    = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/flipVertically.frag"});
  Expected gammaCorrection   = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/gammaCorrection.frag"});
  Expected grayscale         = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/grayscale.frag"});
  Expected invert            = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/invert.frag"});
  Expected kernel3x3         = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/kernel3x3.frag"});
  Expected skybox            = this->createShaderProgram({.vertex = "skybox.vert",     .fragment = "skybox.frag"});

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

  glCreateVertexArrays(1, &mScreenQuadVAO);
  glCreateVertexArrays(1, &mMeshesVAO);
  Vertex::setupVertexAttributes(mMeshesVAO);

  mColorAttachmentSampler = this->addSampler(SamplerOptions { .minFilter = GL_LINEAR });
  mDiffuseTextureSampler = this->addSampler(SamplerOptions {});
  mSpecularTextureSampler = this->addSampler(SamplerOptions {});
  mEmissionTextureSampler = this->addSampler(SamplerOptions {});
  mEnvironmentTextureSampler = this->addSampler(SamplerOptions {});

  mCameraUniformBuffer.emplace(this->createBuffer(GL_UNIFORM_BUFFER));
  mCameraUniformBuffer.value()->allocate(sizeof(CameraUniforms));

  mDirectionalLightsStorageBuffer.emplace(this->createBuffer(GL_SHADER_STORAGE_BUFFER));
  mPointLightsStorageBuffer.emplace(this->createBuffer(GL_SHADER_STORAGE_BUFFER));
  mSpotlightsStorageBuffer.emplace(this->createBuffer(GL_SHADER_STORAGE_BUFFER));
  mInstanceBuffer.emplace(this->createBuffer(GL_SHADER_STORAGE_BUFFER));

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

  mLitSurfaceShaderProgram.reset();
  mLightShaderProgram.reset();
  mOutlineShaderProgram.reset();
  mSurfaceDepthShaderProgram.reset();
  mSurfaceNormalShaderProgram.reset();
  mVertexNormalShaderProgram.reset();
  mPostProcessCopyShaderProgram.reset();
  mPostProcessFlipHorizontallyShaderProgram.reset();
  mPostProcessFlipVerticallyShaderProgram.reset();
  mPostProcessGrayscaleShaderProgram.reset();
  mPostProcessInvertShaderProgram.reset();
  mPostProcessKernel3x3ShaderProgram.reset();
  mSkyboxShaderProgram.reset();

  mColorAttachmentSampler.reset();
  mDiffuseTextureSampler.reset();
  mSpecularTextureSampler.reset();
  mEmissionTextureSampler.reset();
  mEnvironmentTextureSampler.reset();

  mCameraUniformBuffer.reset();
  mDirectionalLightsStorageBuffer.reset();
  mPointLightsStorageBuffer.reset();
  mSpotlightsStorageBuffer.reset();

  mUploadedTextures.clear();
  mUploadedModels.clear();

  mBoundTextureSlots.clear();

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
    TO_STATEMENT(return this->addShaderProgramInstance(ShaderProgramInstance::create<ShaderType>(shaderProgramOption.value()));)

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
    static size_t textureOptionHash(const std::optional<AssetHandle<Bitmap>>& texOpt) {
      const auto path = texOpt
        .and_then([](AssetHandle<Bitmap> bitmap) { return std::optional(bitmap->filePath()); })
        .value_or(std::filesystem::path());
      return std::hash<std::filesystem::path>{}(path);
    }

    size_t operator()(const TexturePack& texturePack) const {
      const size_t hashDiffuse = textureOptionHash(texturePack.diffuseMap);
      const size_t hashSpecular = textureOptionHash(texturePack.specularMap);
      const size_t hashEmission = textureOptionHash(texturePack.emissionMap);
      return hashDiffuse ^ ((hashSpecular ^ (hashEmission << 1)) << 1);
    }
  };

  std::unordered_map<TexturePack, std::vector<NotNull<const MeshData>>, TexturePackHash> texturesToMesh;
  for (size_t meshRes = 0; meshRes < model->meshes.size(); ++meshRes) {
    const auto pack = TexturePack {
      .diffuseMap = model->diffuseMaps[meshRes],
      .specularMap = model->specularMaps[meshRes],
      .emissionMap = model->emissionMaps[meshRes],
    };
    texturesToMesh[pack].push_back(&model->meshes[meshRes].get());
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
    const auto optionalAddTexture2D = [this](const AssetHandle<Bitmap> texture) { return this->addTexture2D(texture); };
    RenderData renderData = {
      .mesh = this->addMesh(mergedMesh),
      .shaderProgramInstance = initialShader,
      .diffuseMap = textures.diffuseMap.transform(optionalAddTexture2D),
      .specularMap = textures.specularMap.transform(optionalAddTexture2D),
      .emissionMap = textures.emissionMap.transform(optionalAddTexture2D),
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

auto RenderingEngine::addTexture2Ds(const std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps)
  -> std::vector<std::optional<Texture2DHandle>>
{
  ZoneScoped;

  std::vector<std::optional<Texture2DHandle>> handles;
  handles.reserve(bitmaps.size());
  for (size_t i = 0; const std::optional bitmap : bitmaps) {
    if (bitmap.has_value()) {
      const Texture2DHandle ref = this->addTexture2D(bitmap.value());
      handles.emplace_back(ref);
    } else {
      handles.emplace_back();
    }
    ++i;
  }
  return handles;
}

Texture2DHandle RenderingEngine::addTexture2D(const AssetHandle<Bitmap> bitmap) {
  ZoneScoped;

  if (mUploadedTextures.contains(bitmap.itemID())) {
    return mUploadedTextures.at(bitmap.itemID());
  }

  const GLint internalFormat = bitmap->bSRGB && bitmap->channels() == 4 ? GL_SRGB8_ALPHA8 :
                               bitmap->bSRGB && bitmap->channels() < 4  ? GL_SRGB8 :
                              !bitmap->bSRGB && bitmap->channels() == 4 ? GL_RGBA8 : GL_RGB8;
  const Texture2DHandle handle = mTexture2Ds.add(Texture2D(bitmap, internalFormat));
  mUploadedTextures.emplace(bitmap.itemID(), handle);
  return handle;
}

TextureCubeMapHandle RenderingEngine::addTextureCubeMap(const TextureCubeMapBitmaps& bitmaps) {
  ZoneScoped;

  const uint32_t maxChannels = std::max({ bitmaps.right->channels(), bitmaps.left->channels(),
                                          bitmaps.top->channels(), bitmaps.bottom->channels(),
                                          bitmaps.front->channels(), bitmaps.back->channels() });

  const GLint internalFormat = bitmaps.bSRGB && maxChannels == 4 ? GL_SRGB8_ALPHA8 :
                               bitmaps.bSRGB && maxChannels < 4  ? GL_SRGB8 :
                              !bitmaps.bSRGB && maxChannels == 4 ? GL_RGBA8 : GL_RGB8;

  return mTextureCubeMaps.add(TextureCubeMap(bitmaps, internalFormat));
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

  mInstanceBuffer.value()->write(instances, first * sizeof(InstanceData));
}

void RenderingEngine::submitRenderPasses(const std::span<RenderPass> renderPasses) {
  ZoneScoped;

  assert(mInitialised);

  for (auto& [viewport, dstFramebuffer, pass] : renderPasses) {
    if (auto* renderScenePass = std::get_if<RenderScenePass>(&pass)) {
      this->waitForBuffers();
      if (renderScenePass->mode == SceneRenderMode::Full) {
        this->renderSceneFull(*renderScenePass, viewport, dstFramebuffer);
      } else {
        this->renderSceneSimple(*renderScenePass, viewport, dstFramebuffer);
      }
      this->lockBuffers();
    } else if (const auto* postProcessingPass = std::get_if<PostProcessingPass>(&pass)) {
      this->postProcess(viewport, postProcessingPass->postProcessingShader, postProcessingPass->srcFramebuffer, dstFramebuffer);
    }
  }
}

void RenderingEngine::renderSceneFull(RenderScenePass& pass, const Viewport& viewport, FramebufferHandle dstFramebuffer) {
  ZoneScoped;
  TracyGpuZone("Render scene");

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

  if (pass.bClearFramebuffer) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }

  const std::span<const Draw> draws = pass.scene->draw(pass.entityCamera, *this);
  if (draws.empty()) {
    return;
  }

  this->updateStorageAndUniformBuffers(pass, dstFramebuffer->size());

  glBindVertexArray(mMeshesVAO);
  mDiffuseTextureSampler.value()->bind(BINDING_SAMPLER_DIFFUSE);
  mSpecularTextureSampler.value()->bind(BINDING_SAMPLER_SPECULAR);
  mEmissionTextureSampler.value()->bind(BINDING_SAMPLER_EMISSION);
  mEnvironmentTextureSampler.value()->bind(BINDING_SAMPLER_ENVIRONMENT);

  const Draw* lastDraw = &draws.front();
  for (size_t drawIdx = 0; drawIdx < draws.size(); drawIdx++) {
    const Draw* currDraw = &draws[drawIdx];

    mInstanceBuffer.value()->bindRange(BINDING_SSBO_INSTANCES,
                                       currDraw->instanceOffset * sizeof(InstanceData),
                                       currDraw->instanceCount * sizeof(InstanceData));

    if (drawIdx == 0 || currDraw->bBackfaceCulling != lastDraw->bBackfaceCulling) [[unlikely]] {
      if (currDraw->bBackfaceCulling) {
        glEnable(GL_CULL_FACE);
      } else {
        glDisable(GL_CULL_FACE);
      }
    }
    if (drawIdx == 0 || currDraw->bDepthTest != lastDraw->bDepthTest) [[unlikely]] {
      if (currDraw->bDepthTest) {
        glEnable(GL_DEPTH_TEST);
      } else {
        glDisable(GL_DEPTH_TEST);
      }
    }
    if (drawIdx == 0 || currDraw->bWriteToDepth != lastDraw->bWriteToDepth) [[unlikely]] {
      if (currDraw->bWriteToDepth) {
        glDepthMask(GL_TRUE);
      } else {
        glDepthMask(GL_FALSE);
      }
    }
    if (drawIdx == 0 || currDraw->bStencilTest != lastDraw->bStencilTest) [[unlikely]] {
      glStencilFunc(currDraw->bStencilTest ? GL_NOTEQUAL : GL_ALWAYS, 1, 0xff);
    }
    if (drawIdx == 0 || currDraw->bWriteToStencil != lastDraw->bWriteToStencil) [[unlikely]] {
      glStencilMask(currDraw->bWriteToStencil ? 0xff : 0x00);
    }
    if (drawIdx == 0 || currDraw->bTransparent != lastDraw->bTransparent) [[unlikely]] {
      if (currDraw->bTransparent) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      } else {
        glDisable(GL_BLEND);
      }
    }

    const ShaderProgramInstance& currShader = currDraw->shaderProgramInstance.get();
    if (drawIdx == 0 || currDraw->shaderProgramInstance.itemID() != lastDraw->shaderProgramInstance.itemID()) {
      const ShaderProgramInstance& lastShader = lastDraw->shaderProgramInstance.get();
      if (drawIdx == 0 || currShader.shaderProgram() != lastShader.shaderProgram()) {
        currShader.use();
      }
      currShader.bindUniforms();
    }

    GLuint slot = 0;
    const auto bindTexture = [&, this](const auto& currTexture, const auto& lastTexture) {
      if (drawIdx == 0 || currTexture != lastTexture) {
        if (currTexture.has_value()) {
          currTexture->get().bind(slot);
          mBoundTextureSlots.insert(slot);
        } else {
          glBindTextureUnit(slot, GL_NONE);
          mBoundTextureSlots.erase(slot);
        }
      }
      ++slot;
    };
    bindTexture(currDraw->diffuseMap, lastDraw->diffuseMap);
    bindTexture(currDraw->specularMap, lastDraw->specularMap);
    bindTexture(currDraw->emissionMap, lastDraw->emissionMap);
    bindTexture(currDraw->environmentMap, lastDraw->environmentMap);

    currDraw->mesh->bind();

    {
      TracyGpuZone("Draw elements instanced");
      glDrawElementsInstanced(GL_TRIANGLES, currDraw->mesh->indexCount(), GL_UNSIGNED_INT,
                              reinterpret_cast<void*>(currDraw->mesh->indicesOffset()),
                              static_cast<GLsizei>(currDraw->instanceCount));
    }

    lastDraw = currDraw;
  }

  dstFramebuffer->resolveMultisample();

  glBindVertexArray(GL_NONE);
  glUseProgram(GL_NONE);
  for (const GLuint slot : mBoundTextureSlots) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
  }
  mBoundTextureSlots.clear();
}

void RenderingEngine::renderSceneSimple(RenderScenePass& pass, const Viewport& viewport, const FramebufferHandle dstFramebuffer) {
  ZoneScoped;
  TracyGpuZone("Render vertex normals");

  assert(mInitialised);

  dstFramebuffer->bind();

  const glm::ivec2 viewportPositionPx = glm::round(viewport.position * glm::vec2(dstFramebuffer->size()));
  const glm::ivec2 viewportSizePx = glm::round(viewport.size * glm::vec2(dstFramebuffer->size()));
  glViewport(viewportPositionPx.x, viewportPositionPx.y, viewportSizePx.x, viewportSizePx.y);

  glEnable(GL_DEPTH_TEST);

  if (pass.bClearFramebuffer) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }

  const std::span<const Draw> draws = pass.scene->draw(pass.entityCamera, *this);
  if (draws.empty()) {
    return;
  }

  this->updateStorageAndUniformBuffers(pass, dstFramebuffer->size());

  glBindVertexArray(mMeshesVAO);

  switch (pass.mode) {
    case SceneRenderMode::Full:
      UNREACHABLE();

    case SceneRenderMode::SurfaceDepth:
      glUseProgram(mSurfaceDepthShaderProgram.value()->id());
      break;

    case SceneRenderMode::SurfaceNormal:
      glUseProgram(mSurfaceNormalShaderProgram.value()->id());
      break;

    case SceneRenderMode::Wireframe:
      glUseProgram(mLightShaderProgram.value()->id());
      glUniform3f(4, 1.0f, 1.0f, 1.0f);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;

    case SceneRenderMode::VertexNormals:
      glUseProgram(mVertexNormalShaderProgram.value()->id());
      break;
  }

  for (const Draw& currDraw : draws) {
    if (!currDraw.bSkybox) {
      mInstanceBuffer.value()->bindRange(BINDING_SSBO_INSTANCES,
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

  if (pass.mode == SceneRenderMode::Wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}

void RenderingEngine::postProcess(const Viewport& viewport, ShaderProgramInstanceHandle postProcessingShader,
                                  FramebufferHandle srcFramebuffer, FramebufferHandle dstFramebuffer) const
{
  ZoneScoped;
  TracyGpuZone("Postprocess");

  assert(mInitialised);

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
  srcFramebuffer->colorAttachments[0].bind(BINDING_SAMPLER_SCREEN);
  mColorAttachmentSampler.value()->bind(BINDING_SAMPLER_SCREEN);

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

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(0, 0, static_cast<GLint>(windowSize.x), static_cast<GLint>(windowSize.y));

  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(mPostProcessCopyShaderProgram.value()->id());
  srcFramebuffer->colorAttachments[0].bind(BINDING_SAMPLER_SCREEN);
  mColorAttachmentSampler.value()->bind(BINDING_SAMPLER_SCREEN);
  glBindVertexArray(mScreenQuadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(GL_NONE);
  glBindVertexArray(GL_NONE);
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void RenderingEngine::waitForBuffers() const {
  ZoneScoped;

  if (mBuffersFence != GL_NONE) {
    GLenum waitResult = GL_UNSIGNALED;
    while (waitResult != GL_ALREADY_SIGNALED && waitResult != GL_CONDITION_SATISFIED) {
      waitResult = glClientWaitSync(mBuffersFence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
    }
  }
}

void RenderingEngine::lockBuffers() {
  ZoneScoped;

  glDeleteSync(mBuffersFence);
  mBuffersFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void RenderingEngine::updateStorageAndUniformBuffers(RenderScenePass& pass, const glm::uvec2 framebufferSize) {
  ZoneScoped;

  const auto [camera, cameraTransform] = pass.scene->ecs.get<const CompCamera, const CompTransform>(pass.entityCamera);
  const CameraUniforms cameraUniformData = CameraUniforms::from(camera, cameraTransform, framebufferSize);
  mCameraUniformBuffer.value()->write(cameraUniformData);
  mCameraUniformBuffer.value()->bindWhole(BINDING_UBO_CAMERA);

  const auto updateLights = [&pass]<typename CompLight>(BufferHandle storageBuffer, const uint32_t bindPoint, const auto getLightUniformData) {
    const entt::basic_view lights = pass.scene->ecs.view<const CompLight>();
    if (lights.begin() == lights.end()) {
      return;
    }

    const auto lightUniformData = getLightUniformData();
    storageBuffer->write(lightUniformData);
    storageBuffer->bindWhole(bindPoint);
    for (auto [entity, light] : lights.each()) {
      if (CompGraphics* graphics = pass.scene->ecs.try_get<CompGraphics>(entity)) {
        for (RenderData& renderData : graphics->renderData) {
          if (renderData.shaderProgramInstance->type() == ShaderProgramType::Light) {
            renderData.shaderProgramInstance->uniforms["uLightColor"] = light.colors.diffuse;
          }
        }
      }
    }
  };

  updateLights.operator()<CompDirectionalLight>(mDirectionalLightsStorageBuffer.value(),
                                                BINDING_SSBO_DIRECTIONAL_LIGHTS,
                                                [&] { return pass.scene->createDirectionalLightUniforms(); });
  updateLights.operator()<CompPointLight>(mPointLightsStorageBuffer.value(),
                                          BINDING_SSBO_POINT_LIGHTS,
                                          [&] { return pass.scene->createPointLightUniforms(); });
  updateLights.operator()<CompSpotlight>(mSpotlightsStorageBuffer.value(),
                                         BINDING_SSBO_SPOTLIGHTS,
                                         [&] { return pass.scene->createSpotlightUniforms(); });
}

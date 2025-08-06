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
#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Dirty.hpp>
#include <Scene/Components/Graphics.hpp>
#include <Graphics/Viewport.hpp>
#include <Scene/Scene.hpp>
#include <Util/Macros/Errors.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <tracy/TracyOpenGL.hpp>

#include <ranges>
#include <Graphics/Buffers/InstanceBuffer.hpp>

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
  Expected grayscale         = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/grayscale.frag"});
  Expected invert            = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/invert.frag"});
  Expected kernel3x3         = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/kernel3x3.frag"});
  Expected flipHorizontally  = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/flipHorizontally.frag"});
  Expected flipVertically    = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/flipVertically.frag"});
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
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessGrayscaleShaderProgram, grayscale);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessInvertShaderProgram, invert);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessKernel3x3ShaderProgram, kernel3x3);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessFlipHorizontallyShaderProgram, flipHorizontally);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessFlipVerticallyShaderProgram, flipVertically);
  ASSIGN_EXPECTED_OR_RETURN(mSkyboxShaderProgram, skybox);

  glCreateVertexArrays(1, &mScreenQuadVAO);
  glCreateVertexArrays(1, &mMeshesVAO);
  Vertex::setupVertexAttributes(mMeshesVAO);

  mColorAttachmentSampler = this->addSampler(SamplerOptions { .minFilter = GL_LINEAR });
  mMeshTextureSampler = this->addSampler(SamplerOptions {});

  mCameraUniformBuffer = this->createBuffer(GL_UNIFORM_BUFFER);
  mCameraUniformBuffer.value()->allocate(sizeof(CameraUniforms));

  mDirectionalLightsStorageBuffer = this->createBuffer(GL_SHADER_STORAGE_BUFFER);
  mPointLightsStorageBuffer = this->createBuffer(GL_SHADER_STORAGE_BUFFER);
  mSpotlightsStorageBuffer = this->createBuffer(GL_SHADER_STORAGE_BUFFER);
  mInstanceBuffer = this->createBuffer(GL_SHADER_STORAGE_BUFFER);

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

  switch (type) {
    case ShaderProgramType::LitSurface:
      return this->addShaderProgramInstance(ShaderProgramInstance::newLitSurface(mLitSurfaceShaderProgram.value()));
    case ShaderProgramType::LitExploded:
      return this->addShaderProgramInstance(ShaderProgramInstance::newLitExploded(mLitExplodedShaderProgram.value()));
    case ShaderProgramType::Light:
      return this->addShaderProgramInstance(ShaderProgramInstance::newLight(mLightShaderProgram.value()));
    case ShaderProgramType::Outline:
      return this->addShaderProgramInstance(ShaderProgramInstance::newOutline(mOutlineShaderProgram.value()));
    case ShaderProgramType::ReflectiveSurface:
      return this->addShaderProgramInstance(ShaderProgramInstance::newReflectiveSurface(mReflectiveSurfaceShaderProgram.value()));
    case ShaderProgramType::RefractiveSurface:
      return this->addShaderProgramInstance(ShaderProgramInstance::newRefractiveSurface(mRefractiveSurfaceShaderProgram.value()));
    case ShaderProgramType::PostProcessCopy:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingCopy(mPostProcessCopyShaderProgram.value()));
    case ShaderProgramType::PostProcessBlur:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingBlur(mPostProcessKernel3x3ShaderProgram.value()));
    case ShaderProgramType::PostProcessEdgeDetection:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingEdgeDetection(mPostProcessKernel3x3ShaderProgram.value()));
    case ShaderProgramType::PostProcessEmboss:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingEmboss(mPostProcessKernel3x3ShaderProgram.value()));
    case ShaderProgramType::PostProcessFlipHorizontally:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingFlipHorizontally(mPostProcessFlipHorizontallyShaderProgram.value()));
    case ShaderProgramType::PostProcessFlipVertically:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingFlipVertically(mPostProcessFlipVerticallyShaderProgram.value()));
    case ShaderProgramType::PostProcessGrayscale:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingGrayscale(mPostProcessGrayscaleShaderProgram.value()));
    case ShaderProgramType::PostProcessInvert:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingInvert(mPostProcessInvertShaderProgram.value()));
    case ShaderProgramType::PostProcessSharpen:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingSharpen(mPostProcessKernel3x3ShaderProgram.value()));
    case ShaderProgramType::PostProcessSobelBottom:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingSobelBottom(mPostProcessKernel3x3ShaderProgram.value()));
    case ShaderProgramType::PostProcessSobelLeft:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingSobelLeft(mPostProcessKernel3x3ShaderProgram.value()));
    case ShaderProgramType::PostProcessSobelRight:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingSobelRight(mPostProcessKernel3x3ShaderProgram.value()));
    case ShaderProgramType::PostProcessSobelTop:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingSobelTop(mPostProcessKernel3x3ShaderProgram.value()));
    case ShaderProgramType::Skybox:
      return this->addShaderProgramInstance(ShaderProgramInstance::newSkybox(mSkyboxShaderProgram.value()));
    case ShaderProgramType::SurfaceDepth:
      return this->addShaderProgramInstance(ShaderProgramInstance::newSurfaceDepth(mSurfaceDepthShaderProgram.value()));
    case ShaderProgramType::SurfaceNormal:
      return this->addShaderProgramInstance(ShaderProgramInstance::newSurfaceNormal(mSurfaceNormalShaderProgram.value()));
    default:
      UNREACHABLE();
  }
}

ShaderProgramInstanceHandle RenderingEngine::addShaderProgramInstance(ShaderProgramInstance&& instance) {
  ZoneScoped;

  return mShaderProgramInstances.add(std::forward<ShaderProgramInstance>(instance));
}

const std::vector<RenderData>& RenderingEngine::addModel(AssetHandle<Model> model, ShaderProgramInstanceHandle initialShader) {
  ZoneScoped;

  if (mUploadedModels.contains(model.itemID())) {
    return mUploadedModels.at(model.itemID());
  }

  const std::vector<MeshHandle> meshes = this->addMeshes(model->meshes);
  const std::vector<std::optional<SamplerHandle>> diffuseSamplers(model->diffuseMaps.size(), mMeshTextureSampler);
  const std::vector<std::optional<SamplerHandle>> specularSamplers(model->specularMaps.size(), mMeshTextureSampler);
  const std::vector<std::optional<SamplerHandle>> emissionSamplers(model->emissionMaps.size(), mMeshTextureSampler);
  const std::vector<std::optional<Texture2DHandle>> diffuseMaps = this->addTexture2Ds(model->diffuseMaps, diffuseSamplers);
  const std::vector<std::optional<Texture2DHandle>> specularMaps = this->addTexture2Ds(model->specularMaps, specularSamplers);
  const std::vector<std::optional<Texture2DHandle>> emissionMaps = this->addTexture2Ds(model->emissionMaps, emissionSamplers);

  std::vector<RenderData> modelResources;
  modelResources.reserve(meshes.size());
  for (size_t meshRes = 0; meshRes < meshes.size(); ++meshRes) {
    RenderData resources { .mesh = meshes[meshRes], .shaderProgramInstance = initialShader };
    resources.diffuseMap = diffuseMaps[meshRes];
    resources.specularMap = specularMaps[meshRes];
    resources.emissionMap = emissionMaps[meshRes];
    modelResources.push_back(resources);
  }

  const auto [it, inserted] = mUploadedModels.emplace(model.itemID(), std::move(modelResources));
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

  return mMeshes.add(Mesh(meshData.get()));
}

SamplerHandle RenderingEngine::addSampler(const SamplerOptions& options) {
  ZoneScoped;

  return mSamplers.add(Sampler(options));
}

auto RenderingEngine::addTexture2Ds(const std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps,
                                    const std::span<const std::optional<SamplerHandle>> samplers)
  -> std::vector<std::optional<Texture2DHandle>>
{
  ZoneScoped;

  assert(bitmaps.size() == samplers.size());

  std::vector<std::optional<Texture2DHandle>> handles;
  handles.reserve(bitmaps.size());
  for (size_t i = 0; const std::optional bitmap : bitmaps) {
    if (bitmap.has_value()) {
      const SamplerHandle sampler = samplers[i].value();
      const Texture2DHandle ref = this->addTexture2D(bitmap.value(), sampler);
      handles.emplace_back(ref);
    } else {
      handles.emplace_back();
    }
    ++i;
  }
  return handles;
}

Texture2DHandle RenderingEngine::addTexture2D(const AssetHandle<Bitmap> bitmap, const SamplerHandle sampler) {
  ZoneScoped;

  if (mUploadedTextures.contains(bitmap.itemID())) {
    return mUploadedTextures.at(bitmap.itemID());
  }

  const Texture2DHandle handle = mTexture2Ds.add(Texture2D(bitmap, sampler));
  mUploadedTextures.emplace(bitmap.itemID(), handle);
  return handle;
}

TextureCubeMapHandle RenderingEngine::addTextureCubeMap(const TextureCubeMapBitmaps& bitmaps, const SamplerHandle sampler) {
  ZoneScoped;

  return mTextureCubeMaps.add(TextureCubeMap(bitmaps, sampler));
}

FramebufferHandle RenderingEngine::addFramebuffer(const FramebufferCreateInfo& info) {
  ZoneScoped;

  return mFramebuffers.add(Framebuffer(info, mColorAttachmentSampler.value()));
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
      this->renderScene(*renderScenePass->scene, renderScenePass->entityCamera, viewport, dstFramebuffer);
      if (bVisualiseVertexNormals) {
        this->renderVertexNormals(*renderScenePass->scene, renderScenePass->entityCamera, viewport, dstFramebuffer);
      }
    } else if (const auto* postProcessingPass = std::get_if<PostProcessingPass>(&pass)) {
      this->postProcess(viewport, postProcessingPass->postProcessingShader, postProcessingPass->srcFramebuffer, dstFramebuffer);
    }
  }
}

void RenderingEngine::renderScene(Scene& scene, const entt::entity entityCamera, const Viewport& viewport, FramebufferHandle dstFramebuffer) {
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

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  const std::span<const Draw> draws = scene.draw(entityCamera, *this);
  if (draws.empty()) {
    return;
  }

  if (sceneRenderMode == SceneRenderMode::Wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  switch (sceneRenderMode) {
    case SceneRenderMode::Normal:
    case SceneRenderMode::Wireframe:
      break;

    case SceneRenderMode::SurfaceDepth:
      glUseProgram(mSurfaceDepthShaderProgram.value()->id());
      break;

    case SceneRenderMode::SurfaceNormal:
      glUseProgram(mSurfaceNormalShaderProgram.value()->id());
      break;
  }

  const auto [camera, cameraTransform] = scene.ecs.get<const CompCamera, const CompTransform>(entityCamera);
  const CameraUniforms cameraUniformData = CameraUniforms::from(camera, cameraTransform, dstFramebuffer->size());
  mCameraUniformBuffer.value()->write(cameraUniformData);
  mCameraUniformBuffer.value()->bindWhole(BINDING_UBO_CAMERA);

  const auto updateLights = [&scene]<typename CompLight>(BufferHandle storageBuffer, const uint32_t bindPoint, const auto getLightUniformData) {
    const entt::basic_view dirtyLights = scene.ecs.view<const CompLight, const CompDirty>();
    if (dirtyLights.begin() == dirtyLights.end()) {
      return;
    }

    const auto lightUniformData = getLightUniformData();
    storageBuffer->write(lightUniformData);
    storageBuffer->bindWhole(bindPoint);
    for (auto [entity, light] : dirtyLights.each()) {
      if (CompGraphics* graphics = scene.ecs.try_get<CompGraphics>(entity)) {
        for (RenderData& renderData : graphics->renderData) {
          if (renderData.shaderProgramInstance->type() == ShaderProgramType::Light) {
            renderData.shaderProgramInstance->uniforms["uLightColor"] = light.colors.diffuse;
          }
        }
      }
    }
    scene.ecs.erase<CompDirty>(dirtyLights.begin(), dirtyLights.end());
  };

  updateLights.operator()<CompDirectionalLight>(mDirectionalLightsStorageBuffer.value(),
                                                BINDING_SSBO_DIRECTIONAL_LIGHTS,
                                                [&] { return scene.createDirectionalLightUniforms(); });
  updateLights.operator()<CompPointLight>(mPointLightsStorageBuffer.value(),
                                          BINDING_SSBO_POINT_LIGHTS,
                                          [&] { return scene.createPointLightUniforms(); });
  updateLights.operator()<CompSpotlight>(mSpotlightsStorageBuffer.value(),
                                         BINDING_SSBO_SPOTLIGHTS,
                                         [&] { return scene.createSpotlightUniforms(); });

  glBindVertexArray(mMeshesVAO);

  const Draw* lastDraw = &draws.front();
  for (size_t drawIdx = 0; drawIdx < draws.size(); drawIdx++) {
    const Draw* currDraw = &draws[drawIdx];

    if (currDraw->bSkybox) {
      switch (sceneRenderMode) {
        case SceneRenderMode::Normal:
        case SceneRenderMode::Wireframe:
          break;

        case SceneRenderMode::SurfaceDepth:
        case SceneRenderMode::SurfaceNormal:
          continue;
      }
    }

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
    if ((sceneRenderMode == SceneRenderMode::Normal || sceneRenderMode == SceneRenderMode::Wireframe) &&
        (drawIdx == 0 || currDraw->shaderProgramInstance.itemID() != lastDraw->shaderProgramInstance.itemID()))
    {
      const ShaderProgramInstance& lastShader = lastDraw->shaderProgramInstance.get();
      if (drawIdx == 0 || currShader.shaderProgram() != lastShader.shaderProgram()) {
        currShader.use();
      }
      currShader.bindUniforms();
    }

    GLuint slot = 0;
    const auto bindTexture = [&, this](const auto& currTexture, const auto& lastTexture, const GLenum target) {
      if (drawIdx == 0 || currTexture != lastTexture) {
        if (currTexture.has_value()) {
          currTexture->get().bind(slot);
          mBoundTextureSlots.insert(slot);
        } else {
          glActiveTexture(GL_TEXTURE0 + slot);
          glBindTexture(target, 0);
          mBoundTextureSlots.erase(slot);
        }
      }
      ++slot;
    };
    bindTexture(currDraw->diffuseMap, lastDraw->diffuseMap, GL_TEXTURE_2D);
    bindTexture(currDraw->specularMap, lastDraw->specularMap, GL_TEXTURE_2D);
    bindTexture(currDraw->emissionMap, lastDraw->emissionMap, GL_TEXTURE_2D);
    bindTexture(currDraw->environmentMap, lastDraw->environmentMap, GL_TEXTURE_CUBE_MAP);

    {
      TracyGpuZone("Draw elements instanced");
      currDraw->mesh->bind();
      glDrawElementsInstanced(GL_TRIANGLES, currDraw->mesh->indexCount, GL_UNSIGNED_INT, nullptr,
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

  if (sceneRenderMode == SceneRenderMode::Wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}

void RenderingEngine::renderVertexNormals(Scene& scene, entt::entity entityCamera, const Viewport& viewport, FramebufferHandle dstFramebuffer) {
  ZoneScoped;
  TracyGpuZone("Render vertex normals");

  assert(mInitialised);

  dstFramebuffer->bind();

  const glm::ivec2 viewportPositionPx = glm::round(viewport.position * glm::vec2(dstFramebuffer->size()));
  const glm::ivec2 viewportSizePx = glm::round(viewport.size * glm::vec2(dstFramebuffer->size()));
  glViewport(viewportPositionPx.x, viewportPositionPx.y, viewportSizePx.x, viewportSizePx.y);

  glEnable(GL_DEPTH_TEST);

  const std::span<const Draw> draws = scene.draw(entityCamera, *this);
  if (draws.empty()) {
    return;
  }

  glBindVertexArray(mMeshesVAO);
  glUseProgram(mVertexNormalShaderProgram.value()->id());

  for (const Draw& currDraw : draws) {
    if (!currDraw.bSkybox) {
      TracyGpuZone("Draw elements instanced");
      mInstanceBuffer.value()->bindRange(BINDING_SSBO_INSTANCES,
                                         currDraw.instanceOffset * sizeof(InstanceData),
                                         currDraw.instanceCount * sizeof(InstanceData));
      currDraw.mesh->bind();
      glDrawElementsInstanced(GL_TRIANGLES, currDraw.mesh->indexCount, GL_UNSIGNED_INT, nullptr,
                              static_cast<GLsizei>(currDraw.instanceCount));
    }
  }

  dstFramebuffer->resolveMultisample();

  glBindVertexArray(GL_NONE);
  glUseProgram(GL_NONE);
}

void RenderingEngine::postProcess(const Viewport& viewport, ShaderProgramInstanceHandle postProcessingShader,
                                  FramebufferHandle srcFramebuffer, FramebufferHandle dstFramebuffer) const
{
  ZoneScoped;
  TracyGpuZone("postProcess");

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
  srcFramebuffer->colorAttachment.bind(0);

  glBindVertexArray(mScreenQuadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  dstFramebuffer->resolveMultisample();

  glUseProgram(GL_NONE);
  glBindVertexArray(GL_NONE);
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void RenderingEngine::present(const glm::uvec2 windowSize, FramebufferHandle srcFramebuffer) const {
  ZoneScoped;
  TracyGpuZone("present");

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
  srcFramebuffer->colorAttachment.bind(0);
  glBindVertexArray(mScreenQuadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(GL_NONE);
  glBindVertexArray(GL_NONE);
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

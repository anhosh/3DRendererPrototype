#include <Graphics/RenderingEngine.hpp>

#include <Assets/Mesh.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/RenderData.hpp>
#include <Graphics/RenderPass.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/Buffers/BindPoints.hpp>
#include <Graphics/Buffers/CameraUniforms.hpp>
#include <Graphics/Buffers/LightSourceUniforms.hpp>
#include <Graphics/Viewport.hpp>
#include <Util/Macros/Errors.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <tracy/TracyOpenGL.hpp>

#include <ranges>
#include <Graphics/Components/Dirty.hpp>
#include <Graphics/Components/Graphics.hpp>

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

  glGenVertexArrays(1, &mScreenQuadVAO);

  mCameraUniformBuffer = this->createBuffer(GL_UNIFORM_BUFFER);
  mCameraUniformBuffer.value()->allocate(sizeof(CameraUniforms));

  mDirectionalLightsStorageBuffer = this->createBuffer(GL_SHADER_STORAGE_BUFFER);
  mPointLightsStorageBuffer = this->createBuffer(GL_SHADER_STORAGE_BUFFER);
  mSpotlightsStorageBuffer = this->createBuffer(GL_SHADER_STORAGE_BUFFER);

  mInitialised = true;
  return {};
}

void RenderingEngine::destroy() {
  ZoneScoped;

  for (ShaderProgram& shaderProgram : std::ranges::views::values(mShaderPrograms)) {
    shaderProgram.destroy();
  }
  for (VertexArray& vertexArray : std::ranges::views::values(mVertexArrays)) {
    vertexArray.destroy();
  }
  for (Texture2D& texture2D : std::ranges::views::values(mTexture2Ds)) {
    texture2D.destroy();
  }
  for (TextureCubeMap& textureCubeMap : std::ranges::views::values(mTextureCubeMaps)) {
    textureCubeMap.destroy();
  }
  for (Framebuffer& framebuffer : std::ranges::views::values(mFramebuffers)) {
    framebuffer.destroy();
  }
  for (Buffer& buffer : std::ranges::views::values(mBuffers)) {
    buffer.destroy();
  }

  mShaderPrograms.clear();
  mShaderProgramInstances.clear();
  mVertexArrays.clear();
  mTexture2Ds.clear();
  mTextureCubeMaps.clear();
  mFramebuffers.clear();
  mBuffers.clear();

  glDeleteVertexArrays(1, &mScreenQuadVAO);
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

  std::vector<SamplerOptions> diffuseSamplers;
  std::vector<SamplerOptions> specularSamplers;
  std::vector<SamplerOptions> emissionSamplers;
  diffuseSamplers.resize(model->diffuseMaps.size());
  specularSamplers.resize(model->specularMaps.size());
  emissionSamplers.resize(model->emissionMaps.size());

  const std::vector<VertexArrayHandle> vertexArrays = this->addMeshes(model->meshes);
  const std::vector<std::optional<Texture2DHandle>> diffuseMaps = this->addTexture2Ds(model->diffuseMaps, diffuseSamplers);
  const std::vector<std::optional<Texture2DHandle>> specularMaps = this->addTexture2Ds(model->specularMaps, specularSamplers);
  const std::vector<std::optional<Texture2DHandle>> emissionMaps = this->addTexture2Ds(model->emissionMaps, emissionSamplers);

  std::vector<RenderData> modelResources;
  modelResources.reserve(vertexArrays.size());
  for (size_t meshRes = 0; meshRes < vertexArrays.size(); ++meshRes) {
    RenderData resources { .vertexArray = vertexArrays[meshRes], .shaderProgramInstance = initialShader };
    resources.diffuseMap = diffuseMaps[meshRes];
    resources.specularMap = specularMaps[meshRes];
    resources.emissionMap = emissionMaps[meshRes];
    modelResources.push_back(resources);
  }

  const auto [it, inserted] = mUploadedModels.emplace(model.itemID(), std::move(modelResources));
  const auto& [index, resources] = *it;
  return resources;
}

std::vector<VertexArrayHandle> RenderingEngine::addMeshes(std::span<const AssetHandle<Mesh>> meshes) {
  ZoneScoped;

  std::vector<VertexArrayHandle> refs;
  refs.reserve(meshes.size());
  for (const AssetHandle<Mesh>& mesh : meshes) {
    const VertexArrayHandle ref = this->addMesh(mesh);
    refs.push_back(ref);
  }
  return refs;
}

VertexArrayHandle RenderingEngine::addMesh(AssetHandle<Mesh> mesh) {
  ZoneScoped;

  return mVertexArrays.add(VertexArray(mesh.get()));
}

auto RenderingEngine::addTexture2Ds(std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps, std::span<const SamplerOptions> options)
  -> std::vector<std::optional<Texture2DHandle>>
{
  ZoneScoped;

  assert(bitmaps.size() == options.size());

  std::vector<std::optional<Texture2DHandle>> handles;
  handles.reserve(bitmaps.size());
  for (size_t i = 0; i < bitmaps.size(); ++i) {
    if (const std::optional<AssetHandle<Bitmap>> bitmap = bitmaps[i]; bitmap.has_value()) {
      const SamplerOptions& option = options[i];
      const Texture2DHandle ref = this->addTexture2D(bitmap.value(), option);
      handles.emplace_back(ref);
    } else {
      handles.emplace_back();
    }
  }
  return handles;
}

Texture2DHandle RenderingEngine::addTexture2D(AssetHandle<Bitmap> bitmap, const SamplerOptions& options) {
  ZoneScoped;

  if (mUploadedTextures.contains(bitmap.itemID())) {
    return mUploadedTextures.at(bitmap.itemID());
  }

  const Texture2DHandle handle = mTexture2Ds.add(Texture2D(bitmap, options));
  mUploadedTextures.emplace(bitmap.itemID(), handle);
  return handle;
}

TextureCubeMapHandle RenderingEngine::addTextureCubeMap(const TextureCubeMapBitmaps& bitmaps, const SamplerOptions& options) {
  ZoneScoped;

  return mTextureCubeMaps.add(TextureCubeMap(bitmaps, options));
}

FramebufferHandle RenderingEngine::addFramebuffer(const FramebufferCreateInfo& info) {
  ZoneScoped;

  return mFramebuffers.add(Framebuffer(info));
}

BufferHandle RenderingEngine::createBuffer(const GLenum type) {
  ZoneScoped;

  return mBuffers.add(Buffer(type));
}

void RenderingEngine::submitRenderPasses(const std::span<RenderPass> renderPasses) {
  ZoneScoped;

  assert(mInitialised);

  for (auto& [viewport, dstFramebuffer, pass] : renderPasses) {
    if (auto* renderScenePass = std::get_if<RenderScenePass>(&pass)) {
      this->renderScene(*renderScenePass->scene, *renderScenePass->camera, viewport, dstFramebuffer);
      if (bVisualiseVertexNormals) {
        this->renderVertexNormals(*renderScenePass->scene, *renderScenePass->camera, viewport, dstFramebuffer);
      }
    } else if (const auto* postProcessingPass = std::get_if<PostProcessingPass>(&pass)) {
      this->postProcess(viewport, postProcessingPass->postProcessingShader, postProcessingPass->srcFramebuffer, dstFramebuffer);
    }
  }
}

void RenderingEngine::renderScene(Scene& scene, const Camera& camera, const Viewport& viewport, FramebufferHandle dstFramebuffer) {
  ZoneScoped;
  TracyGpuZone("renderScene");

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

  const std::span<const Draw> draws = scene.draw(camera, mBuffers);
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

  const CameraUniforms cameraUniformData = CameraUniforms::from(camera, dstFramebuffer->size());
  mCameraUniformBuffer.value()->write(cameraUniformData);
  mCameraUniformBuffer.value()->bindWhole(UBO_BIND_POINT_CAMERA);

  const auto updateLights = [&scene]<typename CompLight>(BufferHandle instanceBuffer, const uint32_t bindPoint, const auto getLightUniformData) {
    const entt::basic_view dirtyLights = scene.ecs.view<const CompLight, const CompDirty>();
    if (dirtyLights.begin() != dirtyLights.end()) {
      const auto lightUniformData = getLightUniformData();
      instanceBuffer->write(lightUniformData);
      instanceBuffer->bindWhole(bindPoint);
      for (auto [entity, light] : dirtyLights.each()) {
        if (CompGraphics* graphics = scene.ecs.try_get<CompGraphics>(entity)) {
          for (RenderData& rd : graphics->renderData) {
            if (rd.shaderProgramInstance->type() == ShaderProgramType::Light) {
              rd.shaderProgramInstance->uniforms["uLightColor"] = light.colors.diffuse;
            }
          }
        }
      }
      scene.ecs.erase<CompDirty>(dirtyLights.begin(), dirtyLights.end());
    }
  };

  updateLights.operator()<CompDirectionalLight>(mDirectionalLightsStorageBuffer.value(), SSBO_BIND_POINT_DIRECTIONAL_LIGHTS, [&] { return scene.createDirectionalLightUniforms(); });
  updateLights.operator()<CompPointLight>(mPointLightsStorageBuffer.value(), SSBO_BIND_POINT_POINT_LIGHTS, [&] { return scene.createPointLightUniforms(); });
  updateLights.operator()<CompSpotlight>(mSpotlightsStorageBuffer.value(), SSBO_BIND_POINT_SPOTLIGHTS, [&] { return scene.createSpotlightUniforms(); });

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

    if (drawIdx == 0 || currDraw->bBackfaceCulling != lastDraw->bBackfaceCulling) {
      if (currDraw->bBackfaceCulling) {
        glEnable(GL_CULL_FACE);
      } else {
        glDisable(GL_CULL_FACE);
      }
    }
    if (drawIdx == 0 || currDraw->bDepthTest != lastDraw->bDepthTest) {
      if (currDraw->bDepthTest) {
        glEnable(GL_DEPTH_TEST);
      } else {
        glDisable(GL_DEPTH_TEST);
      }
    }
    if (drawIdx == 0 || currDraw->bWriteToDepth != lastDraw->bWriteToDepth) {
      if (currDraw->bWriteToDepth) {
        glDepthMask(GL_TRUE);
      } else {
        glDepthMask(GL_FALSE);
      }
    }
    if (drawIdx == 0 || currDraw->bStencilTest != lastDraw->bStencilTest) {
      glStencilFunc(currDraw->bStencilTest ? GL_NOTEQUAL : GL_ALWAYS, 1, 0xff);
    }
    if (drawIdx == 0 || currDraw->bWriteToStencil != lastDraw->bWriteToStencil) {
      glStencilMask(currDraw->bWriteToStencil ? 0xff : 0x00);
    }
    if (drawIdx == 0 || currDraw->bTransparent != lastDraw->bTransparent) {
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

    GLuint slot = GL_TEXTURE0;
    const auto bindTexture = [&, this](const auto& currTexture, const auto& lastTexture, const GLenum target) {
      if (drawIdx == 0 || currTexture != lastTexture) {
        if (currTexture.has_value()) {
          currTexture->get().bind(slot);
          mBoundTextureSlots.insert(slot);
        } else {
          glActiveTexture(slot);
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

    glBindVertexArray(currDraw->vertexArray->vao);
    glDrawElementsInstanced(GL_TRIANGLES, currDraw->vertexArray->indexCount, GL_UNSIGNED_INT, nullptr,
                            static_cast<GLsizei>(currDraw->instanceCount));

    lastDraw = currDraw;
  }

  glBindVertexArray(GL_NONE);
  glUseProgram(GL_NONE);
  for (const GLuint slot : mBoundTextureSlots) {
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
  }
  mBoundTextureSlots.clear();

  if (sceneRenderMode == SceneRenderMode::Wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}

void RenderingEngine::renderVertexNormals(Scene& scene, const Camera& camera, const Viewport& viewport, FramebufferHandle dstFramebuffer) {
  ZoneScoped;
  TracyGpuZone("renderVertexNormals");

  assert(mInitialised);

  dstFramebuffer->bind();

  const glm::ivec2 viewportPositionPx = glm::round(viewport.position * glm::vec2(dstFramebuffer->size()));
  const glm::ivec2 viewportSizePx = glm::round(viewport.size * glm::vec2(dstFramebuffer->size()));
  glViewport(viewportPositionPx.x, viewportPositionPx.y, viewportSizePx.x, viewportSizePx.y);

  glEnable(GL_DEPTH_TEST);

  const std::span<const Draw> draws = scene.draw(camera, mBuffers);
  if (draws.empty()) {
    return;
  }

  glUseProgram(mVertexNormalShaderProgram.value()->id());

  const Draw* lastDraw = &draws.front();
  for (size_t drawIdx = 0; drawIdx < draws.size(); drawIdx++) {
    const Draw* currDraw = &draws[drawIdx];

    if (currDraw->bSkybox) {
      continue;
    }

    const VertexArray& currVA = currDraw->vertexArray.get();
    if (drawIdx == 0 || currDraw->vertexArray != lastDraw->vertexArray) {
      glBindVertexArray(currVA.vao);
    }
    glDrawElements(GL_TRIANGLES, currVA.indexCount, GL_UNSIGNED_INT, nullptr);

    lastDraw = currDraw;
  }

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
  srcFramebuffer->colorAttachment.bind(GL_TEXTURE0);

  glBindVertexArray(mScreenQuadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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

  glUseProgram((*mPostProcessCopyShaderProgram)->id());
  glUniform1i(0, 0); // bind uScreenTexture sampler
  glActiveTexture(GL_TEXTURE0);
  srcFramebuffer->colorAttachment.bind();
  glBindVertexArray(mScreenQuadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(GL_NONE);
  glBindVertexArray(GL_NONE);
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

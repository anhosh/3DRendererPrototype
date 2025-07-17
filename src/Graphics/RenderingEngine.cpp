#include <Graphics/RenderingEngine.hpp>

#include <Assets/Mesh.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Draw.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/RenderData.hpp>
#include <Graphics/RenderPass.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/Viewport.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <ranges>

Expected<void> RenderingEngine::init() {
  bInitialised = true; // Set this flag temporarily to let the shader program creation pass its assertion.
  Expected litSurface       = this->createShaderProgram({.vertex = "simple.vert",     .fragment = "litSurface.frag"});
  Expected light            = this->createShaderProgram({.vertex = "simple.vert",     .fragment = "light.frag"});
  Expected visualiseDepth   = this->createShaderProgram({.vertex = "simple.vert",     .fragment = "visualiseDepth.frag"});
  Expected visualiseNormal  = this->createShaderProgram({.vertex = "simple.vert",     .fragment = "visualiseNormal.frag"});
  Expected outline          = this->createShaderProgram({.vertex = "simple.vert",     .fragment = "outline.frag"});
  Expected copy             = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/copy.frag"});
  Expected grayscale        = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/grayscale.frag"});
  Expected invert           = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/invert.frag"});
  Expected kernel3x3        = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/kernel3x3.frag"});
  Expected flipHorizontally = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/flipHorizontally.frag"});
  Expected flipVertically   = this->createShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/flipVertically.frag"});

  bInitialised = false; // Reset because don't yet know if any of the shader program creations have failed or not.
  ASSIGN_EXPECTED_OR_RETURN(mLitSurfaceShaderProgram, litSurface);
  ASSIGN_EXPECTED_OR_RETURN(mLightShaderProgram, light);
  ASSIGN_EXPECTED_OR_RETURN(mVisualiseDepthShaderProgram, visualiseDepth);
  ASSIGN_EXPECTED_OR_RETURN(mVisualiseNormalShaderProgram, visualiseNormal);
  ASSIGN_EXPECTED_OR_RETURN(mOutlineShaderProgram, outline);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessCopyShaderProgram, copy);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessGrayscaleShaderProgram, grayscale);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessInvertShaderProgram, invert);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessKernel3x3ShaderProgram, kernel3x3);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessFlipHorizontallyShaderProgram, flipHorizontally);
  ASSIGN_EXPECTED_OR_RETURN(mPostProcessFlipVerticallyShaderProgram, flipVertically);

  glGenVertexArrays(1, &mScreenQuadVAO);

  bInitialised = true;
  return {};
}

void RenderingEngine::destroy() {
  for (ShaderProgram& shaderProgram : std::ranges::views::values(mShaderPrograms)) {
    shaderProgram.destroy();
  }
  for (VertexArray& vertexArray : std::ranges::views::values(mVertexArrays)) {
    vertexArray.destroy();
  }
  for (Texture& texture : std::ranges::views::values(mTextures)) {
    texture.destroy();
  }
  for (Framebuffer& framebuffer : std::ranges::views::values(mFramebuffers)) {
    framebuffer.destroy();
  }

  mShaderProgramInstances.clear();
  mVertexArrays.clear();
  mTextures.clear();
  mFramebuffers.clear();

  glDeleteVertexArrays(1, &mScreenQuadVAO);
  mScreenQuadVAO = GL_NONE;

  mLitSurfaceShaderProgram.reset();
  mLightShaderProgram.reset();
  mOutlineShaderProgram.reset();
  mVisualiseDepthShaderProgram.reset();
  mVisualiseNormalShaderProgram.reset();
  mPostProcessCopyShaderProgram.reset();
  mPostProcessFlipHorizontallyShaderProgram.reset();
  mPostProcessFlipVerticallyShaderProgram.reset();
  mPostProcessGrayscaleShaderProgram.reset();
  mPostProcessInvertShaderProgram.reset();
  mPostProcessKernel3x3ShaderProgram.reset();

  mUploadedTextures.clear();
  mUploadedModels.clear();

  mBoundTextureSlots.clear();

  bInitialised = false;
}

Expected<ShaderProgramHandle> RenderingEngine::createShaderProgram(const ShaderProgramPaths& shaderPaths) {
  assert(bInitialised);

  Expected shaderProgram = ShaderPrograms::fromShaders(shaderPaths);
  RETURN_ERROR_IF_UNEXPECTED(shaderProgram);
  return mShaderPrograms.add(std::move(shaderProgram.value()));
}

ShaderProgramHandle RenderingEngine::addShaderProgram(ShaderProgram&& shaderProgram) {
  assert(bInitialised);

  return mShaderPrograms.add(std::forward<ShaderProgram>(shaderProgram));
}

ShaderProgramInstanceHandle RenderingEngine::createShaderProgramInstance(const ShaderProgramType type) {
  assert(bInitialised);

  switch (type) {
    case ShaderProgramType::LitSurface:
      return this->addShaderProgramInstance(ShaderProgramInstance::newLitSurface(mLitSurfaceShaderProgram.value()));
    case ShaderProgramType::Light:
      return this->addShaderProgramInstance(ShaderProgramInstance::newLight(mLightShaderProgram.value()));
    case ShaderProgramType::Outline:
      return this->addShaderProgramInstance(ShaderProgramInstance::newOutline(mOutlineShaderProgram.value()));
    case ShaderProgramType::VisualiseDepth:
      return this->addShaderProgramInstance(ShaderProgramInstance::newVisualiseDepth(mVisualiseDepthShaderProgram.value()));
    case ShaderProgramType::VisualiseNormal:
      return this->addShaderProgramInstance(ShaderProgramInstance::newVisualiseNormal(mVisualiseNormalShaderProgram.value()));
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
    default:
      PANIC("Unsupported shader program type");
  }
}

ShaderProgramInstanceHandle RenderingEngine::addShaderProgramInstance(ShaderProgramInstance&& instance) {
  assert(bInitialised);

  return mShaderProgramInstances.add(std::forward<ShaderProgramInstance>(instance));
}

const std::vector<RenderData>& RenderingEngine::addModel(AssetHandle<Model> model, ShaderProgramInstanceHandle initialShader) {
  assert(bInitialised);

  if (mUploadedModels.contains(model.id())) {
    return mUploadedModels.at(model.id());
  }

  std::vector<SamplerOptions> diffuseSamplers;
  std::vector<SamplerOptions> specularSamplers;
  std::vector<SamplerOptions> emissionSamplers;
  diffuseSamplers.resize(model->diffuseMaps.size());
  specularSamplers.resize(model->specularMaps.size());
  emissionSamplers.resize(model->emissionMaps.size());

  const std::vector<VertexArrayHandle> vertexArrays = this->addMeshes(model->meshes);
  const std::vector<std::optional<TextureHandle>> diffuseMaps = this->addTextures(model->diffuseMaps, diffuseSamplers);
  const std::vector<std::optional<TextureHandle>> specularMaps = this->addTextures(model->specularMaps, specularSamplers);
  const std::vector<std::optional<TextureHandle>> emissionMaps = this->addTextures(model->emissionMaps, emissionSamplers);

  std::vector<RenderData> modelResources;
  modelResources.reserve(vertexArrays.size());
  for (size_t meshRes = 0; meshRes < vertexArrays.size(); ++meshRes) {
    RenderData resources { .vertexArray = vertexArrays[meshRes], .shaderProgramInstance = initialShader };
    resources.diffuseMap = diffuseMaps[meshRes];
    resources.specularMap = specularMaps[meshRes];
    resources.emissionMap = emissionMaps[meshRes];
    modelResources.push_back(resources);
  }

  const auto [it, inserted] = mUploadedModels.emplace(model.id(), std::move(modelResources));
  const auto& [index, resources] = *it;
  return resources;
}

std::vector<VertexArrayHandle> RenderingEngine::addMeshes(std::span<const AssetHandle<Mesh>> meshes) {
  assert(bInitialised);

  std::vector<VertexArrayHandle> refs;
  refs.reserve(meshes.size());
  for (const AssetHandle<Mesh>& mesh : meshes) {
    const VertexArrayHandle ref = this->addMesh(mesh);
    refs.push_back(ref);
  }
  return refs;
}

VertexArrayHandle RenderingEngine::addMesh(AssetHandle<Mesh> mesh) {
  assert(bInitialised);

  return mVertexArrays.add(VertexArray(mesh.get()));
}

auto RenderingEngine::addTextures(std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps, std::span<const SamplerOptions> options)
  -> std::vector<std::optional<TextureHandle>>
{
  assert(bInitialised);
  assert(bitmaps.size() == options.size());

  std::vector<std::optional<TextureHandle>> refs;
  refs.reserve(bitmaps.size());
  for (size_t i = 0; i < bitmaps.size(); ++i) {
    if (const std::optional<AssetHandle<Bitmap>> bitmap = bitmaps[i]; bitmap.has_value()) {
      const SamplerOptions& option = options[i];
      const TextureHandle ref = this->addTexture(bitmap.value(), option);
      refs.emplace_back(ref);
    } else {
      refs.emplace_back();
    }
  }
  return refs;
}

TextureHandle RenderingEngine::addTexture(AssetHandle<Bitmap> bitmap, const SamplerOptions& options) {
  assert(bInitialised);

  if (mUploadedTextures.contains(bitmap.id())) {
    return mUploadedTextures.at(bitmap.id());
  }

  const TextureHandle handle = mTextures.add(Texture(bitmap.get(), options));
  mUploadedTextures.emplace(bitmap.id(), handle);
  return handle;
}

FramebufferHandle RenderingEngine::addFramebuffer(const FramebufferCreateInfo& info) {
  assert(bInitialised);

  return mFramebuffers.add(Framebuffer(info));
}

void RenderingEngine::submitRenderPasses(const std::span<const RenderPass> renderPasses) {
  assert(bInitialised);

  for (const auto& [viewport, dstFramebuffer, pass] : renderPasses) {
    if (const auto* renderScenePass = std::get_if<RenderScenePass>(&pass)) {
      this->renderScene(*renderScenePass->scene, *renderScenePass->camera, viewport, dstFramebuffer);
    } else if (const auto* postProcessingPass = std::get_if<PostProcessingPass>(&pass)) {
      this->postProcess(viewport, postProcessingPass->postProcessingShader, postProcessingPass->srcFramebuffer, dstFramebuffer);
    }
  }
}

void RenderingEngine::renderScene(const Scene& scene, const Camera& camera, const Viewport& viewport, FramebufferHandle dstFramebuffer) {
  assert(bInitialised);

  dstFramebuffer->bind();

  const glm::ivec2 viewportPositionPx = glm::floor(viewport.position * glm::vec2(dstFramebuffer->size()));
  const glm::ivec2 viewportSizePx = glm::floor(viewport.size * glm::vec2(dstFramebuffer->size()));
  glViewport(viewportPositionPx.x, viewportPositionPx.y, viewportSizePx.x, viewportSizePx.y);

  glEnable(GL_STENCIL_TEST);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glStencilMask(0xff);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  const std::vector<Draw> draws = scene.draw(camera);
  if (draws.empty()) {
    return;
  }

  TransformMatrices transforms = { .view = camera.view(), .projection = camera.projection(dstFramebuffer->size()) };

  const Draw* lastDraw = &draws.front();
  for (size_t drawIdx = 0; drawIdx < draws.size(); drawIdx++) {
    const Draw* currDraw = &draws[drawIdx];

    if (drawIdx == 0 || currDraw->bBackfaceCulling != lastDraw->bBackfaceCulling) {
      if (currDraw->bBackfaceCulling) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
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
    if (drawIdx == 0 || currDraw->shaderProgramInstance.id() != lastDraw->shaderProgramInstance.id()) {
      const ShaderProgramInstance& lastShader = lastDraw->shaderProgramInstance.get();
      if (drawIdx == 0 || currShader.shaderProgram != lastShader.shaderProgram) {
        currShader.use();
      }
      currShader.bindUniforms();
    }

    if (drawIdx == 0 || currDraw->shaderProgramInstance.id() != lastDraw->shaderProgramInstance.id() ||
        currDraw->transform != lastDraw->transform)
    {
      transforms.model = currDraw->transform.matrix();
      transforms.normal = glm::transpose(glm::inverse(transforms.model));
      currShader.shaderProgram->bindTransforms(transforms);
    }

    GLuint slot = GL_TEXTURE0;
    const auto bindTextures = [&, this](const std::optional<TextureHandle>& currTexture, const std::optional<TextureHandle>& lastTexture) {
      if (drawIdx == 0 || currTexture != lastTexture) {
        if (currTexture.has_value()) {
          currTexture->get().bind(slot);
          mBoundTextureSlots.insert(slot);
        } else {
          glActiveTexture(slot);
          glBindTexture(GL_TEXTURE_2D, 0);
          mBoundTextureSlots.erase(slot);
        }
      }
      ++slot;
    };
    bindTextures(currDraw->diffuseMapIndex, lastDraw->diffuseMapIndex);
    bindTextures(currDraw->specularMapIndex, lastDraw->specularMapIndex);
    bindTextures(currDraw->emissionMapIndex, lastDraw->emissionMapIndex);

    const VertexArray& currVA = currDraw->vertexArray.get();
    if (drawIdx == 0 || currDraw->vertexArray != lastDraw->vertexArray) {
      glBindVertexArray(currVA.vao);
    }
    glDrawElements(GL_TRIANGLES, currVA.indexCount, GL_UNSIGNED_INT, nullptr);

    lastDraw = currDraw;
  }

  glBindVertexArray(GL_NONE);
  glUseProgram(GL_NONE);
  for (const GLuint slot : mBoundTextureSlots) {
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
  }
  mBoundTextureSlots.clear();
}

void RenderingEngine::postProcess(const Viewport& viewport, ShaderProgramInstanceHandle postProcessingShader,
                                  FramebufferHandle srcFramebuffer, FramebufferHandle dstFramebuffer) const
{
  assert(bInitialised);

  const glm::ivec2 viewportPositionPx = glm::floor(viewport.position * glm::vec2(dstFramebuffer->size()));
  const glm::ivec2 viewportSizePx = glm::floor(viewport.size * glm::vec2(dstFramebuffer->size()));
  glViewport(viewportPositionPx.x, viewportPositionPx.y, viewportSizePx.x, viewportSizePx.y);

  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);

  dstFramebuffer->bind();

  postProcessingShader->use();
  postProcessingShader->bindUniforms();
  srcFramebuffer->colorAttachment.bind();

  glBindVertexArray(mScreenQuadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(GL_NONE);
  glBindVertexArray(GL_NONE);
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void RenderingEngine::present(const glm::uvec2 windowSize, FramebufferHandle srcFramebuffer) const {
  assert(bInitialised);

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

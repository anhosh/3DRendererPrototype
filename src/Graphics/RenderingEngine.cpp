#include <ranges>
#include <Graphics/RenderingEngine.hpp>

#include <Graphics/Draw.hpp>
#include <Graphics/Meshes.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/VertexArray.hpp>

#include <glm/gtc/type_ptr.hpp>

Expected<void> addShaderProgram(const ShaderProgramPaths& stages, std::unique_ptr<ShaderProgram>& outShader) {
  Expected<std::unique_ptr<ShaderProgram>> program = ShaderPrograms::fromShaders(stages);
  RETURN_ERROR_IF_UNEXPECTED(program);
  outShader = std::move(program.value());
  return {};
};

Expected<void> RenderingEngine::init() {
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "simple.vert", .fragment = "litSurface.frag"},
                                              mLitSurfaceShaderProgram));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "simple.vert", .fragment = "light.frag"},
                                              mLightShaderProgram));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "simple.vert", .fragment = "visualiseDepth.frag"},
                                              mVisualiseDepthShaderProgram));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "simple.vert", .fragment = "visualiseNormal.frag"},
                                              mVisualiseNormalShaderProgram));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "simple.vert", .fragment = "outline.frag"},
                                              mOutlineShaderProgram));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/copy.frag"},
                                              mPostProcessCopyShaderProgram));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/grayscale.frag"},
                                              mPostProcessGrayscaleShaderProgram));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/invert.frag"},
                                              mPostProcessInvertShaderProgram));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/kernel3x3.frag"},
                                              mPostProcessKernel3x3ShaderProgram));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/flipHorizontally.frag"},
                                              mPostProcessFlipHorizontally));
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/flipVertically.frag"},
                                              mPostProcessFlipVertically));

  glGenVertexArrays(1, &mScreenQuadVAO);

  return {};
}

void RenderingEngine::destroy() {
  mLitSurfaceShaderProgram->destroy();
  mLightShaderProgram->destroy();
  mOutlineShaderProgram->destroy();
  mVisualiseDepthShaderProgram->destroy();
  mVisualiseNormalShaderProgram->destroy();
  mPostProcessCopyShaderProgram->destroy();
  mPostProcessInvertShaderProgram->destroy();

  for (VertexArray& vertexArray : std::ranges::views::values(mVertexArrays)) {
    vertexArray.destroy();
  }
  for (Texture& texture : std::ranges::views::values(mTextures)) {
    texture.destroy();
  }
  mShaderProgramInstances.clear();
  mVertexArrays.clear();
  mTextures.clear();

  glDeleteVertexArrays(1, &mScreenQuadVAO);
  mScreenQuadVAO = GL_NONE;

  for (Framebuffer& framebuffer : std::ranges::views::values(mFramebuffers)) {
    framebuffer.destroy();
  }
  mFramebuffers.clear();

  mUploadedTextures.clear();
  mUploadedModels.clear();

  mBoundTextureSlots.clear();
}

ShaderProgramInstanceHandle RenderingEngine::createShaderProgramInstance(const ShaderProgramType type) {
  switch (type) {
    case ShaderProgramType::LitSurface:
      return this->addShaderProgramInstance(ShaderProgramInstance::newLitSurface(mLitSurfaceShaderProgram.get()));
    case ShaderProgramType::Light:
      return this->addShaderProgramInstance(ShaderProgramInstance::newLight(mLightShaderProgram.get()));
    case ShaderProgramType::Outline:
      return this->addShaderProgramInstance(ShaderProgramInstance::newOutline(mOutlineShaderProgram.get()));
    case ShaderProgramType::VisualiseDepth:
      return this->addShaderProgramInstance(ShaderProgramInstance::newVisualiseDepth(mVisualiseDepthShaderProgram.get()));
    case ShaderProgramType::VisualiseNormal:
      return this->addShaderProgramInstance(ShaderProgramInstance::newVisualiseNormal(mVisualiseNormalShaderProgram.get()));
    case ShaderProgramType::PostProcessCopy:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingCopy(mPostProcessCopyShaderProgram.get()));
    case ShaderProgramType::PostProcessBlur:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingBlur(mPostProcessKernel3x3ShaderProgram.get()));
    case ShaderProgramType::PostProcessEdgeDetection:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingEdgeDetection(mPostProcessKernel3x3ShaderProgram.get()));
    case ShaderProgramType::PostProcessEmboss:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingEmboss(mPostProcessKernel3x3ShaderProgram.get()));
    case ShaderProgramType::PostProcessFlipHorizontally:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingFlipHorizontally(mPostProcessFlipHorizontally.get()));
    case ShaderProgramType::PostProcessFlipVertically:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingFlipVertically(mPostProcessFlipVertically.get()));
    case ShaderProgramType::PostProcessGrayscale:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingGrayscale(mPostProcessGrayscaleShaderProgram.get()));
    case ShaderProgramType::PostProcessInvert:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingInvert(mPostProcessInvertShaderProgram.get()));
    case ShaderProgramType::PostProcessSharpen:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingSharpen(mPostProcessKernel3x3ShaderProgram.get()));
    case ShaderProgramType::PostProcessSobelBottom:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingSobelBottom(mPostProcessKernel3x3ShaderProgram.get()));
    case ShaderProgramType::PostProcessSobelLeft:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingSobelLeft(mPostProcessKernel3x3ShaderProgram.get()));
    case ShaderProgramType::PostProcessSobelRight:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingSobelRight(mPostProcessKernel3x3ShaderProgram.get()));
    case ShaderProgramType::PostProcessSobelTop:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingSobelTop(mPostProcessKernel3x3ShaderProgram.get()));
    default:
      PANIC("Unsupported shader program type");
  }
}

ShaderProgramInstanceHandle RenderingEngine::addShaderProgramInstance(ShaderProgramInstance&& instance) {
  return mShaderProgramInstances.add(std::forward<ShaderProgramInstance>(instance));
}

const std::vector<RenderData>& RenderingEngine::addModel(AssetHandle<Model> model,
                                                                          ShaderProgramInstanceHandle initialShaderProgramInstance)
{
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
    RenderData resources { .vertexArray = vertexArrays[meshRes], .shaderProgramInstance = initialShaderProgramInstance };
    resources.diffuseMap = diffuseMaps[meshRes];
    resources.specularMap = specularMaps[meshRes];
    resources.emissionMap = emissionMaps[meshRes];
    modelResources.push_back(resources);
  }

  const auto [it, inserted] = mUploadedModels.emplace(model.id(), std::move(modelResources));
  const auto& [index, resources] = *it;
  return resources;
}

VertexArrayHandle RenderingEngine::addMesh(AssetHandle<Mesh> mesh) {
  return mVertexArrays.add(mesh.get());
}

TextureHandle RenderingEngine::addTexture(AssetHandle<Bitmap> bitmap, const SamplerOptions& options) {
  if (mUploadedTextures.contains(bitmap.id())) {
    return mUploadedTextures.at(bitmap.id());
  }

  const TextureHandle handle = mTextures.add(Texture(bitmap.get(), options));
  mUploadedTextures.emplace(bitmap.id(), handle);
  return handle;
}

FramebufferHandle RenderingEngine::addFramebuffer(const FramebufferCreateInfo& info) {
  return mFramebuffers.add(Framebuffer(info));
}

std::vector<VertexArrayHandle> RenderingEngine::addMeshes(std::span<const AssetHandle<Mesh>> meshes) {
  std::vector<VertexArrayHandle> refs;
  refs.reserve(meshes.size());
  for (const AssetHandle<Mesh>& mesh : meshes) {
    const VertexArrayHandle ref = this->addMesh(mesh);
    refs.push_back(ref);
  }
  return refs;
}

auto RenderingEngine::addTextures(std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps, std::span<const SamplerOptions> options)
  -> std::vector<std::optional<TextureHandle>>
{
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

void RenderingEngine::submitRenderPasses(const std::span<const RenderPass> renderPasses) {
  for (const RenderPass& renderPass : renderPasses) {
    if (const auto* renderScenePass = std::get_if<RenderScenePass>(&renderPass.pass)) {
      this->renderScene(*renderScenePass->scene, *renderScenePass->camera, renderPass.viewport, renderPass.dstFramebuffer);
    } else if (const auto* postProcessingPass = std::get_if<PostProcessingPass>(&renderPass.pass)) {
      this->postProcess(postProcessingPass->postProcessingShader, postProcessingPass->srcFramebuffer, renderPass.dstFramebuffer);
    }
  }
}

void RenderingEngine::renderScene(const Scene& scene, const Camera& camera, const Viewport& viewport, FramebufferHandle dstFramebuffer) {
  dstFramebuffer->bind();

  const glm::ivec2 viewportPositionPx = glm::floor(viewport.position * glm::vec2(dstFramebuffer->size()));
  const glm::ivec2 viewportSizePx = glm::floor(viewport.size * glm::vec2(dstFramebuffer->size()));

  glViewport(static_cast<GLint>(viewportPositionPx.x), static_cast<GLint>(viewportPositionPx.y),
             static_cast<GLint>(viewportSizePx.x), static_cast<GLint>(viewportSizePx.y));
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
  for (size_t i = 0; i < draws.size(); i++) {
    const Draw* currDraw = &draws[i];

    if (i == 0 || currDraw->bBackfaceCulling != lastDraw->bBackfaceCulling) {
      if (currDraw->bBackfaceCulling) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
      } else {
        glDisable(GL_CULL_FACE);
      }
    }
    if (i == 0 || currDraw->bDepthTest != lastDraw->bDepthTest) {
      if (currDraw->bDepthTest) {
        glEnable(GL_DEPTH_TEST);
      } else {
        glDisable(GL_DEPTH_TEST);
      }
    }
    if (i == 0 || currDraw->bStencilTest != lastDraw->bStencilTest) {
      glStencilFunc(currDraw->bStencilTest ? GL_NOTEQUAL : GL_ALWAYS, 1, 0xff);
    }
    if (i == 0 || currDraw->bWriteToStencil != lastDraw->bWriteToStencil) {
      glStencilMask(currDraw->bWriteToStencil ? 0xff : 0x00);
    }
    if (i == 0 || currDraw->bTransparent != lastDraw->bTransparent) {
      if (currDraw->bTransparent) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      } else {
        glDisable(GL_BLEND);
      }
    }

    const ShaderProgramInstance& currShader = currDraw->shaderProgramInstance.get();
    if (i == 0 || currDraw->shaderProgramInstance.id() != lastDraw->shaderProgramInstance.id()) {
      const ShaderProgramInstance& lastShader = lastDraw->shaderProgramInstance.get();
      if (i == 0 || currShader.shaderProgram != lastShader.shaderProgram) {
        currShader.use();
      }
      currShader.bindUniforms();
    }

    if (i == 0 || currDraw->shaderProgramInstance.id() != lastDraw->shaderProgramInstance.id() ||
        currDraw->transform != lastDraw->transform)
    {
      transforms.model = currDraw->transform.matrix();
      transforms.normal = glm::transpose(glm::inverse(transforms.model));
      currShader.shaderProgram->bindTransforms(transforms);
    }

    GLuint slot = GL_TEXTURE0;
    const auto bindTextures = [&, this](const std::optional<TextureHandle>& currTexture, const std::optional<TextureHandle>& lastTexture) {
      if (i == 0 || currTexture != lastTexture) {
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
    if (i == 0 || currDraw->vertexArray != lastDraw->vertexArray) {
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

void RenderingEngine::postProcess(ShaderProgramInstanceHandle postProcessingShader,
                                  FramebufferHandle srcFramebuffer, FramebufferHandle dstFramebuffer) const
{
  glViewport(0, 0, static_cast<GLint>(dstFramebuffer->size().x), static_cast<GLint>(dstFramebuffer->size().y));
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);

  dstFramebuffer->bind();
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

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
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(0, 0, static_cast<GLint>(windowSize.x), static_cast<GLint>(windowSize.y));

  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(mPostProcessCopyShaderProgram->id());
  glUniform1i(0, 0); // bind uScreenTexture sampler
  srcFramebuffer->colorAttachment.bind();
  glBindVertexArray(mScreenQuadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(GL_NONE);
  glBindVertexArray(GL_NONE);
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

#include <Graphics/RenderingEngine.hpp>

#include <Graphics/Draw.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/VertexArray.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <ranges>
#include <Graphics/Meshes.hpp>

DEFINE_HANDLE_ITEM_GET(RenderingEngine, VertexArray, mVertexArrays)
DEFINE_HANDLE_ITEM_GET(RenderingEngine, ShaderProgramInstance, mShaderProgramInstances)
DEFINE_HANDLE_ITEM_GET(RenderingEngine, Texture, mTextures)

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
  RETURN_ERROR_IF_UNEXPECTED(addShaderProgram({.vertex = "screenQuad.vert", .fragment = "postProcessing/invert.frag"},
                                              mPostProcessInvertShaderProgram));

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

  for (VertexArray& vertexArray : mVertexArrays) {
    vertexArray.destroy();
  }
  for (Texture& texture : mTextures) {
    texture.destroy();
  }
  mShaderProgramInstances.clear();
  mVertexArrays.clear();
  mTextures.clear();

  glDeleteVertexArrays(1, &mScreenQuadVAO);
  mScreenQuadVAO = GL_NONE;

  for (Framebuffer& framebuffer : mFramebuffers) {
    framebuffer.destroy();
  }
  mFramebuffers.clear();

  mUploadedTextures.clear();
  mUploadedModels.clear();

  mBoundTextureSlots.clear();
}

RenderingEngine::Handle<ShaderProgramInstance> RenderingEngine::createShaderProgramInstance(ShaderProgramType type) {
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
    case ShaderProgramType::PostProcessInvert:
      return this->addShaderProgramInstance(ShaderProgramInstance::newPostProcessingInvert(mPostProcessInvertShaderProgram.get()));
    default:
      PANIC("Unsupported shader program type");
  }
}

RenderingEngine::Handle<ShaderProgramInstance> RenderingEngine::addShaderProgramInstance(ShaderProgramInstance instance) {
  mShaderProgramInstances.push_back(std::move(instance));
  const size_t index = mShaderProgramInstances.size() - 1;
  return {index, this};
}

const std::vector<RenderingEngine::RenderData>& RenderingEngine::addModel(AssetHandle<Model> model, Handle<ShaderProgramInstance> initialShaderProgramInstance)
{
  if (mUploadedModels.contains(model.index)) {
    return mUploadedModels.at(model.index);
  }

  std::vector<SamplerOptions> diffuseSamplers;
  std::vector<SamplerOptions> specularSamplers;
  std::vector<SamplerOptions> emissionSamplers;
  diffuseSamplers.resize(model.get().diffuseMaps.size());
  specularSamplers.resize(model.get().specularMaps.size());
  emissionSamplers.resize(model.get().emissionMaps.size());

  const std::vector<Handle<VertexArray>> vertexArrays = this->addMeshes(model.get().meshes);
  const std::vector<std::optional<Handle<Texture>>> diffuseMaps = this->addTextures(model.get().diffuseMaps, diffuseSamplers);
  const std::vector<std::optional<Handle<Texture>>> specularMaps = this->addTextures(model.get().specularMaps, specularSamplers);
  const std::vector<std::optional<Handle<Texture>>> emissionMaps = this->addTextures(model.get().emissionMaps, emissionSamplers);

  std::vector<RenderData> modelResources;
  modelResources.reserve(vertexArrays.size());
  for (size_t meshRes = 0; meshRes < vertexArrays.size(); ++meshRes) {
    RenderData resources { .vertexArray = vertexArrays[meshRes], .shaderProgramInstance = initialShaderProgramInstance };
    resources.diffuseMap = diffuseMaps[meshRes];
    resources.specularMap = specularMaps[meshRes];
    resources.emissionMap = emissionMaps[meshRes];
    modelResources.push_back(resources);
  }

  const auto [it, inserted] = mUploadedModels.emplace(model.index, std::move(modelResources));
  const auto& [index, resource] = *it;
  return resource;
}

RenderingEngine::Handle<VertexArray> RenderingEngine::addMesh(AssetHandle<Mesh> mesh) {
  mVertexArrays.emplace_back(mesh.get());
  const size_t index = mVertexArrays.size() - 1;
  return {index, this};
}

RenderingEngine::Handle<Texture> RenderingEngine::addTexture(AssetHandle<Bitmap> bitmap, const SamplerOptions& options) {
  if (mUploadedTextures.contains(bitmap.index)) {
    return mUploadedTextures.at(bitmap.index);
  }

  mTextures.emplace_back(bitmap.get(), options);
  const size_t index = mTextures.size() - 1;
  const auto [it, inserted] = mUploadedTextures.emplace(bitmap.index, Handle<Texture>(index, this));
  const auto& [i, handle] = *it;
  return handle;
}

std::vector<RenderingEngine::Handle<VertexArray>> RenderingEngine::addMeshes(std::span<const AssetHandle<Mesh>> meshes) {
  std::vector<Handle<VertexArray>> refs;
  refs.reserve(meshes.size());
  for (const AssetHandle<Mesh>& mesh : meshes) {
    const Handle<VertexArray> ref = this->addMesh(mesh);
    refs.push_back(ref);
  }
  return refs;
}

auto RenderingEngine::addTextures(std::span<const std::optional<AssetHandle<Bitmap>>> bitmaps, std::span<const SamplerOptions> options)
  -> std::vector<std::optional<Handle<Texture>>>
{
  assert(bitmaps.size() == options.size());
  std::vector<std::optional<Handle<Texture>>> refs;
  refs.reserve(bitmaps.size());
  for (size_t i = 0; i < bitmaps.size(); ++i) {
    if (const std::optional<AssetHandle<Bitmap>> bitmap = bitmaps[i]; bitmap.has_value()) {
      const SamplerOptions& option = options[i];
      const Handle<Texture> ref = this->addTexture(bitmap.value(), option);
      refs.emplace_back(ref);
    } else {
      refs.emplace_back();
    }
  }
  return refs;
}

#define RENDER_TO_TEXTURE 1

void RenderingEngine::renderScene(const Scene& scene, const Camera& camera, const glm::uvec2 windowSize) {
#if RENDER_TO_TEXTURE
  if (mFramebuffers.empty()) {
    mFramebuffers.emplace_back(windowSize);
    mLastFramebufferSize = windowSize;
  } else if (mLastFramebufferSize != windowSize) {
    mFramebuffers.front().destroy();
    mFramebuffers.front().init(windowSize);
    mLastFramebufferSize = windowSize;
  }
  mFramebuffers.front().bind();
#endif

  glViewport(0, 0, static_cast<GLint>(windowSize.x), static_cast<GLint>(windowSize.y));
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glStencilMask(0xff);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  const std::vector<Draw> draws = scene.draw();
  if (draws.empty()) {
    return;
  }

  TransformMatrices transforms = { .view = camera.view(), .projection = camera.projection(windowSize) };

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

    const ShaderProgramInstance& currShader = mShaderProgramInstances[currDraw->shaderProgramInstanceIndex];
    if (i == 0 || currDraw->shaderProgramInstanceIndex != lastDraw->shaderProgramInstanceIndex) {
      const ShaderProgramInstance& lastShader = mShaderProgramInstances[lastDraw->shaderProgramInstanceIndex];
      if (i == 0 || currShader.shaderProgram != lastShader.shaderProgram) {
        glUseProgram(NotNull(currShader.shaderProgram)->id());
      }

      for (const ShaderUniform& uniform : std::ranges::views::values(currShader.uniforms)) {
        if (const GLint* int_value = std::get_if<GLint>(&uniform.value)) {
          glUniform1i(uniform.location, *int_value);
        } else if (const GLuint* uint_value = std::get_if<GLuint>(&uniform.value)) {
          glUniform1ui(uniform.location, *uint_value);
        } else if (const GLfloat* float_value = std::get_if<GLfloat>(&uniform.value)) {
          glUniform1f(uniform.location, *float_value);
        } else if (const GLdouble* double_value = std::get_if<GLdouble>(&uniform.value)) {
          glUniform1d(uniform.location, *double_value);
        } else if (const glm::vec2* vec2_value = std::get_if<glm::vec2>(&uniform.value)) {
          glUniform2f(uniform.location, vec2_value->x, vec2_value->y);
        } else if (const glm::vec3* vec3_value = std::get_if<glm::vec3>(&uniform.value)) {
          glUniform3f(uniform.location, vec3_value->x, vec3_value->y, vec3_value->z);
        } else if (const glm::vec4* vec4_value = std::get_if<glm::vec4>(&uniform.value)) {
          glUniform4f(uniform.location, vec4_value->x, vec4_value->y, vec4_value->z, vec4_value->w);
        } else if (const glm::mat2* mat2_value = std::get_if<glm::mat2>(&uniform.value)) {
          glUniformMatrix2fv(uniform.location, 1, GL_FALSE, glm::value_ptr(*mat2_value));
        } else if (const glm::mat3* mat3_value = std::get_if<glm::mat3>(&uniform.value)) {
          glUniformMatrix3fv(uniform.location, 1, GL_FALSE, glm::value_ptr(*mat3_value));
        } else if (const glm::mat4* mat4_value = std::get_if<glm::mat4>(&uniform.value)) {
          glUniformMatrix4fv(uniform.location, 1, GL_FALSE, glm::value_ptr(*mat4_value));
        } else {
          PANIC("Unsupported uniform type");
        }
      }
    }

    if (i == 0 || currDraw->shaderProgramInstanceIndex != lastDraw->shaderProgramInstanceIndex || currDraw->transform != lastDraw->transform) {
      transforms.model = currDraw->transform.matrix();
      transforms.normal = glm::transpose(glm::inverse(transforms.model));
      currShader.shaderProgram->bindTransforms(transforms);
    }

    GLuint slot = GL_TEXTURE0;
    const auto bindTextures = [&, this](const size_t currTextureIndex, const size_t lastTextureIndex) {
      if (i == 0 || currTextureIndex != lastTextureIndex) {
        if (currTextureIndex == SIZE_MAX) {
          glActiveTexture(slot);
          glBindTexture(GL_TEXTURE_2D, 0);
          mBoundTextureSlots.erase(slot);
        } else {
          mTextures[currTextureIndex].bind(slot);
          mBoundTextureSlots.insert(slot);
        }
      }
      ++slot;
    };
    bindTextures(currDraw->diffuseMapIndex, lastDraw->diffuseMapIndex);
    bindTextures(currDraw->specularMapIndex, lastDraw->specularMapIndex);
    bindTextures(currDraw->emissionMapIndex, lastDraw->emissionMapIndex);

    const VertexArray& currVA = mVertexArrays[currDraw->vertexArrayIndex];
    if (i == 0 || currDraw->vertexArrayIndex != lastDraw->vertexArrayIndex) {
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

void RenderingEngine::postProcess() {
  // TODO
}

void RenderingEngine::present(const glm::uvec2 windowSize) {
#if RENDER_TO_TEXTURE
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, static_cast<GLint>(windowSize.x), static_cast<GLint>(windowSize.y));
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(mPostProcessCopyShaderProgram->id());
  glBindVertexArray(mScreenQuadVAO);
  mFramebuffers[mLastFramebufferIndex].colorAttachment.bind();
  glUniform1i(0, 0); // bind uScreenTexture sampler
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glUseProgram(GL_NONE);
  glBindVertexArray(GL_NONE);
  mFramebuffers[mLastFramebufferIndex].colorAttachment.unbind();

  mLastFramebufferIndex = 0;
#endif
}

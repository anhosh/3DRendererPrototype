#include <ranges>
#include <glm/gtc/type_ptr.hpp>
#include <Graphics/SceneRenderer.hpp>

#include <Graphics/Draw.hpp>
#include <Graphics/Material.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/VertexArray.hpp>

template<>
VertexArray& SceneRenderer::Handle<VertexArray>::get() {
  return renderer->mVertexArrays[index];
}

template<>
ShaderProgramInstance& SceneRenderer::Handle<ShaderProgramInstance>::get() {
  return renderer->mShaderProgramInstances[index];
}

template<>
Texture& SceneRenderer::Handle<Texture>::get() {
  return renderer->mTextures[index];
}

Expected<void> SceneRenderer::loadShaders() {
  if (!locateShaders()) {
    return std::unexpected("Could not locate shader directory");
  }

  const auto addShaderProgram = [](const ShaderProgramPaths& stages, std::unique_ptr<ShaderProgram>& outShader) -> Expected<void> {
    auto program = ShaderPrograms::fromShaders(stages);
    RETURN_ERROR_IF_UNEXPECTED(program);
    outShader = std::move(program.value());
    return {};
  };

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

  return {};
}

const std::vector<SceneRenderer::RenderData>& SceneRenderer::addModel(AssetManager::Handle<Model> model,
                                                                      Handle<ShaderProgramInstance> initialShaderProgramInstance)
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
    resources.diffuseMap = std::move(diffuseMaps[meshRes]);
    resources.specularMap = std::move(specularMaps[meshRes]);
    resources.emissionMap = std::move(emissionMaps[meshRes]);
    modelResources.push_back(std::move(resources));
  }

  const auto [it, inserted] = mUploadedModels.emplace(model.index, std::move(modelResources));
  const auto& [index, resource] = *it;
  return resource;
}

SceneRenderer::Handle<VertexArray> SceneRenderer::addMesh(AssetManager::Handle<Mesh> mesh) {
  mVertexArrays.emplace_back(mesh.get());
  const size_t index = mVertexArrays.size() - 1;
  return { index, this };
}

SceneRenderer::Handle<ShaderProgramInstance> SceneRenderer::createShaderProgramInstance(ShaderProgramType type) {
  ShaderProgramInstance newInstance;
  const auto setUniform = [&](const GLchar* name, auto value) {
    newInstance.uniforms[name] = ShaderUniform {
      .location = glGetUniformLocation(newInstance.shaderProgram->id(), name),
      .value = value,
    };
  };

  switch (type) {
    case ShaderProgramType::LitSurface: {
      newInstance.shaderProgram = mLitSurfaceShaderProgram.get();
      setUniform("uViewPos", glm::vec3(0.0f));
      constexpr Material material;
      setUniform("uMaterial.diffuse", material.diffuse);
      setUniform("uMaterial.specular", material.specular);
      setUniform("uMaterial.emission", material.emission);
      setUniform("uMaterial.shininess", material.shininess);
      constexpr DirectionalLight directionalLight;
      setUniform("uDirectionalLight.colors.ambient", directionalLight.colors.ambient);
      setUniform("uDirectionalLight.colors.diffuse", directionalLight.colors.diffuse);
      setUniform("uDirectionalLight.colors.specular", directionalLight.colors.specular);
      setUniform("uDirectionalLight.direction", directionalLight.direction);
      constexpr PointLight pointLight;
      setUniform("uPointLight.colors.ambient", pointLight.colors.ambient);
      setUniform("uPointLight.colors.diffuse", pointLight.colors.diffuse);
      setUniform("uPointLight.colors.specular", pointLight.colors.specular);
      setUniform("uPointLight.position", pointLight.position);
      setUniform("uPointLight.constant", pointLight.constant);
      setUniform("uPointLight.linear", pointLight.linear);
      setUniform("uPointLight.quadratic", pointLight.quadratic);
      constexpr Spotlight spotlight;
      setUniform("uSpotlight.colors.ambient", spotlight.colors.ambient);
      setUniform("uSpotlight.colors.diffuse", spotlight.colors.diffuse);
      setUniform("uSpotlight.colors.specular", spotlight.colors.specular);
      setUniform("uSpotlight.position", spotlight.position);
      setUniform("uSpotlight.direction", spotlight.direction);
      setUniform("uSpotlight.cutOff", spotlight.cutOff);
      setUniform("uSpotlight.outerCutOff", spotlight.outerCutOff);
      break;
    }

    case ShaderProgramType::Light:
      newInstance.shaderProgram = mLightShaderProgram.get();
      setUniform("uLightColor", glm::vec3(1.0f));
      break;

    case ShaderProgramType::Outline:
      newInstance.shaderProgram = mOutlineShaderProgram.get();
      setUniform("uOutlineColor", glm::vec3(1.0f));
      break;

    case ShaderProgramType::VisualiseDepth:
      newInstance.shaderProgram = mVisualiseDepthShaderProgram.get();
      setUniform("uCamera.near", 0.01f);
      setUniform("uCamera.far", 30.0f);
      break;

    case ShaderProgramType::VisualiseNormal:
      newInstance.shaderProgram = mVisualiseNormalShaderProgram.get();
      break;
  }
  mShaderProgramInstances.push_back(newInstance);
  const size_t index = mShaderProgramInstances.size() - 1;
  return { index, this };
}

SceneRenderer::Handle<Texture> SceneRenderer::addTexture(AssetManager::Handle<Bitmap> bitmap, const SamplerOptions& options) {
  if (mUploadedBitmaps.contains(bitmap.index)) {
    return mUploadedBitmaps.at(bitmap.index);
  }

  mTextures.emplace_back(bitmap.get(), options);
  const size_t index = mTextures.size() - 1;
  const auto [it, inserted] = mUploadedBitmaps.emplace(bitmap.index, Handle<Texture>(index, this));
  const auto& [i, handle] = *it;
  return handle;
}

std::vector<SceneRenderer::Handle<VertexArray>> SceneRenderer::addMeshes(std::span<const AssetManager::Handle<Mesh>> meshes) {
  std::vector<Handle<VertexArray>> refs;
  refs.reserve(meshes.size());
  for (const AssetManager::Handle<Mesh>& mesh : meshes) {
    const Handle<VertexArray> ref = this->addMesh(mesh);
    refs.push_back(ref);
  }
  return refs;
}

std::vector<std::optional<SceneRenderer::Handle<Texture>>> SceneRenderer::addTextures(std::span<const std::optional<AssetManager::Handle<Bitmap>>> bitmaps,
                                                                                      std::span<const SamplerOptions> options)
{
  assert(bitmaps.size() == options.size());
  std::vector<std::optional<Handle<Texture>>> refs;
  refs.reserve(bitmaps.size());
  for (size_t i = 0; i < bitmaps.size(); ++i) {
    if (const std::optional<AssetManager::Handle<Bitmap>> bitmap = bitmaps[i]; bitmap.has_value()) {
      const SamplerOptions& option = options[i];
      const Handle<Texture> ref = this->addTexture(bitmap.value(), option);
      refs.emplace_back(ref);
    } else {
      refs.emplace_back();
    }
  }
  return refs;
}

void SceneRenderer::render(std::span<const Draw> draws, TransformMatrices transforms) {
  if (draws.empty()) {
    return;
  }

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

    transforms.model = currDraw->transform.matrix();
    transforms.normal = glm::transpose(glm::inverse(transforms.model));
    currShader.shaderProgram->bindTransforms(transforms);

    GLuint slot = GL_TEXTURE0;
    const auto bindTextures = [&, this](const size_t textureIndex) {
      if (textureIndex == SIZE_MAX) {
        glActiveTexture(slot);
        glBindTexture(GL_TEXTURE_2D, 0);
        mBoundTextureSlots.erase(slot);
      } else {
        mTextures[textureIndex].bind(slot);
        mBoundTextureSlots.insert(slot);
      }
      ++slot;
    };
    bindTextures(currDraw->diffuseMapIndex);
    bindTextures(currDraw->specularMapIndex);
    bindTextures(currDraw->emissionMapIndex);

    const VertexArray& currVA = mVertexArrays[currDraw->vertexArrayIndex];
    if (i == 0 || currDraw->vertexArrayIndex != lastDraw->vertexArrayIndex) {
      glBindVertexArray(currVA.vao);
    }
    glDrawElements(GL_TRIANGLES, currVA.indexCount, GL_UNSIGNED_INT, nullptr);

    lastDraw = currDraw;
  }

  glBindVertexArray(0);
  glUseProgram(0);
  for (const GLuint slot : mBoundTextureSlots) {
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  mBoundTextureSlots.clear();
}

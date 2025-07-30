#include <Graphics/ShaderProgramInstance.hpp>

#include <Util/Macros/Errors.hpp>

#include <ranges>

static constexpr GLint TEXTURE_SLOT_SCREEN = 0;
static constexpr GLint TEXTURE_SLOT_DIFFUSE = 0;
static constexpr GLint TEXTURE_SLOT_SPECULAR = 1;
static constexpr GLint TEXTURE_SLOT_EMISSION = 2;
static constexpr GLint TEXTURE_SLOT_ENVIRONMENT = 3;

void ShaderProgramInstance::use() const {
  glUseProgram(shaderProgram->id());
}

void ShaderProgramInstance::bindUniforms() const {
  ZoneScoped;

  for (const ShaderUniform& uniform : std::ranges::views::values(uniforms)) {
    if (const bool* bool_value = uniform.getPtr<bool>()) {
      glUniform1i(uniform.location, *bool_value ? GL_TRUE : GL_FALSE);
    } else if (const GLint* int_value = uniform.getPtr<GLint>()) {
      glUniform1i(uniform.location, *int_value);
    } else if (const GLuint* uint_value = uniform.getPtr<GLuint>()) {
      glUniform1ui(uniform.location, *uint_value);
    } else if (const GLfloat* float_value = uniform.getPtr<GLfloat>()) {
      glUniform1f(uniform.location, *float_value);
    } else if (const GLdouble* double_value = uniform.getPtr<GLdouble>()) {
      glUniform1d(uniform.location, *double_value);
    } else if (const glm::vec2* vec2_value = uniform.getPtr<glm::vec2>()) {
      glUniform2f(uniform.location, vec2_value->x, vec2_value->y);
    } else if (const glm::vec3* vec3_value = uniform.getPtr<glm::vec3>()) {
      glUniform3f(uniform.location, vec3_value->x, vec3_value->y, vec3_value->z);
    } else if (const glm::vec4* vec4_value = uniform.getPtr<glm::vec4>()) {
      glUniform4f(uniform.location, vec4_value->x, vec4_value->y, vec4_value->z, vec4_value->w);
    } else if (const glm::mat2* mat2_value = uniform.getPtr<glm::mat2>()) {
      glUniformMatrix2fv(uniform.location, 1, GL_FALSE, glm::value_ptr(*mat2_value));
    } else if (const glm::mat3* mat3_value = uniform.getPtr<glm::mat3>()) {
      glUniformMatrix3fv(uniform.location, 1, GL_FALSE, glm::value_ptr(*mat3_value));
    } else if (const glm::mat4* mat4_value = uniform.getPtr<glm::mat4>()) {
      glUniformMatrix4fv(uniform.location, 1, GL_FALSE, glm::value_ptr(*mat4_value));
    } else {
      PANIC("Unsupported uniform type");
    }
  }
}

ShaderProgramInstance ShaderProgramInstance::newLitSurface(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::LitSurface);

  instance.setUniform("uMaterial.diffuse", TEXTURE_SLOT_DIFFUSE);
  instance.setUniform("uMaterial.specular", TEXTURE_SLOT_SPECULAR);
  instance.setUniform("uMaterial.emission", TEXTURE_SLOT_EMISSION);
  instance.setUniform("uMaterial.shininess", 32.0f);
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newLitExploded(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::LitExploded);
  
  instance.setUniform("uExplosionDistance", 0.0f);

  instance.setUniform("uMaterial.diffuse", TEXTURE_SLOT_DIFFUSE);
  instance.setUniform("uMaterial.specular", TEXTURE_SLOT_SPECULAR);
  instance.setUniform("uMaterial.emission", TEXTURE_SLOT_EMISSION);
  instance.setUniform("uMaterial.shininess", 32.0f);
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newLight(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::Light);

  instance.setUniform("uLightColor", glm::vec3(1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newOutline(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::Outline);
  
  instance.setUniform("uOutlineColor", glm::vec3(1.0f));
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newReflectiveSurface(ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::ReflectiveSurface);

  instance.setUniform("uEnvironmentMap", TEXTURE_SLOT_ENVIRONMENT);

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newRefractiveSurface(ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::RefractiveSurface);

  instance.setUniform("uRefractiveIndex", 1.52f);
  instance.setUniform("uEnvironmentMap", TEXTURE_SLOT_ENVIRONMENT);

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newSurfaceDepth(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::SurfaceDepth);

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newSurfaceNormal(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::SurfaceNormal);
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingCopy(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessCopy);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingBlur(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessBlur);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(1.0f, 2.0f, 1.0f,
                                                      2.0f, 4.0f, 2.0f,
                                                      1.0f, 2.0f, 1.0f) / 16.0f);

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingEdgeDetection(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessEdgeDetection);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(1.0f,  1.0f, 1.0f,
                                                      1.0f, -8.0f, 1.0f,
                                                      1.0f,  1.0f, 1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingEmboss(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessEmboss);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-2.0f, -1.0f, 1.0f,
                                                      -1.0f,  1.0f, 1.0f,
                                                       0.0f,  1.0f, 2.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingFlipHorizontally(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessFlipHorizontally);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingFlipVertically(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessFlipVertically);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingGrayscale(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessGrayscale);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingInvert(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessInvert);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingSharpen(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessSharpen);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-1.0f, -1.0f, -1.0f,
                                                      -1.0f,  9.0f, -1.0f,
                                                      -1.0f, -1.0f, -1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingSobelBottom(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessSobelBottom);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-1.0f, -2.0f, -1.0f,
                                                       0.0f,  0.0f,  0.0f,
                                                       1.0f,  2.0f,  1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingSobelLeft(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessSobelLeft);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(1.0f, 0.0f, -1.0f,
                                                      2.0f, 0.0f, -2.0f,
                                                      1.0f, 0.0f, -1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingSobelRight(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessSobelRight);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-1.0f, 0.0f, 1.0f,
                                                      -2.0f, 0.0f, 2.0f,
                                                      -1.0f, 0.0f, 1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingSobelTop(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessSobelTop);

  instance.setUniform("uScreenTexture", TEXTURE_SLOT_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3( 1.0f,  2.0f,  1.0f,
                                                       0.0f,  0.0f,  0.0f,
                                                      -1.0f, -2.0f, -1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newSkybox(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::Skybox);

  instance.setUniform("uSkyTexture", TEXTURE_SLOT_ENVIRONMENT);

  return instance;
}

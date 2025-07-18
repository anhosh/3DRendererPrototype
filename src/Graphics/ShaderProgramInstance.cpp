#include <Graphics/ShaderProgramInstance.hpp>

#include <Graphics/Light.hpp>

#include <ranges>

void ShaderProgramInstance::use() const {
  glUseProgram(shaderProgram->id());
}

void ShaderProgramInstance::bindUniforms() const {
  for (const ShaderUniform& uniform : std::ranges::views::values(this->uniforms)) {
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

ShaderProgramInstance ShaderProgramInstance::newLitSurface(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uViewPos", glm::vec3(0.0f));

  instance.setUniform("uMaterial.diffuse", 0);
  instance.setUniform("uMaterial.specular", 1);
  instance.setUniform("uMaterial.emission", 2);
  instance.setUniform("uMaterial.shininess", 32.0f);
  
  constexpr DirectionalLight directionalLight;
  instance.setUniform("uDirectionalLight.colors.ambient", directionalLight.colors.ambient);
  instance.setUniform("uDirectionalLight.colors.diffuse", directionalLight.colors.diffuse);
  instance.setUniform("uDirectionalLight.colors.specular", directionalLight.colors.specular);
  instance.setUniform("uDirectionalLight.direction", directionalLight.direction);
  
  constexpr PointLight pointLight;
  instance.setUniform("uPointLight.colors.ambient", pointLight.colors.ambient);
  instance.setUniform("uPointLight.colors.diffuse", pointLight.colors.diffuse);
  instance.setUniform("uPointLight.colors.specular", pointLight.colors.specular);
  instance.setUniform("uPointLight.position", pointLight.position);
  instance.setUniform("uPointLight.constant", pointLight.constant);
  instance.setUniform("uPointLight.linear", pointLight.linear);
  instance.setUniform("uPointLight.quadratic", pointLight.quadratic);
  
  constexpr Spotlight spotlight;
  instance.setUniform("uSpotlight.colors.ambient", spotlight.colors.ambient);
  instance.setUniform("uSpotlight.colors.diffuse", spotlight.colors.diffuse);
  instance.setUniform("uSpotlight.colors.specular", spotlight.colors.specular);
  instance.setUniform("uSpotlight.position", spotlight.position);
  instance.setUniform("uSpotlight.direction", spotlight.direction);
  instance.setUniform("uSpotlight.cutOff", spotlight.cutOff);
  instance.setUniform("uSpotlight.outerCutOff", spotlight.outerCutOff);
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newLight(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);
  
  instance.setUniform("uLightColor", glm::vec3(1.0f));
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newOutline(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);
  
  instance.setUniform("uOutlineColor", glm::vec3(1.0f));
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newVisualiseDepth(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uCamera.near", 0.01f);
  instance.setUniform("uCamera.far", 30.0f);  

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newVisualiseNormal(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingCopy(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingBlur(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(1.0f, 2.0f, 1.0f,
                                                      2.0f, 4.0f, 2.0f,
                                                      1.0f, 2.0f, 1.0f) / 16.0f);

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingEdgeDetection(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(1.0f,  1.0f, 1.0f,
                                                      1.0f, -8.0f, 1.0f,
                                                      1.0f,  1.0f, 1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingEmboss(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-2.0f, -1.0f, 1.0f,
                                                      -1.0f,  1.0f, 1.0f,
                                                       0.0f,  1.0f, 2.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingFlipHorizontally(const ShaderProgramHandle program) {
  return ShaderProgramInstance::newPostProcessingCopy(program);
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingFlipVertically(const ShaderProgramHandle program) {
  return ShaderProgramInstance::newPostProcessingCopy(program);
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingGrayscale(const ShaderProgramHandle program) {
  return ShaderProgramInstance::newPostProcessingCopy(program);
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingInvert(const ShaderProgramHandle program) {
  return ShaderProgramInstance::newPostProcessingCopy(program);
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingSharpen(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-1.0f, -1.0f, -1.0f,
                                                      -1.0f,  9.0f, -1.0f,
                                                      -1.0f, -1.0f, -1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingSobelBottom(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-1.0f, -2.0f, -1.0f,
                                                       0.0f,  0.0f,  0.0f,
                                                       1.0f,  2.0f,  1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingSobelLeft(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(1.0f, 0.0f, -1.0f,
                                                      2.0f, 0.0f, -2.0f,
                                                      1.0f, 0.0f, -1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingSobelRight(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-1.0f, 0.0f, 1.0f,
                                                      -2.0f, 0.0f, 2.0f,
                                                      -1.0f, 0.0f, 1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingSobelTop(const ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3( 1.0f,  2.0f,  1.0f,
                                                       0.0f,  0.0f,  0.0f,
                                                      -1.0f, -2.0f, -1.0f));

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newSkybox(ShaderProgramHandle program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uSkyTexture", 3);

  return instance;
}

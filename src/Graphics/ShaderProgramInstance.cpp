#include <Graphics/ShaderProgramInstance.hpp>

#include <Graphics/Buffers/BindPoints.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/Visitor.hpp>

#include <ranges>

void ShaderProgramInstance::use() const {
  glUseProgram(mShaderProgram->id());
}

void ShaderProgramInstance::bindUniforms() const {
  ZoneScoped;

  for (const ShaderUniform& uniform : std::ranges::views::values(uniforms)) {
    uniform.value.visit(Visitor {
      [&](const bool value)       { glUniform1i(uniform.location, value ? GL_TRUE : GL_FALSE); },
      [&](const GLint value)      { glUniform1i(uniform.location, value); },
      [&](const GLuint value)     { glUniform1ui(uniform.location, value); },
      [&](const GLfloat value)    { glUniform1f(uniform.location, value); },
      [&](const GLdouble value)   { glUniform1d(uniform.location, value); },
      [&](const glm::vec2& value) { glUniform2f(uniform.location, value.x, value.y); },
      [&](const glm::vec3& value) { glUniform3f(uniform.location, value.x, value.y, value.z); },
      [&](const glm::vec4& value) { glUniform4f(uniform.location, value.x, value.y, value.z, value.w); },
      [&](const glm::mat2& value) { glUniformMatrix2fv(uniform.location, 1, GL_FALSE, glm::value_ptr(value)); },
      [&](const glm::mat3& value) { glUniformMatrix3fv(uniform.location, 1, GL_FALSE, glm::value_ptr(value)); },
      [&](const glm::mat4& value) { glUniformMatrix4fv(uniform.location, 1, GL_FALSE, glm::value_ptr(value)); },
      [](auto) { UNREACHABLE(); }
    });
  }
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::Light>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::Light);

  instance.setUniform("uLightColor", glm::vec3(1.0f));

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::LitExploded>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::LitExploded);

  instance.setUniform("uExplosionDistance", 0.0f);

  instance.setUniform("uMaterial.diffuse", BINDING_SAMPLER_DIFFUSE);
  instance.setUniform("uMaterial.specular", BINDING_SAMPLER_SPECULAR);
  instance.setUniform("uMaterial.emission", BINDING_SAMPLER_EMISSION);
  instance.setUniform("uMaterial.shininess", 128.0f);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::LitSurface>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::LitSurface);

  instance.setUniform("uMaterial.diffuse", BINDING_SAMPLER_DIFFUSE);
  instance.setUniform("uMaterial.specular", BINDING_SAMPLER_SPECULAR);
  instance.setUniform("uMaterial.emission", BINDING_SAMPLER_EMISSION);
  instance.setUniform("uMaterial.shininess", 128.0f);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::Outline>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::Outline);
  
  instance.setUniform("uOutlineColor", glm::vec3(1.0f));
  
  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::ReflectiveSurface>(ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::ReflectiveSurface);

  instance.setUniform("uEnvironmentMap", BINDING_SAMPLER_ENVIRONMENT);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::RefractiveSurface>(ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::RefractiveSurface);

  instance.setUniform("uRefractiveIndex", 1.52f);
  instance.setUniform("uEnvironmentMap", BINDING_SAMPLER_ENVIRONMENT);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessCopy>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessCopy);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessBlur>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessBlur);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(1.0f, 2.0f, 1.0f,
                                                      2.0f, 4.0f, 2.0f,
                                                      1.0f, 2.0f, 1.0f) / 16.0f);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessEdgeDetection>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessEdgeDetection);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(1.0f,  1.0f, 1.0f,
                                                      1.0f, -8.0f, 1.0f,
                                                      1.0f,  1.0f, 1.0f));

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessEmboss>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessEmboss);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-2.0f, -1.0f, 1.0f,
                                                      -1.0f,  1.0f, 1.0f,
                                                       0.0f,  1.0f, 2.0f));

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessFlipHorizontally>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessFlipHorizontally);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessFlipVertically>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessFlipVertically);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessGammaCorrection>(ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessGammaCorrection);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);
  instance.setUniform("uGamma", 2.2f);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessGrayscale>(ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessGrayscale);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessInvert>(ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessInvert);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessSharpen>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessSharpen);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-1.0f, -1.0f, -1.0f,
                                                      -1.0f,  9.0f, -1.0f,
                                                      -1.0f, -1.0f, -1.0f));

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessSobelBottom>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessSobelBottom);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-1.0f, -2.0f, -1.0f,
                                                       0.0f,  0.0f,  0.0f,
                                                       1.0f,  2.0f,  1.0f));

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessSobelLeft>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessSobelLeft);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(1.0f, 0.0f, -1.0f,
                                                      2.0f, 0.0f, -2.0f,
                                                      1.0f, 0.0f, -1.0f));

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessSobelRight>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessSobelRight);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3(-1.0f, 0.0f, 1.0f,
                                                      -2.0f, 0.0f, 2.0f,
                                                      -1.0f, 0.0f, 1.0f));

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::PostProcessSobelTop>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::PostProcessSobelTop);

  instance.setUniform("uScreenTexture", BINDING_SAMPLER_SCREEN);
  instance.setUniform("uOffset", 1.0f / 3000.0f);
  instance.setUniform("uKernel", glm::mat3( 1.0f,  2.0f,  1.0f,
                                                       0.0f,  0.0f,  0.0f,
                                                      -1.0f, -2.0f, -1.0f));

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::Skybox>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::Skybox);

  instance.setUniform("uSkyTexture", BINDING_SAMPLER_ENVIRONMENT);

  return instance;
}

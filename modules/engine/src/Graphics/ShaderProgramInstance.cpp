#include <Graphics/ShaderProgramInstance.hpp>

#include <Graphics/Buffers/BindPoints.hpp>

#include <ranges>

void ShaderProgramInstance::use() const {
  glUseProgram(mShaderProgram->id());
}

void ShaderProgramInstance::bindUniforms() const {
  ZoneScoped;

  for (const ShaderUniform& uniform : uniforms | std::views::values) {
    uniform.bind();
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
  instance.setUniform("uMaterial.diffuseOverlay", BINDING_SAMPLER_DIFFUSE_OVERLAY);
  instance.setUniform("uMaterial.specular", BINDING_SAMPLER_SPECULAR);
  instance.setUniform("uMaterial.emission", BINDING_SAMPLER_EMISSION);
  instance.setUniform("uMaterial.normal", BINDING_SAMPLER_NORMAL);
  instance.setUniform("uMaterial.shininess", 128.0f);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::LitSurface>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::LitSurface);

  instance.setUniform("uMaterial.diffuse", BINDING_SAMPLER_DIFFUSE);
  instance.setUniform("uMaterial.diffuseOverlay", BINDING_SAMPLER_DIFFUSE_OVERLAY);
  instance.setUniform("uMaterial.specular", BINDING_SAMPLER_SPECULAR);
  instance.setUniform("uMaterial.emission", BINDING_SAMPLER_EMISSION);
  instance.setUniform("uMaterial.normal", BINDING_SAMPLER_NORMAL);
  instance.setUniform("uMaterial.shininess", 128.0f);

  instance.setUniform("uDirectionalLightShadowMaps", BINDING_SAMPLER_DIRECTIONAL_SHADOWS);
  instance.setUniform("uPointLightShadowMaps", BINDING_SAMPLER_POINT_SHADOWS);
  instance.setUniform("uSpotlightShadowMaps", BINDING_SAMPLER_SPOTLIGHT_SHADOWS);

  return instance;
}

template <>
ShaderProgramInstance ShaderProgramInstance::create<ShaderProgramType::Outline>(const ShaderProgramHandle program) {
  ZoneScoped;

  ShaderProgramInstance instance(program, ShaderProgramType::Outline);
  
  instance.setUniform("uColor", glm::vec3(1.0f));
  
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

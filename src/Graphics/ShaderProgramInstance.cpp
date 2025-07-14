#include <Graphics/Light.hpp>
#include <Graphics/Material.hpp>
#include <Graphics/ShaderProgramInstance.hpp>

ShaderProgramInstance ShaderProgramInstance::newLitSurface(NotNull<ShaderProgram> program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uViewPos", glm::vec3(0.0f));
  
  constexpr Material material;
  instance.setUniform("uMaterial.diffuse", material.diffuse);
  instance.setUniform("uMaterial.specular", material.specular);
  instance.setUniform("uMaterial.emission", material.emission);
  instance.setUniform("uMaterial.shininess", material.shininess);
  
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

ShaderProgramInstance ShaderProgramInstance::newLight(NotNull<ShaderProgram> program) {
  ShaderProgramInstance instance(program);
  
  instance.setUniform("uLightColor", glm::vec3(1.0f));
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newOutline(NotNull<ShaderProgram> program) {
  ShaderProgramInstance instance(program);
  
  instance.setUniform("uOutlineColor", glm::vec3(1.0f));
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newVisualiseDepth(NotNull<ShaderProgram> program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uCamera.near", 0.01f);
  instance.setUniform("uCamera.far", 30.0f);  

  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newVisualiseNormal(NotNull<ShaderProgram> program) {
  ShaderProgramInstance instance(program);
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingCopy(NotNull<ShaderProgram> program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);
  
  return instance;
}

ShaderProgramInstance ShaderProgramInstance::newPostProcessingInvert(NotNull<ShaderProgram> program) {
  ShaderProgramInstance instance(program);

  instance.setUniform("uScreenTexture", 0);

  return instance;
}

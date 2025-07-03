#include <ShaderPrograms/LitSurfaceShaderProgram.hpp>

#include <glm/gtc/type_ptr.inl>

LitSurfaceShaderProgram::LitSurfaceShaderProgram(GLuint shaderProgram)
  : ShaderProgram(shaderProgram)
{}

void LitSurfaceShaderProgram::bindUniforms(const TransformMatrices& transforms) const {
  ShaderProgram::bindUniforms(transforms);

  glUniform1i(glGetUniformLocation(mShaderProgram, "uMaterial.diffuse"), material.diffuse);
  glUniform1i(glGetUniformLocation(mShaderProgram, "uMaterial.specular"), material.specular);
  glUniform1i(glGetUniformLocation(mShaderProgram, "uMaterial.emission"), material.emission);
  glUniform1f(glGetUniformLocation(mShaderProgram, "uMaterial.shininess"), material.shininess);

  glUniform3fv(glGetUniformLocation(mShaderProgram, "uDirectionalLight.direction"), 1, glm::value_ptr(directionalLight.direction));
  glUniform3fv(glGetUniformLocation(mShaderProgram, "uDirectionalLight.colors.ambient"), 1, glm::value_ptr(directionalLight.colors.ambient));
  glUniform3fv(glGetUniformLocation(mShaderProgram, "uDirectionalLight.colors.diffuse"), 1, glm::value_ptr(directionalLight.colors.diffuse));
  glUniform3fv(glGetUniformLocation(mShaderProgram, "uDirectionalLight.colors.specular"), 1, glm::value_ptr(directionalLight.colors.specular));

  glUniform3fv(glGetUniformLocation(mShaderProgram, "uPointLight.position"), 1, glm::value_ptr(pointLight.position));
  glUniform3fv(glGetUniformLocation(mShaderProgram, "uPointLight.colors.ambient"), 1, glm::value_ptr(pointLight.colors.ambient));
  glUniform3fv(glGetUniformLocation(mShaderProgram, "uPointLight.colors.diffuse"), 1, glm::value_ptr(pointLight.colors.diffuse));
  glUniform3fv(glGetUniformLocation(mShaderProgram, "uPointLight.colors.specular"), 1, glm::value_ptr(pointLight.colors.specular));
  glUniform1f(glGetUniformLocation(mShaderProgram, "uPointLight.constant"), pointLight.constant);
  glUniform1f(glGetUniformLocation(mShaderProgram, "uPointLight.linear"), pointLight.linear);
  glUniform1f(glGetUniformLocation(mShaderProgram, "uPointLight.quadratic"), pointLight.quadratic);

  glUniform3fv(glGetUniformLocation(mShaderProgram, "uSpotlight.position"), 1, glm::value_ptr(spotlight.position));
  glUniform3fv(glGetUniformLocation(mShaderProgram, "uSpotlight.direction"), 1, glm::value_ptr(spotlight.direction));
  glUniform3fv(glGetUniformLocation(mShaderProgram, "uSpotlight.colors.ambient"), 1, glm::value_ptr(spotlight.colors.ambient));
  glUniform3fv(glGetUniformLocation(mShaderProgram, "uSpotlight.colors.diffuse"), 1, glm::value_ptr(spotlight.colors.diffuse));
  glUniform3fv(glGetUniformLocation(mShaderProgram, "uSpotlight.colors.specular"), 1, glm::value_ptr(spotlight.colors.specular));
  glUniform1f(glGetUniformLocation(mShaderProgram, "uSpotlight.cutOff"), spotlight.cutOff);
  glUniform1f(glGetUniformLocation(mShaderProgram, "uSpotlight.outerCutOff"), spotlight.outerCutOff);
}

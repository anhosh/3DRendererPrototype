#include <ShaderPrograms/LightSourceShaderProgram.hpp>

#include <glm/gtc/type_ptr.inl>

LightSourceShaderProgram::LightSourceShaderProgram(GLuint shaderProgram)
  : ShaderProgram(shaderProgram)
{}

void LightSourceShaderProgram::bindUniforms(const TransformMatrices& transforms) const {
  ShaderProgram::bindUniforms(transforms);
  glUniform3fv(4, 1, glm::value_ptr(emittedColor)); // uLightColor
}

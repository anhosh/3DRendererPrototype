#include <glm/gtc/type_ptr.inl>
#include <Graphics/ShaderPrograms/OutlineShaderProgram.hpp>

OutlineShaderProgram::OutlineShaderProgram(GLuint shaderProgram)
  : ShaderProgram(shaderProgram)
{}

void OutlineShaderProgram::bindUniforms(const TransformMatrices& transforms) const {
  ShaderProgram::bindUniforms(transforms);

  glUniform3fv(4, 1, glm::value_ptr(outlineColor));
}

#include <Graphics/ShaderPrograms/ShaderProgram.hpp>

#include <glm/gtc/type_ptr.hpp>

ShaderProgram::ShaderProgram(GLuint shaderProgram)
  : mShaderProgram(shaderProgram)
{}

void ShaderProgram::use() const {
  glUseProgram(mShaderProgram);
}

void ShaderProgram::stopUsing() const {
  (void)mShaderProgram;
  glUseProgram(0);
}

void ShaderProgram::destroy() {
  glDeleteProgram(mShaderProgram);
  mShaderProgram = 0;
}

void ShaderProgram::bindUniforms(const TransformMatrices& transforms) const {
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(transforms.model));
  glUniformMatrix3fv(1, 1, GL_FALSE, glm::value_ptr(transforms.normal));
  glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(transforms.view));
  glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(transforms.projection));
}

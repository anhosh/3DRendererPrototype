#include <Graphics/ShaderProgram.hpp>

#include <glm/gtc/type_ptr.hpp>

ShaderProgram::ShaderProgram(GLuint shaderProgram)
  : mID(shaderProgram)
{}

void ShaderProgram::bindTransforms(const TransformMatrices& transforms) const {
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(transforms.model));
  glUniformMatrix3fv(1, 1, GL_FALSE, glm::value_ptr(transforms.normal));
  glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(transforms.view));
  glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(transforms.projection));
}

void ShaderProgram::destroy() {
  glDeleteProgram(mID);
  mID = 0;
}

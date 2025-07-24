#include <Graphics/ShaderProgram.hpp>

#include <glm/gtc/type_ptr.hpp>

ShaderProgram::ShaderProgram(const GLuint shaderProgram)
  : mID(shaderProgram)
{}

void ShaderProgram::bindTransforms(const TransformMatrices& transforms) const {
  (void)mID;
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(transforms.model));
  glUniformMatrix3fv(1, 1, GL_FALSE, glm::value_ptr(transforms.normal));
}

void ShaderProgram::destroy() {
  glDeleteProgram(mID);
  mID = 0;
}

#include <Graphics/ShaderPrograms/VisualiseDepthShaderProgram.hpp>


VisualiseDepthShaderProgram::VisualiseDepthShaderProgram(GLuint shaderProgram)
  : ShaderProgram(shaderProgram)
{}

void VisualiseDepthShaderProgram::bindUniforms(const TransformMatrices& transforms) const {
  ShaderProgram::bindUniforms(transforms);

  glUniform1f(glGetUniformLocation(mShaderProgram, "uCamera.near"), frustumNear);
  glUniform1f(glGetUniformLocation(mShaderProgram, "uCamera.far"), frustumFar);
}

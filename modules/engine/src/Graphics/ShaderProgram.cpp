#include <Graphics/ShaderProgram.hpp>

#include <Util/Macros/Errors.hpp>

#include <filesystem>

namespace fs = std::filesystem;

ShaderProgram::ShaderProgram(const GLuint shaderProgram)
  : mID(shaderProgram)
{}

Expected<ShaderProgram> ShaderProgram::fromShaders(const ShaderProgramPaths& shaderPaths) {
  ZoneScoped;

  const Expected shaderProgram = createShaderProgram(shaderPaths);
  RETURN_ERROR_IF_UNEXPECTED(shaderProgram);
  return ShaderProgram(shaderProgram.value());
}

void ShaderProgram::destroy() {
  ZoneScoped;

  glDeleteProgram(mID);
  mID = 0;
}

GLint ShaderProgram::uniformLocation(const std::string_view uniformName) const {
  return glGetUniformLocation(mID, uniformName.data());
}

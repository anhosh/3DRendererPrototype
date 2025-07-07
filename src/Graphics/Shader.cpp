#include <Graphics/Shader.hpp>

#include <Util/Macros.hpp>
#include <Util/Paths.hpp>

#include <fstream>
#include <vector>

static fs::path sShadersDir = "shaders";

bool locateShaders() {
  if (std::optional<fs::path> shadersDir = locateDirectory("shaders")) {
    sShadersDir = shadersDir.value();
    return true;
  }
  return false;
}

std::expected<GLuint, std::string> createShader(GLenum type, const fs::path& sourcePath) {
  auto file = std::ifstream(sShadersDir / sourcePath, std::ios::ate);
  const std::streamsize fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  std::string source;
  source.resize(static_cast<size_t>(fileSize));
  file.read(source.data(), fileSize);
  const GLchar* sourceCStr = source.data();

  const GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &sourceCStr, nullptr);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::string infoLog;
    GLint infoLogLength;

    infoLog.resize(512);
    glGetShaderInfoLog(shader, static_cast<GLint>(infoLog.size()), &infoLogLength, infoLog.data());
    infoLog.resize(static_cast<size_t>(infoLogLength));

    return std::unexpected(std::format("Failed to compile shader '{}':\n{}", sourcePath.c_str(), infoLog));
  }

  return shader;
}

std::expected<GLuint, std::string> createShaderProgram(const ShaderStages& shaderStages) {
  std::vector<GLuint> shaders;
  shaders.reserve(2); // mandatory vertex and fragment shaders

  const auto addShader = [&](GLenum shaderType, const fs::path& sourcePath) -> std::expected<void, std::string> {
    const std::expected<GLuint, std::string> shader = createShader(shaderType, sourcePath);
    if (!shader.has_value()) {
      return std::unexpected(std::move(shader.error()));
    }
    shaders.push_back(shader.value());
    return {};
  };

  RETURN_ERROR_IF_UNEXPECTED(addShader(GL_VERTEX_SHADER, shaderStages.vertex));
  if (shaderStages.tesselationControl.has_value()) {
    RETURN_ERROR_IF_UNEXPECTED(addShader(GL_TESS_CONTROL_SHADER, shaderStages.tesselationControl.value()));
  }
  if (shaderStages.tesselationEvaluation.has_value()) {
    RETURN_ERROR_IF_UNEXPECTED(addShader(GL_TESS_EVALUATION_SHADER, shaderStages.tesselationEvaluation.value()));
  }
  if (shaderStages.geometry.has_value()) {
    RETURN_ERROR_IF_UNEXPECTED(addShader(GL_GEOMETRY_SHADER, shaderStages.geometry.value()));
  }
  RETURN_ERROR_IF_UNEXPECTED(addShader(GL_FRAGMENT_SHADER, shaderStages.fragment));

  const GLuint program = glCreateProgram();
  for (const GLuint shader : shaders) {
    glAttachShader(program, shader);
  }
  glLinkProgram(program);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    std::string infoLog;
    GLint infoLogLength;

    infoLog.resize(512);
    glGetProgramInfoLog(program, static_cast<GLint>(infoLog.size()), &infoLogLength, infoLog.data());
    infoLog.resize(static_cast<size_t>(infoLogLength));

    return std::unexpected(std::format("Failed to link shader program:\n{}", infoLog));
  }

  for (const GLuint shader : shaders) {
    glDeleteShader(shader);
  }

  return program;
}

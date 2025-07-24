#include <Graphics/Shader.hpp>

#include <Graphics/UniformBuffer.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/Paths.hpp>

#include <stb_include.h>

#include <array>
#include <filesystem>
#include <vector>

static fs::path sShadersDir = "shaders";
static bool sLocatedShaders = false;

bool locateShaders() {
  if (sLocatedShaders) {
    return true;
  }
  if (std::optional<fs::path> shadersDir = locateDirectory("shaders")) {
    sShadersDir = shadersDir.value();
    sLocatedShaders = true;
    return true;
  }
  return false;
}

Expected<Shader> createShader(GLenum type, const fs::path& sourcePath) {
  if (!locateShaders()) {
    PANIC("Could not locate shaders directory");
  }

  char error[256];
  const char* sourceCStr = stb_include_file((sShadersDir / sourcePath).c_str(), nullptr, sShadersDir.c_str(), error);
  if (sourceCStr == nullptr) {
    return std::unexpected(std::format("Failed to load shader '{}':\n{}", sourcePath.c_str(), error));
  }

#define NEW_DEFINE(name) std::format("#define " #name " {}\n", name)
  const std::string defines = NEW_DEFINE(UBO_BIND_POINT_CAMERA) +
                              NEW_DEFINE(UBO_BIND_POINT_DIRECTIONAL_LIGHTS) +
                              NEW_DEFINE(UBO_BIND_POINT_POINT_LIGHTS) +
                              NEW_DEFINE(UBO_BIND_POINT_SPOTLIGHTS);
#undef NEW_DEFINE

  const auto sources = std::array {
    "#version 460 core\n",
    defines.c_str(),
    sourceCStr,
  };

  const GLuint shader = glCreateShader(type);
  glShaderSource(shader, sources.size(), sources.data(), nullptr);
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

  return Shader { type, shader };
}

Expected<GLuint> createShaderProgram(const ShaderProgramPaths& shaderStages) {
  std::vector<Shader> shaders;
  shaders.reserve(2); // mandatory vertex and fragment shaders

  const auto addShader = [&](GLenum shaderType, const fs::path& sourcePath) -> Expected<void> {
    const Expected shader = createShader(shaderType, sourcePath);
    RETURN_ERROR_IF_UNEXPECTED(shader);
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
  for (const Shader shader : shaders) {
    glAttachShader(program, shader.shader);
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

  for (const Shader shader : shaders) {
    glDeleteShader(shader.shader);
  }

  return program;
}

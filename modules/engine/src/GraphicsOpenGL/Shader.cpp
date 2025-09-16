#include <GraphicsOpenGL/Shader.hpp>

#include <GraphicsOpenGL/Buffers/BindPoints.hpp>
#include <GraphicsOpenGL/Buffers/Buffer.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/Paths.hpp>

#include <stb_include.h>

#include <array>
#include <filesystem>
#include <format>
#include <vector>

namespace GraphicsOpenGL {
  static fs::path sShadersDir = "shaders";
  static bool sLocatedShaders = false;

  bool locateShaders() {
    ZoneScoped;

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

  Expected<Shader> createShader(GLenum type, const fs::path& sourcePath, std::string_view defines) {
    ZoneScoped;

    if (!locateShaders()) {
      PANIC("Could not locate shaders directory");
    }

    char error[256];
    const char* sourceCStr = stb_include_file((sShadersDir / sourcePath).string().c_str(), nullptr,
                                              sShadersDir.string().c_str(), error);
    if (sourceCStr == nullptr) {
      return std::unexpected(std::format("Failed to load shader '{}':\n{}", sourcePath.string(), error));
    }

    const auto sources = std::array {
      "#version 460 core\n",
      defines.data(),
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

      return std::unexpected(std::format("Failed to compile shader '{}':\n{}", sourcePath.string(), infoLog));
    }

    return Shader { type, shader };
  }

  Expected<GLuint> createShaderProgram(const ShaderProgramPaths& shaderPaths) {
    ZoneScoped;

    std::vector<Shader> shaders;
    shaders.reserve(2); // mandatory vertex and fragment shaders

    const auto addShader = [&](const GLenum shaderType, const fs::path& sourcePath, const std::string_view defines) -> Expected<void> {
      const Expected shader = createShader(shaderType, sourcePath, defines);
      RETURN_ERROR_IF_UNEXPECTED(shader);
      shaders.push_back(shader.value());
      return {};
    };

#define NEW_DEFINE(name) std::format("#define " #name " {}\n", static_cast<int32_t>(name))
    const auto HAS_GEOMETRY_SHADER = static_cast<int32_t>(shaderPaths.geometry.has_value());
    const std::string defines = NEW_DEFINE(BINDING_UBO_CAMERA) +
                                NEW_DEFINE(BINDING_SSBO_INSTANCES) +
                                NEW_DEFINE(BINDING_SSBO_DIRECTIONAL_LIGHTS) +
                                NEW_DEFINE(BINDING_SSBO_POINT_LIGHTS) +
                                NEW_DEFINE(BINDING_SSBO_SPOTLIGHTS) +
                                NEW_DEFINE(HAS_GEOMETRY_SHADER);
#undef NEW_DEFINE

    RETURN_ERROR_IF_UNEXPECTED(addShader(GL_VERTEX_SHADER, shaderPaths.vertex, defines));
    if (shaderPaths.tesselationControl.has_value()) {
      RETURN_ERROR_IF_UNEXPECTED(addShader(GL_TESS_CONTROL_SHADER, shaderPaths.tesselationControl.value(), defines));
    }
    if (shaderPaths.tesselationEvaluation.has_value()) {
      RETURN_ERROR_IF_UNEXPECTED(addShader(GL_TESS_EVALUATION_SHADER, shaderPaths.tesselationEvaluation.value(), defines));
    }
    if (shaderPaths.geometry.has_value()) {
      RETURN_ERROR_IF_UNEXPECTED(addShader(GL_GEOMETRY_SHADER, shaderPaths.geometry.value(), defines));
    }
    RETURN_ERROR_IF_UNEXPECTED(addShader(GL_FRAGMENT_SHADER, shaderPaths.fragment, defines));

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

      std::string shaderList = std::format("VS: {}", shaderPaths.vertex.string());
      if (shaderPaths.tesselationControl.has_value()) {
        shaderList += std::format(", TC: {}", shaderPaths.tesselationControl->string());
      }
      if (shaderPaths.tesselationEvaluation.has_value()) {
        shaderList += std::format(", TE: {}", shaderPaths.tesselationEvaluation->string());
      }
      if (shaderPaths.geometry.has_value()) {
        shaderList += std::format(", GS: {}", shaderPaths.geometry->string());
      }
      shaderList += std::format(", FS: {}", shaderPaths.fragment.string());
      return std::unexpected(std::format("Failed to link shader program ({}):\n{}", shaderList, infoLog));
    }

    for (const Shader shader : shaders) {
      glDeleteShader(shader.shader);
    }

    return program;
  }
}

#pragma once

#include <Util/Expected.hpp>

#include <filesystem>

namespace GraphicsOpenGL {
  struct Shader {
    GLenum type = GL_NONE;
    GLuint shader = GL_NONE;
  };

  struct ShaderProgramPaths {
    std::filesystem::path vertex;
    std::optional<std::filesystem::path> tesselationControl = std::nullopt;
    std::optional<std::filesystem::path> tesselationEvaluation = std::nullopt;
    std::optional<std::filesystem::path> geometry = std::nullopt;
    std::filesystem::path fragment;
  };

  struct ShaderProgramShaders {
    Shader vertex;
    std::optional<Shader> tesselationControl = std::nullopt;
    std::optional<Shader> tesselationEvaluation = std::nullopt;
    std::optional<Shader> geometry = std::nullopt;
    Shader fragment;
  };

  bool locateShaders();

  Expected<Shader> createShader(GLenum type, const std::filesystem::path& sourcePath, std::string_view defines);
  Expected<GLuint> createShaderProgram(const ShaderProgramPaths& shaderPaths);
}

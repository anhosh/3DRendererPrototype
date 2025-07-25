#pragma once

#include <Util/Expected.hpp>

#include <filesystem>

namespace fs = std::filesystem;

struct Shader {
  GLenum type = GL_NONE;
  GLuint shader = GL_NONE;
};

struct ShaderProgramPaths {
  fs::path vertex;
  std::optional<fs::path> tesselationControl = std::nullopt;
  std::optional<fs::path> tesselationEvaluation = std::nullopt;
  std::optional<fs::path> geometry = std::nullopt;
  fs::path fragment;
};

struct ShaderProgramShaders {
  Shader vertex;
  std::optional<Shader> tesselationControl = std::nullopt;
  std::optional<Shader> tesselationEvaluation = std::nullopt;
  std::optional<Shader> geometry = std::nullopt;
  Shader fragment;
};

bool locateShaders();

Expected<Shader> createShader(GLenum type, const fs::path& sourcePath, std::string_view defines);
Expected<GLuint> createShaderProgram(const ShaderProgramPaths& shaderStages);

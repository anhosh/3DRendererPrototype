#pragma once

#include <expected>
#include <filesystem>

namespace fs = std::filesystem;

struct ShaderStages {
  fs::path vertex;
  std::optional<fs::path> tesselationControl = std::nullopt;
  std::optional<fs::path> tesselationEvaluation = std::nullopt;
  std::optional<fs::path> geometry = std::nullopt;
  fs::path fragment;
};

bool locateShaders();

std::expected<GLuint, std::string> createShaderProgram(const ShaderStages& shaderStages);

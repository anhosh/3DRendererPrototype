#pragma once

#include <Graphics/ModelTransform.hpp>

#include <optional>
#include <vector>

struct Model {
  std::vector<size_t> meshIndices;
  ModelTransform transform;
  size_t shaderProgramIndex = SIZE_MAX;
  std::optional<size_t> outlineShaderProgramIndex = std::nullopt;
};

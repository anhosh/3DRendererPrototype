#pragma once

#include <Graphics/ModelTransform.hpp>

#include <vector>

struct Model {
  std::vector<size_t> meshIndices;
  size_t shaderProgramIndex = SIZE_MAX;
  ModelTransform transform;
};

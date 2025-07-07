#pragma once

#include <ModelTransform.hpp>

struct Draw {
  size_t meshIndex = 0;
  size_t shaderProgramIndex = 0;
  ModelTransform transform = {};
  bool backfaceCulling = true;
};

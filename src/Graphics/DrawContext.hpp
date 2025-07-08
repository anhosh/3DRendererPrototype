#pragma once

#include <Graphics/ModelTransform.hpp>

struct Draw {
  size_t meshIndex = 0;
  size_t shaderProgramIndex = 0;
  ModelTransform transform = {};
  bool bBackfaceCulling = true;
  bool bWriteToStencil = false;
  bool bStencilTest = false;
  bool bDepthTest = true;
};

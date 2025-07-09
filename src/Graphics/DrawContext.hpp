#pragma once

#include <Graphics/Transform.hpp>

struct Draw {
  Transform transform = {};
  size_t shaderProgramIndex = 0;
  size_t meshIndex = 0;
  size_t diffuseMapIndex = SIZE_MAX;
  size_t specularMapIndex = SIZE_MAX;
  size_t emissionMapIndex = SIZE_MAX;
  bool bBackfaceCulling = true;
  bool bWriteToStencil = false;
  bool bStencilTest = false;
  bool bDepthTest = true;
};

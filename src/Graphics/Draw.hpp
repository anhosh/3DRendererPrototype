#pragma once

#include <Graphics/Transform.hpp>

struct Draw {
  Transform transform = {};
  ShaderProgramInstanceHandle shaderProgramInstance;
  VertexArrayHandle vertexArray;
  std::optional<TextureHandle> diffuseMapIndex = std::nullopt;
  std::optional<TextureHandle> specularMapIndex = std::nullopt;
  std::optional<TextureHandle> emissionMapIndex = std::nullopt;
  bool bBackfaceCulling = true;
  bool bWriteToStencil = false;
  bool bStencilTest = false;
  bool bDepthTest = true;
  bool bTransparent = false;
};

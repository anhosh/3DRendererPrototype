#pragma once

#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/VertexArray.hpp>
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

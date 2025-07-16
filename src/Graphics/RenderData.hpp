#pragma once

#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/VertexArray.hpp>
#include <Util/Registry.hpp>

#include <optional>

struct RenderOptions {
  bool bBackfaceCulling = true;
  bool bTransparent = false;
  std::optional<ShaderProgramInstanceHandle> outlineShaderInstance = std::nullopt;
};

struct RenderData {
  VertexArrayHandle vertexArray;
  ShaderProgramInstanceHandle shaderProgramInstance;
  std::optional<TextureHandle> diffuseMap = std::nullopt;
  std::optional<TextureHandle> specularMap = std::nullopt;
  std::optional<TextureHandle> emissionMap = std::nullopt;
  RenderOptions renderOptions = {};
};

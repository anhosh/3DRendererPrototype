#pragma once

#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture2D.hpp>
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
  std::optional<Texture2DHandle> diffuseMap = std::nullopt;
  std::optional<Texture2DHandle> specularMap = std::nullopt;
  std::optional<Texture2DHandle> emissionMap = std::nullopt;
  RenderOptions renderOptions = {};
};

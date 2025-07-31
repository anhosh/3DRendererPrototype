#pragma once

#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture2D.hpp>
#include <Graphics/TextureCubeMap.hpp>
#include <Graphics/VertexArray.hpp>

#include <optional>

struct Draw {
  ShaderProgramInstanceHandle shaderProgramInstance;
  VertexArrayHandle vertexArray;

  size_t instanceCount = 1;

  std::optional<Texture2DHandle> diffuseMap = std::nullopt;
  std::optional<Texture2DHandle> specularMap = std::nullopt;
  std::optional<Texture2DHandle> emissionMap = std::nullopt;
  std::optional<TextureCubeMapHandle> environmentMap = std::nullopt;

  bool bBackfaceCulling = true;
  bool bWriteToStencil = false;
  bool bStencilTest = false;
  bool bWriteToDepth = true;
  bool bDepthTest = true;
  bool bTransparent = false;
  bool bSkybox = false;
};

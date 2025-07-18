#pragma once

#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture2D.hpp>
#include <Graphics/TextureCubeMap.hpp>
#include <Graphics/Transform.hpp>
#include <Graphics/VertexArray.hpp>

struct Draw {
  Transform transform = {};

  ShaderProgramInstanceHandle shaderProgramInstance;
  VertexArrayHandle vertexArray;

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
  bool bDisableCameraTranslation = false;
};

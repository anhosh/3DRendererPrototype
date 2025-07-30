#pragma once

#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Texture2D.hpp>
#include <Graphics/TextureCubeMap.hpp>
#include <Graphics/VertexArray.hpp>
#include <Util/Registry.hpp>

#include <optional>

struct RenderOptions {
  bool bBackfaceCulling = true;
  bool bTransparent = false;

  bool operator==(const RenderOptions&) const = default;
  bool operator!=(const RenderOptions&) const = default;
};

struct RenderData {
  VertexArrayHandle vertexArray;

  ShaderProgramInstanceHandle shaderProgramInstance;
  std::optional<ShaderProgramInstanceHandle> outlineShaderInstance = std::nullopt;

  std::optional<Texture2DHandle> diffuseMap = std::nullopt;
  std::optional<Texture2DHandle> specularMap = std::nullopt;
  std::optional<Texture2DHandle> emissionMap = std::nullopt;
  std::optional<TextureCubeMapHandle> environmentMap = std::nullopt;

  RenderOptions renderOptions = {};

  [[nodiscard]]
  bool eqIgnoreOutline(const RenderData& other) const {
    return vertexArray == other.vertexArray &&
           shaderProgramInstance == other.shaderProgramInstance &&
           diffuseMap == other.diffuseMap &&
           specularMap == other.specularMap &&
           emissionMap == other.emissionMap &&
           environmentMap == other.environmentMap &&
           renderOptions == other.renderOptions;
  }

  [[nodiscard]]
  bool eqIgnoreMainShader(const RenderData& other) const {
    return vertexArray == other.vertexArray &&
           outlineShaderInstance == other.outlineShaderInstance &&
           diffuseMap == other.diffuseMap &&
           specularMap == other.specularMap &&
           emissionMap == other.emissionMap &&
           environmentMap == other.environmentMap &&
           renderOptions == other.renderOptions;
  }

  bool operator==(const RenderData&) const = default;
  bool operator!=(const RenderData&) const = default;
};

#pragma once

#include <Graphics/Mesh.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Textures/Texture2D.hpp>
#include <Graphics/Textures/TextureCubeMap.hpp>
#include <Util/Registry.hpp>

struct RenderOptions {
  bool bBackfaceCulling = true;
  bool bTransparent = false;

  bool operator==(const RenderOptions&) const = default;
  bool operator!=(const RenderOptions&) const = default;
};

struct RenderData {
  MeshHandle mesh;

  ShaderProgramInstanceHandle shader;

  Texture2DHandle diffuseMap = Texture2DHandle::null();
  Texture2DHandle specularMap = Texture2DHandle::null();
  Texture2DHandle emissionMap = Texture2DHandle::null();
  TextureCubeMapHandle environmentMap = TextureCubeMapHandle::null();

  RenderOptions renderOptions = {};

  [[nodiscard]]
  bool eqIgnoreMainShader(const RenderData& other) const {
    return mesh == other.mesh &&
           diffuseMap == other.diffuseMap &&
           specularMap == other.specularMap &&
           emissionMap == other.emissionMap &&
           environmentMap == other.environmentMap &&
           renderOptions == other.renderOptions;
  }

  bool operator==(const RenderData&) const = default;
  bool operator!=(const RenderData&) const = default;
};

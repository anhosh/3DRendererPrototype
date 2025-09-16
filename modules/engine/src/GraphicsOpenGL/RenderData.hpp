#pragma once

#include <GraphicsOpenGL/Mesh.hpp>
#include <GraphicsOpenGL/ShaderProgramInstance.hpp>
#include <GraphicsOpenGL/Textures/Texture2D.hpp>
#include <GraphicsOpenGL/Textures/TextureCubeMap.hpp>
#include <Util/Registry.hpp>

namespace GraphicsOpenGL {
  struct RenderOptions {
    bool bBackfaceCulling = true;
    bool bTransparent = false;
    bool bWireframe = false;

    bool operator==(const RenderOptions&) const = default;
    bool operator!=(const RenderOptions&) const = default;
  };

  struct RenderData {
    MeshHandle mesh;

    ShaderProgramInstanceHandle shader;

    Texture2DHandle diffuseMap = Texture2DHandle::null();
    Texture2DHandle diffuseOverlayMap = Texture2DHandle::null();
    Texture2DHandle specularMap = Texture2DHandle::null();
    Texture2DHandle emissionMap = Texture2DHandle::null();
    Texture2DHandle normalMap = Texture2DHandle::null();
    TextureCubeMapHandle environmentMap = TextureCubeMapHandle::null();

    RenderOptions renderOptions = {};

    [[nodiscard]]
    bool eqIgnoreMainShader(const RenderData& other) const {
      return mesh == other.mesh &&
             diffuseMap == other.diffuseMap &&
             diffuseOverlayMap == other.diffuseOverlayMap &&
             specularMap == other.specularMap &&
             emissionMap == other.emissionMap &&
             normalMap == other.normalMap &&
             environmentMap == other.environmentMap &&
             renderOptions == other.renderOptions;
    }

    bool operator==(const RenderData&) const = default;
    bool operator!=(const RenderData&) const = default;
  };
}

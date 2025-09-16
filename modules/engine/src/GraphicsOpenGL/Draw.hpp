#pragma once

#include <GraphicsOpenGL/Mesh.hpp>
#include <GraphicsOpenGL/ShaderProgramInstance.hpp>
#include <GraphicsOpenGL/Textures/Texture2D.hpp>
#include <GraphicsOpenGL/Textures/TextureCubeMap.hpp>

namespace GraphicsOpenGL {
  struct Draw {
    ShaderProgramInstanceHandle shaderProgramInstance;
    MeshHandle mesh;

    size_t instanceOffset = 0;
    size_t instanceCount = 1;

    Texture2DHandle diffuseMap = Texture2DHandle::null();
    Texture2DHandle diffuseOverlayMap = Texture2DHandle::null();
    Texture2DHandle specularMap = Texture2DHandle::null();
    Texture2DHandle emissionMap = Texture2DHandle::null();
    Texture2DHandle normalMap = Texture2DHandle::null();
    TextureCubeMapHandle environmentMap = TextureCubeMapHandle::null();

    bool bBackfaceCulling = true;
    bool bWriteToStencil = false;
    bool bStencilTest = false;
    bool bWriteToDepth = true;
    bool bDepthTest = true;
    bool bTransparent = false;
    bool bSkybox = false;
    bool bWireframe = false;
  };
}

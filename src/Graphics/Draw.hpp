#pragma once

#include <Graphics/Mesh.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Textures/Texture2D.hpp>
#include <Graphics/Textures/TextureCubeMap.hpp>

struct Draw {
  ShaderProgramInstanceHandle shaderProgramInstance;
  MeshHandle mesh;

  size_t instanceOffset = 0;
  size_t instanceCount = 1;

  Texture2DHandle diffuseMap = Texture2DHandle::null();
  Texture2DHandle specularMap = Texture2DHandle::null();
  Texture2DHandle emissionMap = Texture2DHandle::null();
  TextureCubeMapHandle environmentMap = TextureCubeMapHandle::null();

  bool bBackfaceCulling = true;
  bool bWriteToStencil = false;
  bool bStencilTest = false;
  bool bWriteToDepth = true;
  bool bDepthTest = true;
  bool bTransparent = false;
  bool bSkybox = false;
};

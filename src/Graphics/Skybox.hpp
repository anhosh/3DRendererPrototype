#pragma once

#include <Graphics/Mesh.hpp>
#include <Graphics/Textures/TextureCubeMap.hpp>

struct Skybox {
  MeshHandle cubeMesh;
  TextureCubeMapHandle texture;
  ShaderProgramInstanceHandle shader;
};

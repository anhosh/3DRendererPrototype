#pragma once

#include <Graphics/Mesh.hpp>
#include <Graphics/TextureCubeMap.hpp>

struct Skybox {
  MeshHandle cubeMesh;
  TextureCubeMapHandle texture;
  ShaderProgramInstanceHandle shader;
};

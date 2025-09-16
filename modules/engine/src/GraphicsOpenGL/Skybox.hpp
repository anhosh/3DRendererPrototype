#pragma once

#include <GraphicsOpenGL/Mesh.hpp>
#include <GraphicsOpenGL/Textures/TextureCubeMap.hpp>

namespace GraphicsOpenGL {
  struct Skybox {
    MeshHandle cubeMesh;
    TextureCubeMapHandle texture;
    ShaderProgramInstanceHandle shader;
  };
}

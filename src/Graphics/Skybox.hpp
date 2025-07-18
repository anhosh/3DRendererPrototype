#pragma once

#include <Graphics/TextureCubeMap.hpp>
#include <Graphics/VertexArray.hpp>

struct Skybox {
  VertexArrayHandle cubeMesh;
  TextureCubeMapHandle texture;
  ShaderProgramInstanceHandle shader;
};

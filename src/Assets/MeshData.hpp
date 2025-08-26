#pragma once

#include <Graphics/Vertex.hpp>

#include <vector>

struct Frustum;

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  static MeshData createCube(glm::vec3 size = glm::vec3(1.0f));
  static MeshData createQuad(glm::vec2 size = glm::vec2(1.0f));
};


#include <Meshes.hpp>

Mesh createCubeMesh() {
  constexpr auto positions = std::array {
    glm::vec3(-0.5f, +0.5f, +0.5f), // 0: left  top    front
    glm::vec3(-0.5f, +0.5f, -0.5f), // 1: left  top    back
    glm::vec3(-0.5f, -0.5f, +0.5f), // 2: left  bottom front
    glm::vec3(-0.5f, -0.5f, -0.5f), // 3: left  bottom back
    glm::vec3(+0.5f, +0.5f, +0.5f), // 4: right top    front
    glm::vec3(+0.5f, +0.5f, -0.5f), // 5: right top    back
    glm::vec3(+0.5f, -0.5f, +0.5f), // 6: right bottom front
    glm::vec3(+0.5f, -0.5f, -0.5f), // 7: right bottom back
  };

  constexpr auto normals = std::array {
    glm::vec3(-1.0f, +0.0f, +0.0f), // left
    glm::vec3(+1.0f, +0.0f, +0.0f), // right
    glm::vec3(+0.0f, +1.0f, +0.0f), // top
    glm::vec3(+0.0f, -1.0f, +0.0f), // bottom
    glm::vec3(+0.0f, +0.0f, +1.0f), // front
    glm::vec3(+0.0f, +0.0f, -1.0f), // back
  };

  constexpr auto faceIndices = std::array {
    0u, 1u, 2u, 3u, // left
    5u, 4u, 7u, 6u, // right
    5u, 1u, 4u, 0u, // top
    6u, 2u, 7u, 3u, // bottom
    4u, 0u, 6u, 2u, // front
    1u, 5u, 3u, 7u, // back
  };

  constexpr auto texCoords = std::array {
    glm::vec2(1.0f, 1.0f), // 0: top right
    glm::vec2(0.0f, 1.0f), // 1: top left
    glm::vec2(1.0f, 0.0f), // 2: bottom right
    glm::vec2(0.0f, 0.0f), // 3: bottom left
  };

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  vertices.reserve(6 * 4);
  indices.reserve(6 * 6);

  for (uint32_t face = 0; face < faceIndices.size(); face += 4) {
    for (uint32_t vertex = 0; vertex < 4; ++vertex) {
      const uint32_t posIndex = faceIndices[face + vertex];
      vertices.push_back(Vertex {
        .position = positions[posIndex],
        .normal = normals[face / 4],
        .color = glm::vec3(1.0f),
        .texCoord = texCoords[vertex],
      });
    }
    indices.push_back(face + 0);
    indices.push_back(face + 1);
    indices.push_back(face + 2);
    indices.push_back(face + 1);
    indices.push_back(face + 3);
    indices.push_back(face + 2);
  }

  return Mesh(vertices, indices);
}

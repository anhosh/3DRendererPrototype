#pragma once

#include <Assets/Mesh.hpp>

class VertexArray {
public:
  VertexArray();
  VertexArray(const Mesh& mesh);

  void init();
  void destroy();
  void generateMesh(const Mesh& mesh);

public:
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ebo = 0;
  GLsizei indexCount = 0;
};

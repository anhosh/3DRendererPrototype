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
  GLuint vao = GL_NONE;
  GLuint vbo = GL_NONE;
  GLuint ebo = GL_NONE;
  GLsizei indexCount = 0;
};

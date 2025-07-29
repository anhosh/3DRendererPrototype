#pragma once

#include <Assets/Mesh.hpp>
#include <Util/Registry.hpp>

class VertexArray {
public:
  VertexArray();
  explicit VertexArray(const Mesh& mesh);

  void init();
  void destroy();
  void generateMesh(const Mesh& mesh);

public:
  GLuint vao = GL_NONE;
  GLuint vbo = GL_NONE;
  GLuint ebo = GL_NONE;
  GLuint ibo = GL_NONE;
  GLsizei indexCount = 0;
};

using VertexArrayHandle = Registry<VertexArray>::Handle;

#pragma once

#include <Assets/Mesh.hpp>
#include <Graphics/VertexBuffer.hpp>
#include <Util/Registry.hpp>

class VertexArray {
public:
  VertexArray();
  explicit VertexArray(const Mesh& mesh);

  void init();
  void destroy();
  void generateMesh(const Mesh& mesh);
  void bind() const;

public:
  VertexBuffer vertexData;
  VertexBuffer instanceData;
  GLuint ebo = GL_NONE;
  GLsizei indexCount = 0;
};

using VertexArrayHandle = Registry<VertexArray>::Handle;

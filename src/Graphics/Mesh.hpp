#pragma once

#include <Assets/MeshData.hpp>
#include <Graphics/Buffers/VertexBuffer.hpp>
#include <Util/Registry.hpp>

class Mesh {
public:
  Mesh();
  explicit Mesh(const MeshData& mesh);

  void init();
  void destroy();
  void generateMesh(const MeshData& mesh);
  void bind() const;

public:
  VertexBuffer vertexData;
  GLuint ebo = GL_NONE;
  GLsizei indexCount = 0;
};

using MeshHandle = Registry<Mesh>::Handle;

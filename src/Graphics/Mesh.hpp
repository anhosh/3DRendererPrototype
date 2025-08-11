#pragma once

#include <Assets/MeshData.hpp>
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
  GLsizei indexCount() const { return mIndexCount; }
  GLsizei indicesOffset() const { return mIndicesOffset; }

private:
  GLuint mVertexIndexBuffer = GL_NONE;
  GLsizei mIndexCount = 0;
  GLsizei mIndicesOffset = 0;
};

using MeshHandle = Registry<Mesh>::Handle;

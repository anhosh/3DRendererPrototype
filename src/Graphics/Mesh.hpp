#pragma once

#include <Assets/MeshData.hpp>
#include <Util/Math/AABB.hpp>
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
  [[nodiscard]] GLsizei indexCount() const { return mIndexCount; }
  [[nodiscard]] GLsizei indicesOffset() const { return mIndicesOffset; }
  [[nodiscard]] AABB boundingBox() const { return mBoundingBox; }

private:
  GLuint mVertexIndexBuffer = GL_NONE;
  GLsizei mIndexCount = 0;
  GLsizei mIndicesOffset = 0;

  AABB mBoundingBox;
};

using MeshHandle = Registry<Mesh>::Handle;

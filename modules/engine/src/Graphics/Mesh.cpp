#include <Graphics/Mesh.hpp>

#include <Util/Memory.hpp>

#include <cassert>

Mesh::Mesh() {
  ZoneScoped;

  this->init();
}

Mesh::Mesh(const MeshData& mesh) {
  ZoneScoped;

  this->init();
  this->generateMesh(mesh);
}

void Mesh::init() {
  ZoneScoped;

  glCreateBuffers(1, &mVertexIndexBuffer);
}

void Mesh::destroy() {
  ZoneScoped;

  glDeleteBuffers(1, &mVertexIndexBuffer);
  mVertexIndexBuffer = GL_NONE;
}

void Mesh::generateMesh(const MeshData& mesh) {
  ZoneScoped;

  assert(mesh.indices.size() % 3 == 0);

  mBoundingBox = {};
  for (const Vertex& vertex : mesh.vertices) {
    mBoundingBox.includePoint(vertex.position);
  }

  const auto verticesSize = static_cast<GLsizei>(mesh.vertices.size() * sizeof(Vertex));
  const auto indicesSize = static_cast<GLsizei>(mesh.vertices.size() * sizeof(Vertex));

  GLsizei alignment = GL_NONE;
  glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
  const GLsizei verticesSizeAligned = align(verticesSize, alignment);
  const GLsizei indicesSizeAligned = align(indicesSize, alignment);

  constexpr size_t vertexOffset = 0;
  mIndicesOffset = verticesSizeAligned;

  glNamedBufferStorage(mVertexIndexBuffer, verticesSizeAligned + indicesSizeAligned, nullptr, GL_DYNAMIC_STORAGE_BIT);
  glNamedBufferSubData(mVertexIndexBuffer, vertexOffset, verticesSize, mesh.vertices.data());
  glNamedBufferSubData(mVertexIndexBuffer, mIndicesOffset, indicesSize, mesh.indices.data());

  mIndexCount = static_cast<GLsizei>(mesh.indices.size());
}

void Mesh::bind() const {
  glBindVertexBuffer(0, mVertexIndexBuffer, 0, sizeof(Vertex));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVertexIndexBuffer);
}

#include <Graphics/Mesh.hpp>

#include <cassert>
#include <Util/Alignment.hpp>

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

  const size_t verticesSize = mesh.vertices.size() * sizeof(Vertex);
  const size_t indicesSize = mesh.indices.size() * sizeof(uint32_t);

  GLint alignment = GL_NONE;
  glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
  const size_t verticesSizeAligned = align(verticesSize, alignment);
  const size_t indicesSizeAligned = align(indicesSize, alignment);

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

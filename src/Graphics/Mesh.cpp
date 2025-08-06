#include <Graphics/Mesh.hpp>

#include <Graphics/InstanceData.hpp>

#include <cassert>

Mesh::Mesh()
  : vertexData(sizeof(Vertex))
  , instanceData(sizeof(InstanceData))
{
  ZoneScoped;

  this->init();
}

Mesh::Mesh(const MeshData& mesh) : Mesh() {
  ZoneScoped;

  this->generateMesh(mesh);
}

void Mesh::init() {
  ZoneScoped;

  vertexData.init();
  instanceData.init();
  glCreateBuffers(1, &ebo);
}

void Mesh::destroy() {
  ZoneScoped;

  vertexData.destroy();
  instanceData.destroy();
  if (ebo != GL_NONE) {
    glDeleteBuffers(1, &ebo);
    ebo = GL_NONE;
  }
}

void Mesh::generateMesh(const MeshData& mesh) {
  ZoneScoped;

  assert(mesh.indices.size() % 3 == 0);

  vertexData.write(mesh.vertices);
  glNamedBufferData(ebo, static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(uint32_t)), mesh.indices.data(), GL_STATIC_DRAW);

  indexCount = static_cast<GLsizei>(mesh.indices.size());
}

void Mesh::bind() const {
  vertexData.bind(0);
  instanceData.bind(1);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

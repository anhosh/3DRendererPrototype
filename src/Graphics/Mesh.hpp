#pragma once

#include <Graphics/ModelTransform.hpp>
#include <Graphics/Vertex.hpp>

#include <span>
#include <vector>

class Mesh {
public:
  Mesh();
  Mesh(std::span<const Vertex> vertices, std::span<const GLuint> indices);

  void init();
  void destroy();
  void generateMesh(std::span<const Vertex> vertices, std::span<const GLuint> indices);

  void bind() const;
  void unbind() const;
  void bindAndDraw() const;
  void draw() const;

public:
  ModelTransform transform;
  std::vector<size_t> diffuseMapIndices;
  std::vector<size_t> specularMapIndices;
  std::vector<size_t> emissionMapIndices;

private:
  GLuint mVAO = 0;
  GLuint mVBO = 0;
  GLuint mEBO = 0;
  GLsizei mIndexCount = 0;
};

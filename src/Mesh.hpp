#pragma once

#include <Textures.hpp>
#include <Vertex.hpp>

#include <span>

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

private:
  GLuint mVAO = 0;
  GLuint mVBO = 0;
  GLuint mEBO = 0;
  GLsizei mIndexCount = 0;
};

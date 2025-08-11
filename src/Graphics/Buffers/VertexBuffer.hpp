#pragma once

#include <vector>

class VertexBuffer {
public:
  explicit VertexBuffer(size_t stride);

  void init();
  void destroy();
  void bind(uint32_t binding, size_t offset = 0) const;

  template <typename DataType>
  void write(const std::vector<DataType>& data) {
    if (data.empty()) {
      return;
    }
    glNamedBufferData(vbo, static_cast<GLsizeiptr>(data.size() * sizeof(DataType)), data.data(), GL_STATIC_DRAW);
  }

  GLuint vbo = GL_NONE;
  GLsizei stride = 0;
};

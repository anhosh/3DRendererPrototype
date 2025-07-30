#pragma once

#include <Util/IntoBytes.hpp>

#include <vector>

struct InstanceData {
  glm::mat4 model;
  glm::mat3 normal;
};

struct InstanceBufferData {
  std::vector<InstanceData> instances;

  [[nodiscard]] size_t size() const {
    return sizeof(InstanceData) * instances.size();
  }

  void writeToBuffer(std::vector<uint8_t>& buffer) const {
    for (const InstanceData& instance : instances) {
      buffer.append_range(asBytes(instance));
    }
  }

  void setupInstanceVertexAttributes(VertexArrayHandle vertexArray, BufferHandle instanceBuffer) const {
    (void)instances;

    glBindVertexArray(vertexArray->vao);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer->id());

    constexpr GLsizei stride = sizeof(glm::mat4) + sizeof(glm::mat3);
    for (size_t i = 0; i < 4; ++i) {
      const size_t offset = sizeof(glm::vec4) * i;
      glEnableVertexAttribArray(4 + i);
      glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offset));
      glVertexAttribDivisor(4 + i, 1);
    }
    for (size_t i = 0; i < 3; ++i) {
      const size_t offset = sizeof(glm::mat4) + sizeof(glm::vec3) * i;
      glEnableVertexAttribArray(8 + i);
      glVertexAttribPointer(8 + i, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offset));
      glVertexAttribDivisor(8 + i, 1);
    }
    glBindVertexArray(GL_NONE);
  }
};

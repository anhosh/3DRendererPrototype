#pragma once

#include <Graphics/Buffer.hpp>
#include <Graphics/VertexArray.hpp>

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

  void writeToBuffer(GLenum target, size_t offset) const;
  void setupInstanceVertexAttributes(VertexArrayHandle vertexArray, BufferHandle instanceBuffer) const;
};

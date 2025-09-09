#pragma once

#include <Graphics/Buffers/InstanceBuffer.hpp>

#include <span>
#include <vector>

struct InstanceData {
  InstanceData() = default;

  explicit InstanceData(const glm::mat4& model, const glm::mat3& normal)
    : model(model)
  {
    this->normal[0] = glm::vec4(normal[0], 0.0f);
    this->normal[1] = glm::vec4(normal[1], 0.0f);
    this->normal[2] = glm::vec4(normal[2], 0.0f);
  }

  glm::mat4 model;
  glm::mat3x4 normal; // Matrix rows are aligned to vec4 in std430.
};

struct InstanceBuffer {
  std::vector<InstanceData> instances;

  [[nodiscard]] size_t size() const {
    return this->instances.size() * sizeof(InstanceData);
  }

  void writeToBuffer(std::span<uint8_t> buffer, size_t offset = 0) const;
};

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
};

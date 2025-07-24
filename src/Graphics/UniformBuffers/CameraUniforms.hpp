#pragma once

struct CameraUniforms {
  glm::mat4 view;
  glm::mat4 projection;
  glm::vec3 position;
  float _padding0 = 0.0f;
  float near;
  float far;

  static size_t size() {
    return sizeof(CameraUniforms);
  }

  void writeToBuffer(std::vector<uint8_t>& buffer) const {
    const std::array bytes = asBytes(*this);
    buffer.insert(buffer.begin(), bytes.begin(), bytes.end());
  }
};

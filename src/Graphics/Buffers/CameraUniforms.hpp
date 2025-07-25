#pragma once

struct CameraUniforms {
  glm::mat4 view;
  glm::mat4 projection;
  glm::vec3 position;
  float near;
  float far;
  float _padding0 = 0.0f;
  float _padding1 = 0.0f;
  float _padding3 = 0.0f;

  static CameraUniforms from(const Camera& camera, const glm::uvec2 screenSize) {
    return CameraUniforms {
      .view = camera.view(),
      .projection = camera.projection(screenSize),
      .position = camera.position,
      .near = camera.near,
      .far = camera.far,
    };
  }

  static size_t size() {
    return sizeof(CameraUniforms);
  }

  void writeToBuffer(std::vector<uint8_t>& buffer) const {
    buffer.append_range(asBytes(*this));
  }
};

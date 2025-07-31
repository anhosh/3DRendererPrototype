#pragma once

struct CompCamera;
struct CompTransform;

struct CameraUniforms {
  glm::mat4 view;
  glm::mat4 projection;
  glm::vec3 position;
  float near;
  float far;
  float _padding0 = 0.0f;
  float _padding1 = 0.0f;
  float _padding3 = 0.0f;

  static CameraUniforms from(const CompCamera& camera, const CompTransform& transform, glm::uvec2 screenSize);

  static constexpr size_t size() {
    return sizeof(CameraUniforms);
  }

  void writeToBuffer(GLenum target, size_t offset) const;
};

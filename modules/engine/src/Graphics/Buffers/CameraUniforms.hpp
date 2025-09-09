#pragma once

#include <span>

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

  static CameraUniforms fromPerspective(const CompCamera& camera, const CompTransform& transform);
  static CameraUniforms fromOrthographic(const CompCamera& camera, const CompTransform& transform);

  static constexpr size_t size() {
    return sizeof(CameraUniforms);
  }

  void writeToBuffer(std::span<uint8_t> buffer, size_t offset) const;
};

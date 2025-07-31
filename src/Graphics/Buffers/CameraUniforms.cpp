#include <Graphics/Buffers/CameraUniforms.hpp>

#include <Graphics/Components/Camera.hpp>
#include <Graphics/Components/Transform.hpp>

CameraUniforms CameraUniforms::from(const CompCamera& camera, const CompTransform& transform, glm::uvec2 screenSize) {
  ZoneScoped;

  return CameraUniforms {
    .view = transform.viewMatrix(),
    .projection = camera.projection(screenSize),
    .position = transform.translation,
    .near = camera.near,
    .far = camera.far,
  };
}

void CameraUniforms::writeToBuffer(const GLenum target, const size_t offset) const {
  ZoneScoped;

  glBufferSubData(target, offset, size(), this);
}

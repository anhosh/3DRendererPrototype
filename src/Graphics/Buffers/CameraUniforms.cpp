#include <Graphics/Buffers/CameraUniforms.hpp>

#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Transform.hpp>

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

void CameraUniforms::writeToBuffer(const GLuint buffer, const size_t offset) const {
  ZoneScoped;

  glNamedBufferSubData(buffer, static_cast<GLintptr>(offset), this->size(), this);
}

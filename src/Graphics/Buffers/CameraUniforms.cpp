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

void CameraUniforms::writeToBuffer(const std::span<uint8_t> buffer, const size_t offset) const {
  ZoneScoped;

  std::copy_n(this, 1, reinterpret_cast<CameraUniforms*>(buffer.subspan(offset).data()));
}

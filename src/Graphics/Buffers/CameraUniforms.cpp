#include <Graphics/Buffers/CameraUniforms.hpp>

#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Transform.hpp>

#include <algorithm>

CameraUniforms CameraUniforms::from(const CompCamera& camera, const CompTransform& transform, const glm::uvec2 screenSize,
                                    const bool bSnapViewToScreenPixelGrid)
{
  ZoneScoped;

  glm::mat4 view = transform.viewMatrix();
  if (bSnapViewToScreenPixelGrid) {
    for (int32_t row = 0; row < 3; ++row) {
      view[3][row] -= glm::mod(view[3][row], 1.0f / static_cast<float>(screenSize[row % 2]));
    }
  }

  return CameraUniforms {
    .view = view,
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

#include <GraphicsOpenGL/Buffers/CameraUniforms.hpp>

#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Transform.hpp>

#include <algorithm>

namespace GraphicsOpenGL {
  CameraUniforms CameraUniforms::fromPerspective(const CompCamera& camera, const CompTransform& transform) {
    ZoneScoped;

    return CameraUniforms {
      .view = transform.viewMatrix(),
      .projection = camera.perspective(),
      .position = transform.translation,
      .near = camera.clipBox.min.z,
      .far = camera.clipBox.max.z,
    };
  }

  CameraUniforms CameraUniforms::fromOrthographic(const CompCamera& camera, const CompTransform& transform) {
    ZoneScoped;

    return CameraUniforms {
      .view = transform.viewMatrix(),
      .projection = camera.orthographic(),
      .position = transform.translation,
      .near = camera.clipBox.min.z,
      .far = camera.clipBox.max.z,
    };
  }

  void CameraUniforms::writeToBuffer(const std::span<uint8_t> buffer, const size_t offset) const {
    ZoneScoped;

    std::copy_n(this, 1, reinterpret_cast<CameraUniforms*>(buffer.subspan(offset).data()));
  }
}

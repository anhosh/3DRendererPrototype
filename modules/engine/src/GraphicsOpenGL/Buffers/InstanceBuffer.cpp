#include <GraphicsOpenGL/Buffers/InstanceBuffer.hpp>

#include <tracy/TracyOpenGL.hpp>

namespace GraphicsOpenGL {
  void InstanceBuffer::writeToBuffer(const std::span<uint8_t> buffer, const size_t offset) const {
    ZoneScoped;

    std::ranges::copy(this->instances, reinterpret_cast<InstanceData*>(buffer.subspan(offset).data()));
  }
}

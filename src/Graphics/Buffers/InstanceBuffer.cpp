#include <Graphics/Buffers/InstanceBuffer.hpp>

#include <tracy/TracyOpenGL.hpp>

void InstanceBuffer::writeToBuffer(const std::span<uint8_t> buffer, const size_t offset) const {
  ZoneScoped;
  TracyGpuZone("LightSourceBuffer::writeToBuffer");

  std::ranges::copy(this->instances, reinterpret_cast<InstanceData*>(buffer.subspan(offset).data()));
}

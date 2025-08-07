#include <Graphics/Buffers/InstanceBuffer.hpp>

#include <tracy/TracyOpenGL.hpp>

void InstanceBuffer::writeToBuffer(const GLuint buffer, const size_t offset) const {
  ZoneScoped;
  TracyGpuZone("LightSourceBuffer::writeToBuffer");

  glNamedBufferSubData(buffer, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size()), this->instances.data());
}

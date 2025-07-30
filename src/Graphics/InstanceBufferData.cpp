#include <Graphics/Buffers/InstanceBufferData.hpp>

void InstanceBufferData::writeToBuffer(GLenum target, size_t offset) const {
  ZoneScoped;

  glBufferSubData(target, offset, size(), instances.data());
}

void InstanceBufferData::setupInstanceVertexAttributes(VertexArrayHandle vertexArray, BufferHandle instanceBuffer) const {
  ZoneScoped;

  (void)instances;

  glBindVertexArray(vertexArray->vao);
  glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer->id());

  constexpr GLsizei stride = sizeof(glm::mat4) + sizeof(glm::mat3);
  for (size_t i = 0; i < 4; ++i) {
    const size_t offset = sizeof(glm::vec4) * i;
    glEnableVertexAttribArray(4 + i);
    glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offset));
    glVertexAttribDivisor(4 + i, 1);
  }
  for (size_t i = 0; i < 3; ++i) {
    const size_t offset = sizeof(glm::mat4) + sizeof(glm::vec3) * i;
    glEnableVertexAttribArray(8 + i);
    glVertexAttribPointer(8 + i, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offset));
    glVertexAttribDivisor(8 + i, 1);
  }
  glBindVertexArray(GL_NONE);
}

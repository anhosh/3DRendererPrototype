#pragma once

#include <Graphics/Sampler.hpp>
#include <Graphics/Texture2D.hpp>
#include <Util/Registry.hpp>

struct FramebufferCreateInfo {
  glm::uvec2 size;
  GLint colorFormat = GL_RGB8;
  bool bDepthStencil = true;
  uint32_t samples = 1;
};

class Framebuffer {
public:
  explicit Framebuffer(const FramebufferCreateInfo& info, SamplerHandle colorAttachmentSampler);

  void init();
  void destroy();

  void resize(glm::uvec2 size);
  void resolveMultisample() const;

  void bind() const;
  void unbind() const;

  [[nodiscard]] glm::uvec2 size() const { return mInfo.size; }

public:
  Texture2D colorAttachment;

private:
  GLuint mFBO = GL_NONE;
  GLuint mMultisampledFBO = GL_NONE;
  GLuint mMultisampledColorAttachment = GL_NONE;
  GLuint mDepthStencilRBO = GL_NONE;
  FramebufferCreateInfo mInfo;
};

using FramebufferHandle = Registry<Framebuffer>::Handle;

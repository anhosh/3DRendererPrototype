#include <Graphics/Framebuffer.hpp>
#include <tracy/TracyOpenGL.hpp>

#include <Util/Macros/Errors.hpp>

Framebuffer::Framebuffer(const FramebufferCreateInfo& info) : mInfo(info) {
  ZoneScoped;

  this->init();
}

void Framebuffer::init() {
  ZoneScoped;
  TracyGpuZone("Init framebuffer");

  glGenFramebuffers(1, &mFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

  this->colorAttachment.init();
  this->colorAttachment.bind();
  glBindTexture(GL_TEXTURE_2D, colorAttachment.id());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, mInfo.colorFormat,
               static_cast<GLint>(mInfo.size.x), static_cast<GLint>(mInfo.size.y),
               0, mInfo.colorFormat, GL_UNSIGNED_BYTE, nullptr);
  this->colorAttachment.unbind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment.id(), 0);

  if (mInfo.samples > 1) {
    glGenFramebuffers(1, &mMultisampledFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mMultisampledFBO);
    glGenTextures(1, &mMultisampledTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mMultisampledTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mInfo.samples, mInfo.colorFormat,
                            static_cast<GLint>(mInfo.size.x), static_cast<GLint>(mInfo.size.y), GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mMultisampledTexture, 0);
  }

  if (mInfo.bDepthStencil) {
    glGenRenderbuffers(1, &mDepthStencilRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRBO);
    if (mInfo.samples > 1) {
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, mInfo.samples, GL_DEPTH24_STENCIL8,
                                       static_cast<GLsizei>(mInfo.size.x), static_cast<GLsizei>(mInfo.size.y));
    } else {
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                            static_cast<GLsizei>(mInfo.size.x), static_cast<GLsizei>(mInfo.size.y));
    }
    glBindRenderbuffer(GL_RENDERBUFFER, GL_NONE);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRBO);
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    PANIC("Framebuffer not complete");
  }

  this->unbind();
}

void Framebuffer::destroy() {
  ZoneScoped;

  if (mFBO != GL_NONE) {
    glDeleteFramebuffers(1, &mFBO);
    mFBO = GL_NONE;
  }
  if (mDepthStencilRBO != GL_NONE) {
    glDeleteRenderbuffers(1, &mDepthStencilRBO);
    mDepthStencilRBO = GL_NONE;
  }
  colorAttachment.destroy();
}

void Framebuffer::resize(const glm::uvec2 size) {
  ZoneScoped;

  this->destroy();
  mInfo.size = size;
  this->init();
}

void Framebuffer::resolveMultisample() const {
  if (mInfo.samples > 1) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mMultisampledTexture);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
    glBlitFramebuffer(0, 0, mInfo.size.x, mInfo.size.y, 0, 0, mInfo.size.x, mInfo.size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  }
}

void Framebuffer::bind() const {
  ZoneScoped;

  glBindFramebuffer(GL_FRAMEBUFFER, mInfo.samples > 1 ? mMultisampledFBO : mFBO);
}

void Framebuffer::unbind() const {
  ZoneScoped;

  (void)mFBO;
  glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

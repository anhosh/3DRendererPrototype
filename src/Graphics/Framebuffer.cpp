#include <Graphics/Framebuffer.hpp>

#include <Util/Macros/Errors.hpp>

Framebuffer::Framebuffer(const FramebufferCreateInfo& info) : mInfo(info) {
  this->init(info);
}

void Framebuffer::init(const FramebufferCreateInfo& info) {
  glGenFramebuffers(1, &mFBO);
  this->bind();

  this->colorAttachment.init();
  colorAttachment.bind();
  glTexImage2D(GL_TEXTURE_2D, 0, info.colorFormat,
               static_cast<GLint>(info.size.x), static_cast<GLint>(info.size.y),
               0, info.colorFormat, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  colorAttachment.unbind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment.id(), 0);

  if (info.bDepthStencil) {
    glGenRenderbuffers(1, &mDepthStencilRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, info.size.x, info.size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, GL_NONE);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRBO);
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    PANIC("Framebuffer not complete");
  }
  this->unbind();
}

void Framebuffer::destroy() {
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
  this->destroy();
  mInfo.size = size;
  this->init(mInfo);
}

void Framebuffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
}

void Framebuffer::unbind() const {
  (void)mFBO;
  glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

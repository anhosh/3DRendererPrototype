#include <Graphics/Framebuffer.hpp>

#include <Util/Macros/Errors.hpp>

#include <tracy/TracyOpenGL.hpp>

Framebuffer::Framebuffer(const FramebufferCreateInfo& info, SamplerHandle colorAttachmentSampler)
  : colorAttachment(colorAttachmentSampler)
  , mInfo(info)
{
  ZoneScoped;

  this->init();
}

void Framebuffer::init() {
  ZoneScoped;
  TracyGpuZone("Init framebuffer");

  glCreateFramebuffers(1, &mFBO);

  colorAttachment.init();
  colorAttachment.allocate(mInfo.size, mInfo.colorFormat);
  glNamedFramebufferTexture(mFBO, GL_COLOR_ATTACHMENT0, colorAttachment.id(), 0);

  if (mInfo.samples > 1) {
    glCreateFramebuffers(1, &mMultisampledFBO);
    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &mMultisampledColorAttachment);
    glTextureStorage2DMultisample(mMultisampledColorAttachment, static_cast<GLsizei>(mInfo.samples), mInfo.colorFormat,
                                  static_cast<GLint>(mInfo.size.x), static_cast<GLint>(mInfo.size.y), GL_TRUE);
    glNamedFramebufferTexture(mMultisampledFBO, GL_COLOR_ATTACHMENT0, mMultisampledColorAttachment, 0);
  }

  if (mInfo.bDepthStencil) {
    glCreateRenderbuffers(1, &mDepthStencilRBO);
    if (mInfo.samples > 1) {
      glNamedRenderbufferStorageMultisample(mDepthStencilRBO, static_cast<GLsizei>(mInfo.samples), GL_DEPTH24_STENCIL8,
                                            static_cast<GLsizei>(mInfo.size.x), static_cast<GLsizei>(mInfo.size.y));
      glNamedFramebufferRenderbuffer(mMultisampledFBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRBO);
    } else {
      glNamedRenderbufferStorage(mDepthStencilRBO, GL_DEPTH24_STENCIL8,
                                 static_cast<GLsizei>(mInfo.size.x), static_cast<GLsizei>(mInfo.size.y));
      glNamedFramebufferRenderbuffer(mFBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRBO);
    }
  }

  if (glCheckNamedFramebufferStatus(mFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    PANIC("Framebuffer is not complete");
  }
  if (mInfo.samples > 1 && glCheckNamedFramebufferStatus(mMultisampledFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    PANIC("Multisampled framebuffer is not complete");
  }

  this->unbind();
}

void Framebuffer::destroy() {
  ZoneScoped;

  if (mFBO != GL_NONE) {
    glDeleteFramebuffers(1, &mFBO);
    mFBO = GL_NONE;
  }
  if (mMultisampledFBO != GL_NONE) {
    glDeleteFramebuffers(1, &mMultisampledFBO);
    mMultisampledFBO = GL_NONE;
  }
  colorAttachment.destroy();
  if (mMultisampledColorAttachment != GL_NONE) {
    glDeleteTextures(1, &mMultisampledColorAttachment);
    mMultisampledColorAttachment = GL_NONE;
  }
  if (mDepthStencilRBO != GL_NONE) {
    glDeleteRenderbuffers(1, &mDepthStencilRBO);
    mDepthStencilRBO = GL_NONE;
  }
}

void Framebuffer::resize(const glm::uvec2 size) {
  ZoneScoped;

  this->destroy();
  mInfo.size = size;
  this->init();
}

void Framebuffer::resolveMultisample() const {
  if (mInfo.samples > 1) {
    ZoneScoped;
    TracyGpuZone("Blit framebuffer");

    glBlitNamedFramebuffer(mMultisampledFBO, mFBO,
                           0, 0, mInfo.size.x, mInfo.size.y,
                           0, 0, mInfo.size.x, mInfo.size.y,
                           GL_COLOR_BUFFER_BIT, GL_NEAREST);
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

#include <ranges>
#include <Graphics/Framebuffer.hpp>

#include <Util/Macros/Errors.hpp>

#include <tracy/TracyOpenGL.hpp>

Framebuffer::Framebuffer(const FramebufferCreateInfo& info)
  : mInfo(info)
{
  ZoneScoped;

  this->init();
}

void Framebuffer::init() {
  ZoneScoped;
  TracyGpuZone("Init framebuffer");

  glCreateFramebuffers(1, &mFBO);

  colorAttachments.resize(mInfo.colorAttachments.size());
  for (auto [i, colorAttachment] : colorAttachments | std::views::enumerate) {
    colorAttachment.init();
    colorAttachment.allocate(mInfo.size, mInfo.colorAttachments[i].internalFormat);
    glNamedFramebufferTexture(mFBO, GL_COLOR_ATTACHMENT0 + i, colorAttachment.id(), 0);
  }

  switch (mInfo.depthStencilMode) {
    case DepthStencilMode::None:
      // no-op
      break;

    case DepthStencilMode::DepthAttachment:
      depthStencilAttachment.emplace();
      depthStencilAttachment->init();
      depthStencilAttachment->allocate(mInfo.size, GL_DEPTH_COMPONENT24);
      glNamedFramebufferTexture(mFBO, GL_DEPTH_ATTACHMENT, depthStencilAttachment->id(), 0);
      break;

    case DepthStencilMode::DepthStencilAttachment:
      depthStencilAttachment.emplace();
      depthStencilAttachment->init();
      depthStencilAttachment->allocate(mInfo.size, GL_DEPTH24_STENCIL8);
      glNamedFramebufferTexture(mFBO, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilAttachment->id(), 0);
      break;

    case DepthStencilMode::DepthRBO:
      glCreateRenderbuffers(1, &mDepthStencilRBO);
      glNamedRenderbufferStorage(mDepthStencilRBO, GL_DEPTH_COMPONENT24,
                                 static_cast<GLsizei>(mInfo.size.x), static_cast<GLsizei>(mInfo.size.y));
      glNamedFramebufferRenderbuffer(mFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRBO);
      break;

    case DepthStencilMode::DepthStencilRBO:
      glCreateRenderbuffers(1, &mDepthStencilRBO);
      glNamedRenderbufferStorage(mDepthStencilRBO, GL_DEPTH24_STENCIL8,
                                 static_cast<GLsizei>(mInfo.size.x), static_cast<GLsizei>(mInfo.size.y));
      glNamedFramebufferRenderbuffer(mFBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRBO);
      break;
  }

  if (mInfo.colorAttachments.empty()) {
    glNamedFramebufferDrawBuffer(mFBO, GL_NONE);
    glNamedFramebufferReadBuffer(mFBO, GL_NONE);
  }

  if (glCheckNamedFramebufferStatus(mFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    PANIC("Framebuffer is not complete");
  }

  if (mInfo.samples > 1) {
    this->initMultisampled();
  }

  this->unbind();
}

void Framebuffer::initMultisampled() {
  ZoneScoped;

  glCreateFramebuffers(1, &mMultisampledFBO);
  mMultisampledColorAttachments.resize(mInfo.colorAttachments.size());
  for (auto [i, colorAttachment] : mMultisampledColorAttachments | std::views::enumerate) {
    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &colorAttachment);
    glTextureStorage2DMultisample(colorAttachment, static_cast<GLsizei>(mInfo.samples), mInfo.colorAttachments[i].internalFormat,
                                  static_cast<GLint>(mInfo.size.x), static_cast<GLint>(mInfo.size.y), GL_TRUE);
    glNamedFramebufferTexture(mMultisampledFBO, GL_COLOR_ATTACHMENT0 + i, colorAttachment, 0);
  }

  switch (mInfo.depthStencilMode) {
    case DepthStencilMode::None:
      // no-op
      break;

    case DepthStencilMode::DepthAttachment:
      UNIMPLEMENTED();
      break;

    case DepthStencilMode::DepthStencilAttachment:
      UNIMPLEMENTED();
      break;

    case DepthStencilMode::DepthRBO:
      glCreateRenderbuffers(1, &mMultisampledDepthStencilRBO);
      glNamedRenderbufferStorageMultisample(mMultisampledDepthStencilRBO, static_cast<GLsizei>(mInfo.samples), GL_DEPTH_COMPONENT24,
                                            static_cast<GLsizei>(mInfo.size.x), static_cast<GLsizei>(mInfo.size.y));
      glNamedFramebufferRenderbuffer(mMultisampledFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mMultisampledDepthStencilRBO);
      break;

    case DepthStencilMode::DepthStencilRBO:
      glCreateRenderbuffers(1, &mMultisampledDepthStencilRBO);
      glNamedRenderbufferStorageMultisample(mMultisampledDepthStencilRBO, static_cast<GLsizei>(mInfo.samples), GL_DEPTH24_STENCIL8,
                                            static_cast<GLsizei>(mInfo.size.x), static_cast<GLsizei>(mInfo.size.y));
      glNamedFramebufferRenderbuffer(mMultisampledFBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mMultisampledDepthStencilRBO);
      break;
  }

  if (mInfo.colorAttachments.empty()) {
    glNamedFramebufferDrawBuffer(mMultisampledFBO, GL_NONE);
    glNamedFramebufferReadBuffer(mMultisampledFBO, GL_NONE);
  }

  if (glCheckNamedFramebufferStatus(mMultisampledFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    PANIC("Multisampled framebuffer is not complete");
  }
}

void Framebuffer::destroy() {
  ZoneScoped;

  for (Texture2D& colorAttachment : colorAttachments) {
    colorAttachment.destroy();
  }
  depthStencilAttachment.transform([](Texture2D& t) { t.destroy(); return t; });

  colorAttachments.clear();
  depthStencilAttachment.reset();

  if (mFBO != GL_NONE) {
    glDeleteFramebuffers(1, &mFBO);
    mFBO = GL_NONE;
  }
  if (mMultisampledFBO != GL_NONE) {
    glDeleteFramebuffers(1, &mMultisampledFBO);
    mMultisampledFBO = GL_NONE;
  }
  if (mDepthStencilRBO != GL_NONE) {
    glDeleteRenderbuffers(1, &mDepthStencilRBO);
    mDepthStencilRBO = GL_NONE;
  }

  for (GLuint& attachment : mMultisampledColorAttachments) {
    glDeleteTextures(1, &attachment);
  }
  mMultisampledColorAttachments.clear();
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
                           0, 0, static_cast<GLint>(mInfo.size.x), static_cast<GLint>(mInfo.size.y),
                           0, 0, static_cast<GLint>(mInfo.size.x), static_cast<GLint>(mInfo.size.y),
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

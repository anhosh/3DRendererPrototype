#pragma once

#include <GraphicsOpenGL/Textures/Texture2D.hpp>
#include <Util/Registry.hpp>

#include <optional>
#include <span>
#include <vector>

namespace GraphicsOpenGL {
  struct FramebufferAttachment {
    NotNull<TextureBase> texture;
    uint32_t layer = 0;
  };

  enum class DepthStencilMode {
    None,
    DepthAttachment,
    DepthStencilAttachment,
    DepthRBO,
    DepthStencilRBO,
  };

  struct FramebufferCreateInfo {
    glm::uvec2 size;
    uint32_t samples = 1;
    DepthStencilMode depthStencilMode = DepthStencilMode::DepthStencilRBO;
    std::vector<FramebufferAttachment> colorAttachments = {};
    std::optional<FramebufferAttachment> depthStencilAttachment = std::nullopt;
  };

  class Framebuffer {
  public:
    explicit Framebuffer(FramebufferCreateInfo info);

    void init();
  private:
    void initMultisampled();

  public:
    void destroy();

    void resize(glm::uvec2 size);
    void resolveMultisample() const;

    void bind() const;
    void unbind() const;

    void setColorAttachment(FramebufferAttachment attachment, size_t index);
    void setDepthStencilAttachment(FramebufferAttachment attachment);

    [[nodiscard]] GLuint fbo() const { return mFBO; }
    [[nodiscard]] glm::uvec2 size() const { return mInfo.size; }
    [[nodiscard]] DepthStencilMode depthStencilMode() const { return mInfo.depthStencilMode; }
    [[nodiscard]] std::span<const FramebufferAttachment> colorAttachments() const { return mInfo.colorAttachments; }
    [[nodiscard]] const std::optional<FramebufferAttachment>& depthStencilAttachment() const { return mInfo.depthStencilAttachment; }

  private:
    GLuint mFBO = GL_NONE;
    GLuint mDepthStencilRBO = GL_NONE;
    GLuint mMultisampledFBO = GL_NONE;
    GLuint mMultisampledDepthStencilRBO = GL_NONE;
    std::vector<GLuint> mMultisampledColorAttachments;
    FramebufferCreateInfo mInfo;
  };

  using FramebufferHandle = Registry<Framebuffer>::Handle;
}

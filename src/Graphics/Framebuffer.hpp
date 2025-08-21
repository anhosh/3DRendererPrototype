#pragma once

#include <Graphics/Textures/Texture2D.hpp>
#include <Util/Registry.hpp>

#include <vector>

struct ColorAttachmentInfo {
  GLint internalFormat = GL_RGB8;
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
  std::vector<ColorAttachmentInfo> colorAttachments;
  DepthStencilMode depthStencilMode = DepthStencilMode::DepthStencilRBO;
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

  [[nodiscard]] GLuint fbo() const { return mFBO; }
  [[nodiscard]] glm::uvec2 size() const { return mInfo.size; }
  [[nodiscard]] DepthStencilMode depthStencilMode() const { return mInfo.depthStencilMode; }

public:
  std::vector<Texture2D> colorAttachments;
  std::optional<Texture2D> depthStencilAttachment;

private:
  GLuint mFBO = GL_NONE;
  GLuint mDepthStencilRBO = GL_NONE;
  GLuint mMultisampledFBO = GL_NONE;
  GLuint mMultisampledDepthStencilRBO = GL_NONE;
  std::vector<GLuint> mMultisampledColorAttachments;
  FramebufferCreateInfo mInfo;
};

using FramebufferHandle = Registry<Framebuffer>::Handle;

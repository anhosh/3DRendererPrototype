#pragma once

#include <Graphics/Texture.hpp>
#include <Util/NoInit.hpp>
#include <Util/Registry.hpp>

struct FramebufferCreateInfo {
  glm::uvec2 size;
  GLint colorFormat = GL_RGB;
  bool bDepthStencil = true;
};

class Framebuffer {
public:
  explicit Framebuffer(const FramebufferCreateInfo& info);

  void init(const FramebufferCreateInfo& info);
  void destroy();

  void resize(glm::uvec2 size);

  void bind() const;
  void unbind() const;

  [[nodiscard]] GLuint fbo() const { return mFBO; }
  [[nodiscard]] GLuint depthStencilRBO() const { return mDepthStencilRBO; }
  [[nodiscard]] glm::uvec2 size() const { return mInfo.size; }

  Texture colorAttachment = NoInit{};

private:
  GLuint mFBO = GL_NONE;
  GLuint mDepthStencilRBO = GL_NONE;
  FramebufferCreateInfo mInfo;
};

using FramebufferHandle = Registry<Framebuffer>::Handle;

#pragma once

#include <Graphics/Texture.hpp>
#include <Util/NoInit.hpp>

class Framebuffer {
public:
  Framebuffer(glm::uvec2 size);

  void init(glm::uvec2 size);

  void destroy();

  void bind() const;
  void unbind() const;

  [[nodiscard]] GLuint fbo() const { return mFBO; }
  [[nodiscard]] GLuint depthStencilRBO() const { return mDepthStencilRBO; }

  Texture colorAttachment = NoInit{};

private:
  GLuint mFBO = GL_NONE;
  GLuint mDepthStencilRBO = GL_NONE;
};

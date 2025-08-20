#pragma once

class TextureBase {
public:
  void init();
  void destroy();
  void bind(GLuint unit) const;
  void unbind(GLuint unit) const;

  [[nodiscard]] GLuint id() const { return mID; }
  [[nodiscard]] GLuint target() const { return mTarget; }
  [[nodiscard]] glm::uvec3 size() const { return mSize; }

protected:
  explicit TextureBase(GLenum target);

  void allocate2D(glm::uvec2 size, GLint internalFormat);
  void allocate3D(glm::uvec3 size, GLint internalFormat);

protected:
  GLint mInternalFormat = GL_NONE;

private:
  glm::uvec3 mSize = glm::uvec3(0);
  GLenum mTarget = GL_NONE;
  GLuint mID = GL_NONE;
};
#include <GraphicsOpenGL/Textures/TextureBase.hpp>

#include <tracy/TracyOpenGL.hpp>

namespace GraphicsOpenGL {
  void TextureBase::init() {
    if (mID ==  GL_NONE) {
      glCreateTextures(mTarget, 1, &mID);
    }
  }

  void TextureBase::destroy() {
    if (mID != GL_NONE) {
      glDeleteTextures(1, &mID);
      mID = GL_NONE;
    }
  }

  void TextureBase::bind(const GLuint unit) const {
    glBindTextureUnit(unit, mID);
  }

  void TextureBase::unbind(const GLuint unit) const {
    (void)mID;
    glBindTextureUnit(unit, GL_NONE);
  }

  bool TextureBase::layered() const {
    switch (mTarget) {
    case GL_TEXTURE_2D_ARRAY:
    case GL_TEXTURE_3D:
    case GL_TEXTURE_CUBE_MAP:
    case GL_TEXTURE_CUBE_MAP_ARRAY:
      return true;

    default:
      return false;
    }
  }

  TextureBase::TextureBase(const GLenum target)
    : mTarget(target)
  {
    this->init();
  }

  void TextureBase::allocate2D(const glm::uvec2 size, const GLint internalFormat) {
    ZoneScoped;
    TracyGpuZone("Allocate Texture2D");

    glTextureStorage2D(mID, 1, internalFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
    mInternalFormat = internalFormat;
    mSize = glm::uvec3(size, 1);
  }

  void TextureBase::allocate3D(const glm::uvec3 size, const GLint internalFormat) {
    ZoneScoped;
    TracyGpuZone("Allocate Texture3D");

    glTextureStorage3D(mID, 1, internalFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), static_cast<GLsizei>(size.z));
    mInternalFormat = internalFormat;
    mSize = size;
  }
}

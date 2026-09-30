#include <GraphicsOpenGL/Sampler.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace GraphicsOpenGL {
  Sampler::Sampler(const SamplerOptions& options) {
    glCreateSamplers(1, &mID);

    glSamplerParameteri(mID, GL_TEXTURE_WRAP_S, options.wrapS);
    glSamplerParameteri(mID, GL_TEXTURE_WRAP_T, options.wrapT);
    glSamplerParameteri(mID, GL_TEXTURE_WRAP_R, options.wrapR);
    glSamplerParameteri(mID, GL_TEXTURE_MIN_FILTER, options.minFilter);
    glSamplerParameteri(mID, GL_TEXTURE_MAG_FILTER, options.magFilter);
    glSamplerParameterfv(mID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(options.borderColor));
    glSamplerParameteri(mID, GL_TEXTURE_MIN_LOD, options.minLOD);
    glSamplerParameteri(mID, GL_TEXTURE_MAX_LOD, options.maxLOD);
    glSamplerParameteri(mID, GL_TEXTURE_COMPARE_MODE, options.compareMode);
    glSamplerParameteri(mID, GL_TEXTURE_COMPARE_FUNC, options.compareFunc);
  }

  void Sampler::destroy() {
    glDeleteSamplers(1, &mID);
    mID = GL_NONE;
  }

  void Sampler::bind(const size_t unit) const {
    glBindSampler(static_cast<GLuint>(unit), mID);
  }

  void Sampler::unbind(const size_t unit) const {
    (void)mID;
    glBindSampler(static_cast<GLuint>(unit), GL_NONE);
  }
}

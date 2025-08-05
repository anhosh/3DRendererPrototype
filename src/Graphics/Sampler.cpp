#include <Graphics/Sampler.hpp>

Sampler::Sampler(const SamplerOptions& options) {
  glCreateSamplers(1, &mID);

  glSamplerParameteri(mID, GL_TEXTURE_WRAP_S, options.wrapS);
  glSamplerParameteri(mID, GL_TEXTURE_WRAP_T, options.wrapT);
  glSamplerParameteri(mID, GL_TEXTURE_MIN_FILTER, options.minFilter);
  glSamplerParameteri(mID, GL_TEXTURE_MAG_FILTER, options.magFilter);
}

void Sampler::destroy() {
  glDeleteSamplers(1, &mID);
  mID = GL_NONE;
}

void Sampler::bind(const size_t unit) const {
  glBindSampler(unit, mID);
}

void Sampler::unbind(size_t unit) const {
  (void)mID;
  glBindSampler(unit, GL_NONE);
}

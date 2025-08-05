#pragma once

#include <Util/Registry.hpp>

struct SamplerOptions {
  GLint wrapS = GL_CLAMP_TO_EDGE;
  GLint wrapT = GL_CLAMP_TO_EDGE;
  GLint wrapR = GL_CLAMP_TO_EDGE;
  GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
  GLint magFilter = GL_LINEAR;
};

class Sampler {
public:
  explicit Sampler(const SamplerOptions& options = {});

  void destroy();
  void bind(size_t unit) const;
  void unbind(size_t unit) const;

private:
  GLuint mID = GL_NONE;
};

using SamplerHandle = Registry<Sampler>::Handle;

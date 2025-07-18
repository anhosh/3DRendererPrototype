#pragma once

struct SamplerOptions {
  GLint wrapS = GL_CLAMP_TO_EDGE;
  GLint wrapT = GL_CLAMP_TO_EDGE;
  GLint wrapR = GL_CLAMP_TO_EDGE;
  GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
  GLint magFilter = GL_LINEAR;
};

#pragma once

#include <Graphics/Shader.hpp>
#include <Util/Expected.hpp>
#include <Util/Registry.hpp>

class ShaderProgram {
public:
  explicit ShaderProgram(GLuint shaderProgram);

  static Expected<ShaderProgram> fromShaders(const ShaderProgramPaths& shaderPaths);

  void destroy();

  [[nodiscard]] GLuint id() const { return mID; }

protected:
  GLuint mID = 0;
};

using ShaderProgramHandle = Registry<ShaderProgram>::Handle;

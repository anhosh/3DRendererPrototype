#pragma once

#include <GraphicsOpenGL/Shader.hpp>
#include <Util/Expected.hpp>
#include <Util/Registry.hpp>

namespace GraphicsOpenGL {
  class ShaderProgram {
  public:
    explicit ShaderProgram(GLuint shaderProgram);

    static Expected<ShaderProgram> fromShaders(const ShaderProgramPaths& shaderPaths);

    void destroy();

    [[nodiscard]] GLuint id() const { return mID; }
    [[nodiscard]] GLint uniformLocation(std::string_view uniformName) const;

  protected:
    GLuint mID = 0;
  };

  using ShaderProgramHandle = Registry<ShaderProgram>::Handle;
}

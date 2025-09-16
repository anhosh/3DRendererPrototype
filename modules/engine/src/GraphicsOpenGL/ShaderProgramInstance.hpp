#pragma once

#include <GraphicsOpenGL/ShaderProgram.hpp>
#include <GraphicsOpenGL/ShaderUniform.hpp>
#include <Util/Log.hpp>
#include <Util/Registry.hpp>

#include <unordered_map>

namespace GraphicsOpenGL {
  enum class ShaderProgramType : int32_t {
    Light,
    LitExploded,
    LitSurface,
    Outline,
    ReflectiveSurface,
    RefractiveSurface,

    PostProcessCopy, // Keep PostProcessCopy as the first PostProcessX entry, for easy enumeration in the debug menu.
    PostProcessBlur,
    PostProcessEdgeDetection,
    PostProcessEmboss,
    PostProcessFlipHorizontally,
    PostProcessFlipVertically,
    PostProcessGammaCorrection,
    PostProcessGrayscale,
    PostProcessInvert,
    PostProcessSharpen,
    PostProcessSobelBottom,
    PostProcessSobelLeft,
    PostProcessSobelRight,
    PostProcessSobelTop,

    Skybox,

    SurfaceDepth,
    SurfaceNormal,
  };

  class ShaderProgramInstance {
    friend class RenderingEngine;

  public:
    ShaderProgramInstance(const ShaderProgramInstance&) = default;
    ShaderProgramInstance(ShaderProgramInstance&&) = default;

    ShaderProgramInstance& operator=(const ShaderProgramInstance&) = default;
    ShaderProgramInstance& operator=(ShaderProgramInstance&&) = default;

    template <typename UniformType>
    void setUniform(const GLchar* name, const UniformType& value) {
      ZoneScoped;

      if (auto uniform = this->uniforms.find(name); uniform != this->uniforms.end()) {
        uniform->second = value;
      } else {
        this->uniforms.emplace(name, ShaderUniform { glGetUniformLocation(mShaderProgram->id(), name), value });
      }
    }

    template <typename UniformType>
    void setAndBindUniform(const GLchar* name, const UniformType& value) {
      this->setUniform(name, value);
      this->uniforms[name].bind();
    }

    void use() const;
    void bindUniforms() const;

    ShaderProgramHandle shaderProgram() const { return mShaderProgram; }
    ShaderProgramType type() const { return mType; }

  public:
    std::unordered_map<std::string, ShaderUniform> uniforms;

  private:
    explicit ShaderProgramInstance(const ShaderProgramHandle program, const ShaderProgramType type)
      : mShaderProgram(program)
      , mType(type)
    {}

    template <ShaderProgramType SHADER_TYPE>
    static ShaderProgramInstance create(const ShaderProgramHandle program) {
      ZoneScoped;
      LOG_INFO("Creating an empty shader instance");
      return ShaderProgramInstance(program, SHADER_TYPE);
    }

  private:
    ShaderProgramHandle mShaderProgram;
    ShaderProgramType mType;
  };

  using ShaderProgramInstanceHandle = Registry<ShaderProgramInstance>::Handle;
}

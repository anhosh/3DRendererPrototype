#pragma once

#include <Graphics/ShaderProgram.hpp>
#include <Graphics/ShaderUniform.hpp>
#include <Util/Registry.hpp>

#include <unordered_map>

enum class ShaderProgramType : int32_t {
  Light,
  LitSurface,
  LitExploded,
  Outline,
  ReflectiveSurface,
  RefractiveSurface,
  PostProcessCopy,
  PostProcessBlur,
  PostProcessEdgeDetection,
  PostProcessEmboss,
  PostProcessFlipHorizontally,
  PostProcessFlipVertically,
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

    this->uniforms[name] = ShaderUniform {
      .location = glGetUniformLocation(mShaderProgram->id(), name),
      .value = value,
    };
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

  static ShaderProgramInstance newLitSurface(ShaderProgramHandle program);
  static ShaderProgramInstance newLitExploded(ShaderProgramHandle program);
  static ShaderProgramInstance newLight(ShaderProgramHandle program);
  static ShaderProgramInstance newOutline(ShaderProgramHandle program);
  static ShaderProgramInstance newReflectiveSurface(ShaderProgramHandle program);
  static ShaderProgramInstance newRefractiveSurface(ShaderProgramHandle program);
  static ShaderProgramInstance newSurfaceDepth(ShaderProgramHandle program);
  static ShaderProgramInstance newSurfaceNormal(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingCopy(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingBlur(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingEdgeDetection(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingEmboss(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingFlipHorizontally(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingFlipVertically(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingGrayscale(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingInvert(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingSharpen(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingSobelBottom(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingSobelLeft(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingSobelRight(ShaderProgramHandle program);
  static ShaderProgramInstance newPostProcessingSobelTop(ShaderProgramHandle program);
  static ShaderProgramInstance newSkybox(ShaderProgramHandle program);

private:
  ShaderProgramHandle mShaderProgram;
  ShaderProgramType mType;
};

using ShaderProgramInstanceHandle = Registry<ShaderProgramInstance>::Handle;

#pragma once

#include <Graphics/ShaderProgram.hpp>
#include <Util/Registry.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <unordered_map>
#include <variant>

enum class ShaderProgramType : int32_t {
  Light,
  LitSurface,
  Outline,
  VisualiseDepth,
  VisualiseNormal,
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
};

struct ShaderUniform {
  GLint location;
  std::variant<
    GLint,
    GLuint,
    GLfloat,
    GLdouble,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat2,
    glm::mat3,
    glm::mat4
  > value;

  template <typename T>
  T& getRef() {
    return std::get<T>(value);
  }

  template <typename T>
  const T& getRef() const {
    return std::get<T>(value);
  }

  template <typename T>
  T* getPtr() {
    return std::get_if<T>(&value);
  }

  template <typename T>
  const T* getPtr() const {
    return std::get_if<T>(&value);
  }

  template <typename T>
  auto* getValuePtr() {
    return glm::value_ptr(getRef<T>());
  }

  template <typename T>
  const auto* getValuePtr() const {
    return glm::value_ptr(getRef<T>());
  }

  template <typename T>
  ShaderUniform& operator=(const T& v) {
    value = v;
    return *this;
  }
};

class ShaderProgramInstance {
  friend class RenderingEngine;

public:
  ShaderProgramInstance(const ShaderProgramInstance&) = default;
  ShaderProgramInstance(ShaderProgramInstance&&) = default;

  template <typename UniformType>
  void setUniform(const GLchar* name, const UniformType& value) {
    this->uniforms[name] = ShaderUniform {
      .location = glGetUniformLocation(this->shaderProgram->id(), name),
      .value = value,
    };
  }

  void use() const;
  void bindUniforms() const;

  ShaderProgramHandle shaderProgram;
  std::unordered_map<std::string, ShaderUniform> uniforms;

private:
  explicit ShaderProgramInstance(const ShaderProgramHandle program) : shaderProgram(program) {}

  static ShaderProgramInstance newLitSurface(ShaderProgramHandle program);
  static ShaderProgramInstance newLight(ShaderProgramHandle program);
  static ShaderProgramInstance newOutline(ShaderProgramHandle program);
  static ShaderProgramInstance newVisualiseDepth(ShaderProgramHandle program);
  static ShaderProgramInstance newVisualiseNormal(ShaderProgramHandle program);
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
};

using ShaderProgramInstanceHandle = Registry<ShaderProgramInstance>::Handle;

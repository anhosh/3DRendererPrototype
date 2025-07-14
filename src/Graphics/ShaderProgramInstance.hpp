#pragma once

#include <Graphics/ShaderProgram.hpp>
#include <Util/NotNull.hpp>

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
  PostProcessInvert,
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

  NotNull<ShaderProgram> shaderProgram;
  std::unordered_map<std::string, ShaderUniform> uniforms;

private:
  explicit ShaderProgramInstance(NotNull<ShaderProgram> program) : shaderProgram(program) {}

  static ShaderProgramInstance newLitSurface(NotNull<ShaderProgram> program);
  static ShaderProgramInstance newLight(NotNull<ShaderProgram> program);
  static ShaderProgramInstance newOutline(NotNull<ShaderProgram> program);
  static ShaderProgramInstance newVisualiseDepth(NotNull<ShaderProgram> program);
  static ShaderProgramInstance newVisualiseNormal(NotNull<ShaderProgram> program);
  static ShaderProgramInstance newPostProcessingCopy(NotNull<ShaderProgram> program);
  static ShaderProgramInstance newPostProcessingInvert(NotNull<ShaderProgram> program);
};

#pragma once

#include <glm/gtc/type_ptr.hpp>

#include <variant>

struct ShaderUniform {
  using ValueType = std::variant<
    bool,
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
  >;

  GLint location;
  ValueType value;

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

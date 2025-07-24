#pragma once

#include <Graphics/Shader.hpp>
#include <Util/Expected.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/Registry.hpp>

#include <concepts>
#include <filesystem>

namespace fs = std::filesystem;

struct TransformMatrices {
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat3 normal = glm::mat3(1.0f);
};

class ShaderProgram {
public:
  explicit ShaderProgram(GLuint shaderProgram);

  template <std::derived_from<ShaderProgram> MaterialClass = ShaderProgram>
  MaterialClass& as() {
    auto ret = dynamic_cast<MaterialClass*>(this);
    assert(ret != nullptr);
    return *ret;
  }

  GLuint id() const { return mID; }

  void bindTransforms(const TransformMatrices& transforms) const;

  void destroy();

protected:
  GLuint mID = 0;
};

using ShaderProgramHandle = Registry<ShaderProgram>::Handle;

namespace ShaderPrograms {
  inline Expected<ShaderProgram> fromShaders(const ShaderProgramPaths& shaderPaths) {
    const Expected shaderProgram = createShaderProgram(shaderPaths);
    RETURN_ERROR_IF_UNEXPECTED(shaderProgram);
    return ShaderProgram(shaderProgram.value());
  }
}

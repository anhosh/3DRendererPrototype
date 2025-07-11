#pragma once

#include <Graphics/Shader.hpp>
#include <Util/Expected.hpp>
#include <Util/Macros.hpp>

#include <concepts>
#include <filesystem>

namespace fs = std::filesystem;

struct TransformMatrices {
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);
  glm::mat3 normal = glm::mat3(1.0f);
};

class ShaderProgram {
public:
  explicit ShaderProgram(GLuint shaderProgram);
  virtual ~ShaderProgram() = default;

  template <std::derived_from<ShaderProgram> MaterialClass = ShaderProgram>
  MaterialClass& as() {
    auto ret = dynamic_cast<MaterialClass*>(this);
    assert(ret != nullptr);
    return *ret;
  }

  GLuint id() const { return mID; }

  void destroy();

  virtual void bindTransforms(const TransformMatrices& transforms) const;

protected:
  GLuint mID = 0;
};

namespace ShaderPrograms {
  template <std::derived_from<ShaderProgram> MaterialClass = ShaderProgram>
  Expected<std::unique_ptr<ShaderProgram>> fromShaders(const ShaderProgramPaths& shaderStages) {
    const Expected<GLuint> shaderProgram = createShaderProgram(shaderStages);
    RETURN_ERROR_IF_UNEXPECTED(shaderProgram);
    return std::make_unique<MaterialClass>(shaderProgram.value());
  }
}

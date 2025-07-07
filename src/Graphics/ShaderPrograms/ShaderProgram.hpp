#pragma once

#include <Graphics/Shader.hpp>
#include <Util/Macros.hpp>

#include <concepts>
#include <expected>
#include <filesystem>

namespace fs = std::filesystem;

struct TransformMatrices {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat3 normal;
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

  void use() const;
  void stopUsing() const;
  void destroy();

  virtual void bindUniforms(const TransformMatrices& transforms) const;

protected:
  GLuint mShaderProgram = 0;
};

namespace ShaderPrograms {
  template <std::derived_from<ShaderProgram> MaterialClass = ShaderProgram>
  std::expected<std::unique_ptr<ShaderProgram>, std::string> fromShaders(const ShaderStages& shaderStages) {
    const std::expected<GLuint, std::string> shaderProgram = createShaderProgram(shaderStages);
    RETURN_ERROR_IF_UNEXPECTED(shaderProgram);
    return std::make_unique<MaterialClass>(shaderProgram.value());
  }
}

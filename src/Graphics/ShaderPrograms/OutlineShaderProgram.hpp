#pragma once

#include <Graphics/ShaderProgram.hpp>

class OutlineShaderProgram : public ShaderProgram {
public:
  OutlineShaderProgram(GLuint shaderProgram);

  virtual void bindUniforms(const TransformMatrices& transforms) const override;

public:
  glm::vec3 outlineColor = glm::vec3(1.0f);
};

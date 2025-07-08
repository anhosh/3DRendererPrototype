#pragma once

#include <Graphics/ShaderProgram.hpp>

class LightSourceShaderProgram : public ShaderProgram {
public:
  explicit LightSourceShaderProgram(GLuint shaderProgram);

  virtual void bindUniforms(const TransformMatrices& transforms) const override;

public:
  glm::vec3 emittedColor = glm::vec3(1.0f);
};

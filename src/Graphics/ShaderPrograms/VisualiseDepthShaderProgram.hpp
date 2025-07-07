#pragma once

#include <Graphics/ShaderPrograms/ShaderProgram.hpp>

class VisualiseDepthShaderProgram : public ShaderProgram {
public:
  explicit VisualiseDepthShaderProgram(GLuint shaderProgram);

  virtual void bindUniforms(const TransformMatrices& transforms) const override;

public:
  float frustumNear = 0.01f;
  float frustumFar = 100.0f;
};

#pragma once

#include <Light.hpp>
#include <Material.hpp>
#include <ShaderPrograms/ShaderProgram.hpp>

class LitSurfaceShaderProgram : public ShaderProgram {
public:
  explicit LitSurfaceShaderProgram(GLuint shaderProgram);

  virtual void bindUniforms(const TransformMatrices& transforms) const override;

public:
  Material material;
  DirectionalLight directionalLight;
  PointLight pointLight;
  Spotlight spotlight;
};


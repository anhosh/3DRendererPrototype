#pragma once

#include <Graphics/Light.hpp>
#include <Graphics/Material.hpp>
#include <Graphics/ShaderProgram.hpp>

class LitSurfaceShaderProgram : public ShaderProgram {
public:
  explicit LitSurfaceShaderProgram(GLuint shaderProgram);

  virtual void bindUniforms(const TransformMatrices& transforms) const override;

public:
  Material material;
  DirectionalLight directionalLight;
  PointLight pointLight;
  Spotlight spotlight;
  glm::vec3 viewPos;
};


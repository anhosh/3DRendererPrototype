#pragma once

struct LightColors {
  glm::vec3 ambient = glm::vec3(0.1f);
  glm::vec3 diffuse = glm::vec3(0.5f);
  glm::vec3 specular = glm::vec3(1.0f);
};

struct DirectionalLight {
  LightColors colors;
  glm::vec3 direction = glm::vec3(-0.2f, -1.0f, -0.3f);
};

struct PointLight {
  LightColors colors;
  glm::vec3 position = glm::vec3(0.0f);
  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;
};

struct Spotlight {
  LightColors colors;
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 direction = glm::vec3(0.0f);
  float cutOff = 12.5f;
  float outerCutOff = 17.5f;
};

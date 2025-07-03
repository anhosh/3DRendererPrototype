#pragma once

struct LightColors {
  glm::vec3 ambient = glm::vec3(0.1f);
  glm::vec3 diffuse = glm::vec3(0.5f);
  glm::vec3 specular = glm::vec3(1.0f);
};

struct DirectionalLight {
  glm::vec3 direction = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));
  LightColors colors;
};

struct PointLight {
  glm::vec3 position = glm::vec3(0.0f);
  LightColors colors;
  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;
};

struct Spotlight {
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 direction = glm::vec3(0.0f);
  LightColors colors;
  float cutOff = 12.5f;
  float outerCutOff = 17.5f;
};

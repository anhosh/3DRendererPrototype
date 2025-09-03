#pragma once

inline constexpr uint32_t SHADOW_MAP_SIZE = 4096;

struct LightColors {
  glm::vec3 ambient = glm::vec3(0.1f);
  glm::vec3 diffuse = glm::vec3(0.5f);
  glm::vec3 specular = glm::vec3(1.0f);
};

struct CompDirectionalLight {
  LightColors colors;
  glm::vec3 direction = glm::vec3(-0.2f, -1.0f, -0.3f);
};

struct CompPointLight {
  LightColors colors;
  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;
};

struct CompSpotlight {
  LightColors colors;
  glm::vec3 direction = glm::vec3(0.0f);
  float cutOff = 12.5f;
  float outerCutOff = 17.5f;
};

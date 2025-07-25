#ifndef LIGHT_SOURCE_UNIFORMS_GLSL
#define LIGHT_SOURCE_UNIFORMS_GLSL

struct LightColors {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct DirectionalLight {
  LightColors colors;
  vec3 direction;
};

struct PointLight {
  LightColors colors;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};

struct Spotlight {
  LightColors colors;
  vec3 position;
  vec3 direction;
  float cutOff;
  float outerCutOff;
};

layout (std430, binding = SSBO_BIND_POINT_DIRECTIONAL_LIGHTS)
readonly buffer DirectionalLightSources {
  uint count;
  DirectionalLight[] sources;
} uDirectionalLights;

layout (std430, binding = SSBO_BIND_POINT_POINT_LIGHTS)
readonly buffer PointLightSources {
  uint count;
  PointLight[] sources;
} uPointLights;

layout (std430, binding = SSBO_BIND_POINT_SPOTLIGHTS)
readonly buffer SpotightSources {
  uint count;
  Spotlight[] sources;
} uSpotlights;

#endif // LIGHT_SOURCE_UNIFORMS_GLSL

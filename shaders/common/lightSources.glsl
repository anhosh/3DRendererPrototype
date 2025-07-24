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
//
//layout (std140, binding = UBO_BIND_POINT_DIRECTIONAL_LIGHTS)
//uniform DirectionalLightSources {
//  uint count;
//  DirectionalLight[] sources;
//} uDirectionalLights;
//
//layout (std140, binding = UBO_BIND_POINT_POINT_LIGHTS)
//uniform PointLightSources {
//  uint count;
//  PointLight[] sources;
//} uPointLights;
//
//layout (std140, binding = UBO_BIND_POINT_SPOTLIGHTS)
//uniform SpotightSources {
//  uint count;
//  Spotlight[] sources;
//} uSpotlights;
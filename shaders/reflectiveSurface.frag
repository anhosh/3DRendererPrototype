#version 460 core

uniform vec3 uViewPos;
uniform samplerCube uEnvironmentMap;

in VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} fsIn;

out vec4 outColor;

void main() {
  vec3 incidental = normalize(fsIn.position - uViewPos);
  vec3 reflected = reflect(incidental, normalize(fsIn.normal));
  outColor = vec4(texture(uEnvironmentMap, reflected).rgb, 1);
}
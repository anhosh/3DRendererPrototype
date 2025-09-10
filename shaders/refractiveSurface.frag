#include "common/cameraUniforms.glsl"

uniform float uRefractiveIndex;
uniform samplerCube uEnvironmentMap;

in VS_OUT {
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec3 bitangent;
  vec2 texCoord;
} fsIn;

out vec4 outColor;

void main() {
  float refractiveIndexRatio = 1 / uRefractiveIndex;
  vec3 incidental = normalize(fsIn.position - uCamera.position);
  vec3 reflected = refract(incidental, normalize(fsIn.normal), refractiveIndexRatio);
  outColor = vec4(texture(uEnvironmentMap, reflected).rgb, 1);
}
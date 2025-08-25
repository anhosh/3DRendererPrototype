#include "common/cameraUniforms.glsl"

out vec4 outColor;

void main() {
  float linearDepth = linearizeDepth(gl_FragCoord.z) / uCamera.far;
  outColor = vec4(vec3(linearDepth), 1);
}

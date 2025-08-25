#include "common/cameraUniforms.glsl"

uniform bool ubPerspective;

void main() {
  if (ubPerspective) {
    gl_FragDepth = linearizeDepth(gl_FragCoord.z) / uCamera.far;
  } else {
    gl_FragDepth = gl_FragCoord.z;
  }
}

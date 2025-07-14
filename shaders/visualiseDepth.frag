#version 460 core

struct Camera {
  float near;
  float far;
};

uniform Camera uCamera;

out vec4 outColor;

float linearizeDepth(float depth) {
  float z = depth * 2 - 1;
  return (2 * uCamera.near * uCamera.far) /
         (uCamera.far + uCamera.near - z * (uCamera.far - uCamera.near));
}

void main() {
  float linearDepth = linearizeDepth(gl_FragCoord.z) / uCamera.far;
  outColor = vec4(vec3(linearDepth), 1);
}

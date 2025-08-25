#ifndef CAMERA_UNIFORMS_GLSL
#define CAMERA_UNIFORMS_GLSL

layout (std140, binding = BINDING_UBO_CAMERA)
uniform Camera {
  mat4 view;
  mat4 projection;
  vec3 position;
  float near;
  float far;
} uCamera;

float linearizeDepth(float depth) {
  float z = depth * 2 - 1;
  return (2 * uCamera.near * uCamera.far) /
         (uCamera.far + uCamera.near - z * (uCamera.far - uCamera.near));
}

#endif // CAMERA_UNIFORMS_GLSL

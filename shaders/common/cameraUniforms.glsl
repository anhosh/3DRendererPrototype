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

#endif // CAMERA_UNIFORMS_GLSL

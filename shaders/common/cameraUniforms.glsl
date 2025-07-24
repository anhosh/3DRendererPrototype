layout (std140, binding = UBO_BIND_POINT_CAMERA)
uniform Camera {
  mat4 view;
  mat4 projection;
  vec3 position;
  float near;
  float far;
} uCamera;

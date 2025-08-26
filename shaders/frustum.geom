#include "common/cameraUniforms.glsl"

struct Frustum {
  vec3 nearBottomLeft;
  vec3 nearBottomRight;
  vec3 nearTopLeft;
  vec3 nearTopRight;
  vec3 farBottomLeft;
  vec3 farBottomRight;
  vec3 farTopLeft;
  vec3 farTopRight;
};

layout (points) in;
layout (line_strip, max_vertices = 16) out;

uniform Frustum uFrustum;

void addVertex(in vec3 position, in mat4 transform) {
  gl_Position = transform * vec4(position, 1);
  EmitVertex();
}

void main() {
  // Override near and far planes to make the whole frustum wireframe visible.
  const float zNear = 0.01;
  const float zFar = 1000;
  mat4 projection = uCamera.projection;
  projection[2][2] = zFar / (zNear - zFar);
  projection[3][2] = -(zFar * zNear) / (zFar - zNear);
  mat4 viewProjection = projection * uCamera.view;

  addVertex(uFrustum.nearBottomLeft, viewProjection);
  addVertex(uFrustum.nearBottomRight, viewProjection);
  addVertex(uFrustum.nearTopRight, viewProjection);
  addVertex(uFrustum.nearTopLeft, viewProjection);
  EndPrimitive();

  addVertex(uFrustum.farTopLeft, viewProjection);
  addVertex(uFrustum.nearTopLeft, viewProjection);
  addVertex(uFrustum.nearBottomLeft, viewProjection);
  addVertex(uFrustum.farBottomLeft, viewProjection);
  EndPrimitive();

  addVertex(uFrustum.farTopRight, viewProjection);
  addVertex(uFrustum.farTopLeft, viewProjection);
  addVertex(uFrustum.farBottomLeft, viewProjection);
  addVertex(uFrustum.farBottomRight, viewProjection);
  EndPrimitive();

  addVertex(uFrustum.nearTopRight, viewProjection);
  addVertex(uFrustum.farTopRight, viewProjection);
  addVertex(uFrustum.farBottomRight, viewProjection);
  addVertex(uFrustum.nearBottomRight, viewProjection);
  EndPrimitive();
}

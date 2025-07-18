#version 460 core

layout (location = 2) uniform mat4 uViewTransform;
layout (location = 3) uniform mat4 uProjectionTransform;

layout (location = 0) in vec3 inPosition;

out VS_OUT {
  vec3 texCoords;
} vsOut;

void main() {
  gl_Position = uProjectionTransform * uViewTransform * vec4(inPosition, 1.0);
  vsOut.texCoords = inPosition;
}

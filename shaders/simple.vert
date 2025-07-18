#version 460 core

layout (location = 0) uniform mat4 uModelTransform;
layout (location = 1) uniform mat3 uNormalTransform;
layout (location = 2) uniform mat4 uViewTransform;
layout (location = 3) uniform mat4 uProjectionTransform;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

out VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} vsOut;

void main() {
  vec4 vertexPosWorld = uModelTransform * vec4(inPosition, 1);
  gl_Position = uProjectionTransform * uViewTransform * vertexPosWorld;
  vsOut.position = vertexPosWorld.xyz;
  vsOut.normal = uNormalTransform * inNormal;
  vsOut.texCoord = inTexCoord;
}

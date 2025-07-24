#include "common/cameraUniforms.glsl"

layout (location = 0) uniform mat4 uModelTransform;
layout (location = 1) uniform mat3 uNormalTransform;

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
  gl_Position = uCamera.projection * uCamera.view * vertexPosWorld;
  vsOut.position = vertexPosWorld.xyz;
  vsOut.normal = uNormalTransform * inNormal;
  vsOut.texCoord = inTexCoord;
}

#include "common/cameraUniforms.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

layout (location = 3) in mat4 inModelTransform;  // (column 0)
//      location = 4                                (column 1)
//      location = 5                                (column 2)
//      location = 6                                (column 3)
layout (location = 7) in mat3 inNormalTransform; // (column 0)
//      location = 8                                (column 1)
//      location = 9                                (column 2)

out VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} vsOut;

void main() {
  vec4 vertexPosWorld = inModelTransform * vec4(inPosition, 1);
  gl_Position = uCamera.projection * uCamera.view * vertexPosWorld;
  vsOut.position = vertexPosWorld.xyz;
  vsOut.normal = normalize(inNormalTransform * inNormal);
  vsOut.texCoord = inTexCoord;
}

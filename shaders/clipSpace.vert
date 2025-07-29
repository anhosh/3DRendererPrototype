#include "common/cameraUniforms.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

layout (location = 4) in mat4 inModelTransform;
//      location = 5    (column 2)
//      location = 6    (column 3)
//      location = 7    (column 4)
layout (location = 8) in mat3 inNormalTransform;
//      location = 9    (column 2)
//      location = 10   (column 3)

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

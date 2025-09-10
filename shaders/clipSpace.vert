#include "common/cameraUniforms.glsl"

struct InstanceData {
  mat4 model;
  mat3 normal;
};

layout (std430, binding = BINDING_SSBO_INSTANCES) readonly buffer Instances {
  InstanceData data[];
} uInstances;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec2 inTexCoord;

out VS_OUT {
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec3 bitangent;
  vec2 texCoord;
} vsOut;

void main() {
  InstanceData currentInstance = uInstances.data[gl_InstanceID];
  vec4 vertexPosWorld = currentInstance.model * vec4(inPosition, 1);

  gl_Position = uCamera.projection * uCamera.view * vertexPosWorld; // clip space

  vsOut.position = vertexPosWorld.xyz;
  vsOut.normal = normalize(currentInstance.normal * inNormal);
  vsOut.tangent = normalize(currentInstance.normal * inTangent);
  vsOut.bitangent = cross(vsOut.normal, vsOut.tangent);
  vsOut.texCoord = inTexCoord;
}

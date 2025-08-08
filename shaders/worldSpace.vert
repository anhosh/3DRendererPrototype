layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

struct InstanceData {
  mat4 model;
  mat3 normal;
};

layout (std430, binding = BINDING_SSBO_INSTANCES) readonly buffer Instances {
  InstanceData data[];
} uInstances;

out VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} vsOut;

void main() {
  vec4 vertexPosWorld = uInstances.data[gl_InstanceID].model * vec4(inPosition, 1);
  gl_Position = vertexPosWorld;
  vsOut.position = vertexPosWorld.xyz;
  vsOut.normal = normalize(uInstances.data[gl_InstanceID].normal * inNormal);
  vsOut.texCoord = inTexCoord;
}

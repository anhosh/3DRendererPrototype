#include "common/cameraUniforms.glsl"

struct InstanceData {
  mat4 model;
  mat3 normal;
};

layout (std430, binding = BINDING_SSBO_INSTANCES) readonly buffer Instances {
  InstanceData data[];
} uInstances;

layout (location = 0) in vec3 inPosition;

void main() {
  gl_Position = uCamera.projection * uCamera.view * uInstances.data[gl_InstanceID].model * vec4(inPosition, 1.0);
}

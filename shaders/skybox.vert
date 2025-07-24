#include "common/cameraUniforms.glsl"

layout (location = 0) in vec3 inPosition;

out VS_OUT {
  vec3 texCoords;
} vsOut;

void main() {
  vec4 vertexPosition = uCamera.projection * mat4(mat3(uCamera.view)) * vec4(inPosition, 1.0);
  gl_Position = vertexPosition.xyww;
  vsOut.texCoords = inPosition;
}

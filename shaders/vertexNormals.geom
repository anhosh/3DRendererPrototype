#include "common/cameraUniforms.glsl"

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in VS_OUT {
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec3 bitangent;
  vec2 texCoord;
} gsIn[];

out GS_OUT {
  vec3 color;
} gsOut;

const float MAGNITUDE = 0.1;

void generateLine(uint index) {
  gl_Position = uCamera.projection * uCamera.view * vec4(gsIn[index].position, 1);
  gsOut.color = vec3(1, 0, 0);
  EmitVertex();

  vec3 tipPosition = gsIn[index].position + gsIn[index].normal * MAGNITUDE;
  gl_Position = uCamera.projection * uCamera.view * vec4(tipPosition, 1);
  gsOut.color = vec3(1, 1, 0);
  EmitVertex();

  EndPrimitive();
}

void main() {
  generateLine(0);
  generateLine(1);
  generateLine(2);
}

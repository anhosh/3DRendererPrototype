#include "common/cameraUniforms.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} gsIn[];

out GS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} gsOut;

uniform float uExplosionDistance;

vec3 getNormal() {
  vec3 a = gsIn[0].position.xyz - gsIn[1].position.xyz;
  vec3 b = gsIn[2].position.xyz - gsIn[1].position.xyz;
  return normalize(cross(b, a));
}

vec4 explode(in vec4 position, in vec3 normal) {
  vec3 direction = normal * uExplosionDistance;
  return position + vec4(direction, 0);
}

void main() {
  vec3 normal = getNormal();

  for (uint i = 0; i < 3; ++i) {
    gsOut.position = explode(vec4(gsIn[i].position, 1), normal).xyz;
    gsOut.normal = gsIn[i].normal;
    gsOut.texCoord = gsIn[i].texCoord;
    gl_Position = uCamera.projection * uCamera.view * vec4(gsOut.position, 1);
    EmitVertex();
  }

  EndPrimitive();
}

#version 460 core

in VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} fsIn;

out vec4 outColor;

void main() {
  outColor = vec4((normalize(fsIn.normal) + 1) * 0.5, 1);
}

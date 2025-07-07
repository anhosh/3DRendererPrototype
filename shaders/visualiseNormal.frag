#version 460

in VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} fsIn;

out vec4 outColor;

void main() {
  outColor = vec4(normalize(fsIn.normal) * 0.5 + 1, 1.0);
}

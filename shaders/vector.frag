out vec4 outColor;

in GS_OUT {
  vec3 color;
} fsIn;

void main() {
  outColor = vec4(fsIn.color, 1);
}

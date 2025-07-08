#version 460

layout(location = 4) uniform vec3 uOutlineColor;

out vec4 outColor;

void main() {
  outColor = vec4(uOutlineColor, 1.0);
}

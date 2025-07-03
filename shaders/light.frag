#version 460 core

layout(location = 4) uniform vec3 uLightColor;

out vec4 outColor;

void main() {
  outColor = vec4(uLightColor, 1.0);
}

#version 460 core

layout (location = 0) uniform sampler2D uScreenTexture;

in VS_OUT {
  vec2 texCoord;
} fsIn;

out vec4 outColor;

void main() {
  outColor = vec4(1 - texture(uScreenTexture, fsIn.texCoord).rgb, 1);
}

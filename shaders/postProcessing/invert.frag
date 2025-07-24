layout (location = 0) uniform sampler2D uScreenTexture;

in VS_OUT {
  vec2 texCoord;
} fsIn;

out vec4 outColor;

void main() {
  outColor = texture(uScreenTexture, fsIn.texCoord);
  outColor = vec4(1 - outColor.rgb, 1);
}

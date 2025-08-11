uniform sampler2D uScreenTexture;

in VS_OUT {
  vec2 texCoord;
} fsIn;

out vec4 outColor;

void main() {
  vec2 texCoord = vec2(fsIn.texCoord.x, 1 - fsIn.texCoord.y);
  outColor = texture(uScreenTexture, texCoord);
}

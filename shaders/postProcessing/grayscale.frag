uniform sampler2D uScreenTexture;

in VS_OUT {
  vec2 texCoord;
} fsIn;

out vec4 outColor;

void main() {
  outColor = texture(uScreenTexture, fsIn.texCoord);
  float average = 0.2126 * outColor.r +
                  0.7152 * outColor.g +
                  0.0722 * outColor.b;
  outColor = vec4(vec3(average), 1);
}

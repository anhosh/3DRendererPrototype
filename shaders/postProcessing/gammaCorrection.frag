uniform sampler2D uScreenTexture;
uniform float uGamma;

in VS_OUT {
  vec2 texCoord;
} fsIn;

out vec4 outColor;

void main() {
  vec3 color = texture(uScreenTexture, fsIn.texCoord).rgb;
  outColor.rgb = pow(color, vec3(1 / uGamma));
}
uniform samplerCube uSkyTexture;

in VS_OUT {
  vec3 texCoords;
} fsIn;

out vec4 outColor;

void main() {
  outColor = texture(uSkyTexture, fsIn.texCoords);
}

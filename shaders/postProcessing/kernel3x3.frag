layout (location = 0) uniform sampler2D uScreenTexture;
layout (location = 1) uniform float uOffset;
layout (location = 2) uniform mat3 uKernel;

in VS_OUT {
  vec2 texCoord;
} fsIn;

out vec4 outColor;

void main() {
  const vec2 offsets[9] = vec2[](
    vec2(-uOffset,  uOffset), // top-left
    vec2( 0.0f,     uOffset), // top-center
    vec2( uOffset,  uOffset), // top-right
    vec2(-uOffset,  0.0f),   // center-left
    vec2( 0.0f,     0.0f),   // center-center
    vec2( uOffset,  0.0f),   // center-right
    vec2(-uOffset, -uOffset), // bottom-left
    vec2( 0.0f,    -uOffset), // bottom-center
    vec2( uOffset, -uOffset)  // bottom-right    
  );

  vec3 color = vec3(0);
  for (uint i = 0; i < 3; ++i) {
    for (uint j = 0; j < 3; ++j) {
      color += texture(uScreenTexture, fsIn.texCoord + offsets[i * 3 + j]).rgb * uKernel[i][j];
    }
  }
  outColor = vec4(color, 1);
}

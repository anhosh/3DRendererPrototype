#version 460 core

layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec2 inTexCoord;

out VS_OUT {
  vec2 texCoord;
} vsOut;

void main() {
//  const vec2 positions[4] = vec2[](
//    vec2(-1,  1),
//    vec2(-1, -1),
//    vec2( 1,  1),
//    vec2( 1, -1)
//  );
//  gl_Position = vec4(positions[gl_VertexID], 0, 1);
//  vsOut.texCoord = positions[gl_VertexID].xy * 0.5 + 0.5;
  gl_Position = vec4(inPosition, 1);
  vsOut.texCoord = inTexCoord;
}

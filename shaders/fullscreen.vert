#version 450

// No vertex attributes needed

// Output UV coordinates to fragment shader
layout(location = 0) out vec2 outUV;

// Draw a single triangle covering the whole screen
// Vertices are generated directly in the shader
// UVs are calculated to cover the 0..1 range
vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0), // Bottom-left
    vec2( 3.0, -1.0), // Far right (covers right half)
    vec2(-1.0,  3.0)  // Far top (covers top half)
);

vec2 uvs[3] = vec2[](
    vec2(0.0, 0.0), // Corresponding UVs
    vec2(2.0, 0.0),
    vec2(0.0, 2.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    outUV = uvs[gl_VertexIndex];
} 
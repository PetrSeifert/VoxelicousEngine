#version 450

// Input UV coordinates from vertex shader
layout(location = 0) in vec2 inUV;

// Output color
layout(location = 0) out vec4 outColor;

// Texture sampler for the raytraced image
layout(set = 0, binding = 0) uniform sampler2D raytraceTexture;

void main() {
    // Sample the texture using the UV coordinates
    outColor = texture(raytraceTexture, inUV);
    // Optional: Apply gamma correction or tonemapping here if needed
    // outColor.rgb = pow(outColor.rgb, vec3(1.0/2.2));
} 
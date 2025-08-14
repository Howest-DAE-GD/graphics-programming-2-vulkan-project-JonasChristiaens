#version 450

layout(constant_id = 0) const uint TEXTURE_ARRAY_SIZE = 2;
layout(push_constant) uniform constants {
    uint textureIndex;
} pushConstants;

layout(set = 0, binding = 0) uniform sampler sharedSampler;
layout(set = 0, binding = 1) uniform texture2D textures[TEXTURE_ARRAY_SIZE];

// G-buffer outputs
layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec4 fragAlbedo;
layout(location = 4) in vec2 fragMaterial;

// Multiple render targets
layout(location = 0) out vec4 outPosition;    // RGBA16F
layout(location = 1) out vec2 outNormal;      // RG16_SNORM
layout(location = 2) out vec4 outAlbedo;      // RGBA8_UNORM
layout(location = 3) out vec2 outMaterial;    // RG8_UNORM

void main() {
    // Sample texture if needed
    vec4 texColor = texture(sampler2D(textures[pushConstants.textureIndex], sharedSampler), fragTexCoord); 
    
    // Write to G-buffer
    outPosition = vec4(fragPosition, 1.0);
    outNormal = fragNormal;
    outAlbedo = texColor * fragAlbedo; // Combine vertex color with texture
    outMaterial = fragMaterial;
}
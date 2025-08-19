#version 450

layout(constant_id = 0) const uint TEXTURE_ARRAY_SIZE = 2;
layout(push_constant) uniform constants {
    uint textureIndex;
} pushConstants;

layout(set = 0, binding = 0) uniform sampler sharedSampler;
layout(set = 0, binding = 1) uniform texture2D textures[TEXTURE_ARRAY_SIZE];      // Albedo
layout(set = 0, binding = 6) uniform texture2D normalMaps[TEXTURE_ARRAY_SIZE];    // Normal maps

// G-buffer inputs
layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 fragAlbedo;
layout(location = 3) in vec2 fragMaterial;
layout(location = 4) in mat3 fragTBN;

// G-buffer outputs
layout(location = 0) out vec4 outPosition;    // RGBA16F
layout(location = 1) out vec2 outNormal;      // RG16_SNORM (oct-encoded)
layout(location = 2) out vec4 outAlbedo;      // RGBA8_UNORM
layout(location = 3) out vec2 outMaterial;    // RG8_UNORM

const float alphaThreshold = 0.95f;

vec2 octEncode(vec3 n) {
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    vec2 p = n.z >= 0.0 ? n.xy : (1.0 - abs(n.yx)) * (sign(n.xy));
    return p;
}

void main() {
    // Sample albedo texture
    vec4 texColor = texture(sampler2D(textures[pushConstants.textureIndex], sharedSampler), fragTexCoord);

    // Alpha clipping
    if (texColor.a < alphaThreshold)
        discard;

    // Sample tangent-space normal map
    vec3 tangentNormal = texture(sampler2D(normalMaps[pushConstants.textureIndex], sharedSampler), fragTexCoord).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0); // From [0,1] to [-1,1]

    // Transform to world space
    vec3 worldNormal = normalize(fragTBN * tangentNormal);

    // Write to G-buffer
    outPosition = vec4(fragPosition, 1.0);
    outNormal = octEncode(worldNormal);
    outAlbedo = texColor; 
    outMaterial = fragMaterial;
}
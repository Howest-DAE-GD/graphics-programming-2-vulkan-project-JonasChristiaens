#version 450

layout(constant_id = 0) const uint TEXTURE_ARRAY_SIZE = 2;
layout(set = 0, binding = 0) uniform sampler sharedSampler;
layout(set = 0, binding = 1) uniform texture2D textures[TEXTURE_ARRAY_SIZE];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uint fragTextureIndex;

const float alphaThreshold = 0.95f;

void main() {
    vec4 texColor = texture(sampler2D(textures[fragTextureIndex], sharedSampler), fragTexCoord);

    if (texColor.a < alphaThreshold)
        discard;
}
#version 450

layout(set = 1, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBiTangent;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec4 fragAlbedo;
layout(location = 4) out vec2 fragMaterial;

vec2 octEncode(vec3 n) {
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    vec2 p = n.z >= 0.0 ? n.xy : (1.0 - abs(n.yx)) * (sign(n.xy));
    return p;
}

void main() {
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * worldPos;
    
    fragPosition = worldPos.xyz;
    fragNormal = octEncode(normalize(mat3(ubo.model) * inNormal));
    fragTexCoord = inTexCoord;
    fragAlbedo = vec4(inColor, 1.0);
    fragMaterial = vec2(0.5, 0.0); 
}
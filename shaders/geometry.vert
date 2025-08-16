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
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec4 fragAlbedo;
layout(location = 3) out vec2 fragMaterial;
layout(location = 4) out mat3 fragTBN; // TBN matrix for normal mapping

void main() {
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * worldPos;

    fragPosition = worldPos.xyz;
    fragTexCoord = inTexCoord;
    fragAlbedo = vec4(inColor, 1.0);
    fragMaterial = vec2(0.5, 0.0);

    // Transform tangent-space basis vectors to world space
    vec3 T = normalize(mat3(ubo.model) * inTangent);
    vec3 B = normalize(mat3(ubo.model) * inBiTangent);
    vec3 N = normalize(mat3(ubo.model) * inNormal);
    fragTBN = mat3(T, B, N);
}
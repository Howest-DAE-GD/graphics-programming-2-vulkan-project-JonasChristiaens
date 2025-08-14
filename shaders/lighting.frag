#version 450

layout(set = 0, binding = 2) uniform sampler2D gPosition;
layout(set = 0, binding = 3) uniform sampler2D gNormal;
layout(set = 0, binding = 4) uniform sampler2D gAlbedo;
layout(set = 0, binding = 5) uniform sampler2D gMaterial;

layout(location = 0) out vec4 outColor;

vec3 octDecode(vec2 f) {
    vec3 n = vec3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = clamp(-n.z, 0.0, 1.0);
    // Manually handle the conditional addition
    n.xy += vec2(
        (n.x >= 0.0) ? -t : t,
        (n.y >= 0.0) ? -t : t
    );
    return normalize(n);
}

void main() {
    vec2 uv = gl_FragCoord.xy / textureSize(gPosition, 0).xy;
    vec3 fragPos = texture(gPosition, uv).rgb;
    vec3 normal = octDecode(texture(gNormal, uv).rg);
    vec4 albedo = texture(gAlbedo, uv);
    vec2 material = texture(gMaterial, uv).rg;
    
    float roughness = material.r;
    float metallic = material.g;
    
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * albedo.rgb;
    
    vec3 ambient = vec3(0.1) * albedo.rgb;
    
    outColor = vec4(ambient + diffuse, 1.0);
}
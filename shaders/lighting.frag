#version 450

layout(set = 0, binding = 2) uniform sampler2D gPosition;
layout(set = 0, binding = 3) uniform sampler2D gNormal;
layout(set = 0, binding = 4) uniform sampler2D gAlbedo;
layout(set = 0, binding = 5) uniform sampler2D gMaterial;
layout(set = 0, binding = 7) uniform sampler2D gDepth;

layout(set = 0, binding = 8) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    mat4 invView;
    mat4 invProj;
} ubo;

layout(location = 0) out vec4 outColor;

const float PI = 3.141592;

// Kelvin temperature to RGB conversion
vec3 KelvinToRgb(float kelvin)
{
    const float temp = kelvin / 100.0;
    float r = (temp <= 66.0)
        ? 1.0
        : clamp(1.29293618606 * pow(temp - 60.0, -0.1332047592), 0.0, 1.0);
    float g = (temp <= 66.0)
        ? clamp(0.390081578769 * log(temp) - 0.631841443784, 0.0, 1.0)
        : clamp(1.129890860895 * pow(temp - 60.0, -0.0755148492), 0.0, 1.0);
    float b = (temp >= 66.0)
        ? 1.0
        : (temp <= 19.0)
            ? 0.0
            : clamp(0.543206789911 * log(temp - 10.0) - 1.19625408914, 0.0, 1.0);
    return vec3(r, g, b);
}

// Octahedral normal decoding
vec3 octDecode(vec2 f) {
    vec3 n = vec3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = clamp(-n.z, 0.0, 1.0);
    n.xy += vec2(
        (n.x >= 0.0) ? -t : t,
        (n.y >= 0.0) ? -t : t
    );
    return normalize(n);
}

// World position reconstruction from depth
vec3 reconstructWorldPosition(vec2 uv, float depth)
{
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = ubo.invProj * ndc;
    viewPos /= viewPos.w;
    vec4 worldPos = ubo.invView * viewPos;
    return worldPos.xyz;
}

// Schlick Fresnel
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Smith's Geometry (GGX)
float geometrySmith(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float G_V = NdotV / (NdotV * (1.0 - k) + k);
    float G_L = NdotL / (NdotL * (1.0 - k) + k);
    return G_V * G_L;
}

// GGX Normal Distribution Function (Unity perceptual roughness)
float distributionGGX(float NdotH, float roughness)
{
    float alpha = roughness;
    float alpha2 = alpha * alpha;
    float denom = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * denom * denom);
}

void main() {
    // Read depth
    ivec2 pixelCoord = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(gDepth, pixelCoord, 0).r;

    vec2 uv = gl_FragCoord.xy / textureSize(gPosition, 0).xy;
    vec3 fragPos = reconstructWorldPosition(uv, depth);

    // G-buffer
    vec3 normal = octDecode(texture(gNormal, uv).rg);
    vec3 albedo = texture(gAlbedo, uv).rgb;
    vec2 material = texture(gMaterial, uv).rg;
    float roughness = material.r;
    float metallic  = material.g;

    vec3 V = normalize(ubo.cameraPos - fragPos);
    vec3 N = normalize(normal);

    // === Omni Light ===
    vec3 lightPos = vec3(2.0, 9.5, 0.0);
    float kelvin = 1000.0;
    vec3 lightColor = KelvinToRgb(kelvin);
    float intensity = 100.0; 

    float r2 = dot(lightPos - fragPos, lightPos - fragPos);
    vec3 irradiance = lightColor * (intensity / r2);

    // Calculate other terms
    vec3 L = normalize(lightPos - fragPos);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    // Fresnel
    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
    vec3 F = fresnelSchlick(VdotH, F0);

    // NDF
    float D = distributionGGX(NdotH, roughness);

    // Geometry
    float G = geometrySmith(NdotV, NdotL, roughness);

    // Specular BRDF
    vec3 numerator = D * F * G;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular = numerator / denominator;

    // Diffuse BRDF
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    // Final radiance (Lambertian diffuse + specular)
    vec3 Lo = (diffuse + specular) * irradiance * NdotL;

    // Ambient term
    vec3 ambient = vec3(0.03) * albedo;
    vec3 color = ambient + Lo;

    outColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}
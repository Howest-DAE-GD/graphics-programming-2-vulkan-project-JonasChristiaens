#version 450

layout(set = 0, binding = 0) uniform sampler2D hdrImage;
layout(set = 0, binding = 1) uniform CameraSettings {
    float aperture;      
    float shutterSpeed;  
    float ISO;
} camera;

layout(location = 0) out vec4 outColor;

float CalculateEV100FromPhysicalCamera(float aperture, float shutterTime, float ISO)
{
    return log2(pow(aperture, 2.0) / shutterTime * 100.0 / ISO);
}

float ConvertEV100ToExposure(float EV100)
{
    float maxLuminance = 1.2 * pow(2.0, EV100);
    return 1.0 / max(maxLuminance, 0.0001);
}

vec3 Uncharted2Tonemap(vec3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;

    vec3 curr = ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    float whiteScale = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    return curr / whiteScale;
}

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(hdrImage, 0));
    vec3 hdrColor = texture(hdrImage, uv).rgb;

    float EV100 = CalculateEV100FromPhysicalCamera(camera.aperture, camera.shutterSpeed, camera.ISO);
    float exposure = ConvertEV100ToExposure(EV100);

    vec3 exposedColor = hdrColor * exposure;

    // Apply Uncharted2 filmic tone mapping
    vec3 ldrColor = Uncharted2Tonemap(exposedColor);
    ldrColor = clamp(ldrColor, 0.0, 1.0);

    outColor = vec4(ldrColor, 1.0);
}
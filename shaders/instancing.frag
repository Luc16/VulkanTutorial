#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 lightDirection;
} ubo;

layout(location = 0) out vec4 outColor;

const float AMBIENT = 0.05;

void main() {
    float lightIntensity = AMBIENT + max(dot(normalize(fragNormalWorld), normalize(ubo.lightDirection)), 0);

    outColor = vec4(lightIntensity*fragColor, 1.0);
}
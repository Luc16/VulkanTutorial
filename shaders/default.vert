#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;


layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 lightDirection;
    float time;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
} push;

const float AMBIENT = 0.05;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.proj * ubo.view * push.model * vec4(inPosition, 1.0);

    vec3 worldNormal = normalize(mat3(push.model) * inNormal);

    float lightIntensity = AMBIENT + max(dot(worldNormal, ubo.lightDirection), 0);

    fragColor = lightIntensity*inColor;
}
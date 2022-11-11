#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 lightDirection;
} ubo;

const float AMBIENT = 0.05;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out float fragLightIntensity;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    vec3 worldNormal = normalize(mat3(ubo.model) * inNormal);

    fragLightIntensity = AMBIENT + max(dot(worldNormal, ubo.lightDirection), 0);

    fragTexCoord = inTexCoord;
}
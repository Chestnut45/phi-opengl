#version 460

layout(binding = 4) uniform sampler2D lightTexture;

in vec2 texCoords;
out vec3 finalColor;

void main()
{
    finalColor = texture(lightTexture, texCoords).rgb;
}
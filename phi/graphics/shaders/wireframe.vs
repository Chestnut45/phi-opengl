#version 460

// Camera uniform block
layout(std140, binding = 0) uniform CameraBlock
{
    mat4 viewProj;
    mat4 invViewProj;
    mat4 view;
    mat4 invView;
    mat4 proj;
    vec4 cameraPos;
    vec2 hres;
};

in vec3 vPos;

void main()
{
    gl_Position = viewProj * vec4(vPos, 1.0);
}
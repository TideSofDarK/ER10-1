#version 410 core

in vec3 positionWorldSpace;
in vec4 vertexColor;
in vec3 eyeDirectionCameraSpace;

out vec4 color;

uniform mat4 MV;

void main()
{
    color = vertexColor;
}
#version 410 core

layout (location = 0) in vec3 vertexPositionModelSpace;

layout (std140) uniform ub_common
{
    mat4 Projection;
    mat4 View;
};
uniform mat4 Model;

out vec3 positionWorldSpace;
out vec4 vertexColor;
out vec3 eyeDirectionCameraSpace;

void main()
{
    gl_Position = Projection * View * Model * vec4(vertexPositionModelSpace, 1.0);
    positionWorldSpace = (Model * vec4(vertexPositionModelSpace, 1)).xyz;

    vec3 vertexPositionCameraSpace = (View * Model * vec4(vertexPositionModelSpace, 1)).xyz;
    eyeDirectionCameraSpace = vec3(0, 0, 0) - vertexPositionCameraSpace;

    vertexColor = vec4(1.0, 0.0, 0.0, 1.0);
}
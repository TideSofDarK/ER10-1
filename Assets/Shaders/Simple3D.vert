#version 410 core

layout (location = 0) in vec3 vertexPositionModelSpace;

layout (std140) uniform ub_common
{
    mat4 u_projection;
    mat4 u_view;
};
uniform mat4 u_model;

out vec3 f_positionWorldSpace;
out vec4 f_vertexColor;
out vec3 f_eyeDirectionCameraSpace;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(vertexPositionModelSpace, 1.0);
    f_positionWorldSpace = (u_model * vec4(vertexPositionModelSpace, 1)).xyz;

    vec3 vertexPositionCameraSpace = (u_view * u_model * vec4(vertexPositionModelSpace, 1)).xyz;
    f_eyeDirectionCameraSpace = vec3(0, 0, 0) - vertexPositionCameraSpace;

    f_vertexColor = vec4(1.0, 0.0, 0.0, 1.0);
}
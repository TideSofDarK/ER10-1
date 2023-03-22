layout (location = 0) in vec3 a_vertexPositionModelSpace;
layout (location = 1) in vec2 a_texCoord;

layout (std140) uniform ub_common
{
    mat4 u_projection;
    mat4 u_view;
};
uniform int u_mode;
uniform vec4 u_modeControlA;
uniform mat4 u_model[SIMPLE3D_MODEL_COUNT];

out vec3 f_positionWorldSpace;
out vec4 f_vertexColor;
out vec3 f_eyeDirectionCameraSpace;

void main()
{
    mat4 model = u_model[gl_InstanceID];

    gl_Position = u_projection * u_view * model * vec4(a_vertexPositionModelSpace, 1.0);

    f_positionWorldSpace = (model * vec4(a_vertexPositionModelSpace, 1)).xyz;

    vec3 vertexPositionCameraSpace = (u_view * model * vec4(a_vertexPositionModelSpace, 1)).xyz;
    f_eyeDirectionCameraSpace = vec3(0, 0, 0) - vertexPositionCameraSpace;

    f_vertexColor = vec4(1.0, 0.0, 0.0, 1.0);
}
uniform int u_mode;
uniform vec4 u_modeControlA;

in vec3 f_positionWorldSpace;
in vec4 f_vertexColor;
in vec3 f_eyeDirectionCameraSpace;

out vec4 color;

void main()
{
    color = f_vertexColor;
}
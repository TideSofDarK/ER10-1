uniform int u_mode;
uniform vec4 u_modeControlA;
uniform sampler2D u_commonAtlas;
uniform sampler2D u_primaryAtlas;

in vec2 f_texCoord;
in vec3 f_positionWorldSpace;
in vec4 f_vertexColor;
in vec3 f_eyeDirectionCameraSpace;
in float f_distanceToCamera;

out vec4 color;

void main()
{
    if (u_mode == UBER3D_MODE_BASIC) {
        color = texture(u_primaryAtlas, f_texCoord);
    }

    if (u_mode == UBER3D_MODE_LEVEL) {
        color = texture(u_primaryAtlas, f_texCoord);
    }

    float fog = 1.0 - (f_distanceToCamera * 0.2);
    fog *= 64.0;
    fog = round(fog);
    fog = fog / 64.0;
    fog = clamp(fog, 0.0, 1.0);
    color = color * fog;
}
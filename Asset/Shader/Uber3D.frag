uniform int u_mode;
uniform vec4 u_modeControlA;
uniform sampler2D u_commonAtlas;
uniform sampler2D u_primaryAtlas;

in vec2 f_texCoord;
in vec4 f_positionViewSpace;
in vec3 f_positionWorldSpace;
in vec4 f_vertexColor;
in vec3 f_eyeDirectionCameraSpace;

out vec4 color;

void main()
{
    if (u_mode == UBER3D_MODE_BASIC) {
        color = texture(u_primaryAtlas, f_texCoord);
    }

    if (u_mode == UBER3D_MODE_LEVEL) {
        color = texture(u_primaryAtlas, f_texCoord);
    }

    float dist = (f_positionViewSpace.x * f_positionViewSpace.x) + (f_positionViewSpace.y * f_positionViewSpace.y) + (f_positionViewSpace.z * f_positionViewSpace.z);

    color = mix(vec4(0.0), color, getFogFactor(dist));
}


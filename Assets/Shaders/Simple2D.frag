#version 410 core

layout (std140) uniform ub_common
{
    vec2 u_screenSize;
    float u_time;
};
uniform int u_mode;
uniform vec4 u_modeControlA;
uniform vec2 u_sizeScreenSpace;
uniform sampler2D u_colorTexture;

in vec2 f_texCoord;
in vec4 f_modeControlOutA;

out vec4 color;

void main()
{
    vec2 texCoord = f_texCoord;

    // Haze
    if (u_mode == 1) {
        float xIntensity = 0.07;
        float yIntensity = 4.0;
        float speed = 4.0;
        texCoord.x += sin(texCoord.y * yIntensity * 3.14159 + (u_time * speed)) * xIntensity;
    }

    color = texture(u_colorTexture, texCoord);

    // Back Blur
    if (u_mode == 2) {
        float step = 1.0 / u_modeControlA.x;
        float from = step * f_modeControlOutA.x;
        float to = from + step;
        color.a *= 1.0 - (mix(from, to, fract(u_time * u_modeControlA.y)));
        color.a *= 0.9;
    }
}
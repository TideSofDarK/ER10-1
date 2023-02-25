#version 410 core

layout (std140) uniform ub_common
{
    vec2 u_screenSize;
    float u_time;
};
uniform vec2 u_sizeScreenSpace;
uniform sampler2D u_colorTexture;

out vec4 color;

in vec2 texCoord;

varying vec2 f_texcoord;

void main()
{
    vec2 pixelPos = f_texcoord * u_sizeScreenSpace;
    float borderSize = 2.0;
    float borderX = clamp(abs( (((f_texcoord.x * 2) - 1.0) * u_sizeScreenSpace.x))  - (u_sizeScreenSpace.x - borderSize), 0.0, 1.0);
    float borderY = clamp(abs( (((f_texcoord.y * 2) - 1.0) * u_sizeScreenSpace.y))  - (u_sizeScreenSpace.y - borderSize), 0.0, 1.0);
    float border = borderX + borderY;
//    float border = clamp(floor(f_texcoord.x) + 0.1 + floor(f_texcoord.y), 0.0, 1.0);
    color = vec4(border, border, border, 1.0);
//    color = texture(u_colorTexture, f_texcoord);
}

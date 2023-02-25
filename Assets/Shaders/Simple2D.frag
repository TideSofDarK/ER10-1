#version 410 core

layout (std140) uniform ub_common
{
    vec2 u_screenSize;
    float u_time;
};
uniform sampler2D u_colorTexture;

out vec4 color;

in vec2 texCoord;

varying vec2 f_texcoord;

void main()
{
    vec2 texcoord = f_texcoord;
    texcoord.x += sin(texcoord.y * 4*2*3.14159 + (u_time * 2*3.14159 * 0.75)) / 100;
    color = texture(u_colorTexture, texcoord);
//    color = vec4(1.0, 1.0, 1.0, 1.0);
}
uniform sampler2D u_colorTexture;

in vec2 f_texCoord;

out vec4 color;

void main()
{
    color = texture(u_colorTexture, f_texCoord);
}
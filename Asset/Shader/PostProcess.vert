layout(location = 0) in vec2 a_vertexPositionModelSpace;
layout(location = 1) in vec2 a_texCoord;

out vec2 f_texCoord;

void main()
{
    gl_Position = vec4(a_vertexPositionModelSpace.x * 2.0 - 1.0, a_vertexPositionModelSpace.y * 2.0 - 1.0, 0.0, 1.0);
    f_texCoord = a_texCoord;
}


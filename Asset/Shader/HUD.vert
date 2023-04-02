layout (location = 0) in vec2 vertexPositionModelSpace;
layout (location = 1) in vec2 texCoord;

layout (std140) uniform ub_common
{
    vec2 u_screenSize;
    float u_time;
};
uniform vec2 u_positionScreenSpace;
uniform vec2 u_sizeScreenSpace;

out vec2 f_texCoord;

void main()
{
    vec2 positionScreenSpace = u_positionScreenSpace;
    positionScreenSpace = round(positionScreenSpace);

    float ndcX = (((positionScreenSpace.x / u_screenSize.x) * 2.0) - 1.0);
    float ndcY = (((positionScreenSpace.y / u_screenSize.y) * 2.0) - 1.0);
    float width = ((u_sizeScreenSpace.x / u_screenSize.x)) * 2.0;
    float height = ((u_sizeScreenSpace.y / u_screenSize.y)) * 2.0;
    gl_Position = vec4(ndcX + (vertexPositionModelSpace.x * width), -(ndcY + (vertexPositionModelSpace.y * height)), 0.0, 1.0);
    f_texCoord = texCoord;
}

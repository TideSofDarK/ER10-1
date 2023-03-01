#version 410 core

layout (location = 0) in vec2 vertexPositionModelSpace;
layout (location = 1) in vec2 texCoord;

layout (std140) uniform ub_common
{
    vec2 u_screenSize;
    float u_time;
};
uniform int u_mode;
uniform vec4 u_modeControlA;
uniform vec2 u_positionScreenSpace;
uniform vec2 u_sizeScreenSpace;

out vec2 f_texCoord;
out vec4 f_modeControlOutA;

// Mode 0: Normal

// Mode 1: Haze
// u_modeControlA.x: X Intensity
// u_modeControlA.y: Y Intensity
// u_modeControlA.x: Speed
//

// Mode 2: Back Blur
// u_modeControlA.x: Count
// u_modeControlA.y: Speed
// u_modeControlA.x: Step
//
// f_modeControlOutA: Reversed Index
//

void main()
{
    vec2 positionScreenSpace = u_positionScreenSpace;
//    positionScreenSpace = round(positionScreenSpace);

    float ndcX = (((positionScreenSpace.x / u_screenSize.x) * 2.0) - 1.0);
    float ndcY = (((positionScreenSpace.y / u_screenSize.y) * 2.0) - 1.0);

    float width = (u_sizeScreenSpace.x / u_screenSize.x) * 2.0;
    float height = (u_sizeScreenSpace.y / u_screenSize.y) * 2.0;

    // Back Blur
    if (u_mode == 2) {
        f_modeControlOutA.x = ((u_modeControlA.x-1) - gl_InstanceID);
        float from = u_modeControlA.z * f_modeControlOutA.x;
        float to = from + u_modeControlA.z;
        float scale = 1.0 + (mix(from, to, fract(u_time * u_modeControlA.y)));
        ndcX *= scale;
        ndcY *= scale;
        width *= scale;
        height *= scale;
    }

    gl_Position = vec4(ndcX + (vertexPositionModelSpace.x * width), -(ndcY + (vertexPositionModelSpace.y * height)), 0.0, 1.0);

    f_texCoord = texCoord;
}
layout (location = 0) in vec2 a_vertexPositionModelSpace;
layout (location = 1) in vec2 a_texCoord;

layout (std140) uniform ub_common
{
    vec2 u_screenSize;
    float u_time;
    float u_random;
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

// Mode 3: Glow
// u_modeControlA.x: Red
// u_modeControlA.y: Green
// u_modeControlA.x: Blue
// u_modeControlA.w: Intensity
//

// Mode 4: Disintegrate
// u_modeControlA.x: Progress
// u_modeControlA.y:
// u_modeControlA.x:
// u_modeControlA.w:
// u_modeControlB: Noise UVRect
//

void main()
{
    vec2 vertexPositionModelSpace = a_vertexPositionModelSpace;
    vec2 positionScreenSpace = u_positionScreenSpace;

    vec2 ndcSize = vec2((u_sizeScreenSpace.x / u_screenSize.x) * 2.0, (u_sizeScreenSpace.y / u_screenSize.y) * 2.0);
    vec2 ndcOrigin = vec2((((positionScreenSpace.x / u_screenSize.x) * 2.0) - 1.0), (((positionScreenSpace.y / u_screenSize.y) * 2.0) - 1.0));
    vec2 ndcCenter = vec2(ndcOrigin.x + (ndcSize.x / 2.0), ndcOrigin.y + (ndcSize.y / 2.0));

    // Back Blur
    if (u_mode == UBER2D_MODE_BACK_BLUR) {
        f_modeControlOutA.x = ((u_modeControlA.x - 1) - gl_InstanceID);
        float from = u_modeControlA.z * f_modeControlOutA.x;
        float to = from + u_modeControlA.z;
        float scale = 1.0 + (mix(from, to, fract(u_time * u_modeControlA.y)));

        ndcOrigin.x -= ((ndcSize.x * scale) - ndcSize.x) / 2.0;
        ndcOrigin.y -= ((ndcSize.y * scale) - ndcSize.y) / 2.0;
        ndcSize *= scale;
    }

    gl_Position = vec4(
    ndcOrigin.x + (vertexPositionModelSpace.x * ndcSize.x),
    -(ndcOrigin.y + (vertexPositionModelSpace.y * ndcSize.y)),
    0.0,
    1.0);

    f_texCoord = a_texCoord;
}

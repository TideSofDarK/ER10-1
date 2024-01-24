/* Binding Point: 0 */
layout (std140) uniform ub_common
{
    vec2 u_screenSize;
    float u_time;
};

struct TileData
{
    uint flags;
    uint specialFlags;
    uint edgeFlags;
    uint specialEdgeFlags;
};

/* Binding Point: 1 */
layout (std140) uniform ub_map {
    uint width;
    uint height;
    uint povX;
    uint povY;
    TileData[MAX_LEVEL_TILE_COUNT] tile;
} u_map;

uniform int u_mode;
uniform vec2 u_sizeScreenSpace;
uniform sampler2D u_colorTexture;

in vec2 f_texCoord;

out vec4 color;

void main()
{
    vec2 pixelPos = f_texCoord * u_sizeScreenSpace;

    vec2 texCoordNDC = (f_texCoord * 2) - 1.0;

    // Border setup
    vec3 borderColor = vec3(0.5, 0.5, 0.5);
    float borderSize = 2.0;

    // Border mask
    float borderX = clamp(abs((texCoordNDC.x * u_sizeScreenSpace.x))  - (u_sizeScreenSpace.x - borderSize), 0.0, 1.0);
    float borderY = clamp(abs((texCoordNDC.y * u_sizeScreenSpace.y))  - (u_sizeScreenSpace.y - borderSize), 0.0, 1.0);

    if (u_mode == HUD_MODE_BORDER_DASHED) {
        // Border dash
        float borderDashSize = 8.0;
        float borderDashSpeed = u_time * 10.0;
        float borderDashDirectionX = sign(texCoordNDC.x);
        float borderDashDirectionY = sign(texCoordNDC.y);
        borderX = borderX * (round(mod(pixelPos.y + (borderDashSpeed * -borderDashDirectionX), borderDashSize) / borderDashSize));
        borderY = borderY * (round(mod(pixelPos.x + (borderDashSpeed * borderDashDirectionY), borderDashSize) / borderDashSize));

        float borderMask = clamp(borderX + borderY, 0.0, 1.0);

        // Border color
        borderColor = borderMask * mix(vec3(0.4, 0.1, 0.7), vec3(0.5, 0.2, 0.3), (f_texCoord.x + f_texCoord.y) / 2.0);
    }

    if (u_mode == HUD_MODE_BUTTON) {
        float borderMask = clamp(borderX + borderY, 0.0, 1.0);
        borderColor = borderMask * vec3(1.0, 1.0, 1.0);
    }

    if (u_mode == HUD_MODE_MAP) {
        float levelWidth = float(u_map.width);
        float levelHeight = float(u_map.height);
        float tileX = trunc(f_texCoord.x * levelWidth);
        float tileY = trunc(f_texCoord.y * levelHeight);
        int index = int(clamp(tileY * levelWidth + tileX, 0.0, MAX_LEVEL_TILE_COUNT - 1.0));

        float povMask = (1.0 - min(abs(tileX - u_map.povX), 1.0)) * (1.0 - min(abs(tileY - u_map.povY), 1.0));
        vec3 pov = vec3(1.0, 1.0, 1.0);

        float floorTileMask = bitMask(u_map.tile[index].flags, TILE_FLOOR_BIT);
        vec3 floorTile = vec3(0.0, 0.0, 1.0);

        vec3 finalColor = floorTile * floorTileMask;

        finalColor = finalColor * (1.0 - povMask) + (povMask * pov);

        color = vec4(finalColor, 1.0);
    }
    else
    {
        color = vec4(borderColor, 1.0);
    }
}

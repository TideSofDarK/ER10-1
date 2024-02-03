struct TileData
{
    uint flags;
    uint specialFlags;
    uint edgeFlags;
    uint specialEdgeFlags;
};

layout (std140) uniform ub_common
{
    vec4 temp;
};

layout (std140) uniform ub_map
{
    int width;
    int height;
    float povX;
    float povY;
    TileData[MAX_LEVEL_TILE_COUNT] tiles;
} u_map;

uniform int u_mode;
uniform vec4 u_modeControlA;
uniform vec4 u_modeControlB;
uniform vec4 u_uvRect; // minX, minY, maxX, maxY
uniform vec2 u_sizeScreenSpace;
uniform sampler2D u_commonAtlas;
uniform sampler2D u_primaryAtlas;

in vec2 f_texCoord;

out vec4 color;

int calculateTileIndex(float tileX, float tileY, float levelWidth, float levelHeight)
{
    int index = int(clamp(tileY * levelWidth + tileX, 0.0, (levelWidth * levelHeight) - 1.0));
    return index;
}

TileData getTileData(float tileX, float tileY, float levelWidth, float levelHeight)
{
    return u_map.tiles[calculateTileIndex(tileX, tileY, levelWidth, levelHeight)];
}

float calculateValidTileMask(float tileX, float tileY, float levelWidth, float levelHeight)
{
    float validTileMask = max(0.0, sign(tileX + 1));
    validTileMask = min(validTileMask, sign(tileY + 1));
    validTileMask = min(validTileMask, 1.0 - step(levelWidth - tileX, 0.0));
    validTileMask = min(validTileMask, 1.0 - step(levelHeight - tileY, 0.0));
    validTileMask = saturate(validTileMask);
    return validTileMask;
}

vec4 pixelToTile(vec2 texCoord, float tileSize)
{
    vec4 tileInfo;
    tileInfo.x = floor(texCoord.x / tileSize);
    tileInfo.y = floor(texCoord.y / tileSize);
    tileInfo.z = (texCoord.x - (tileInfo.x * tileSize)) / tileSize;
    tileInfo.w = (texCoord.y - (tileInfo.y * tileSize)) / tileSize;
    return tileInfo;
}

void main()
{
    vec2 pixelPos = f_texCoord * u_sizeScreenSpace;

    vec2 texCoordNDC = (f_texCoord * 2) - 1.0;

    vec3 finalColor = vec3(0.5, 0.5, 0.5);
    float borderSize = 2.0;

    float borderX = clamp(abs((texCoordNDC.x * u_sizeScreenSpace.x))  - (u_sizeScreenSpace.x - borderSize), 0.0, 1.0);
    float borderY = clamp(abs((texCoordNDC.y * u_sizeScreenSpace.y))  - (u_sizeScreenSpace.y - borderSize), 0.0, 1.0);

    if (u_mode == HUD_MODE_BORDER_DASHED)
    {
        // Border dash
        float borderDashSize = 8.0;
        float borderDashSpeed = u_globals.time * 10.0;
        float borderDashDirectionX = sign(texCoordNDC.x);
        float borderDashDirectionY = sign(texCoordNDC.y);
        borderX = borderX * (round(mod(pixelPos.y + (borderDashSpeed * -borderDashDirectionX), borderDashSize) / borderDashSize));
        borderY = borderY * (round(mod(pixelPos.x + (borderDashSpeed * borderDashDirectionY), borderDashSize) / borderDashSize));

        float borderMask = clamp(borderX + borderY, 0.0, 1.0);

        // Border color
        finalColor = borderMask * mix(vec3(0.4, 0.1, 0.7), vec3(0.5, 0.2, 0.3), (f_texCoord.x + f_texCoord.y) / 2.0);
    }

    if (u_mode == HUD_MODE_BUTTON)
    {
        float borderMask = clamp(borderX + borderY, 0.0, 1.0);
        finalColor = borderMask * vec3(1.0, 1.0, 1.0);
    }

    color = vec4(finalColor, 1.0);
}

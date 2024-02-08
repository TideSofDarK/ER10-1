struct TileData
{
    uint flags;
    uint specialFlags;
    uint edgeFlags;
    uint specialEdgeFlags;
};

layout(std140) uniform ub_common
{
    Sprite[MAP_ICON_COUNT] icons;
} u_common;

layout(std140) uniform ub_map
{
    int width;
    int height;
    float povX;
    float povY;
    uint povDirection;
    uint paddingA;
    TileData[MAX_LEVEL_TILE_COUNT] tiles;
} u_map;

uniform int u_mode;
uniform vec4 u_modeControlA;
uniform vec4 u_modeControlB;
uniform vec2 u_sizeScreenSpace;
uniform sampler2D u_commonAtlas;

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

float visitedMask(uint flags)
{
    if (u_mode == MAP_MODE_EDITOR)
    {
        return 1.0;
    }
    else
    {
        return bitMask(flags, TILE_SPECIAL_VISITED_BIT);
    }
}

float exploredMask(uint flags)
{
    if (u_mode == MAP_MODE_EDITOR)
    {
        return 1.0;
    }
    else
    {
        return bitMask(flags, TILE_SPECIAL_EXPLORED_BIT);
    }
}

float nonEmptyMask(uint flags)
{
    return step(TILE_FLOOR_BIT, flags);
}

vec4 putIcon(vec2 texCoord, vec2 tileCoords, uint direction, float tileSize, float tileEdgeSize, Sprite sprite)
{
    float rotatedMask = 1.0 - mod(direction + 1, 2);
    vec2 spriteSize = vec2(
            sprite.height * rotatedMask + sprite.width * (1.0 - rotatedMask),
            sprite.width * rotatedMask + sprite.height * (1.0 - rotatedMask)
        );

    vec2 spriteOffset = vec2((spriteSize.x - tileSize - tileEdgeSize) / 2.0, (spriteSize.y - tileSize - tileEdgeSize) / 2.0);
    spriteOffset = floor(spriteOffset);

    float edgeA = tileCoords.x * tileSize - spriteOffset.x;
    float edgeB = edgeA + spriteSize.x;
    float edgeC = tileCoords.y * tileSize - spriteOffset.y;
    float edgeD = edgeC + spriteSize.y;

    float maskA = step(edgeA, texCoord.x);
    float maskB = 1.0 - step(edgeB, texCoord.x);
    float maskC = step(edgeC, texCoord.y);
    float maskD = 1.0 - step(edgeD, texCoord.y);
    float masks = maskA * maskB * maskC * maskD;

    const vec2 atlasPixelReciprocal = vec2(1.0 / float(ATLAS_SIZE));
    vec2 sizeAtlasSpace = vec2(sprite.uvRect.z - sprite.uvRect.x, sprite.uvRect.w - sprite.uvRect.y);

    vec2 spriteUV = vec2(inverseMix(edgeA, edgeB, texCoord.x), inverseMix(edgeC, edgeD, texCoord.y));
    spriteUV = convertUV(spriteUV, sprite.uvRect);
    spriteUV += atlasPixelReciprocal / 2.0;
    spriteUV = rotateUV(spriteUV, (float(direction) * 0.5 * PI), vec2(sprite.uvRect.x + sizeAtlasSpace.x / 2, sprite.uvRect.y + sizeAtlasSpace.y / 2));

    spriteUV *= ATLAS_SIZE;
    vec4 finalColor = texelFetch(u_commonAtlas, ivec2(int(spriteUV.x), int(spriteUV.y)), 0);
    finalColor.a *= masks;
    return finalColor;
}

void main()
{
    vec3 finalColor = mix(vec3(0.0, 0.0, 0.0), vec3(0.03, 0.03, 0.08), 1.0 - f_texCoord.y);

    float levelWidth = float(u_map.width);
    float levelHeight = float(u_map.height);
    float levelTileCount = levelWidth * levelHeight;

    float povX = u_map.povX;
    float povY = u_map.povY;
    vec2 pov = vec2(povX, povY);

    vec2 texCoord = f_texCoord;
    vec2 texCoordOriginal = texCoord;
    texCoord *= u_sizeScreenSpace;
    texCoord = floor(texCoord);

    float tileSize = 0.0;
    float tileCellSize = 0.0;
    float tileEdgeSize = 0.0;
    if (u_mode == MAP_MODE_GAME_NORMAL)
    {
        tileSize = MAP_TILE_SIZE_PIXELS;
        tileCellSize = MAP_TILE_CELL_SIZE_PIXELS;
        tileEdgeSize = MAP_TILE_EDGE_SIZE_PIXELS;

        vec2 centerOffset = pov * tileSize - u_sizeScreenSpace * 0.5 + vec2(tileSize + tileEdgeSize) / 2;

        texCoord += centerOffset;
    }
    else if (u_mode == MAP_MODE_GAME_ISO)
    {
        tileSize = MAP_ISO_TILE_SIZE_PIXELS;

        texCoord = cartesianToIsometric(texCoord);
        texCoordOriginal = cartesianToIsometric(texCoordOriginal);

        vec2 sizeIso = round(cartesianToIsometric(u_sizeScreenSpace / 2.0));

        float centerOffsetX = round(sizeIso.x - (povX + 0.5) * tileSize);
        float centerOffsetY = round(sizeIso.y - (povY + 0.5) * tileSize);

        texCoord -= vec2(centerOffsetX, centerOffsetY);
    }
    else if (u_mode == MAP_MODE_EDITOR)
    {
        tileSize = MAP_TILE_SIZE_PIXELS;
        tileCellSize = MAP_TILE_CELL_SIZE_PIXELS;
        tileEdgeSize = MAP_TILE_EDGE_SIZE_PIXELS;

        float centerOffsetX = round(u_sizeScreenSpace.x * 0.5 - float(u_map.width) / 2.0 * tileSize);
        float centerOffsetY = round(u_sizeScreenSpace.y * 0.5 - float(u_map.height) / 2.0 * tileSize);

        texCoord += vec2(-centerOffsetX, -centerOffsetY);
    }

    texCoord = floor(texCoord);

    float tileSizeReciprocal = 1.0 / tileSize;

    vec4 tileInfo = pixelToTile(texCoord, tileSize);
    TileData tileData = getTileData(tileInfo.x, tileInfo.y, levelWidth, levelHeight);
    float validTileMask = calculateValidTileMask(tileInfo.x, tileInfo.y, levelWidth, levelHeight);
    float levelBoundsMask = step(texCoord.x, levelWidth * tileSize + 1) * step(texCoord.y, levelHeight * tileSize + 1) * step(0.0, texCoord.x) * step(0.0, texCoord.y);

    float tileExploredMask = exploredMask(tileData.specialFlags);

    validTileMask *= tileExploredMask;

    float tileVisitedMask = visitedMask(tileData.specialFlags);

    float tileNonEmptyMask = nonEmptyMask(tileData.flags);

    // Floor
    float holeMask = bitMask(tileData.flags, TILE_HOLE_BIT) * validTileMask;
    float floorTileMask = (bitMask(tileData.flags, TILE_FLOOR_BIT) + holeMask) * validTileMask;

    // float checkerMask = floor(mod(tileX + mod(tileY, 2.0), 2.0));

    vec3 floorTile = vec3(0.0, 0.0, 1.0 - ((1.0 - tileVisitedMask) * 0.6));
    finalColor = overlay(finalColor, floorTile, floorTileMask);

    holeMask *= step(abs(map1to1(tileInfo.z - tileSizeReciprocal / 2.0)), 0.55);
    holeMask *= step(abs(map1to1(tileInfo.w - tileSizeReciprocal / 2.0)), 0.55);
    vec3 holeColor = vec3(0.0, 0.0, 0.1);
    finalColor = overlay(finalColor, holeColor, holeMask);

    // Edges
    float edgeMaskHor = step(abs(mod(texCoord.y, tileSize)), tileEdgeSize - 1);
    float edgeMaskVert = step(abs(mod(texCoord.x, tileSize)), tileEdgeSize - 1);
    float edgeMask = 1.0 - step(saturate(edgeMaskHor + edgeMaskVert), 0.0);
    float bothEdgesMask = ceil(edgeMaskHor * edgeMaskVert);

    vec4 tileInfoNorth = pixelToTile(texCoord + vec2(-tileEdgeSize, -tileEdgeSize), tileSize);
    float validTileMaskNorth = calculateValidTileMask(tileInfoNorth.x, tileInfoNorth.y, levelWidth, levelHeight);
    TileData tileDataNorth = getTileData(tileInfoNorth.x, tileInfoNorth.y, levelWidth, levelHeight);
    float nonEmptyMaskNorth = nonEmptyMask(tileDataNorth.flags);
    float exploredMaskNorth = exploredMask(tileDataNorth.specialFlags) * nonEmptyMaskNorth;

    vec4 tileInfoWest = pixelToTile(texCoord + vec2(-tileEdgeSize, tileEdgeSize), tileSize);
    float validTileMaskWest = calculateValidTileMask(tileInfoWest.x, tileInfoWest.y, levelWidth, levelHeight);
    TileData tileDataWest = getTileData(tileInfoWest.x, tileInfoWest.y, levelWidth, levelHeight);
    float nonEmptyMaskWest = nonEmptyMask(tileDataWest.flags);
    float exploredMaskWest = exploredMask(tileDataWest.specialFlags) * nonEmptyMaskWest;

    vec4 tileInfoEast = pixelToTile(texCoord + vec2(tileEdgeSize, -tileEdgeSize), tileSize);
    float validTileMaskEast = calculateValidTileMask(tileInfoEast.x, tileInfoEast.y, levelWidth, levelHeight);
    TileData tileDataEast = getTileData(tileInfoEast.x, tileInfoEast.y, levelWidth, levelHeight);
    float nonEmptyMaskEast = nonEmptyMask(tileDataEast.flags);
    float exploredMaskEast = exploredMask(tileDataEast.specialFlags) * nonEmptyMaskEast;

    vec4 tileInfoUp = pixelToTile(texCoord + vec2(0, -tileEdgeSize), tileSize);
    float validTileMaskUp = calculateValidTileMask(tileInfoWest.x, tileInfoUp.y, levelWidth, levelHeight);
    TileData tileDataUp = getTileData(tileInfoUp.x, tileInfoUp.y, levelWidth, levelHeight);
    float nonEmptyMaskUp = nonEmptyMask(tileDataUp.flags);
    float exploredMaskUp = exploredMask(tileDataUp.specialFlags);
    validTileMaskUp *= exploredMaskUp;

    vec4 tileInfoLeft = pixelToTile(texCoord + vec2(-tileEdgeSize, 0), tileSize);
    float validTileMaskLeft = calculateValidTileMask(tileInfoWest.x, tileInfoLeft.y, levelWidth, levelHeight);
    TileData tileDataLeft = getTileData(tileInfoLeft.x, tileInfoLeft.y, levelWidth, levelHeight);
    float nonEmptyMaskLeft = nonEmptyMask(tileDataLeft.flags);
    float exploredMaskLeft = exploredMask(tileDataLeft.specialFlags);
    validTileMaskLeft *= exploredMaskLeft;

    // Walls
    float wallMaskHor = bitMask(tileDataNorth.edgeFlags, TILE_EDGE_WALL_SOUTH_BIT) * validTileMaskNorth * edgeMaskHor * saturate(exploredMaskNorth + exploredMaskWest * validTileMaskWest);
    wallMaskHor += bitMask(tileData.edgeFlags, TILE_EDGE_WALL_BIT) *
            (validTileMask + validTileMaskUp * bitMask(tileDataUp.edgeFlags, TILE_EDGE_WALL_WEST_BIT) * bitMask(tileDataUp.edgeFlags, TILE_EDGE_WALL_EAST_BIT)) *
            (tileNonEmptyMask + validTileMask * bitMask(tileData.edgeFlags, TILE_EDGE_WALL_WEST_BIT) * bitMask(tileData.edgeFlags, TILE_EDGE_WALL_EAST_BIT)) *
            edgeMaskHor;

    float wallMaskVert = bitMask(tileDataNorth.edgeFlags, TILE_EDGE_WALL_EAST_BIT) * validTileMaskNorth * edgeMaskVert * saturate(exploredMaskNorth + exploredMaskEast * validTileMaskEast);
    wallMaskVert += bitMask(tileData.edgeFlags, TILE_EDGE_WALL_WEST_BIT) *
            (validTileMask + validTileMaskLeft * bitMask(tileDataLeft.edgeFlags, TILE_EDGE_WALL_BIT) * bitMask(tileDataLeft.edgeFlags, TILE_EDGE_WALL_SOUTH_BIT)) *
            (tileNonEmptyMask + validTileMask * bitMask(tileData.edgeFlags, TILE_EDGE_WALL_BIT) * bitMask(tileData.edgeFlags, TILE_EDGE_WALL_SOUTH_BIT)) *
            edgeMaskVert;

    float wallMaskCorner = 0;
    wallMaskCorner += bitMask(tileDataEast.edgeFlags, TILE_EDGE_WALL_SOUTH_BIT) * exploredMaskEast * validTileMaskEast;
    wallMaskCorner += bitMask(tileDataWest.edgeFlags, TILE_EDGE_WALL_EAST_BIT) * exploredMaskWest * validTileMaskWest;
    wallMaskCorner *= bothEdgesMask;

    float wallMasks = saturate(wallMaskHor + wallMaskVert + wallMaskCorner);
    wallMasks *= edgeMask;
    wallMasks *= levelBoundsMask;

    vec3 wallColor = vec3(1.0, 1.0, 1.0);
    finalColor = mix(finalColor, wallColor, wallMasks);

    // Doors
    // float doorMaskNorth = bitMask(tileDataNorth.edgeFlags, TILE_EDGE_DOOR_SOUTH_BIT) * validDoorHor;
    // doorMaskNorth *= edgeMaskHor * round(abs(map1to1(tileInfo.z - tileSizeReciprocal)) + 0.05);
    // doorMaskNorth = ceil(doorMaskNorth);
    // float doorMaskSouth = bitMask(tileDataSouth.edgeFlags, TILE_EDGE_DOOR_BIT) * validDoorHor;
    // doorMaskSouth *= edgeMaskHor * round(abs(map1to1(tileInfo.z)) + 0.05);
    // doorMaskSouth = ceil(doorMaskSouth);
    // float doorMaskEast = bitMask(tileDataEast.edgeFlags, TILE_EDGE_DOOR_WEST_BIT) * validDoorVert;
    // doorMaskEast *= edgeMaskVert * round(abs(map1to1(tileInfo.w - tileSizeReciprocal)) + 0.05);
    // doorMaskEast = ceil(doorMaskEast);
    // float doorMaskWest = bitMask(tileDataWest.edgeFlags, TILE_EDGE_DOOR_EAST_BIT) * validDoorVert;
    // doorMaskWest *= edgeMaskVert * round(abs(map1to1(tileInfo.w)) + 0.05);
    // doorMaskWest = ceil(doorMaskWest);
    // float doorMaskCorner = (bitMask(tileDataCorner.edgeFlags, TILE_EDGE_DOOR_SOUTH_BIT) + bitMask(tileDataCorner.edgeFlags, TILE_EDGE_DOOR_EAST_BIT));
    // doorMaskCorner *= validTileMaskCorner;
    // doorMaskCorner *= bothEdgesMask;
    // doorMaskCorner = ceil(doorMaskCorner);
    //
    // float doorMasks = saturate(doorMaskNorth + doorMaskSouth + doorMaskEast + doorMaskWest + doorMaskCorner);
    // doorMasks *= edgeMask;
    // doorMasks *= levelBoundsMask;

    // finalColor = overlay(finalColor, wallColor, doorMasks);

    // Grid
    float gridMasks = edgeMask;
    float gridPulseX = saturate(abs((fract(texCoordOriginal.x + (u_globals.time * 0.25)) * 2.0) - 1.0));
    gridPulseX = pow(gridPulseX, 4);
    float gridPulseY = saturate(abs((fract(texCoordOriginal.y + (u_globals.time * 0.15)) * 2.0) - 1.0));
    gridPulseY = pow(gridPulseY, 4);
    float gridPulse = (max(gridPulseX, gridPulseY) * 0.5) + 0.5;
    float tileGrid = floorTileMask; // + wallMasks;
    vec3 grid = mix(vec3(0.05, 0.15, 0.6) * gridPulse, vec3(0.15, 0.25, 0.5) * 1.2, tileGrid * 0.7f);
    // texCoordOriginal *= u_sizeScreenSpace;
    // texCoordOriginal = floor(texCoordOriginal);
    // float gridForceMask = (1.0 - step(0.001, mod(texCoordOriginal.x, u_sizeScreenSpace.x - 1))) + (1.0 - step(0.001, mod(texCoordOriginal.y, u_sizeScreenSpace.y - 1)));
    // gridForceMask = saturate(gridForceMask);
    // gridForceMask = 0.0;
    // finalColor = mix(finalColor, grid, saturate(gridMasks * (1.0 - wallMasks) * (1.0 - doorMasks)));
    finalColor = mix(finalColor, grid, saturate(gridMasks * (1.0 - wallMasks)));

    // Current POV
    vec4 playerIcon = putIcon(texCoord, pov, u_map.povDirection, tileSize, tileEdgeSize, u_common.icons[MAP_ICON_PLAYER]);
    finalColor = overlay(finalColor, playerIcon.rgb, playerIcon.a);

    // float povAnim = (abs(sin((u_globals.time * 10.0))) * 0.6) + 0.4;
    // float povMask = saturate((povTileMask * (1.0 - distance(vec2(map1to1(tileInfo.z - tileSizeReciprocal / 2.0), map1to1(tileInfo.w - tileSizeReciprocal / 2.0)) * 2.0, vec2(0.0, 0.0))))) * povAnim;
    // vec3 povColor = vec3(1.0, 1.0, 1.0);
    // finalColor = overlay(finalColor, povColor, povMask * levelBoundsMask);

    color = vec4(finalColor, 1.0);
}

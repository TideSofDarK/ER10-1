struct STile
{
    uint flags;
    uint specialFlags;
    uint edgeFlags;
    uint specialEdgeFlags;
};

struct STileMasks
{
    float valid;
    float explored;
    float visited;
    float nonEmpty;
    float wallSouth;
    float wallNorth;
    float wallEast;
    float wallWest;
    float doorSouth;
    float doorNorth;
    float doorEast;
    float doorWest;
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
    STile[MAX_LEVEL_TILE_COUNT] tiles;
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

STile getTileData(float tileX, float tileY, float levelWidth, float levelHeight)
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

STileMasks getTileMasks(float tileX, float tileY)
{
    STile tile = getTileData(tileX, tileY, float(u_map.width), float(u_map.height));
    STileMasks tileMasks;
    tileMasks.valid = calculateValidTileMask(tileX, tileY, float(u_map.width), float(u_map.height));
    tileMasks.visited = bitMask(tile.specialFlags, TILE_SPECIAL_VISITED_BIT);
    tileMasks.explored = exploredMask(tile.specialFlags);
    tileMasks.nonEmpty = nonEmptyMask(tile.flags);
    tileMasks.wallNorth = bitMask(tile.edgeFlags, TILE_EDGE_WALL_BIT);
    tileMasks.wallSouth = bitMask(tile.edgeFlags, TILE_EDGE_WALL_SOUTH_BIT);
    tileMasks.wallEast = bitMask(tile.edgeFlags, TILE_EDGE_WALL_EAST_BIT);
    tileMasks.wallWest = bitMask(tile.edgeFlags, TILE_EDGE_WALL_WEST_BIT);
    tileMasks.doorNorth = bitMask(tile.edgeFlags, TILE_EDGE_DOOR_BIT);
    tileMasks.doorSouth = bitMask(tile.edgeFlags, TILE_EDGE_DOOR_SOUTH_BIT);
    tileMasks.doorEast = bitMask(tile.edgeFlags, TILE_EDGE_DOOR_EAST_BIT);
    tileMasks.doorWest = bitMask(tile.edgeFlags, TILE_EDGE_DOOR_WEST_BIT);
    return tileMasks;
}

vec4 putIcon(vec2 texCoord, vec2 tileCoords, uint direction, float tileSize, float tileEdgeSize, Sprite sprite)
{
    float rotatedMask = 1.0 - mod(direction + 1, 2);
    vec2 spriteSize = vec2(
            sprite.height * rotatedMask + sprite.width * (1.0 - rotatedMask),
            sprite.width * rotatedMask + sprite.height * (1.0 - rotatedMask)
        );

    vec2 spriteOffset = vec2((spriteSize.x - tileSize - tileEdgeSize) / 2.0, (spriteSize.y - tileSize - tileEdgeSize) / 2.0);
    // spriteOffset = floor(spriteOffset);

    float edgeA = tileCoords.x * tileSize - spriteOffset.x;
    // edgeA = floor(edgeA);
    float edgeB = edgeA + spriteSize.x;
    float edgeC = tileCoords.y * tileSize - spriteOffset.y;
    // edgeC = floor(edgeC);
    float edgeD = edgeC + spriteSize.y;

    float maskA = step(edgeA, texCoord.x);
    float maskB = 1.0 - step(edgeB, texCoord.x);
    float maskC = step(edgeC, texCoord.y);
    float maskD = 1.0 - step(edgeD, texCoord.y);
    float masks = maskA * maskB * maskC * maskD;

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
        centerOffset = floor(centerOffset);

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

        vec2 centerOffset = round(u_sizeScreenSpace * 0.5 - vec2(u_map.width, u_map.height) / 2.0 * tileSize);

        texCoord += centerOffset;
    }

    float tileSizeReciprocal = 1.0 / tileSize;

    vec4 tileInfo = pixelToTile(texCoord, tileSize);
    STile tileData = getTileData(tileInfo.x, tileInfo.y, levelWidth, levelHeight);
    float validTileMask = calculateValidTileMask(tileInfo.x, tileInfo.y, levelWidth, levelHeight);
    float levelBoundsMask = step(texCoord.x, levelWidth * tileSize + 1) * step(texCoord.y, levelHeight * tileSize + 1) * step(0.0, texCoord.x) * step(0.0, texCoord.y);

    STileMasks tileMasks = getTileMasks(tileInfo.x, tileInfo.y);

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

    vec4 tileInfoNorth = pixelToTile(texCoord + vec2(0, -tileEdgeSize), tileSize);
    STileMasks tileMasksNorth = getTileMasks(tileInfoNorth.x, tileInfoNorth.y);

    vec4 tileInfoWest = pixelToTile(texCoord + vec2(-tileEdgeSize, 0), tileSize);
    STileMasks tileMasksWest = getTileMasks(tileInfoWest.x, tileInfoWest.y);

    vec4 tileInfoNorthWest = pixelToTile(texCoord + vec2(-tileEdgeSize, -tileEdgeSize), tileSize);
    STileMasks tileMasksNorthWest = getTileMasks(tileInfoNorthWest.x, tileInfoNorthWest.y);

    // Walls
    float wallMaskHor = 0.0;
    wallMaskHor += tileMasks.wallNorth;
    wallMaskHor += tileMasksNorth.wallSouth;
    wallMaskHor *= tileMasks.explored * tileMasks.nonEmpty * tileMasks.valid + tileMasksNorth.explored * tileMasksNorth.nonEmpty * tileMasksNorth.valid;
    wallMaskHor *= edgeMaskHor;

    float wallMaskVert = 0.0;
    wallMaskVert += tileMasks.wallWest;
    wallMaskVert += tileMasksWest.wallEast;
    wallMaskVert *= tileMasks.explored * tileMasks.nonEmpty * tileMasks.valid + tileMasksWest.explored * tileMasksWest.nonEmpty * tileMasksWest.valid;
    wallMaskVert *= edgeMaskVert;

    float wallMaskCorner = 0;
    wallMaskCorner += (tileMasksNorthWest.wallSouth + tileMasksNorthWest.wallEast) * tileMasksNorthWest.valid * tileMasksNorthWest.explored * tileMasksNorthWest.nonEmpty;
    wallMaskCorner += tileMasksWest.wallNorth * tileMasksWest.explored * tileMasksWest.valid * tileMasksWest.nonEmpty;
    wallMaskCorner += tileMasksNorth.wallWest * tileMasksNorth.explored * tileMasksNorth.valid * tileMasksNorth.nonEmpty;
    wallMaskCorner *= bothEdgesMask;

    float wallMasks = saturate(wallMaskHor + wallMaskVert + wallMaskCorner);
    wallMasks *= edgeMask;
    wallMasks *= levelBoundsMask;

    vec3 wallColor = vec3(1.0, 1.0, 1.0);
    finalColor = mix(finalColor, wallColor, wallMasks);

    vec2 normalizedEdgeUV = vec2(inverseMix(tileSizeReciprocal * round(tileEdgeSize/2.0), 1.0, tileInfo.z), inverseMix(tileSizeReciprocal * round(tileEdgeSize/2.0), 1.0, tileInfo.w));

    // Doors
    float doorSize = floor(tileCellSize * 0.55);
    float doorBlockMaskNorth = step(normalizedEdgeUV.y * tileSize, floor(tileEdgeSize * 1.5));
    float doorBlockMaskSouth = 1.0 - step(normalizedEdgeUV.y * tileSize, tileSize - round(tileEdgeSize * 1.5));
    float doorBlockMaskWest = step(normalizedEdgeUV.x * tileSize, floor(tileEdgeSize * 1.5));
    float doorBlockMaskEast = 1.0 - step(normalizedEdgeUV.x * tileSize, tileSize - round(tileEdgeSize * 1.5));
    float doorBlockSizeMaskHor = step(abs(normalizedEdgeUV.x * 2.0 - 1.0) * tileSize, tileSize - doorSize);
    float doorBlockSizeMaskVert = step(abs(normalizedEdgeUV.y * 2.0 - 1.0) * tileSize, tileSize - doorSize);
    float doorSizeMaskHor = step(tileSize - doorSize - tileEdgeSize * 1.5, abs(normalizedEdgeUV.x * 2.0 - 1.0) * tileSize);
    float doorSizeMaskVert = step(tileSize - doorSize - tileEdgeSize * 1.5, abs(normalizedEdgeUV.y * 2.0 - 1.0) * tileSize);

    float doorMaskHor = 0.0;
    doorMaskHor += tileMasks.doorNorth;
    doorMaskHor += tileMasksNorth.doorSouth;
    doorMaskHor *= edgeMaskHor;
    doorMaskHor *= doorSizeMaskHor;
    doorMaskHor += tileMasks.doorNorth * doorBlockMaskNorth * doorBlockSizeMaskHor * (1.0 - edgeMaskHor);
    doorMaskHor += tileMasks.doorSouth * doorBlockMaskSouth * doorBlockSizeMaskHor * (1.0 - edgeMaskHor);
    doorMaskHor *= tileMasks.explored * tileMasks.nonEmpty * tileMasks.valid + tileMasksNorth.explored * tileMasksNorth.nonEmpty * tileMasksNorth.valid;

    float doorMaskVert = 0.0;
    doorMaskVert += tileMasks.doorWest;
    doorMaskVert += tileMasksWest.doorEast;
    doorMaskVert *= edgeMaskVert;
    doorMaskVert *= doorSizeMaskVert;
    doorMaskVert += tileMasks.doorWest * doorBlockMaskWest * doorBlockSizeMaskVert * (1.0 - edgeMaskVert);
    doorMaskVert += tileMasksWest.doorEast * doorBlockMaskEast * doorBlockSizeMaskVert * (1.0 - edgeMaskVert);
    doorMaskVert *= tileMasks.explored * tileMasks.nonEmpty * tileMasks.valid + tileMasksWest.explored * tileMasksWest.nonEmpty * tileMasksWest.valid;

    float doorMasks = saturate(doorMaskHor + doorMaskVert);
    // doorMasks *= edgeMask;
    doorMasks *= levelBoundsMask;

    finalColor = overlay(finalColor, wallColor, doorMasks);

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
    finalColor = mix(finalColor, grid, saturate(gridMasks * (1.0 - wallMasks) * (1.0 - doorMasks)));

    // Current POV
    vec4 playerIcon = putIcon(texCoord, pov, u_map.povDirection, tileSize, tileEdgeSize, u_common.icons[MAP_ICON_PLAYER]);
    finalColor = overlay(finalColor, playerIcon.rgb, playerIcon.a);

    // float povAnim = (abs(sin((u_globals.time * 10.0))) * 0.6) + 0.4;
    // float povMask = saturate((povTileMask * (1.0 - distance(vec2(map1to1(tileInfo.z - tileSizeReciprocal / 2.0), map1to1(tileInfo.w - tileSizeReciprocal / 2.0)) * 2.0, vec2(0.0, 0.0))))) * povAnim;
    // vec3 povColor = vec3(1.0, 1.0, 1.0);
    // finalColor = overlay(finalColor, povColor, povMask * levelBoundsMask);

    color = vec4(finalColor, 1.0);
}

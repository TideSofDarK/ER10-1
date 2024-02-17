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
    float floor;
    float hole;
    float wallSouth;
    float wallNorth;
    float wallEast;
    float wallWest;
    float doorSouth;
    float doorNorth;
    float doorEast;
    float doorWest;
};

struct SWorldLayer
{
    vec3 color;
    uint index;
    vec2 position;
    vec2 textureSize;
};

layout(std140) uniform ub_common
{
    vec2 cursor;
    float paddingA;
    float paddingB;
    Sprite[MAP_ICON_COUNT] icons;
} u_common;

layout(std140) uniform ub_editor
{
    vec4 selectedBlock;
    vec2 selectedTile;
    bool enabled;
    bool toggleMode;
    bool blockMode;
    bool paddingA;
    bool paddingB;
    bool paddingC;
} u_editor;

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

layout(std140) uniform ub_world
{
    vec4 position;
    SWorldLayer[WORLD_MAX_LAYERS] layers;
} u_world;

uniform int u_mode;
uniform vec4 u_modeControlA;
uniform vec4 u_modeControlB;
uniform vec2 u_sizeScreenSpace;
uniform sampler2D u_commonAtlas;
uniform sampler2DArray u_worldTextures;

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
    if (u_editor.enabled)
    {
        return 1.0f;
    }
    else
    {
        return bitMask(flags, TILE_SPECIAL_VISITED_BIT);
    }
}

float exploredMask(uint flags)
{
    if (u_editor.enabled)
    {
        return 1.0f;
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
    tileMasks.nonEmpty = nonEmptyMask(tile.flags);
    tileMasks.floor = bitMask(tile.flags, TILE_FLOOR_BIT);
    tileMasks.hole = bitMask(tile.flags, TILE_HOLE_BIT);
    tileMasks.visited = visitedMask(tile.specialFlags);
    tileMasks.explored = exploredMask(tile.specialFlags);
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

vec4 putIconEx(float mask, vec2 uv, Sprite sprite)
{
    vec2 spriteUV = convertUV(uv, sprite.uvRect);
    spriteUV *= ATLAS_SIZE;
    vec4 color = texelFetch(u_commonAtlas, ivec2(int(spriteUV.x), int(spriteUV.y)), 0);
    color.a *= mask;
    return color;
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
    // vec3 finalColor = mix(vec3(0.0, 0.0, 0.0), vec3(0.03, 0.03, 0.08), 1.0 - f_texCoord.y);
    vec3 finalColor = vec3(0.0f);

    float levelWidth = float(u_map.width);
    float levelHeight = float(u_map.height);
    float levelTileCount = levelWidth * levelHeight;

    vec2 pov = vec2(u_map.povX, u_map.povY);

    vec2 texCoord = f_texCoord;
    if (u_mode == MAP_MODE_WORLD_LAYER)
    {
        // texCoord.y = 1.0f - texCoord.y;
        texCoord.x = 1.0f - texCoord.x;
    }

    vec2 halfSizeFloored = floor(u_sizeScreenSpace / 2.0);

    vec2 texCoordOriginal = texCoord;
    texCoord *= u_sizeScreenSpace;

    float tileSize = 0.0f;
    float tileCellSize = 0.0f;
    float tileEdgeSize = 0.0f;

    if (u_mode == MAP_MODE_WORLD)
    {
        vec2 position = round(u_world.position.xy);

        texCoord += floor(u_common.cursor.xy);

        tileSize = MAP_ISO_TILE_SIZE_PIXELS;
        tileCellSize = MAP_ISO_TILE_CELL_SIZE_PIXELS;
        tileEdgeSize = MAP_ISO_TILE_EDGE_SIZE_PIXELS;

        texCoord = cartesianToIsometric(texCoord);
        texCoordOriginal = cartesianToIsometric(texCoordOriginal);

        vec2 sizeIso = cartesianToIsometric(halfSizeFloored);

        vec2 centerOffset = sizeIso - (position + vec2(0.5)) * tileSize;

        texCoord -= centerOffset;
        texCoord = floor(texCoord);

        /* Draw lower levels. */
        // if (false)
        {
            for (int i = WORLD_MAX_LAYERS; i >= 1; i--)
            {
                SWorldLayer layer = u_world.layers[i];
                vec2 pixelCoord = f_texCoord * u_sizeScreenSpace;
                pixelCoord += floor(u_common.cursor.xy);
                pixelCoord -= vec2(0, floor(tileSize / 2.0f) * i);
                pixelCoord = cartesianToIsometric(pixelCoord);
                centerOffset = sizeIso - (layer.position + vec2(0.5)) * tileSize;
                pixelCoord -= centerOffset;
                pixelCoord = vec2(layer.textureSize - pixelCoord);
                vec4 layerColor = texelFetch(u_worldTextures, ivec3(pixelCoord, i), 0);
                float layerMask = withinMask(pixelCoord, layer.textureSize);
                float layerAlpha = ceil(layerColor.r + layerColor.g + layerColor.b);
                finalColor = mix(finalColor, layerColor.rgb * layer.color, saturate(layerMask * layerAlpha));
            }
        }
    }
    else if (u_mode == MAP_MODE_WORLD_LAYER)
    {
        texCoord = floor(texCoord);

        /* Render in isometric scale for further transformation. */
        tileSize = MAP_ISO_TILE_SIZE_PIXELS;
        tileCellSize = MAP_ISO_TILE_CELL_SIZE_PIXELS;
        tileEdgeSize = MAP_ISO_TILE_EDGE_SIZE_PIXELS;
    }
    else
    {
        texCoord += floor(u_common.cursor.xy);
        texCoord = floor(texCoord);

        tileSize = MAP_TILE_SIZE_PIXELS;
        tileCellSize = MAP_TILE_CELL_SIZE_PIXELS;
        tileEdgeSize = MAP_TILE_EDGE_SIZE_PIXELS;

        vec2 fullMapSize = vec2(levelWidth * tileSize + tileEdgeSize, levelHeight * tileSize + tileEdgeSize);

        vec2 centerOffset = vec2(tileSize + tileEdgeSize) / 2;
        if (u_editor.enabled)
        {
            /* Center out if in editor. */
            centerOffset -= halfSizeFloored;
            centerOffset += fullMapSize * 0.5f;
        }
        else
        {
            /* Align to POV if not in editor. */
            centerOffset += pov * tileSize - halfSizeFloored;
        }
        centerOffset = floor(centerOffset);

        texCoord += centerOffset;
    }

    vec3 edgeColor = vec3(1.0f);
    vec3 floorColor = vec3(0.0f, 0.0f, 1.0f);
    vec3 tileGridColor = vec3(0.05f, 0.11f, 0.61f);

    if (u_mode == MAP_MODE_WORLD_LAYER)
    {
        edgeColor = vec3(1.0f);
        floorColor = edgeColor * 0.2f;
        tileGridColor = edgeColor * 0.1f;
    }

    float tileSizeReciprocal = 1.0 / tileSize;
    float levelBoundsMask = step(texCoord.x, levelWidth * tileSize + 1) * step(texCoord.y, levelHeight * tileSize + 1) * step(0.0, texCoord.x) * step(0.0, texCoord.y);

    vec4 tileInfo = pixelToTile(texCoord, tileSize);
    STileMasks tileMasks = getTileMasks(tileInfo.x, tileInfo.y);

    /* Floor */
    float floorTileMask = (tileMasks.hole + tileMasks.floor) * tileMasks.valid * tileMasks.explored;

    // float checkerMask = floor(mod(tileX + mod(tileY, 2.0), 2.0));
    vec3 floorTile = floorColor * (0.5f + tileMasks.visited / 2.0f);
    finalColor = overlay(finalColor, floorTile, floorTileMask);

    /* Edges */
    vec2 normalizedCellUV = vec2(inverseMix(tileSizeReciprocal * round(tileEdgeSize / 2.0), 1.0, tileInfo.z), inverseMix(tileSizeReciprocal * round(tileEdgeSize / 2.0), 1.0, tileInfo.w));
    vec2 mappedCellUV = normalizedCellUV * 2.0 - vec2(1.0);

    float edgeMaskHor = step(abs(mod(texCoord.y, tileSize)), tileEdgeSize - 1);
    float edgeMaskVert = step(abs(mod(texCoord.x, tileSize)), tileEdgeSize - 1);
    float edgeMask = 1.0 - step(saturate(edgeMaskHor + edgeMaskVert), 0.0);
    float bothEdgesMask = ceil(edgeMaskHor * edgeMaskVert);

    vec4 tileInfoNorth = pixelToTile(texCoord + vec2(0, -tileSize / 2.0), tileSize);
    STileMasks tileMasksNorth = getTileMasks(tileInfoNorth.x, tileInfoNorth.y);

    vec4 tileInfoSouth = pixelToTile(texCoord + vec2(0, tileSize / 2.0), tileSize);
    STileMasks tileMasksSouth = getTileMasks(tileInfoSouth.x, tileInfoSouth.y);

    vec4 tileInfoEast = pixelToTile(texCoord + vec2(tileSize / 2.0, 0), tileSize);
    STileMasks tileMasksEast = getTileMasks(tileInfoEast.x, tileInfoEast.y);

    vec4 tileInfoWest = pixelToTile(texCoord + vec2(-tileSize / 2.0, 0), tileSize);
    STileMasks tileMasksWest = getTileMasks(tileInfoWest.x, tileInfoWest.y);

    vec4 tileInfoNorthWest = pixelToTile(texCoord + vec2(-tileSize / 2.0, -tileSize / 2.0), tileSize);
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

    finalColor = mix(finalColor, edgeColor, wallMasks);

    // Doors
    float doorSize = floor(tileCellSize * 0.525);
    float doorBlockMaskNorth = step(normalizedCellUV.y * tileCellSize, tileEdgeSize);
    float doorBlockMaskSouth = step(tileCellSize - tileEdgeSize, normalizedCellUV.y * tileCellSize);
    float doorBlockMaskWest = step(normalizedCellUV.x * tileCellSize, tileEdgeSize);
    float doorBlockMaskEast = step(tileCellSize - tileEdgeSize, normalizedCellUV.x * tileCellSize);
    float doorBlockSizeMaskHor = step(abs(mappedCellUV.x) * tileCellSize, tileCellSize - doorSize);
    float doorBlockSizeMaskVert = step(abs(mappedCellUV.y) * tileCellSize, tileCellSize - doorSize);
    float doorSizeMaskHor = step(tileCellSize - doorSize, abs(mappedCellUV.x) * tileCellSize + tileEdgeSize * 2.0);
    float doorSizeMaskVert = step(tileCellSize - doorSize, abs(mappedCellUV.y) * tileCellSize + tileEdgeSize * 2.0);

    float doorMaskHor = 0.0;
    doorMaskHor += tileMasks.doorNorth;
    doorMaskHor += tileMasksNorth.doorSouth;
    doorMaskHor *= edgeMaskHor;
    doorMaskHor *= doorSizeMaskHor;
    doorMaskHor += tileMasks.doorNorth * doorBlockMaskNorth * doorBlockSizeMaskHor * (1.0 - edgeMaskHor);
    doorMaskHor += tileMasks.doorSouth * doorBlockMaskSouth * doorBlockSizeMaskHor * (1.0 - edgeMaskHor);
    doorMaskHor *= tileMasks.explored * tileMasks.valid * tileMasks.nonEmpty +
            tileMasksNorth.explored * tileMasksNorth.valid * tileMasksNorth.nonEmpty +
            tileMasksSouth.explored * tileMasksSouth.valid * tileMasksSouth.nonEmpty;

    float doorMaskVert = 0.0;
    doorMaskVert += tileMasks.doorWest;
    doorMaskVert += tileMasksWest.doorEast;
    doorMaskVert *= edgeMaskVert;
    doorMaskVert *= doorSizeMaskVert;
    doorMaskVert += tileMasks.doorWest * doorBlockMaskWest * doorBlockSizeMaskVert * (1.0 - edgeMaskVert);
    doorMaskVert += tileMasksWest.doorEast * doorBlockMaskEast * doorBlockSizeMaskVert * (1.0 - edgeMaskVert);
    doorMaskVert *= tileMasks.explored * tileMasks.valid * tileMasks.nonEmpty +
            tileMasksWest.explored * tileMasksWest.valid * tileMasksWest.nonEmpty +
            tileMasksEast.explored * tileMasksEast.valid * tileMasksEast.nonEmpty;

    float doorMaskCorner = 0;
    doorMaskCorner += (tileMasksNorthWest.doorSouth + tileMasksNorthWest.doorEast) * tileMasksNorthWest.valid * tileMasksNorthWest.explored * tileMasksNorthWest.nonEmpty;
    doorMaskCorner += tileMasksWest.doorNorth * tileMasksWest.explored * tileMasksWest.valid * tileMasksWest.nonEmpty;
    doorMaskCorner += tileMasksNorth.doorWest * tileMasksNorth.explored * tileMasksNorth.valid * tileMasksNorth.nonEmpty;
    doorMaskCorner *= bothEdgesMask;

    float doorMasks = saturate(doorMaskHor + doorMaskVert + doorMaskCorner);
    doorMasks *= levelBoundsMask;

    finalColor = overlay(finalColor, edgeColor, doorMasks);

    /* Grid */
    float gridMasks = edgeMask;
    float gridPulseX = saturate(abs((fract(texCoordOriginal.x + (u_globals.time * 0.25)) * 2.0) - 1.0));
    gridPulseX = pow(gridPulseX, 4);
    float gridPulseY = saturate(abs((fract(texCoordOriginal.y + (u_globals.time * 0.15)) * 2.0) - 1.0));
    gridPulseY = pow(gridPulseY, 4);
    float gridPulse = (max(gridPulseX, gridPulseY) * 0.5) + 0.5;
    float tileGrid = floorTileMask; // + wallMasks;
    vec3 grid = mix(vec3(0.05, 0.15, 0.6) * gridPulse, tileGridColor, tileGrid);

    if (u_mode != MAP_MODE_WORLD_LAYER && u_mode != MAP_MODE_WORLD)
    {
        finalColor = mix(finalColor, grid, saturate(gridMasks * (1.0 - wallMasks) * (1.0 - doorMasks)));
    }
    else
    {
        finalColor = mix(finalColor, grid, saturate(gridMasks * (1.0 - wallMasks) * (1.0 - doorMasks) * floorTileMask));
    }

    /* Map Icons */
    vec4 holeColor = putIconEx(tileMasks.hole, normalizedCellUV, u_common.icons[MAP_ICON_HOLE]);
    finalColor = mix(finalColor, holeColor.rgb, tileMasks.valid * tileMasks.explored * holeColor.a * (1.0 - edgeMask));

    /* Current POV */
    if (u_mode == MAP_MODE_NORMAL)
    {
        vec4 playerIcon = putIcon(texCoord, pov, u_map.povDirection, tileSize, tileEdgeSize, u_common.icons[MAP_ICON_PLAYER]);
        finalColor = overlay(finalColor, playerIcon.rgb, playerIcon.a);
    }

    // float povAnim = (abs(sin((u_globals.time * 10.0))) * 0.6) + 0.4;
    // float povMask = saturate((povTileMask * (1.0 - distance(vec2(map1to1(tileInfo.z - tileSizeReciprocal / 2.0), map1to1(tileInfo.w - tileSizeReciprocal / 2.0)) * 2.0, vec2(0.0, 0.0))))) * povAnim;
    // vec3 povColor = vec3(1.0, 1.0, 1.0);
    // finalColor = overlay(finalColor, povColor, povMask * levelBoundsMask);

    color = vec4(finalColor, 1.0f);
}

layout (std140) uniform ub_common
{
    vec2 u_screenSize;
    float u_time;
};
uniform int u_mode;
uniform vec4 u_modeControlA;
uniform vec4 u_modeControlB;
uniform vec4 u_uvRect; // minX, minY, maxX, maxY
uniform vec2 u_sizeScreenSpace;
uniform sampler2D u_commonAtlas;
uniform sampler2D u_primaryAtlas;

in vec2 f_texCoord;
in vec4 f_modeControlOutA;

out vec4 color;

vec2 convertUV(in vec2 normalizedUV, in vec4 uvRect) {
    return vec2(mix(uvRect.x, uvRect.z, normalizedUV.x), mix(uvRect.y, uvRect.w, normalizedUV.y));
}

vec2 tileUV(in vec2 normalizedUV, in vec2 tiling, in vec4 uvRect) {
    return convertUV(vec2(fract(normalizedUV.x * tiling.x), fract(normalizedUV.y * tiling.y)), uvRect);
}

vec2 tileAndOffsetUV(in vec2 normalizedUV, in vec2 tiling, in vec2 offset, in vec4 uvRect) {
    return convertUV(vec2(fract((normalizedUV.x * tiling.x) + offset.x), fract((normalizedUV.y * tiling.y) + offset.y)), uvRect);
}

vec2 clampUV(in vec2 uv) {
    return clamp(uv, vec2(u_uvRect.x, u_uvRect.y), vec2(u_uvRect.z, vec2(u_uvRect.w)));
}

void main()
{
    // Convert UV to atlas space
    vec2 texCoordAtlasSpace = convertUV(f_texCoord, u_uvRect);
    vec2 sizeAtlasSpace = vec2(u_uvRect.z - u_uvRect.x, u_uvRect.w - u_uvRect.y);

    if (u_mode == SIMPLE2D_MODE_HAZE) {
        float xIntensity = u_modeControlA.x;
        float yIntensity = u_modeControlA.y;
        float speed = u_modeControlA.z;
        texCoordAtlasSpace.x += sin(f_texCoord.y * yIntensity * 3.14159 + (u_time * speed)) * sizeAtlasSpace.x * xIntensity;
        texCoordAtlasSpace = clampUV(texCoordAtlasSpace);
    }

    color = texture(u_primaryAtlas, texCoordAtlasSpace);

    if (u_mode == SIMPLE2D_MODE_BACK_BLUR) {
        float step = 1.0 / u_modeControlA.x;
        float from = step * f_modeControlOutA.x;
        float to = from + step;
        color.a *= 1.0 - (mix(from, to, fract(u_time * u_modeControlA.y)));
        color.a *= 0.5;
    }

    if (u_mode == SIMPLE2D_MODE_GLOW) {
        float pixelSizeX = sizeAtlasSpace.x / u_sizeScreenSpace.x;
        float pixelSizeY = sizeAtlasSpace.y / u_sizeScreenSpace.y;

        float outlineMask = round(1.0 - color.a);
        outlineMask *= clamp(
        texture(u_primaryAtlas, clampUV(texCoordAtlasSpace + vec2(pixelSizeX, 0.0))).a +
        texture(u_primaryAtlas, clampUV(texCoordAtlasSpace + vec2(-pixelSizeX, 0.0))).a +
        texture(u_primaryAtlas, clampUV(texCoordAtlasSpace + vec2(0.0, pixelSizeY))).a +
        texture(u_primaryAtlas, clampUV(texCoordAtlasSpace + vec2(0.0, -pixelSizeY))).a,
        0.0, 1.0);

        float pulse = abs((fract(f_texCoord.y + u_time) * 2) - 1.0);
        pulse *= pulse;
        float outlineColorAlpha = outlineMask * (0.4 + mix(0.0, 0.6, pulse));

        color = vec4(mix(color.rgb * round(color.a), vec3(0.2, 0.7, 0.9), outlineColorAlpha), color.a + outlineMask);
    }

    if (u_mode == SIMPLE2D_MODE_DISINTEGRATE) {
        vec2 noiseTexCoordAtlasSpace = tileAndOffsetUV(f_texCoord, vec2(1.0, 1.0), vec2(u_time / 10.0, u_time / 10.0), u_modeControlB);
        float noise = texture(u_commonAtlas, noiseTexCoordAtlasSpace).g;
        float scanlineHeightNormalized = 0.4;
        float progress = fract(u_modeControlA.x);
        color.a -= round((noise * 2.0) - smoothstep(progress, progress + scanlineHeightNormalized, f_texCoord.y));
    }
}
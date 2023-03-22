layout (std140) uniform ub_common
{
    vec2 u_screenSize;
    float u_time;
    float u_random;
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

void main()
{
    // Convert UV to atlas space
    vec2 texCoordAtlasSpace = convertUV(f_texCoord, u_uvRect);
    vec2 sizeAtlasSpace = vec2(u_uvRect.z - u_uvRect.x, u_uvRect.w - u_uvRect.y);

    if (u_mode == UBER2D_MODE_HAZE) {
        float xIntensity = u_modeControlA.x;
        float yIntensity = u_modeControlA.y;
        float speed = u_modeControlA.z;
        texCoordAtlasSpace.x += sin(f_texCoord.y * yIntensity * 3.14159 + (u_time * speed)) * sizeAtlasSpace.x * xIntensity;
        texCoordAtlasSpace = clampUV(texCoordAtlasSpace, u_uvRect);
    }

    color = texture(u_primaryAtlas, texCoordAtlasSpace);

    if (u_mode == UBER2D_MODE_BACK_BLUR) {
        float step = 1.0 / u_modeControlA.x;
        float from = step * f_modeControlOutA.x;
        float to = from + step;
        color.a *= 1.0 - (mix(from, to, fract(u_time * u_modeControlA.y)));
        color.a *= 0.5;
    }

    if (u_mode == UBER2D_MODE_GLOW) {
        float pixelSizeX = sizeAtlasSpace.x / u_sizeScreenSpace.x;
        float pixelSizeY = sizeAtlasSpace.y / u_sizeScreenSpace.y;

        float outlineMask = round(1.0 - color.a);
        outlineMask *= clamp(
            texture(u_primaryAtlas, clampUV(texCoordAtlasSpace + vec2(pixelSizeX, 0.0), u_uvRect)).a +
            texture(u_primaryAtlas, clampUV(texCoordAtlasSpace + vec2(-pixelSizeX, 0.0), u_uvRect)).a +
            texture(u_primaryAtlas, clampUV(texCoordAtlasSpace + vec2(0.0, pixelSizeY), u_uvRect)).a +
            texture(u_primaryAtlas, clampUV(texCoordAtlasSpace + vec2(0.0, -pixelSizeY), u_uvRect)).a,
            0.0, 1.0);

        float pulse = abs((fract(f_texCoord.y + u_time) * 2) - 1.0);
        pulse *= pulse;
        float outlineColorAlpha = outlineMask * (0.4 + mix(0.0, 0.6, pulse));

        color = vec4(mix(color.rgb * round(color.a), vec3(0.2, 0.7, 0.9), outlineColorAlpha), color.a + outlineMask);
    }

    if (u_mode == UBER2D_MODE_DISINTEGRATE) {
        vec2 noiseTexCoordAtlasSpace = tileAndOffsetUV(f_texCoord, vec2(1.0, 1.0), vec2(u_time / 10.0, u_time / 10.0), u_modeControlB);
        float noise = texture(u_commonAtlas, noiseTexCoordAtlasSpace).g;
        float progress = fract(u_modeControlA.x);
        progress = sineIn(progress);
        float progressA = clamp(progress * 2.0, 0.0, 1.0);
        float progressB = clamp((progress * 2.0) - 1.0, 0.0, 1.0);

        float scanlineHeightNormalized = 1.0;
        float scanlineHeight = mix(0.0, scanlineHeightNormalized, clamp(progressA / scanlineHeightNormalized, 0.0, 1.0));
        color.xyz += vec3(round((noise * 2.0) - smoothstep(progressA, progressA + scanlineHeight, f_texCoord.y)));

        float scanlineHeightB = mix(0.0, scanlineHeightNormalized, clamp(progressB / scanlineHeightNormalized, 0.0, 1.0));
        color.a -= color.a * ceil(progressB) * round((noise * 2.0) - smoothstep(progressB, progressB + scanlineHeightB, f_texCoord.y));
    }

    if (u_mode == UBER2D_MODE_DISINTEGRATE_PLASMA) {
        vec2 noiseTexCoordAtlasSpace = tileAndOffsetUV(f_texCoord, vec2(0.65, 0.65), vec2(u_random), u_modeControlB);
        float noise = texture(u_commonAtlas, noiseTexCoordAtlasSpace).b;
        float progress = fract(u_modeControlA.x);
        float mask = round(noise * 2.0 - progress);

        float maskA = round(noise * 2.0 - progress);
        float maskB = round(noise * 2.0 - (progress + 0.075));

        //        color.rgb -= vec3(maskB);
        color.a *= mask;

        color.rgb += (maskA - maskB) * u_modeControlA.yzw;
    }
}
vec2 convertUV(in vec2 normalizedUV, in vec4 uvRect) {
    return vec2(mix(uvRect.x, uvRect.z, normalizedUV.x), mix(uvRect.y, uvRect.w, normalizedUV.y));
}

vec2 tileUV(in vec2 normalizedUV, in vec2 tiling, in vec4 uvRect) {
    return convertUV(vec2(fract(normalizedUV.x * tiling.x), fract(normalizedUV.y * tiling.y)), uvRect);
}

vec2 tileAndOffsetUV(in vec2 normalizedUV, in vec2 tiling, in vec2 offset, in vec4 uvRect) {
    return convertUV(vec2(fract((normalizedUV.x * tiling.x) + offset.x), fract((normalizedUV.y * tiling.y) + offset.y)), uvRect);
}

vec2 clampUV(in vec2 uv, in vec4 uvRect) {
    return clamp(uv, vec2(uvRect.x, uvRect.y), vec2(uvRect.z, vec2(uvRect.w)));
}

#ifndef HALF_PI
#define HALF_PI 1.5707963267948966
#endif

float sineIn(float t) {
    return sin((t - 1.0) * HALF_PI) + 1.0;
}

mat4 translationMatrix(vec3 delta)
{
    return mat4(
    vec4(1.0, 0.0, 0.0, 0.0),
    vec4(0.0, 1.0, 0.0, 0.0),
    vec4(0.0, 0.0, 1.0, 0.0),
    vec4(delta, 1.0));
}

float getFogFactor(float d)
{
    const float FogMax = 2.5;
    const float FogMin = 0.5;

    float fogAmount = (FogMax - sqrt(d)) / (FogMax * FogMin);

    return clamp(fogAmount, 0.0, 1.0);
}
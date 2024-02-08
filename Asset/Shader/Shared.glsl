#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

#ifndef HALF_PI
#define HALF_PI 1.5707963267948966
#endif

const vec2 atlasPixelReciprocal = vec2(1.0 / float(ATLAS_SIZE));

struct Sprite
{
    vec4 uvRect;
    int width;
    int height;
    int paddingA;
    int paddingB;
};

layout(std140) uniform ub_globals
{
    vec2 screenSize;
    float time;
    float random;
} u_globals;

float degToRad(float deg)
{
    return PI * deg / 180.0;
}

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

float saturate(float value) {
    return clamp(value, 0.0, 1.0);
}

vec2 saturate(vec2 value) {
    return clamp(value, 0.0, 1.0);
}

vec3 saturate(vec3 value) {
    return clamp(value, 0.0, 1.0);
}

float map1to1(float value) {
    return value * 2.0 - 1.0;
}

float sineIn(float t) {
    return sin((t - 1.0) * HALF_PI) + 1.0;
}

mat4 identityMatrix()
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(vec3(0.0), 1.0));
}

mat4 translationMatrix(float tX, float tY, float tZ)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(vec3(tX, tY, tZ), 1.0));
}

mat4 translationMatrix(vec3 delta)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(delta, 1.0));
}

mat4 rotationXMatrix(float angle)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, cos(angle), -sin(angle), 0.0),
        vec4(0.0, sin(angle), cos(angle), 0.0),
        vec4(vec3(0.0), 1.0));
}

mat4 rotationYMatrix(float angle)
{
    return mat4(
        vec4(cos(angle), 0.0, sin(angle), 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(-sin(angle), 0.0, cos(angle), 0.0),
        vec4(vec3(0.0), 1.0));
}

mat4 rotationZMatrix(float angle)
{
    return mat4(
        vec4(cos(angle), -sin(angle), 0.0, 0.0),
        vec4(sin(angle), cos(angle), 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(vec3(0.0), 1.0));
}

mat4 scaleMatrix(float sX, float sY, float sZ)
{
    return mat4(
        vec4(sX, 0.0, 0.0, 0.0),
        vec4(0.0, sY, 0.0, 0.0),
        vec4(0.0, 0.0, sZ, 0.0),
        vec4(vec3(0.0), 1.0));
}

const mat2 isometricMat = mat2(vec2(-0.5, 0.5), vec2(-1.0));

vec2 cartesianToIsometric(in vec2 cartesian) {
    return isometricMat * cartesian;
}

float getFogFactor(float d)
{
    const float FogMax = 2.5;
    const float FogMin = 0.5;

    float fogAmount = (FogMax - sqrt(d)) / (FogMax * FogMin);

    return clamp(fogAmount, 0.0, 1.0);
}

float bitMask(uint flags, uint flag)
{
    return float(clamp(flags & flag, 0u, 1u));
}

float overlay(float original, float toAdd, float mask)
{
    mask = saturate(mask);
    return original * (1.0 - mask) + (mask * toAdd);
}

vec3 overlay(vec3 original, vec3 toAdd, float mask)
{
    mask = saturate(mask);
    return original * (1.0 - mask) + (mask * toAdd);
}

float inverseMix(float from, float to, float value) {
    return (value - from) / (to - from);
}

vec2 rotateUV(vec2 uv, float rotation)
{
    float cosAngle = cos(rotation);
    float sinAngle = sin(rotation);
    vec2 p = uv - vec2(0.5);
    return vec2(
        cosAngle * p.x + sinAngle * p.y + 0.5,
        cosAngle * p.y - sinAngle * p.x + 0.5
    );
}

vec2 rotateUV(vec2 uv, float rotation, vec2 mid)
{
    float cosAngle = cos(rotation);
    float sinAngle = sin(rotation);
    vec2 p = uv - mid;
    return vec2(
        cosAngle * p.x + sinAngle * p.y + mid.x,
        cosAngle * p.y - sinAngle * p.x + mid.y
    );
}

vec2 rotateUV(vec2 uv, float rotation, float mid)
{
    float cosAngle = cos(rotation);
    float sinAngle = sin(rotation);
    vec2 p = uv - vec2(mid);
    return vec2(
        cosAngle * p.x + sinAngle * p.y + mid,
        cosAngle * p.y - sinAngle * p.x + mid
    );
}

mat2 rotationMatrix(float angle)
{
    angle *= PI / 180.0;
    float sine = sin(angle), cosine = cos(angle);
    return mat2(cosine, -sine,
        sine, cosine);
}

layout (std140) uniform ub_common
{
    vec2 u_screenSize;
    float u_time;
};
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

    if (u_mode == 0) {
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

    if (u_mode == 1) {
        float borderMask = clamp(borderX + borderY, 0.0, 1.0);
        borderColor = borderMask * vec3(1.0, 1.0, 1.0);
    }

    // Border color
    //    vec3 borderColor = border * vec3(0.5, 0.5, 0.5);
    //    float border = clamp(floor(f_texcoord.x) + 0.1 + floor(f_texcoord.y), 0.0, 1.0);
    color = vec4(borderColor, 1.0);
    //    color = texture(u_colorTexture, f_texcoord);
}

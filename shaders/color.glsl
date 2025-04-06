vec4 unpack_color(uint packed_color)
{
    float r = float((packed_color >> 24u) & 0xFFu) / 255.0;
    float g = float((packed_color >> 16u) & 0xFFu) / 255.0;
    float b = float((packed_color >> 8u) & 0xFFu) / 255.0;
    float a = float(packed_color & 0xFFu) / 255.0;
    return vec4(r, g, b, a);
}

uint pack_color(vec3 color)
{
    uint r = uint(color.r * 255.0) & 0xFFu;
    uint g = uint(color.g * 255.0) & 0xFFu;
    uint b = uint(color.b * 255.0) & 0xFFu;
    uint a = 0xFFu;
    return (r << 24u) | (g << 16u) | (b << 8u) | a;
}
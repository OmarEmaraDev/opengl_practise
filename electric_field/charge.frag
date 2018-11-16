#version 140
#define WIDTH 0.3
#define RADIUS 0.08
in vec2 pos;
out vec4 outColor;
void main()
{
    float len = length(pos);
    float x = max(abs(pos.x) - WIDTH, 0);
    float y = max(abs(pos.y) - WIDTH, 0);
    float sdf = min(length(vec2(pos.x, y)), length(vec2(x, pos.y)));
    float icon = smoothstep(RADIUS, RADIUS - fwidth(sdf), sdf);
    vec3 color = mix(vec3(0.15, 0.75, 0.9), vec3(1, 1, 1), icon);
    outColor = vec4(color, smoothstep(1, 1 - fwidth(len), len));
}

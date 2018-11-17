#version 140
uniform int chargeCount;
uniform vec2 positions[10];
uniform float values[10];
in vec2 pos;
out vec4 outColor;
void main()
{
    float p = 0;
    for (int i = 0; i < chargeCount; i++)
    {
        p += values[i] / distance(pos, positions[i]);
    }
    p /= 15;
    outColor = vec4(p, p, p, 1);
}

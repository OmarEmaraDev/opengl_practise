#version 140
uniform int chargeCount;
uniform vec2 charges[10];
in vec2 pos;
out vec4 outColor;
void main()
{
    float p = 0;
    for (int i = 0; i < chargeCount; i++)
    {
        p += 1 / distance(pos, charges[i]);
    }
    p /= 25;
    outColor = vec4(p, p, p, 1);
}

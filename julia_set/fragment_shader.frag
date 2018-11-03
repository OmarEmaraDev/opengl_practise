#version 140
uniform vec2 mousePos;
in vec2 pos;
out vec4 outColor;
void main()
{
    vec2 c = mousePos;
    float x = pos.x * 1.75;
    float y = pos.y;
    int max_iteration = 1000;
    int i = 0;
    float xtemp;
    while (x*x + y*y < 4 && i < max_iteration) {
        xtemp = x * x - y * y;
        y = 2 * x * y + c.y;
        x = xtemp + c.x;
        i++;
    };
    if (i == max_iteration) {
        outColor = vec4(0, 0, 0, 1);
    } else {
        float f = float(i) / 100;
        outColor = vec4(f, f, f, 1);
    }
}

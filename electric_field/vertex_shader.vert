#version 140
uniform vec4 transformation;
in vec2 position;
out vec2 pos;
void main()
{
    pos = position;
    gl_Position = vec4(position * transformation.xy + transformation.zw, 0.0, 1);
}

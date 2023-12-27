#version 460 core

out vec4 frag_color;

uniform vec4 light_color;

void main()
{
    frag_color = light_color;
}
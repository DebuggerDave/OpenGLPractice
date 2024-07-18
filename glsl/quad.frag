#version 460 core

out vec4 frag_color;

in vec2 tex_coords;

uniform sampler2D quad_tex;

void main()
{
    float color = texture(quad_tex, tex_coords).r;
    frag_color = vec4(vec3(color), 1.0);
}
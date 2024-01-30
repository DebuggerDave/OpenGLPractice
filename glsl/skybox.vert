#version 460 core
layout (location = 0) in vec3 a_pos;

out vec3 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	tex_coords = a_pos;
	gl_Position = projection * view * vec4(a_pos, 1.0f);
}
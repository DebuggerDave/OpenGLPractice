#version 460 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_tex_coord;

out vec2 tex_coord;
out vec4 norm;
flat out vec4 norm_local_space;
out vec4 frag_pos;
out vec4 light_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 light_pos_world_space;

void main()
{
	gl_Position = projection * view * model * vec4(a_pos, 1.0f);
	tex_coord = a_tex_coord;
	norm = normalize(transpose(inverse(view * model)) * vec4(a_norm, 0.0f));
	norm_local_space = normalize(vec4(a_norm, 0.0f));
	frag_pos = view * model * vec4(a_pos, 1.0);
	light_pos = view * light_pos_world_space;
}
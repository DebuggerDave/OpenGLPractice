#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_tex_coord;

out vec2 tex_coord;
out vec3 norm;
flat out vec3 norm_local_space;
out vec3 frag_pos;
out vec3 light_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_pos_world_space;

void main()
{
	gl_Position = projection * view * model * vec4(a_pos, 1.0f);
	tex_coord = a_tex_coord;
	norm = mat3(transpose(inverse(view * model))) * a_norm;
	norm_local_space = a_norm;
	frag_pos = vec3(view * model * vec4(a_pos, 1.0));
	light_pos = vec3(view * vec4(light_pos_world_space, 1.0));
}
#version 460 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_tex_coord;

out vec2 tex_coord;
out vec4 norm;
out vec4 frag_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_mat;

void main()
{
	gl_Position = projection * view * model * vec4(a_pos, 1.0f);
	tex_coord = a_tex_coord;
	norm = vec4(normalize(normal_mat * a_norm), 0.0);
	frag_pos = view * model * vec4(a_pos, 1.0);
}
#version 460 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_tex_coord;
layout (location = 3) in mat4 a_instancing_model;
layout (location = 7) in mat3 a_instancing_normal_matrix;

out vec2 tex_coord;
out vec4 light_space_pos;
out vec4 norm;
out vec4 frag_pos;

uniform mat4 view;
uniform mat4 light_view;
uniform mat4 projection;
uniform mat4 light_projection;

void main()
{
	gl_Position = projection * view * a_instancing_model * vec4(a_pos, 1.0f);
	light_space_pos = light_projection * light_view * a_instancing_model * vec4(a_pos, 1.0f);
	// perspective division
	light_space_pos = light_space_pos /  light_space_pos.w;
	// transform from [-1, 1] to [0, 1]
	light_space_pos = light_space_pos * 0.5 + 0.5;
	tex_coord = a_tex_coord;
	norm = vec4(normalize(a_instancing_normal_matrix * a_norm), 0.0);
	frag_pos = view * a_instancing_model * vec4(a_pos, 1.0);
}
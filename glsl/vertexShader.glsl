#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_tex_coord;

out vec2 tex_coord;
flat out int tex_type;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(a_pos, 1.0f);
	tex_coord = vec2(a_tex_coord.x, a_tex_coord.y);
	if (a_norm.y == 1) {
		tex_type = 0;
	} else if (a_norm.y == 0) {
		tex_type = 1;
	} else if (a_norm.y == -1){
		tex_type = 2;
	}
}
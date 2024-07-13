#version 460 core
layout (location = 0) in vec3 a_pos;
layout (location = 3) in mat4 a_instanced_model;
layout (location = 7) in mat3 a_instancing_normal_mat;

uniform mat4 view;
uniform mat4 projection;

void main()
{

	gl_Position = projection * view * a_instanced_model * vec4(a_pos, 1.0f);
}
#version 460 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;

out vec4 norm;

uniform mat4 view;
uniform mat4 model;
uniform mat3 normal_mat;

void main()
{
    gl_Position = view * model * vec4(a_pos, 1.0);
    norm = vec4(normalize(normal_mat * a_norm), 0.0);
}
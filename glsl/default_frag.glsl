#version 330 core
out vec4 frag_color;

in vec2 tex_coord;
in vec3 norm;
flat in vec3 norm_local_space;
in vec3 frag_pos;
in vec3 light_pos;

// texture sampler
uniform sampler2D top_texture;
uniform sampler2D side_texture;
uniform sampler2D bottom_texture;

uniform vec3 light_color;

void main()
{
	if (norm_local_space.y == 1) {
		frag_color = texture(top_texture, tex_coord);
	} else if (norm_local_space.y == 0) {
		frag_color = texture(side_texture, tex_coord);
	} else if (norm_local_space.y == -1) {
		frag_color = texture(bottom_texture, tex_coord);
	}

	float ambient_strength = 0.2;
	float specular_strength = 0.5;
	vec3 light_dir = normalize(light_pos - frag_pos);
	vec3 reflect_dir = reflect(-light_dir, norm);

	vec3 ambient = ambient_strength * light_color;
	vec3 diffuse = max(dot(norm, light_dir), 0.0) * light_color;

	vec3 specular = pow(max(dot(normalize(-frag_pos), reflect_dir), 0.0), 32) * specular_strength * light_color;
	frag_color = (frag_color * vec4(ambient + diffuse, 1)) + vec4(specular, 1);
}
#version 460 core
out vec4 frag_color;

in vec2 tex_coord;
in vec4 norm;
flat in vec4 norm_local_space;
in vec4 frag_pos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

uniform Material top_material;
uniform Material side_material;
uniform Material bottom_material;

uniform mat3 light_normal_mat;
uniform mat4 view;

void main()
{
	vec4 output_color = vec4(0.0);

	vec4 diffuse_tex = vec4(0.0);
	vec4 specular_tex = vec4(0.0);
	float shininess = 0.0;
	if (norm_local_space.y == 1) {
		diffuse_tex = texture(top_material.diffuse, tex_coord);
		specular_tex = texture(top_material.specular, tex_coord);
		shininess = top_material.shininess;
	} else if (norm_local_space.y == 0) {
		diffuse_tex = texture(side_material.diffuse, tex_coord);
		specular_tex = texture(side_material.specular, tex_coord);
		shininess = side_material.shininess;
	} else if (norm_local_space.y == -1) {
		diffuse_tex = texture(bottom_material.diffuse, tex_coord);
		specular_tex = texture(bottom_material.specular, tex_coord);
		shininess = bottom_material.shininess;
	}

	vec4 invalid = vec4(-1.0);
	for(int i=0; i<NUM_LIGHTS; i++) {
		Light cur_light = light[i];
		if (any(equal(cur_light.ambient, invalid)) || any(equal(cur_light.diffuse, invalid)) || any(equal(cur_light.specular, invalid))) {
			continue;
		}

		vec4 light_dir = vec4(0.0);
		if (cur_light.dir_pos.w == 0.0) {
			light_dir = vec4(light_normal_mat * vec3(cur_light.dir_pos), 0.0);
		}
		else if (cur_light.dir_pos.w == 1.0) {
			vec4 light_pos = view * cur_light.dir_pos;
			light_dir = vec4(normalize(vec3(frag_pos - light_pos)), 0.0);
		}
		vec4 reflect_dir = reflect(light_dir, norm);

		float diffuse_scale = max(dot(norm, -light_dir), 0.0);
		float specular_alignment = max(dot(normalize(-frag_pos), reflect_dir), 0.0);
		float specular_scale = pow(specular_alignment, shininess);

		vec4 ambient = diffuse_tex * cur_light.ambient;
		vec4 diffuse = diffuse_tex * diffuse_scale * cur_light.diffuse;
		vec4 specular = specular_tex * specular_scale * cur_light.specular;

		output_color += ambient + diffuse + specular;
	}

	frag_color = output_color;
}
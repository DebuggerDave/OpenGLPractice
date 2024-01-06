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

	for(int i=0; i<NUM_LIGHTS; i++) {
		Light cur_light = lights[i];

		vec4 light_pos = view * cur_light.pos;
		vec3 light_diff = vec3(frag_pos - light_pos);
		float light_distance = length(light_diff);
		vec4 light_dir = vec4(normalize(light_diff), 0.0);
		vec4 reflect_dir = reflect(light_dir, norm); 

		float attenuation = 1;
		if ((cur_light.constant != 0.0) ||
			((light_distance != 0.0) &&
				(cur_light.linear != 0.0) ||
				(cur_light.quadratic != 0.0))
		) {
			attenuation = 1.0 / (
				cur_light.constant +
				(cur_light.linear * light_distance) +
				(cur_light.quadratic * (light_distance * light_distance)));
		}
		
		float theta = dot(vec3(light_dir), normalize(light_normal_mat * vec3(cur_light.dir)));
		float damping_angle_cosine = cur_light.inner_angle_cosine - cur_light.outer_angle_cosine;
		float eye_to_outer_cone_angle_cosine = theta - cur_light.outer_angle_cosine;
		float intensity = clamp((eye_to_outer_cone_angle_cosine / damping_angle_cosine), 0.0, 1.0);

		float diffuse_scale = max(dot(norm, -light_dir), 0.0);
		float specular_alignment = max(dot(normalize(-vec3(frag_pos)), vec3(reflect_dir)), 0.0);
		float specular_scale = pow(specular_alignment, shininess);

		vec4 ambient = diffuse_tex * cur_light.ambient;
		vec4 diffuse = diffuse_tex * diffuse_scale * cur_light.diffuse;
		vec4 specular = specular_tex * specular_scale * cur_light.specular;

		output_color += (ambient + diffuse + specular) * attenuation * intensity;
	}

	for(int i=0; i<NUM_DIRECTIONAL_LIGHTS; i++) {
		Directional_Light cur_light = directional_lights[i];

		vec4 light_dir = vec4(normalize(light_normal_mat * vec3(cur_light.dir)), 0.0);
		vec4 reflect_dir = reflect(light_dir, norm); 

		float diffuse_scale = max(dot(norm, -light_dir), 0.0);
		float specular_alignment = max(dot(normalize(-frag_pos), reflect_dir), 0.0);
		float specular_scale = pow(specular_alignment, shininess);

		vec4 ambient = diffuse_tex * cur_light.ambient;
		vec4 diffuse = diffuse_tex * diffuse_scale * cur_light.diffuse;
		vec4 specular = specular_tex * specular_scale * cur_light.specular;

		output_color += (ambient + diffuse + specular);
	}

	frag_color = output_color;
}
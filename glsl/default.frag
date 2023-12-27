#version 460 core
out vec4 frag_color;

in vec2 tex_coord;
in vec4 norm;
flat in vec4 norm_local_space;
in vec4 frag_pos;
in vec4 light_pos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

uniform Material top_material;
uniform Material side_material;
uniform Material bottom_material;

void main()
{
	for(int i=0; i<light.length(); i++) {
		Light cur_light = light[i];
		vec4 light_dir = normalize(frag_pos - light_pos);
		vec4 reflect_dir = -reflect(light_dir, norm);

		float diffuse_scale = max(dot(norm, -light_dir), 0.0);
		float specular_scale_non_exponent = max(dot(normalize(-frag_pos), reflect_dir), 0.0);
		vec4 ambient =  vec4(0.0);
		vec4 diffuse =  vec4(0.0);
		vec4 specular_tex = vec4(0.0);
		float specular_scale = 0.0;

		if (norm_local_space.y == 1) {
			ambient = texture(top_material.diffuse, tex_coord) * cur_light.ambient;
			diffuse = diffuse_scale * texture(top_material.diffuse, tex_coord) * cur_light.diffuse;
			specular_tex = texture(top_material.specular, tex_coord);
			specular_scale = pow(specular_scale_non_exponent, top_material.shininess);
		} else if (norm_local_space.y == 0) {
			ambient = texture(side_material.diffuse, tex_coord) * cur_light.ambient;
			diffuse = diffuse_scale * texture(side_material.diffuse, tex_coord) * cur_light.diffuse;
			specular_tex = texture(side_material.specular, tex_coord);
			specular_scale = pow(specular_scale_non_exponent, side_material.shininess);
		} else if (norm_local_space.y == -1) {
			ambient = texture(bottom_material.diffuse, tex_coord) * cur_light.ambient;
			diffuse = diffuse_scale * texture(bottom_material.diffuse, tex_coord) * cur_light.diffuse;
			specular_tex = texture(bottom_material.specular, tex_coord);
			specular_scale = pow(specular_scale_non_exponent, bottom_material.shininess);
		}

		vec4 specular = specular_scale * specular_tex * cur_light.specular;

		frag_color += ambient + diffuse + specular;
	}
}
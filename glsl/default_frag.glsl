#version 330 core
out vec4 frag_color;

in vec2 tex_coord;
in vec3 norm;
flat in vec3 norm_local_space;
in vec3 frag_pos;
in vec3 light_pos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material top_material;
uniform Material side_material;
uniform Material bottom_material;
uniform Light light;

void main()
{
	vec3 light_dir = normalize(light_pos - frag_pos);
	vec3 reflect_dir = reflect(-light_dir, norm);

	float diffuse_scale = max(dot(norm, light_dir), 0.0);
	float specular_scale_non_exponent = max(dot(normalize(-frag_pos), reflect_dir), 0.0);
	vec3 ambient =  vec3(0.0, 0.0, 0.0);
	vec3 diffuse =  vec3(0.0, 0.0, 0.0);
	vec3 specular_tex = vec3(0.0, 0.0, 0.0);
	float specular_scale = 0.0;

	if (norm_local_space.y == 1) {
		ambient = vec3(texture(top_material.diffuse, tex_coord)) * light.ambient;
		diffuse = diffuse_scale * vec3(texture(top_material.diffuse, tex_coord)) * light.diffuse;
		specular_tex = vec3(texture(top_material.specular, tex_coord));
		specular_scale = pow(specular_scale_non_exponent, top_material.shininess);
	} else if (norm_local_space.y == 0) {
		ambient = vec3(texture(side_material.diffuse, tex_coord)) * light.ambient;
		diffuse = diffuse_scale * vec3(texture(side_material.diffuse, tex_coord)) * light.diffuse;
		specular_tex = vec3(texture(side_material.specular, tex_coord));
		specular_scale = pow(specular_scale_non_exponent, side_material.shininess);
	} else if (norm_local_space.y == -1) {
		ambient = vec3(texture(bottom_material.diffuse, tex_coord)) * light.ambient;
		diffuse = diffuse_scale * vec3(texture(bottom_material.diffuse, tex_coord)) * light.diffuse;
		specular_tex = vec3(texture(bottom_material.specular, tex_coord));
		specular_scale = pow(specular_scale_non_exponent, bottom_material.shininess);
	}

	vec3 specular = specular_scale * specular_tex * light.specular;

	frag_color = vec4(ambient + diffuse + specular, 1);
}
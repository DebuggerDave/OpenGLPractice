#version 460 core

out vec4 frag_color;

in vec2 tex_coord;
in vec4 light_space_pos;
in vec4 norm;
in vec4 frag_pos;

struct Material {
	sampler2D texture_diffuse0;
	sampler2D texture_specular0;
	sampler2D texture_normal0;
	float shininess;
};

uniform sampler2D depth_map;
uniform Material material;
uniform mat3 light_normal_mat;
uniform mat4 view;

float calc_attenuation(float light_distance, float constant, float linear, float quadratic);
float calc_spotlight_intensity(vec4 frag_dir, vec4 light_dir, float inner_angle_cosine, float outer_angle_cosine);
float calc_spotlight_intensity(vec3 frag_dir, vec3 light_dir, float inner_angle_cosine, float outer_angle_cosine);
vec4 calc_light(vec4 normal, vec4 light_pos, vec4 frag_pos, float shininess, vec4 diffuse_tex, vec4 specular_tex, vec4 ambient_light, vec4 diffuse_light, vec4 specular_light);
vec4 calc_light(vec3 normal, vec3 light_pos, vec3 frag_pos, float shininess, vec4 diffuse_tex, vec4 specular_tex, vec4 ambient_light, vec4 diffuse_light, vec4 specular_light);
vec4 better_normalize(vec4 in_vec);
vec3 better_normalize(vec3 in_vec);
bool fuzzy_equal(float one, float two);
float unfuzzy(float val);
float better_dot(vec3 one, vec3 two);

const float eps = 0.00001;
const bool average_shadows = true;

void main()
{
	vec4 diffuse_tex = texture(material.texture_diffuse0, tex_coord);
	vec4 specular_tex = texture(material.texture_specular0, tex_coord);
	vec4 normal_tex = texture(material.texture_normal0, tex_coord);
	float shininess = material.shininess;

	vec4 output_color = vec4(0.0);
	for(int i=0; i<NUM_POINT_LIGHTS; i++) {
		PointLight cur_light = point_lights[i];

		vec4 light_pos = view * cur_light.pos;
		vec4 light_to_frag_dir = better_normalize(frag_pos - light_pos);
		float light_distance = distance(frag_pos, light_pos);

		float attenuation = calc_attenuation(light_distance, cur_light.attenuation.constant, cur_light.attenuation.linear, cur_light.attenuation.quadratic);
		vec4 light_calc = calc_light(norm, light_to_frag_dir, frag_pos, shininess, diffuse_tex, specular_tex, cur_light.color.ambient, cur_light.color.diffuse, cur_light.color.specular);

		output_color += light_calc * attenuation;
	}

	for (int i=0; i<NUM_SPOT_LIGHTS; i++) {
		SpotLight cur_light = spot_lights[i];

		vec4 light_dir = vec4(light_normal_mat * cur_light.dir.xyz, 0.0);
		light_dir = better_normalize(light_dir);
		vec4 light_pos = view * cur_light.pos;
		vec4 light_to_frag_dir = better_normalize(frag_pos - light_pos);
		float light_distance = distance(frag_pos, light_pos);
		
		float spotlight = calc_spotlight_intensity(light_to_frag_dir, light_dir, cur_light.inner_angle_cosine, cur_light.outer_angle_cosine);
		float attenuation = calc_attenuation(light_distance, cur_light.attenuation.constant, cur_light.attenuation.linear, cur_light.attenuation.quadratic);
		vec4 light_calc = calc_light(norm, light_to_frag_dir, frag_pos, shininess, diffuse_tex, specular_tex, cur_light.color.ambient, cur_light.color.diffuse, cur_light.color.specular);

		output_color += light_calc * attenuation * spotlight;
	}

	for(int i=0; i<NUM_DIRECTIONAL_LIGHTS; i++) {
		DirectionalLight cur_light = directional_lights[i];

		vec4 light_dir = vec4(light_normal_mat * cur_light.dir.xyz, 0.0);
		light_dir = better_normalize(light_dir);
		output_color += calc_light(norm, light_dir, frag_pos, shininess, diffuse_tex, specular_tex, cur_light.color.ambient, cur_light.color.diffuse, cur_light.color.specular);
	}

	frag_color = output_color;
}

float calc_attenuation(float light_distance, float constant, float linear, float quadratic) {
	float attenuation = 1;
	if ((constant != 0.0) ||
		((light_distance != 0.0) &&
			((linear != 0.0) ||
			(quadratic != 0.0)))
	) {
		attenuation = 1.0 / (
			constant +
			(linear * light_distance) +
			(quadratic * (light_distance * light_distance)));
	}

	return attenuation;
}

float calc_spotlight_intensity(vec4 light_to_frag_dir, vec4 light_dir, float inner_angle_cosine, float outer_angle_cosine) {
	return calc_spotlight_intensity(light_to_frag_dir.xyz, light_dir.xyz, inner_angle_cosine, outer_angle_cosine);
}

float calc_spotlight_intensity(vec3 light_to_frag_dir, vec3 light_dir, float inner_angle_cosine, float outer_angle_cosine) {
	if (all(equal(light_dir, vec3(0.0)))) {
		return 1.0;
	}

	float theta = dot(light_to_frag_dir, light_dir);
	float damping_angle_cosine = inner_angle_cosine - outer_angle_cosine;
	float eye_to_outer_cone_angle_cosine = theta - outer_angle_cosine;
	return clamp((eye_to_outer_cone_angle_cosine / damping_angle_cosine), 0.0, 1.0);
}

vec4 calc_light(vec4 normal, vec4 light_to_frag_dir, vec4 frag_pos, float shininess, vec4 diffuse_tex, vec4 specular_tex, vec4 ambient_light, vec4 diffuse_light, vec4 specular_light) {
	return calc_light(normal.xyz, light_to_frag_dir.xyz, frag_pos.xyz, shininess, diffuse_tex, specular_tex, ambient_light, diffuse_light, specular_light);
}

vec4 calc_light(vec3 normal, vec3 light_to_frag_dir, vec3 frag_pos, float shininess, vec4 diffuse_tex, vec4 specular_tex, vec4 ambient_light, vec4 diffuse_light, vec4 specular_light) {
	vec3 view_light_half_point = -better_normalize(light_to_frag_dir + frag_pos);
	float diffuse_scale = max(better_dot(normal, -light_to_frag_dir), 0.0);
	float specular_alignment = max(better_dot(view_light_half_point, normal), 0.0);
	float specular_scale = pow(specular_alignment, shininess);
	// make sure specular only affects surfaces with nonzero diffuse
	specular_scale *= ceil(diffuse_scale);

	float shadow_average = 0.0;
	vec2 texelSize = 1.0 / textureSize(depth_map, 0);
	float current_depth = light_space_pos.z;
	// send values greater than 1.0 to 0.0
	current_depth = clamp(current_depth, 0.0, 1.0);
	// iterate over a 3x3 grid around the corresponding texel
	if (average_shadows) {
		for (int x = -1; x <= 1; ++x) {
			for (int y = -1; y <= 1; ++y) {
				float texel = texture(depth_map, light_space_pos.xy + vec2(x, y) * texelSize).r;
				shadow_average += (current_depth > texel) ? 1.0 : 0.0;
			}
		}
		shadow_average /= 9.0;
	} else {
		float texel = texture(depth_map, light_space_pos.xy).r;
		shadow_average = (current_depth > texel) ? 1.0 : 0.0;
	}

	vec4 ambient = diffuse_tex * ambient_light;
	vec4 diffuse = diffuse_tex * diffuse_scale * diffuse_light;
	vec4 specular = specular_tex * specular_scale * specular_light;

	return ambient + ((diffuse + specular) * (1 - shadow_average));
}

vec4 better_normalize(vec4 in_vec) {
	return vec4(better_normalize(in_vec.xyz), 0.0);
}

vec3 better_normalize(vec3 in_vec) {
	vec3 zero = vec3(0.0);
	if (all(equal(in_vec, zero))) {
		return zero;
	} else if (any(not(equal(in_vec, zero)))) {
		return normalize(in_vec);
	}

}

bool fuzzy_equal(float one, float two) {
	return (abs(one - two) < eps);
}

float unfuzzy(float val) {
	if (fuzzy_equal(val, 0.0)) {
		return 0.0;
	} else {
		return val;
	}
}

float better_dot(vec3 one, vec3 two) {
	float res = dot(one, two);
	return unfuzzy(res);
}

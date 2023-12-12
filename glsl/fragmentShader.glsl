#version 330 core
out vec4 frag_color;

in vec2 tex_coord;
flat in int tex_type;

// texture sampler
uniform sampler2D top_texture;
uniform sampler2D side_texture;
uniform sampler2D bottom_texture;

void main()
{
	if (tex_type == 0) {
		frag_color = texture(top_texture, tex_coord);
	} else if (tex_type == 1) {
		frag_color = texture(side_texture, tex_coord);
	} else if (tex_type == 2){
		frag_color = texture(bottom_texture, tex_coord);
	}
}
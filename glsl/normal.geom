#version 460 core
layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in vec4[] norm;

uniform mat4 projection;

const float MAG = 0.1;
const float NUM_VERT = 3;

void main()
{
	vec4 pos_sum = vec4(0.0);
	vec4 norm_sum = vec4(0.0);
	for (int i=0; i<NUM_VERT; i++) {
		pos_sum += gl_in[i].gl_Position;
		norm_sum += norm[i];
	}
	vec4 avg_pos = pos_sum / NUM_VERT;
	vec4 avg_norm = norm_sum / NUM_VERT;

	gl_Position = projection * avg_pos;
	EmitVertex();

	gl_Position = projection * (avg_pos + (avg_norm * MAG));
	EmitVertex();

	EndPrimitive();
}
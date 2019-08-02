#version 330 core
in vec3 color;
in vec2 texCoord;
out vec4 FragColor;
	
uniform sampler2D tex1;
uniform sampler2D tex2;

void main()
{
	FragColor = mix(texture(tex1, texCoord), texture(tex2, texCoord), 2.0) * vec4(color, 1.0);
}
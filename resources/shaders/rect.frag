#version 330

uniform vec4 color_modifier;

out vec4 out_color;

void main()
{
	out_color = vec4(1, 1, 1, 1) * color_modifier;
}

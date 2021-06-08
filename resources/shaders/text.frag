#version 330

in vec2 texture_coord;

uniform sampler2D texture;
uniform vec4 color_modifier;

out vec4 out_color;

void main()
{
	out_color = vec4(1, 1, 1, texture2D(texture, texture_coord).r) * color_modifier;
}

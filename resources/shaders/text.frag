#version 330

in vec2 texture_coord;

uniform sampler2D texture;
uniform vec4 color_modifier;

out vec4 out_color;

void main()
{
	// vec2 fragment_point_size = 1.0 / viewport_size;
	// out_color = vec4(1, 1, 1, texture2D(texture, texture_coord + (vec2(0.5, 0.5) * fragmentSize)).r) * color_modifier;
	out_color = vec4(1, 1, 1, texture2D(texture, texture_coord).r) * color_modifier;
}

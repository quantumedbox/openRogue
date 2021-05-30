#version 300

in vec2 texture_coord;

uniform sampler2D texture;

out vec4 out_color;

void main()
{
	out_color = vec4(1, 1, 1, texture2D(texture, texture_coord).r);
}

// in vec2 texture_coord;

// uniform sampler2D texture;
// uniform vec4 color;

// out vec4 out_color;

// void main()
// {
// 	out_color = vec4(1, 1, 1, texture2D(texture, texture_coord).r) * color;
// }

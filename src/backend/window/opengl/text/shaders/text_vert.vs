#version 300

uniform vec2 start_pos;
uniform float width;

layout(location = 0) in vec2 in_texture_coord;
layout(location = 1) in uint index;

out vec2 texture_coord;

void main()
{
	gl_Position = vec4(start_pos.x + width*index, start_pos.y, 0, 1);
	texture_coord = in_texture_coord;
}

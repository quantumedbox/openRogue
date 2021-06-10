#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

// uniform mat4 projection;
// uniform vec2 viewport_size;

out vec2 texture_coord;

void main()
{
	gl_Position = /* projection * */ vec4(pos, 0.0, 1.0);
	texture_coord = uv;
}

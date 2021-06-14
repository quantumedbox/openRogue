#version 330

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

// uniform mat4 projection;
// uniform vec2 viewport_size;

out vec2 texture_coord;

void main()
{
	// Shifting is used for aligning glyphs
	gl_Position = vec4(pos /* / (viewport_size / 2) - 1.0 + (1.0 / viewport_size) */, 0.0, 1.0);
	texture_coord = uv;
}

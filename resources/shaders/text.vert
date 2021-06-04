#version 420

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

uniform mat4 projection;
uniform vec2 viewport_size;

out vec2 texture_coord;

void main()
{
	// float x_pos = ((pos.x - (viewport_size.x/2)) / (viewport_size.x/2));
	// float y_pos = ((viewport_size.y/2 - pos.y) / (viewport_size.y/2));
	// float y_pos = ((pos.y - (viewport_size.y/2)) / (viewport_size.y/2));
	// gl_Position = projection * vec4(x_pos, y_pos, 0.0, 1.0);
	gl_Position = projection * vec4(pos, 0.0, 1.0);
	texture_coord = uv;
}

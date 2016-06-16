#version 330

// Data from vertex shader
uniform vec3 color;

// Output data from fragment shader
layout(location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(color, 1);
}

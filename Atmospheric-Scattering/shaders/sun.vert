#version 400 core

in vec3 vPosition;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{

	mat4 modelView = view * model;
	gl_Position = projection * modelView * vec4(vPosition, 1.0);
}
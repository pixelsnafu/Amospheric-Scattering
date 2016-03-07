#version 400 core

uniform int isSun;

out vec4 frag_color;

void main()
{
	if (isSun == 1)
		frag_color = vec4(1.0, 0.941, 0.898, 1.0);
	else
		frag_color = vec4(0.0, 0.0, 0.0, 1.0);
}
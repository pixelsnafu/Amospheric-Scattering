#version 400 core

uniform int isSun;

void main()
{
	if (isSun == 1)
		gl_FragColor = vec4(1.0, 0.941, 0.898, 1.0);
	else
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
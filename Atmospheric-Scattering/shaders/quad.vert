#version 400 core

in vec3 vPosition;
in vec2 vTexCoord;

noperspective out vec2 uv;

void main(){
	uv = vTexCoord;
	gl_Position = vec4(vPosition, 1.0);
}
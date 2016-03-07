#version 450 core

in vec3 texCoords;
out vec4 frag_color;

uniform samplerCube skybox;

void main(){
	frag_color = texture(skybox, texCoords);
}
#version 450 core
in vec3 vPosition;

uniform mat4 view;
uniform mat4 projection;

out vec3 texCoords;

void main(){
	vec4 pos = projection * view * vec4(vPosition, 1.0);
	/*
	The z component of the output position should be equal to its w component which will 
	result in a z component that is always equal to 1.0, because when the perspective division
	is applied its z component translates to w / w = 1.0. This way, the depth buffer is tricked
	into believing that the skybox has the maximum depth value of 1.0 and it fails the depth 
	test wherever there's a different object in front of it.
	*/
	gl_Position = pos.xyww;
	texCoords = vPosition;
}
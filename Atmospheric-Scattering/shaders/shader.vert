#version 400 core

in vec3 vPosition;
in vec2 vTexCoord;
in vec3 vNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec4 lightPosition;

out vec2 texCoord;

out vec3 lPos;
out vec3 vPos;
out vec3 vNorm;

void main(){

	mat4 modelView = view * model;
	
	//lighting calculation
	vec4 vertexInEye = modelView * vec4(vPosition, 1.0);
	vec4 lightInEye = view * lightPosition;
	vec4 normalInEye = normalize(modelView * vec4(vNormal, 0.0));

					
    lPos = lightInEye.xyz;
    vPos = vertexInEye.xyz;
    vNorm = normalInEye.xyz;

	texCoord = vTexCoord;

	gl_Position = projection * modelView * vec4(vPosition, 1.0);
}
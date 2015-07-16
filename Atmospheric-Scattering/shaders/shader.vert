#version 400 core

in vec3 vPosition;
in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vTangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec4 lightPosition;

out vec2 texCoord;

out vec3 lightVec;
out vec3 eyeVec;
out vec3 halfVec;

out vec3 vPos;

void main(){

	mat4 modelView = view * model;
	mat4 normalMatrix = transpose(inverse(modelView));

	vec3 n = normalize ( ( normalMatrix * vec4( vNormal, 0.0 ) ).xyz );
	vec3 t = normalize ( ( normalMatrix * vec4( vTangent, 0.0 ) ).xyz );
	vec3 b = cross (n, t);
	
	vec4 vertexInEye = modelView * vec4(vPosition, 1.0);
    vPos = vertexInEye.xyz;

	vec3 lightDir = normalize( lightPosition.xyz - vertexInEye.xyz );
	vec3 temp;
	temp.x = dot (lightDir, t);
	temp.y = dot (lightDir, b);
	temp.z = dot (lightDir, n);
	lightVec = normalize(temp);

	temp.x = dot (vertexInEye.xyz, t);
	temp.y = dot (vertexInEye.xyz, b);
	temp.z = dot (vertexInEye.xyz, n);

	eyeVec = normalize(temp);

	vertexInEye = normalize(vertexInEye);
	vec3 halfVector = normalize(vertexInEye.xyz + lightDir);
	temp.x = dot (halfVector, t);
	temp.y = dot (halfVector, b);
	temp.z = dot (halfVector, n);

	halfVec = temp ; 

	texCoord = vTexCoord;

	gl_Position = projection * modelView * vec4(vPosition, 1.0);
}
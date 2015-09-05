#version 400 core

uniform vec4 lightColor;
uniform vec4 diffuseColor;
//uniform sampler2D mySampler;
//uniform sampler2D night;
uniform sampler2D clouds;
//uniform sampler2D specMap;
uniform sampler2D cloudBumpMap;

in vec3 vPos;

in vec3 lightVec;
in vec3 eyeVec;
in vec3 halfVec;

in vec2 texCoord;

out vec4 frag_color;	


void main(){
	
	vec3 normal = 2.0 * texture(cloudBumpMap, texCoord).rgb - 1.0;
	//normal.z = 1 - normal.x * normal.x - normal.y * normal.y;
	normal = normalize ( normal );

	vec3 L = -lightVec;
    vec3 N = normal;
	float diffuse = max(dot(N, L), 0.0);

	
	
	vec3 cloud_color			=	texture2D( clouds, texCoord).rgb;
	vec3 day_color				=	cloud_color * diffuse;
	vec3 night_color			=	cloud_color * 0.2;
	
	
	vec3 color = day_color;
	if(dot(N, L) < 0.1)
		color = mix(night_color, day_color, (diffuse + 0.1) * 5.0);
	frag_color = vec4(color, 1.0);
	
	//frag_color = vec4(day_color, 1.0);
}
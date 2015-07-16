#version 400 core

uniform vec4 lightColor;
uniform vec4 diffuseColor;
uniform sampler2D mySampler;
uniform sampler2D night;
uniform sampler2D clouds;
uniform sampler2D specMap;
uniform sampler2D bumpMap;

in vec3 vPos;

in vec3 lightVec;
in vec3 eyeVec;
in vec3 halfVec;

in vec2 texCoord;

out vec4 frag_color;	

void main(){

	vec3 normal = 2.0 * texture(bumpMap, texCoord).rgb - 1.0;
	//normal.z = 1 - normal.x * normal.x - normal.y * normal.y;
	normal = normalize ( normal );
	
	vec4 spec = vec4(1.0, 0.941, 0.898, 1.0);
	vec4 specMapColor = texture2D(specMap, texCoord);

	vec3 L = lightVec;
	vec3 N = normal;
    vec3 Emissive = normalize(-vPos);
    vec3 R = reflect(-L, N);
    float dotProd = max(dot(R, Emissive), 0.0);
    vec4 specColor = spec * pow(dotProd,6.0) * 0.6;
	float diffuse = max(dot(N, L), 0.0);

	vec2 cloud_color			=	texture2D( clouds, texCoord).rg;
	vec3 day_color				=	(texture2D( mySampler, texCoord ).rgb * diffuse + specColor.rgb * specMapColor.g) * (1 - cloud_color.r) + cloud_color.r * diffuse;
	vec3 night_color			=	texture2D( night, texCoord ).rgb * (1 - cloud_color.r) * 0.5;
	
	vec3 color = day_color;
	if(dot(N, L) < 0.1)
		color = mix(night_color, day_color, (diffuse + 0.1) * 5.0);
	frag_color = vec4(color, 1.0);
}
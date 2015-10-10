#version 400 core

uniform sampler2D clouds;
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

	vec3 L = lightVec;
    vec3 N = normal;
	float diffuse = max(dot(N, L), 0.0);
	float inv_diffuse = max(-dot(N, L), 0.0);
	

	vec4 cloud_color			=	texture2D( clouds, texCoord);
	vec3 day_color				=	cloud_color.rgb * clamp(diffuse, 0.0, 0.6);
	vec3 night_color			=	cloud_color.rgb * 0.15;// * clamp(inv_diffuse, 0.0, 0.5);
	
	
	vec3 color = day_color;
	if(dot(N, L) < 0.1)
		color = mix(night_color, day_color, (diffuse + 0.1) * 5.0);

	
	frag_color = vec4(color, cloud_color.a);
	
	//frag_color = vec4(day_color, 1.0);
}
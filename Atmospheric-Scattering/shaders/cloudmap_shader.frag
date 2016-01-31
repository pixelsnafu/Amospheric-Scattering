#version 400 core

uniform sampler2D clouds;
uniform sampler2D cloudBumpMap;
uniform sampler2D nightLights;

uniform float yRotation;

in vec3 vPos;
in vec3 vNorm;
in vec3 lPos;

in vec3 lightVec;
in vec3 eyeVec;
in vec3 halfVec;

in vec2 texCoord;

out vec4 frag_color;	


vec3 upperCut(vec3 color, float upper)
{
	vec3 vUpper = vec3(upper);
	if (lessThan(color, vUpper) == bvec3(true))
		return color;
	else
		return vec3(0);
}

void main(){
	
	vec3 normal = 2.0 * texture(cloudBumpMap, texCoord).rgb - 1.0;
	//normal.z = 1 - normal.x * normal.x - normal.y * normal.y;
	normal = normalize ( normal );

	vec3 L = lightVec;
    vec3 N = normal;
	vec3 L2 = normalize(lPos - vPos);
	vec3 N2 = normalize(vNorm);
	float diffuse = max(dot(N, L), 0.0);
	float normalDiffuse = max(dot(N2, L2), 0.0);
	
	vec2 nightLightTexCoord		=	texCoord + vec2(yRotation/360, 0);

	vec4 cloud_color			=	texture2D( clouds, texCoord);
	vec3 nightLightColor		=	texture2D( nightLights, nightLightTexCoord).rgb;
	vec3 day_color				=	cloud_color.rgb * clamp(diffuse, 0.0, 0.6);
	vec3 night_color			=	cloud_color.rgb * 0.25;// + nightLightColor * cloud_color.rgb * 5;// * clamp(inv_diffuse, 0.0, 0.5);
	
	
	vec3 color = day_color;
	vec3 nightLights = vec3(0, 0, 0);
	vec3 nightLightClouds = nightLightColor * cloud_color.rgb * 5;
	if (dot(N, L) < 0.1)
		color = mix(night_color, day_color, (diffuse + 0.1) * 5.0);

	if (dot(N2, L2) < 0.1)
		nightLights = mix(nightLightClouds, nightLights, (normalDiffuse + 0.1) * 5.0);

	frag_color = vec4(color + nightLights, cloud_color.a);
	
	//frag_color = vec4(day_color, 1.0);
}
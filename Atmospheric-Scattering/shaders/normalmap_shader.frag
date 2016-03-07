#version 450 core

uniform sampler2D day;
uniform sampler2D bumpMap;
uniform sampler2D night;
uniform sampler2D specMap;
uniform sampler2D clouds;

uniform mat4 cloudRotation;
uniform float yRotation;

in vec3 vPos;

in vec3 lightVec;
in vec3 eyeVec;
in vec3 halfVec;

in vec2 texCoord;

in vec4 frontColor;
in vec4 frontSecondaryColor;

in float cameraDistance;

out vec4 frag_color;

void main()
{

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
    //vec4 specColor = spec * pow(dotProd,6.0) * 0.5;
	float diffuse = max(dot(N, L), 0.0);
	vec4 specColor = spec * pow(diffuse, 32.0) * 0.6;
	

	vec2 cloudTexCoord			=	texCoord - vec2(yRotation/360, 0);

	vec3 cloud_color			=	texture2D( clouds, cloudTexCoord ).rgb;
	vec3 day_color				=	texture2D( day, texCoord ).rgb * diffuse + specColor.rgb * specMapColor.g - cloud_color * 0.5;// * (1 - cloud_color.r) + cloud_color.r * diffuse;
	vec3 night_color			=	texture2D( night, texCoord ).rgb * 10;// * (1 - cloud_color.r) * 0.5;
	
	vec3 color = day_color;

	// TODO : Declare tweaking constants as separate variables
	vec4 frontAtm = mix(frontColor * 0.01, frontColor * cameraDistance, (diffuse + 0.1) * 5.0);
	vec4 secondaryAtm = mix(frontSecondaryColor * 0.01, frontSecondaryColor * 0.3, (diffuse + 0.1) * 5.0);
	if(dot(N, L) < 0.1)
		color = mix(night_color, day_color, (diffuse + 0.1) * 5.0);
	//frag_color = secondaryAtm;
	frag_color = frontAtm + secondaryAtm * vec4(color, 1.0);
}
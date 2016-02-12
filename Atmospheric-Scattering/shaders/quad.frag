#version 400 core

noperspective in vec2 uv;

uniform sampler2D scene;
uniform sampler2D blur32;
uniform sampler2D blur16;
uniform sampler2D blur64;
uniform sampler2D blurX;
uniform sampler2D lightScatter;

void main()
{
	vec3 sceneTex = texture(scene, uv).rgb;
	vec3 blur32Tex = texture(blur32, uv).rgb;
	vec3 blur16Tex = texture(blur16, uv).rgb;
	vec3 blur64Tex = texture(blur64, uv).rgb;
	vec3 blurXTex = texture(blurX, uv).rgb;
	vec3 lightScatterTex = texture(lightScatter, uv).rgb;
	vec3 finalColor = sceneTex + blur16Tex + blur32Tex + blur64Tex + blurXTex + lightScatterTex;
	gl_FragColor =  vec4(finalColor, 1.0);
	//gl_FragColor = vec4(texture(lightScatter, uv).rgb, 1.0);
}
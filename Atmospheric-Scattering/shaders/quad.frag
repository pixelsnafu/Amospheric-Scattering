#version 400 core

noperspective in vec2 uv;

uniform mat3 lensStarMatrix;


uniform sampler2D anamorphic16;
uniform sampler2D anamorphic32;
uniform sampler2D anamorphic64;
uniform sampler2D atmosphere;
uniform sampler2D atmosphere2;
uniform sampler2D blur16;
uniform sampler2D blur24;
uniform sampler2D blur32;
uniform sampler2D blur64;
uniform sampler2D lightScatter;
uniform sampler2D lensFlareHalo;
uniform sampler2D lensDirt;
uniform sampler2D lensStar;
uniform sampler2D scene;

out vec4 frag_color;

void main()
{
	vec3 anamorphic16Tex = texture(anamorphic16, uv).rgb;
	vec3 anamorphic32Tex = texture(anamorphic32, uv).rgb;
	vec3 anamorphic64Tex = texture(anamorphic64, uv).rgb;
	vec3 atmosphereTex = texture(atmosphere, uv).rgb;
	vec3 atmosphereTex2 = texture(atmosphere2, uv).rgb;
	vec3 blur16Tex = texture(blur16, uv).rgb;
	vec3 blur24Tex = texture(blur24, uv).rgb;
	vec3 blur32Tex = texture(blur32, uv).rgb;
	vec3 blur64Tex = texture(blur64, uv).rgb;
	vec3 lightScatterTex = texture(lightScatter, uv).rgb;
	vec3 lensFlareHaloTex = texture(lensFlareHalo, uv).rgb;
	vec3 sceneTex = texture(scene, uv).rgb;

	vec2 lensStarTexCoord = (lensStarMatrix * vec3(uv, 1.0)).xy;
	vec3 lensStarTex = texture(lensStar, lensStarTexCoord).rgb;

	vec3 anamorphicColor = (anamorphic16Tex + anamorphic32Tex + anamorphic64Tex) * vec3(0.8, 0.3, 0.0);

	vec3 finalColor = sceneTex + anamorphic16Tex + blur16Tex + blur24Tex + blur32Tex + blur64Tex 
		+ lightScatterTex /* texture(lensDirt, uv).rgb*/ + lensFlareHaloTex * lensStarTex + anamorphicColor;
	frag_color =  vec4(finalColor + atmosphereTex + atmosphereTex2, 1.0);
}
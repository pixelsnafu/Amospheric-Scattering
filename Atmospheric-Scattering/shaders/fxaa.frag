#version 400 core

noperspective in vec2 uv;

uniform sampler2D scene;
uniform vec2 InverseFBOScreenRatio;

out vec4 frag_color;

void main()
{
	float FXAA_SPAN_MAX = 8.0;
	float FXAA_REDUCE_MUL = 1.0/8.0;
    float FXAA_REDUCE_MIN = 1.0/128.0;

	vec3 luma=vec3(0.299, 0.587, 0.114);

	float lumaTL  = dot(luma, texture2D(scene, uv + (vec2(-1.0, -1.0) * InverseFBOScreenRatio)).xyz);
    float lumaTR  = dot(luma, texture2D(scene, uv + (vec2(1.0, -1.0) * InverseFBOScreenRatio)).xyz);
    float lumaBL  = dot(luma, texture2D(scene, uv + (vec2(-1.0, 1.0) * InverseFBOScreenRatio)).xyz);
    float lumaBR  = dot(luma, texture2D(scene, uv + (vec2(1.0, 1.0) * InverseFBOScreenRatio)).xyz);
    float lumaM   = dot(luma, texture2D(scene, uv).xyz);

	vec2 dir;
	dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
	dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));

	float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * (FXAA_REDUCE_MUL * 0.25), FXAA_REDUCE_MIN);
	float inverseDirAdjustment = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), 
		max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * inverseDirAdjustment)) * InverseFBOScreenRatio;

	vec3 result1 = (1.0/2.0) * (
		texture2D(scene, uv + (dir * vec2(1.0/3.0 - 0.5))).xyz +
		texture2D(scene, uv + (dir * vec2(2.0/3.0 - 0.5))).xyz);

	vec3 result2 = result1 * (1.0/2.0) + (1.0/4.0) * (
		texture2D(scene, uv + (dir * vec2(0.0/3.0 - 0.5))).xyz +
		texture2D(scene, uv + (dir * vec2(3.0/3.0 - 0.5))).xyz);

	float lumaMin = min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
	float lumaMax = max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));
	float lumaResult2 = dot(luma, result2);

	if(lumaResult2 < lumaMin || lumaResult2 > lumaMax)
		frag_color = vec4(result1, 1.0);
	else
		frag_color = vec4(result2, 1.0);
}
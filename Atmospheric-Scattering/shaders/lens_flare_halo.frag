#version 400 core

uniform sampler2D lightScene;

noperspective in vec2 uv;

out vec4 frag_color;

vec3 texture2DDistorted(sampler2D Texture, vec2 TexCoord, vec2 Direction, vec3 Distortion)
{
    return vec3(
        texture2D(Texture, TexCoord + Direction * Distortion.r).r,
        texture2D(Texture, TexCoord + Direction * Distortion.g).g,
        texture2D(Texture, TexCoord + Direction * Distortion.b).b
    );
}

void main()
{
	float dispersal = 0.2, haloWidth = 0.45;
	vec2 texCoord = vec2(1.0) - uv;
	vec2 ghostVec = (vec2(0.5) - texCoord) * dispersal;
	vec2 haloVec = normalize(ghostVec) * haloWidth;
	vec2 direction = normalize(ghostVec);
	vec3 Distortion = vec3(0.94f, 0.97f, 1.00f);
	float weight = length(vec2(0.5) - fract(texCoord + haloVec)) / length(vec2(0.5));
	weight = pow(1.0 - weight, 10.0);
	vec3 halo = texture2DDistorted(
		lightScene,
		texCoord,
		direction * haloWidth,
		Distortion
	);

	//halo /= 5.0;

	frag_color = vec4(halo, 1.0);
}
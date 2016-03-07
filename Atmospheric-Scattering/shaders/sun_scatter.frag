#version 400 core

uniform vec2 lightPosition;

uniform sampler2D lightScene;
uniform sampler1D lensColor;

noperspective in vec2 uv;

out vec4 frag_color;

//vec3 Distortion = vec3(0.94f, 0.97f, 1.00f) * 1.5;

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
	// Calculate Sun rays
	int samples = 100;
	float intensity = 0.125, decay = 0.97375;
	vec2 direction = lightPosition - uv, texCoord = uv;
	direction /= samples;
	vec3 color = texture(lightScene, texCoord).rgb;

	for (int Sample = 0; Sample < samples; Sample++)
	{
		color += texture(lightScene, texCoord).rgb * intensity;
		intensity *= decay;
		texCoord += direction;
	}

	// Calculate Lens flare ghosts
	float dispersal = 0.15, distortion = 1.25;
	int ghosts = 6;

	texCoord = vec2(1.0) - uv;
	vec2 texelSize = 1.0 / vec2(textureSize(lightScene, 0));
	vec3 Distortion = vec3(-texelSize.x * distortion, -texelSize.y * distortion, texelSize.x * distortion);
	vec2 ghostVec = (vec2(0.5) - texCoord) * dispersal;
	direction = normalize(ghostVec);
	vec3 result = vec3(0.0);
	for (int i = 0; i < ghosts; i++)
	{
		vec2 offset = fract(texCoord + ghostVec * float(i));
		float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
		result += texture2DDistorted(lightScene, offset, direction, Distortion) * weight;
	}

	// Radial gradient of 1D rainbow color texture
	result *= texture(lensColor, length(vec2(0.5) - texCoord) / length(vec2(0.5))).rgb;

	vec3 finalColor = color + result;

	frag_color = vec4(finalColor, 1.0);
}
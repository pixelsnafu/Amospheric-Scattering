#version 400 core

uniform vec2 lightPosition;

uniform sampler2D lightScene;

noperspective in vec2 uv;

void main()
{
	int samples = 100;
	float intensity = 0.125, decay = 0.96875;
	vec2 direction = lightPosition - uv, texCoord = uv;
	direction /= samples;
	vec3 color = texture(lightScene, texCoord).rgb;

	for (int Sample = 0; Sample < samples; Sample++)
	{
		color += texture(lightScene, texCoord).rgb * intensity;
		intensity *= decay;
		texCoord += direction;
	}

	gl_FragColor = vec4(color, 1.0);
}
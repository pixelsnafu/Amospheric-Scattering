#version 120

uniform vec3 v3LightPos;
uniform float g;
uniform float g2;

varying vec3 v3Direction;

void main()
{
	float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);
	float fRayleighPhase = 1.0 + fCos * fCos;
	float fMiePhase = (1.0 - g2) / (2.0 + g2) * (1.0 + fCos * fCos) / pow(1.0 + g2 - 2.0 * g * fCos, 1.5);
	gl_FragColor = vec4(1.0 - exp(-1.5 * (fRayleighPhase * gl_Color.rgb + fMiePhase * gl_SecondaryColor.rgb)), 1.0);
	//gl_FragColor = gl_Color + fMiePhase * gl_SecondaryColor;
	gl_FragColor.a = gl_FragColor.b;
}
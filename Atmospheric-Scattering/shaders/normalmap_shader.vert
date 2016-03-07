#version 450 core

in vec3 vPosition;
in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vTangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec4 lightPosition;

uniform vec3 v3CameraPos;			// The camera's current position
uniform vec3 v3LightPos;			// The direction vector to the light source
uniform vec3 v3InvWavelength;		// 1 / pow(wavelength, 4) for the red, green, and blue channels
uniform float fCameraHeight;		// The camera's current height
uniform float fCameraHeight2;		// fCameraHeight^2
uniform float fOuterRadius;			// The outer (atmosphere) radius
uniform float fOuterRadius2;		// fOuterRadius^2
uniform float fInnerRadius;			// The inner (planetary) radius
uniform float fInnerRadius2;		// fInnerRadius^2
uniform float fKrESun;				// Kr * ESun
uniform float fKmESun;				// Km * ESun
uniform float fKr4PI;				// Kr * 4 * PI
uniform float fKm4PI;				// Km * 4 * PI
uniform float fScale;				// 1 / (fOuterRadius - fInnerRadius)
uniform float fScaleDepth;			// The scale depth (i.e. the altitude at which the atmosphere's average density is found)
uniform float fScaleOverScaleDepth;	// fScale / fScaleDepth

uniform int Samples;

out vec2 texCoord;

out vec3 lightVec;
out vec3 eyeVec;
out vec3 halfVec;

out vec3 vPos;
out vec3 vNorm;
out vec3 lPos;

out vec4 frontColor;
out vec4 frontSecondaryColor;
out float cameraDistance;

float scale(float fCos)
{
	float x = 1.0 - fCos;
	return fScaleDepth * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}

void main()
{

	mat4 modelView = view * model;
	mat3 normalMatrix = mat3(transpose(inverse(model)));

	vec3 n = normalize ( normalMatrix * vNormal );
	vec3 t = normalize ( normalMatrix * vTangent );
	vec3 b = cross (n, t);
	
	vec4 vertexInEye = modelView * vec4(vPosition, 1.0);

    vPos = vertexInEye.xyz;

	vec3 lightDir = normalize( lightPosition.xyz - vertexInEye.xyz );
	vec3 temp;
	temp.x = dot (lightDir, t);
	temp.y = dot (lightDir, b);
	temp.z = dot (lightDir, n);
	lightVec = normalize(temp);

	temp.x = dot (vertexInEye.xyz, t);
	temp.y = dot (vertexInEye.xyz, b);
	temp.z = dot (vertexInEye.xyz, n);

	eyeVec = normalize(temp);

	vertexInEye = normalize(vertexInEye);
	vec3 halfVector = normalize(vertexInEye.xyz + lightDir);
	temp.x = dot (halfVector, t);
	temp.y = dot (halfVector, b);
	temp.z = dot (halfVector, n);

	halfVec = temp ; 

	vec4 lightInEye = view * lightPosition;
	vec4 normalInEye = normalize(modelView * vec4(vNormal, 0.0));

	lPos = lightInEye.xyz;
	vNorm = normalInEye.xyz;

	texCoord = vTexCoord;

	vec3 v3Pos = mat3(model) * vPosition;
	vec3 v3Ray = v3Pos - v3CameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	// Calculate the closest intersection of the ray with the outer atmosphere (which is the near point of the ray passing through the atmosphere)
	float B = 2.0 * dot(v3CameraPos, v3Ray);
	float C = fCameraHeight2 - fOuterRadius2;
	float fDet = max(0.0, B * B - 4.0 * C);
	float fNear = 0.5 * (-B - sqrt(fDet));

	bool gfs;
	if ( fCameraHeight > fOuterRadius )
		gfs = true;
	else
		gfs = false;

	// Calculate the ray's starting position, then calculate its scattering offset
	vec3 v3Start;
	if (gfs)
		v3Start = v3CameraPos + v3Ray * fNear;
	else
		v3Start = v3CameraPos;

	if(gfs)
		fFar -= fNear;


	float fDepth;
	if (gfs)
		fDepth = exp((fInnerRadius - fOuterRadius) / fScaleDepth);
	else
		fDepth = exp((fInnerRadius - fCameraHeight) / fScaleDepth);

	float fCameraAngle = dot(-v3Ray, v3Pos) / length(v3Pos);
	float fLightAngle = dot(v3LightPos, v3Pos) / length(v3Pos);
	float fCameraScale = scale(fCameraAngle);
	float fLightScale = scale(fLightAngle);
	float fCameraOffset = fDepth * fCameraScale;
	float fTemp = (fLightScale + fCameraScale);

	// Initialize the scattering loop variables
	float fSampleLength = fFar / Samples;
	float fScaledLength = fSampleLength * fScale;
	vec3 v3SampleRay = v3Ray * fSampleLength;
	vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

	// Now loop through the sample rays
	vec3 v3FrontColor = vec3(0.0);
	vec3 v3Attenuate = vec3(0.0);
	for(int i = 0; i < Samples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
		float fScatter = fDepth*fTemp - fCameraOffset;
		v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}

	// Calculate earth's surface brightness according to distance of the camera 
	// from the outer radius.
	cameraDistance = clamp(abs(fOuterRadius - fCameraHeight), 0.1, 0.5);

	frontColor.rgb = v3FrontColor * (v3InvWavelength * fKrESun + fKmESun);
	// Calculate the attenuation factor for the ground
	frontSecondaryColor.rgb = v3Attenuate;

	gl_Position = projection * modelView * vec4(vPosition, 1.0);
}
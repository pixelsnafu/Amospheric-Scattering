#include <gl/glew.h>
#include <gl//freeglut.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\ext.hpp>
#include <glm\gtx\string_cast.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <string>

#include "camera.h"
#include "shaderProgram.h"
#include "Vec3.h"
#include "Sphere.h"
#include "SkyBox.h"
#include "TextureManager.h"

using namespace std;

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}


int VIEW_SIZE_WIDTH = 1024;
int VIEW_SIZE_HEIGHT = 576;

float FULLSCREEN_WIDTH, FULLSCREEN_HEIGHT, FULLSCREEN_ASPECT_RATIO;
float VIEW_ASPECT_RATIO = (float)VIEW_SIZE_WIDTH/(float)VIEW_SIZE_HEIGHT;
float SCENE_WIDTH = 1600, SCENE_HEIGHT = 900;
float SUN_WIDTH = SCENE_WIDTH / 8.0, SUN_HEIGHT = SCENE_HEIGHT / 8.0;
float ANAMORPHIC_WIDTH = SCENE_WIDTH / 32.0, ANAMORPHIC_HEIGHT = SCENE_HEIGHT / 32.0;
int VIEWPORT_WIDTH = VIEW_SIZE_WIDTH, VIEWPORT_HEIGHT = VIEW_SIZE_HEIGHT;

//camera variables
int prevX = -1, prevY = -1;
camera* cam;
bool fullScreen = false;

float l = -0.005f;
float r = 0.005f;
float t = 0.005f;
float b = -0.005f;
float n = 0.01f;
float f = 10000.5f;

glm::vec3 up_vector(0, 1, 0);
glm::vec3 camera_position(-20.0, 0.0, 50.0);
glm::vec3 camera_lookat(0.0, 0.0, 0.0);


//Buffer/shader variables
GLuint vao[40];
GLuint vbo[2];
GLuint sceneFbo, sunFbo, blurFbo, sunScatterFbo, lensFlareHaloFbo, quadFbo;
GLuint textureColorBufferMultiSampled;
GLuint vertexAttribute, texAttribute;
GLuint cloudmap_program, skybox_program, normalmap_program, atmosphere_program, fxaa_program, curr_program,
	   sun_program, sun_scatter_program, blur_program, quad_program, lens_flare_halo_program;
GLuint vao_index = 0;

// Object transformation values
glm::mat4 scale(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f)));
float rotation[3] = {0.0, 0.0, 0.0};
float sphereScale = 20.0f, cloudScale = sphereScale + 0.1, atmosphereScale = sphereScale + 0.3f;

// Light Variables
glm::vec4 lightColor(1.25, 1.25, 1.25, 1.0);
glm::vec4 lightPosition(100.0, 0.0, 100.0, 1.0);
glm::vec3 sunPos(20.0, 0.0, 20.0);

//quad variables
// x,y vertex positions
float quad_vertices[] = {
	-1.0, -1.0, 0.0,
	1.0, -1.0, 0.0,
	1.0,  1.0, 0.0,
	1.0,  1.0, 0.0,
	-1.0,  1.0, 0.0,
	-1.0, -1.0, 0.0
};

// per-vertex texture coordinates
float quad_texcoords[] = {
	0.0, 0.0,
	1.0, 0.0,
	1.0, 1.0,
	1.0, 1.0,
	0.0, 1.0,
	0.0, 0.0
};

// Object variables
shared_ptr<Sphere> s, cs, as, ss;
Skybox* skybox;
TextureManager& textureManager = TextureManager::GetInstance();

struct Screen
{
	GLuint width, height;
	GLfloat aspectRatio;
};

const Screen& getScreenDimensions()
{
	Screen screen;
	MONITORINFO target;
	target.cbSize = sizeof(MONITORINFO);
	HWND windowHandle = FindWindow(NULL, "Atmostpheric Scattering");
	HMONITOR hMon = MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
	GetMonitorInfo(hMon, &target);
	screen.width = abs(target.rcMonitor.right - target.rcMonitor.left);
	screen.height = abs(target.rcMonitor.bottom - target.rcMonitor.top);
	screen.aspectRatio = (GLfloat)screen.width / screen.height;
	return screen;
}

void initBuffers(GLuint& index, GLuint program){
	vertexAttribute = glGetAttribLocation(program, "vPosition");
	texAttribute = glGetAttribLocation(program, "vTexCoord");

	glBindVertexArray(vao[index]);

	glGenBuffers(2, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexAttribute);
	glVertexAttribPointer(vertexAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_texcoords), quad_texcoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(texAttribute);
	glVertexAttribPointer(texAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

GLuint generateFrameBufferObject(const GLuint& renderTexture, const GLuint& depthTexture = 0){
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
	if (depthTexture)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		cout << "Frame Buffer Object error status : " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return fbo;
}

void initAtmosphericUniforms(vector<GLuint> shaderPrograms)
{
	GLfloat innerRadius = s->getRadius() * s->getSize().x;
	GLfloat outerRadius = as->getRadius() * as->getSize().x;
	GLfloat scale = 1.0f / (outerRadius - innerRadius);
	GLfloat scaleDepth = 0.25;
	GLfloat scaleOverScaleDepth = scale / scaleDepth;
	GLfloat Kr = 0.0025f;
	GLfloat Km = 0.0010f;
	GLfloat ESun = 16.0f;
	GLfloat g = -0.99f;

	for (GLuint i = 0; i < shaderPrograms.size(); i++)
	{
		glUseProgram(shaderPrograms[i]);
		glUniform3f(glGetUniformLocation(shaderPrograms[i], "v3InvWavelength"), 1.0f / powf(0.650f, 4.0f), 1.0f / powf(0.570f, 4.0f), 1.0f / powf(0.475f, 4.0f));
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fInnerRadius"), innerRadius);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fInnerRadius2"), innerRadius * innerRadius);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fOuterRadius"), outerRadius);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fOuterRadius2"), outerRadius * outerRadius);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fKrESun"), Kr * ESun);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fKmESun"), Km * ESun);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fKr4PI"), Kr * 4.0f * (float)PI);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fKm4PI"), Km * 4.0f * (float)PI);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fScale"), scale);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fScaleDepth"), scaleDepth);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "fScaleOverScaleDepth"), scaleOverScaleDepth);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "g"), g);
		glUniform1f(glGetUniformLocation(shaderPrograms[i], "g2"), g * g);
		glUniform1i(glGetUniformLocation(shaderPrograms[i], "Samples"), 4);
	}

	glUseProgram(0);
}

void renderHorizontalBlurTexture(GLuint program, string inputTexture, string outputTexture, int radius, Screen& screen)
{
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "uBlurRadius"), radius);

	// Horizontal Pass
	glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureManager[outputTexture], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screen.width, screen.height);

	glUniform2f(glGetUniformLocation(program, "uBlurDirection"), 1.0, 0.0);

	textureManager.BindTexture2D(inputTexture, "uInputTex", program);

	glBindVertexArray(vao[vao_index]);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void renderBlurTexture(GLuint program, string inputTexture, string outputTexture, int radius, Screen& screen)
{
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "uBlurRadius"), radius);

	// Horizontal Pass
	glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureManager["blurHColorTexture"], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screen.width, screen.height);

	glUniform2f(glGetUniformLocation(program, "uBlurDirection"), 1.0, 0.0);
	
	textureManager.BindTexture2D(inputTexture, "uInputTex", program);

	glBindVertexArray(vao[vao_index]);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Vertical Pass
	glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureManager[outputTexture], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screen.width, screen.height);

	glUniform2f(glGetUniformLocation(program, "uBlurDirection"), 0.0, 1.0);

	textureManager.BindTexture2D("blurHColorTexture", "uInputTex", program);

	glBindVertexArray(vao[vao_index]);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init(){
	cloudmap_program = setUpAShader("shaders/normalmap_shader.vert", "shaders/cloudmap_shader.frag");
	if (!cloudmap_program)
	{
		cerr << "Error setting up Shaders!";
		exit(1);
	}

	normalmap_program = setUpAShader("shaders/normalmap_shader.vert", "shaders/normalmap_shader.frag");
	if (!normalmap_program)
	{
		cerr << "Error Setting Up scene shaders!" << endl;
		exit(1);
	}

	atmosphere_program = setUpAShader("shaders/atmosphere_shader.vert", "shaders/atmosphere_shader.frag");
	if (!atmosphere_program)
	{
		cerr << "Error Setting Up atmosphere shaders!" << endl;
		exit(1);
	}

	skybox_program = setUpAShader("shaders/skybox.vert", "shaders/skybox.frag");
	if (!skybox_program)
	{
		cerr << "Error setting up skybox shaders!";
		exit(1);
	}

	fxaa_program = setUpAShader("shaders/quad.vert", "shaders/fxaa.frag");
	if (!fxaa_program)
	{
		cerr << "Error setting up FXAA shaders!";
		exit(1);
	}

	sun_program = setUpAShader("shaders/sun.vert", "shaders/sun.frag");
	if (!sun_program)
	{
		cerr << "Error setting up sun shaders!";
		exit(1);
	}

	blur_program = setUpAShader("shaders/quad.vert", "shaders/blur.frag");
	if (!blur_program)
	{
		cerr << "Error setting up blur shaders!";
		exit(1);
	}

	quad_program = setUpAShader("shaders/quad.vert", "shaders/quad.frag");
	if (!quad_program)
	{
		cerr << "Error setting up quad shaders!";
		exit(1);
	}

	
	sun_scatter_program = setUpAShader("shaders/quad.vert", "shaders/sun_scatter.frag");
	if (!sun_scatter_program)
	{
		cerr << "Error setting up sun scatter shaders!";
		exit(1);
	}
	
	lens_flare_halo_program = setUpAShader("shaders/quad.vert", "shaders/lens_flare_halo.frag");
	if (!lens_flare_halo_program)
	{
		cerr << "Error setting up lens flare shaders!";
		exit(1);
	}
	

	glGenVertexArrays(40, vao);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_POLYGON_SMOOTH);
	//glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GLfloat aspect_ratio = (GLfloat)VIEW_SIZE_WIDTH / (GLfloat)VIEW_SIZE_HEIGHT;
	cam = new camera(camera_position, camera_lookat, up_vector, l, r, t, b, n, f, aspect_ratio);

	vector<string> faces;
	string spaceboxDirectoryName("dark-space-skybox/");
	faces.push_back(spaceboxDirectoryName + "right.png");
	faces.push_back(spaceboxDirectoryName + "left.png");
	faces.push_back(spaceboxDirectoryName + "top.png");
	faces.push_back(spaceboxDirectoryName + "bottom.png");
	faces.push_back(spaceboxDirectoryName + "front.png");
	faces.push_back(spaceboxDirectoryName + "back.png");

	textureManager.LoadTextureCubeMap(faces, "skybox");
	textureManager.LoadTexture1D("textures/lenscolor.png", "lensColor");
	textureManager.LoadTexture2D("textures/earth_day_8k.jpg", "earthDay");
	textureManager.LoadTexture2D("textures/earth_night_8k.png", "earthNight");
	textureManager.LoadTexture2D("textures/earth_night_8k_blur.png", "earthNightBlur");
	textureManager.LoadTexture2D("textures/earth_specular.jpg", "earthSpecularMap");
	textureManager.LoadTexture2D("textures/earth_normalmap_8k.jpg", "earthNormalMap");
	textureManager.LoadTexture2D("textures/earth_clouds_8k.jpg", "earthClouds");
	textureManager.LoadTexture2D("textures/clouds_normalmap_8k.jpg", "earthCloudsNormalMap");
	textureManager.LoadTexture2D("textures/lensstar.png", "lensStar");
	textureManager.LoadTexture2D("textures/lensdirt_lowc.jpg", "lensDirt");
	textureManager.GenerateFBOTexture2D("sceneColorTexture", SCENE_WIDTH, SCENE_HEIGHT);
	textureManager.GenerateFBOTexture2D("sceneDepthTexture", SCENE_WIDTH, SCENE_HEIGHT, true);
	textureManager.GenerateFBOTexture2D("atmosphereColorTexture", SUN_WIDTH, SUN_HEIGHT);
	textureManager.GenerateFBOTexture2D("atmosphereColorTexture2", SUN_WIDTH, SUN_HEIGHT);
	textureManager.GenerateFBOTexture2D("sunColorTexture", SUN_WIDTH, SUN_HEIGHT);
	textureManager.GenerateFBOTexture2D("sunDepthTexture", SUN_WIDTH, SUN_HEIGHT, true);
	textureManager.GenerateFBOTexture2D("blurHColorTexture", SUN_WIDTH, SUN_HEIGHT);
	textureManager.GenerateFBOTexture2D("blur16ColorTexture", SUN_WIDTH, SUN_HEIGHT);
	textureManager.GenerateFBOTexture2D("blur24ColorTexture", SUN_WIDTH, SUN_HEIGHT);
	textureManager.GenerateFBOTexture2D("blur32ColorTexture", SUN_WIDTH, SUN_HEIGHT);
	textureManager.GenerateFBOTexture2D("blur64ColorTexture", SUN_WIDTH, SUN_HEIGHT);
	textureManager.GenerateFBOTexture2D("sunScatterColorTexture", SUN_WIDTH, SUN_HEIGHT);
	textureManager.GenerateFBOTexture2D("lensFlareHaloColorTexture", SUN_WIDTH, SUN_HEIGHT);
	textureManager.GenerateFBOTexture2D("anamorphicLensColorTexture", ANAMORPHIC_WIDTH, ANAMORPHIC_HEIGHT);
	textureManager.GenerateFBOTexture2D("anamorphicLensDepthTexture", ANAMORPHIC_WIDTH, ANAMORPHIC_HEIGHT);
	textureManager.GenerateFBOTexture2D("anamorphicBlur16ColorTexture", ANAMORPHIC_WIDTH, ANAMORPHIC_HEIGHT);
	textureManager.GenerateFBOTexture2D("anamorphicBlur32ColorTexture", ANAMORPHIC_WIDTH, ANAMORPHIC_HEIGHT);
	textureManager.GenerateFBOTexture2D("anamorphicBlur64ColorTexture", ANAMORPHIC_WIDTH, ANAMORPHIC_HEIGHT);
	textureManager.GenerateFBOTexture2D("quadColorTexture", SCENE_WIDTH, SCENE_HEIGHT);

	sceneFbo = generateFrameBufferObject(textureManager["sceneColorTexture"], textureManager["sceneDepthTexture"]);
	sunFbo = generateFrameBufferObject(textureManager["sunColorTexture"], textureManager["sunDepthTexture"]);
	blurFbo = generateFrameBufferObject(textureManager["blurHColorTexture"]);
	sunScatterFbo = generateFrameBufferObject(textureManager["sunScatterColorTexture"]);
	lensFlareHaloFbo = generateFrameBufferObject(textureManager["lensFlareHaloColorTexture"]);
	quadFbo = generateFrameBufferObject(textureManager["quadColorTexture"]);
	
	map<string, string> textureHandles;

	curr_program = normalmap_program;
	
	textureHandles.clear();
	textureHandles.insert(make_pair("earthDay", "day"));
	textureHandles.insert(make_pair("earthNight", "night"));
	textureHandles.insert(make_pair("earthSpecularMap", "specMap"));
	textureHandles.insert(make_pair("earthNormalMap", "bumpMap"));
	textureHandles.insert(make_pair("earthClouds", "clouds"));

	s = shared_ptr<Sphere>(new Sphere(vao[vao_index], 1.0f, 50, 50, 0.015f, 0.f));
	s->generateMesh();
	s->setTextureHandles(textureHandles);
	s->setScale(sphereScale, sphereScale, sphereScale);
	s->setDiffuseColor(1.0f, 0.941f, 0.898f);
	s->initBuffers(curr_program);
	
	vao_index++;
	curr_program = cloudmap_program;

	textureHandles.clear();
	textureHandles.insert(make_pair("earthClouds", "clouds"));
	textureHandles.insert(make_pair("earthCloudsNormalMap", "cloudBumpMap"));
	textureHandles.insert(make_pair("earthNightBlur", "nightLights"));


	cs = shared_ptr<Sphere>(new Sphere(vao[vao_index], 1.0f, 50, 50, 0.02f, 0.f));
	cs->generateMesh();
	cs->setTextureHandles(textureHandles);
	cs->setScale(cloudScale, cloudScale, cloudScale);
	cs->setDiffuseColor(1.0f, 0.941f, 0.898f);
	cs->initBuffers(curr_program);

	vao_index++;
	curr_program = atmosphere_program;

	atmosphereScale = sphereScale + 0.3;

	as = shared_ptr<Sphere>(new Sphere (vao[vao_index], 1.0f, 50, 50, 0.0, 0.0));
	as->setScale(atmosphereScale, atmosphereScale, atmosphereScale);
	as->generateMesh();
	as->initBuffers(curr_program);

	vao_index++;
	curr_program = sun_program;
	ss = shared_ptr<Sphere>(new Sphere(vao[vao_index], 1.0f, 50, 50, 0.0, 0.0));
	ss->setPosition(sunPos.x, sunPos.y, sunPos.z);
	ss->setScale(100, 100, 100);
	ss->generateMesh();
	ss->initBuffers(curr_program);

	vao_index++;
	curr_program = skybox_program;

	textureHandles.clear();
	textureHandles.insert(make_pair("skybox", "skybox"));

	skybox = new Skybox(vao[vao_index], faces);
	skybox->generateMesh();
	skybox->setScale(5.0, 5.0, 5.0);
	skybox->setDiffuseColor(0.0, 0.0, 0.5);
	skybox->setTextureHandles(textureHandles);
	skybox->enableCubemap();
	skybox->initBuffers(curr_program);

	vao_index++;
	curr_program = fxaa_program;

	initBuffers(vao_index, fxaa_program);

	vector<GLuint> shaderPrograms;
	shaderPrograms.push_back(atmosphere_program);
	shaderPrograms.push_back(normalmap_program);
	initAtmosphericUniforms(shaderPrograms);
}

void render(){
	
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, SCENE_WIDTH, SCENE_HEIGHT);
	
	
	GLfloat camHeight = glm::length(cam->getCamPosition());
	glm::vec3 lightPos = glm::normalize(glm::vec3(lightPosition) - cam->getCamPosition());
	
	curr_program = normalmap_program;
	glUseProgram(curr_program);
	cam->setupCamera(curr_program);

	glUniform3f(glGetUniformLocation(curr_program, "v3LightDir"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3fv(glGetUniformLocation(curr_program, "v3CameraPos"), 1, glm::value_ptr(cam->getCamPosition()));
	glUniform1f(glGetUniformLocation(curr_program, "fCameraHeight"), camHeight);
	glUniform1f(glGetUniformLocation(curr_program, "fCameraHeight2"), camHeight * camHeight);

	glUniform4fv(glGetUniformLocation(curr_program, "lightPosition"), 1, glm::value_ptr(lightPosition));
	glUniform4fv(glGetUniformLocation(curr_program, "lightColor"), 1, glm::value_ptr(lightColor));

	glm::vec3 diffRotation = cs->getRotation() - s->getRotation();	
	glm::mat4 diffRotationMatrix = glm::rotate(glm::mat4(1.0f), diffRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(glGetUniformLocation(curr_program, "cloudRotation"), 1, false, glm::value_ptr(diffRotationMatrix));
	glUniform1f(glGetUniformLocation(curr_program, "yRotation"), diffRotation.y);

	s->render(curr_program, textureManager);
	
	glCullFace(GL_FRONT);
	
	curr_program = atmosphere_program;
	glUseProgram(curr_program);
	cam->setupCamera(curr_program);	

	atmosphereScale = sphereScale + 0.25f;
	as->setScale(atmosphereScale, atmosphereScale, atmosphereScale);

	GLfloat outerRadius = as->getRadius() * as->getSize().x;

	glUniform1f(glGetUniformLocation(curr_program, "fOuterRadius"), outerRadius);
	glUniform1f(glGetUniformLocation(curr_program, "fOuterRadius2"), outerRadius * outerRadius);
	glUniform3f(glGetUniformLocation(curr_program, "v3LightDir"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3fv(glGetUniformLocation(curr_program, "v3CameraPos"), 1, glm::value_ptr(cam->getCamPosition()));
	glUniform1f(glGetUniformLocation(curr_program, "fCameraHeight"), camHeight);
	glUniform1f(glGetUniformLocation(curr_program, "fCameraHeight2"), camHeight * camHeight);

	as->render(curr_program, textureManager);
 
	glCullFace(GL_BACK);
	
	curr_program = cloudmap_program;

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);

	cam->setupCamera(curr_program);
	double scaleOffset = max(0.00001, camHeight / 1000.0 * exp(camHeight/250.0));
	double newCloudScale = cloudScale + scaleOffset;
	cs->setScale(newCloudScale, newCloudScale, newCloudScale);
	glUniform4fv(glGetUniformLocation(curr_program, "lightPosition"), 1, glm::value_ptr(lightPosition));
	glUniform4fv(glGetUniformLocation(curr_program, "lightColor"), 1, glm::value_ptr(lightColor * glm::vec4(0.75, 0.75, 0.75, 1.0)));
	glUniform1f(glGetUniformLocation(curr_program, "yRotation"), diffRotation.y);

	cs->render(curr_program, textureManager);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	
	curr_program = sun_program;

	cam->setupCamera(curr_program);
	glUniform1i(glGetUniformLocation(curr_program, "isSun"), 1);
	ss->render(curr_program, textureManager);

	curr_program = skybox_program;

	//rendering the skybox
	glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content
	glUseProgram(curr_program);
	//cast out the translation from the view matrix of the camera, to make the skybox appear at infinity
	glm::mat4 view = glm::mat4(glm::mat3(cam->getCameraViewMatrix()));
	glm::mat4 projection = cam->getCameraProjectionMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	skybox->render(skybox_program, textureManager);
	glDepthFunc(GL_LESS); // Set the depth function to default after rendering the skybox

	//animate the cloud and earth spheres
	s->animate();
	cs->animate();
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//--------------------------------------------------------------------------------------
	// Render Atmosphere sphere on a low res texture for blurring/bloom
	
	glBindFramebuffer(GL_FRAMEBUFFER, sunFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureManager["atmosphereColorTexture"], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SUN_WIDTH, SUN_HEIGHT);

	glCullFace(GL_FRONT);

	curr_program = atmosphere_program;
	glUseProgram(curr_program);

	cam->setupCamera(curr_program);

	atmosphereScale = sphereScale + 0.5f;
	as->setScale(atmosphereScale, atmosphereScale, atmosphereScale);

	outerRadius = as->getRadius() * as->getSize().x;

	glUniform1f(glGetUniformLocation(curr_program, "fOuterRadius"), outerRadius);
	glUniform1f(glGetUniformLocation(curr_program, "fOuterRadius2"), outerRadius * outerRadius);
	glUniform3f(glGetUniformLocation(curr_program, "v3LightDir"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3fv(glGetUniformLocation(curr_program, "v3CameraPos"), 1, glm::value_ptr(cam->getCamPosition()));
	glUniform1f(glGetUniformLocation(curr_program, "fCameraHeight"), camHeight);
	glUniform1f(glGetUniformLocation(curr_program, "fCameraHeight2"), camHeight * camHeight);

	as->render(curr_program, textureManager);

	glCullFace(GL_BACK);
	
	curr_program = sun_program;
	glUseProgram(curr_program);

	cam->setupCamera(curr_program);

	GLuint isSunLocation = glGetUniformLocation(curr_program, "isSun");
	
	glUniform1i(isSunLocation, 0);
	s->render(curr_program, textureManager);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//--------------------------------------------------------------------------------------
	// Render minimalistic sun for blurring/bloom.

	glBindFramebuffer(GL_FRAMEBUFFER, sunFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureManager["sunColorTexture"], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureManager["sunDepthTexture"], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SUN_WIDTH, SUN_HEIGHT);
	
	curr_program = sun_program;
	glUseProgram(curr_program);
	
	cam->setupCamera(curr_program);

	isSunLocation = glGetUniformLocation(curr_program, "isSun");
	
	glUniform1i(isSunLocation, 1);
	ss->setScale(100, 100, 100);
	ss->render(curr_program, textureManager);
	
	glUniform1i(isSunLocation, 0);
	s->render(curr_program, textureManager);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//--------------------------------------------------------------------------------------
	// Render God rays + Lens Flare to texture
	glBindFramebuffer(GL_FRAMEBUFFER, sunScatterFbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SUN_WIDTH, SUN_HEIGHT);

	curr_program = sun_scatter_program;
	glUseProgram(curr_program);

	glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), sunPos);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(100));
	// Transform light coordinates from World to Screen-space coordinates.
	glm::vec3 lightScreen = glm::project(glm::vec3(0.0), cam->getCameraViewMatrix() * scaleMatrix * translateMatrix, cam->getCameraProjectionMatrix(), glm::vec4(0, 0, SUN_WIDTH, SUN_HEIGHT));

	glUniform2f(glGetUniformLocation(curr_program, "lightPosition"), lightScreen.x / SUN_WIDTH,  lightScreen.y / SUN_HEIGHT);
	textureManager.BindTexture2D("sunColorTexture", "lightScene", curr_program);
	textureManager.BindTexture1D("lensColor", "lensColor", curr_program);

	glBindVertexArray(vao[vao_index]);
	glDrawArrays(GL_TRIANGLES, 0, 6);	
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//--------------------------------------------------------------------------------------
	//Render Anamorphic Lens texture
	glBindFramebuffer(GL_FRAMEBUFFER, sunFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureManager["anamorphicLensColorTexture"], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, ANAMORPHIC_WIDTH, ANAMORPHIC_HEIGHT);

	curr_program = sun_program;
	glUseProgram(curr_program);

	cam->setupCamera(curr_program);

	isSunLocation = glGetUniformLocation(curr_program, "isSun");

	glUniform1i(isSunLocation, 1);
	ss->setScale(100, 30, 100);
	ss->render(curr_program, textureManager);
	
	glUniform1i(isSunLocation, 0);
	s->render(curr_program, textureManager);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//--------------------------------------------------------------------------------------
	// Gaussian Blur textures
	Screen sunScreen = Screen{ SUN_WIDTH, SUN_HEIGHT, SUN_WIDTH / SUN_HEIGHT };
	Screen anamorphicScreen = Screen{ ANAMORPHIC_WIDTH, ANAMORPHIC_HEIGHT, ANAMORPHIC_WIDTH / ANAMORPHIC_HEIGHT };
	renderBlurTexture(blur_program, "sunColorTexture", "blur16ColorTexture", 16, sunScreen);
	renderBlurTexture(blur_program, "sunColorTexture", "blur24ColorTexture", 24, sunScreen);
	renderBlurTexture(blur_program, "sunColorTexture", "blur32ColorTexture", 32, sunScreen);
	renderBlurTexture(blur_program, "sunColorTexture", "blur64ColorTexture", 64, sunScreen);
	renderBlurTexture(blur_program, "sunScatterColorTexture", "sunScatterColorTexture", 24, sunScreen);
	renderBlurTexture(blur_program, "atmosphereColorTexture", "atmosphereColorTexture", 24, sunScreen);
	renderBlurTexture(blur_program, "atmosphereColorTexture", "atmosphereColorTexture2", 32, sunScreen);
	renderHorizontalBlurTexture(blur_program, "anamorphicLensColorTexture", "anamorphicBlur16ColorTexture", 128, anamorphicScreen);
	renderHorizontalBlurTexture(blur_program, "anamorphicLensColorTexture", "anamorphicBlur32ColorTexture", 256, anamorphicScreen);
	renderHorizontalBlurTexture(blur_program, "anamorphicLensColorTexture", "anamorphicBlur64ColorTexture", 512, anamorphicScreen);

	//--------------------------------------------------------------------------------------
	// Render Lens Flare Halo to texture
	curr_program = lens_flare_halo_program;
	glUseProgram(curr_program);

	glBindFramebuffer(GL_FRAMEBUFFER, lensFlareHaloFbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SUN_WIDTH, SUN_HEIGHT);

	textureManager.BindTexture2D("blur16ColorTexture", "lightScene", curr_program);

	glBindVertexArray(vao[vao_index]);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//--------------------------------------------------------------------------------------
	// Render to a quad
	curr_program = quad_program;
	glUseProgram(curr_program);

	glBindFramebuffer(GL_FRAMEBUFFER, quadFbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, SCENE_WIDTH, SCENE_HEIGHT);

	glm::vec3 camX = glm::vec3(glm::column(cam->getCameraViewMatrix(), 1));
	glm::vec3 camZ = glm::vec3(glm::column(cam->getCameraViewMatrix(), 2));

	float camRot = glm::dot(camX, glm::vec3(0, 0, 1)) + glm::dot(camZ, glm::vec3(0, 1, 0));

	glm::mat3 scaleBias1 = glm::mat3(
			2.0f, 0.0f, -1.0f,
			0.0f, 2.0f, -1.0f,
			0.0f, 0.0f, 1.0f
		);

	glm::mat3 rotation = glm::mat3(
			cos(camRot), -sin(camRot), 0.0f,
			sin(camRot), cos(camRot), 0.0f,
			0.0f, 0.0f, 1.0f
		);

	glm::mat3 scaleBias2 = glm::mat3(
			0.5f, 0.0f, 0.5f,
			0.0f, 0.5f, 0.5f,
			0.0f, 0.0f, 1.0f
		);

	glm::mat3 lensStarMatrix = scaleBias2 * rotation * scaleBias1;

	glUniformMatrix3fv(glGetUniformLocation(curr_program, "lensStarMatrix"), 1, GL_FALSE, glm::value_ptr(lensStarMatrix));

	textureManager.BindTexture2D("anamorphicBlur16ColorTexture", "anamorphic16", curr_program);
	textureManager.BindTexture2D("anamorphicBlur32ColorTexture", "anamorphic32", curr_program);
	textureManager.BindTexture2D("anamorphicBlur64ColorTexture", "anamorphic64", curr_program);
	textureManager.BindTexture2D("atmosphereColorTexture", "atmosphere", curr_program);
	textureManager.BindTexture2D("atmosphereColorTexture2", "atmosphere2", curr_program);
	textureManager.BindTexture2D("blur16ColorTexture", "blur16", curr_program);
	textureManager.BindTexture2D("blur24ColorTexture", "blur24", curr_program);
	textureManager.BindTexture2D("blur32ColorTexture", "blur32", curr_program);
	textureManager.BindTexture2D("blur64ColorTexture", "blur64", curr_program);
	textureManager.BindTexture2D("sceneColorTexture", "scene", curr_program);
	textureManager.BindTexture2D("sunScatterColorTexture", "lightScatter", curr_program);
	textureManager.BindTexture2D("lensFlareHaloColorTexture", "lensFlareHalo", curr_program);
	textureManager.BindTexture2D("lensDirt", "lensDirt", curr_program);
	textureManager.BindTexture2D("lensStar", "lensStar", curr_program);

	glBindVertexArray(vao[vao_index]);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//--------------------------------------------------------------------------------------
	// Finally, FXAA implementation on the whole scene
	curr_program = fxaa_program;
	glUseProgram(curr_program);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	
	glUniform2f(glGetUniformLocation(curr_program, "InverseFBOScreenRatio"), 1.f / SCENE_WIDTH, 1.f / SCENE_HEIGHT);

	textureManager.BindTexture2D("quadColorTexture", "scene", curr_program);
	
	glBindVertexArray(vao[vao_index]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	textureManager.unbindAllTextures();
	
	glutPostRedisplay();
	glutSwapBuffers();
	
}

string getScreenShotFileName()
{
	time_t t = time(0);
	struct tm* now = localtime(&t);
	string underscore = string("_");
	now->tm_mon++;
	return string("screenshots/Earth") + underscore + to_string(now->tm_year + 1900) + underscore +
		(now->tm_mon < 10 ? (string("0") + to_string(now->tm_mon)) : to_string(now->tm_mon)) + 
		underscore + (now->tm_mday < 10 ? (string("0") + to_string(now->tm_mday)) : to_string(now->tm_mday)) + 
		underscore + (now->tm_hour < 10 ? (string("0") + to_string(now->tm_hour)) : to_string(now->tm_hour)) +
		underscore + (now->tm_min < 10 ? (string("0") + to_string(now->tm_min)) : to_string(now->tm_min)) +
		underscore + (now->tm_sec < 10 ? (string("0") + to_string(now->tm_sec)) : to_string(now->tm_sec)) +
		string(".bmp");

}

void keyboard(unsigned char key, int x, int y)
{

	const float cameraIntersectionThreshold = 1.005f;

	switch(key){
	case 27:

		glutLeaveMainLoop();
		break;

	case 'D':
	case 'd':
		cam->moveCameraRight();
		cam->updateCamera(curr_program);
		if (glm::length(cam->getCamPosition()) <= s->getRadius() * s->getSize().x * cameraIntersectionThreshold){
			cam->setCameraPosition(s->getRadius() * s->getSize().x * cameraIntersectionThreshold * glm::normalize(cam->getCamPosition()));
		}
		break;

	case 'a':
	case 'A':
		cam->moveCameraLeft();
		cam->updateCamera(curr_program);
		if (glm::length(cam->getCamPosition()) <= s->getRadius() * s->getSize().x * cameraIntersectionThreshold){
			cam->setCameraPosition(s->getRadius() * s->getSize().x * cameraIntersectionThreshold * glm::normalize(cam->getCamPosition()));
		}
		break;

	case 'w':
	case 'W':
		cam->moveCameraForward();
		cam->updateCamera(curr_program);
		if (glm::length(cam->getCamPosition()) <= s->getRadius() * s->getSize().x * cameraIntersectionThreshold){
			cam->setCameraPosition(s->getRadius() * s->getSize().x * cameraIntersectionThreshold * glm::normalize(cam->getCamPosition()));
		}
		break;

	case 's':
	case 'S':
		cam->moveCameraBackward();
		cam->updateCamera(curr_program);
		if (glm::length(cam->getCamPosition()) <= s->getRadius() * s->getSize().x * cameraIntersectionThreshold){
			cam->setCameraPosition(s->getRadius() * s->getSize().x * cameraIntersectionThreshold * glm::normalize(cam->getCamPosition()));
		}
		break;
	case 'f':
	case 'F':
		if(fullScreen){
			fullScreen = false;
			//glViewport(0, 0, VIEW_SIZE_WIDTH, VIEW_SIZE_HEIGHT);
			VIEWPORT_WIDTH = VIEW_SIZE_WIDTH;
			VIEWPORT_HEIGHT = VIEW_SIZE_HEIGHT;
			cam->setAspectRatio(VIEW_ASPECT_RATIO);
			glutReshapeWindow(VIEW_SIZE_WIDTH, VIEW_SIZE_HEIGHT);
		}else{
			fullScreen = true;
			MONITORINFO target;
			target.cbSize = sizeof(MONITORINFO);
			HWND windowHandle = FindWindow(NULL, "Atmostpheric Scattering");
			HMONITOR hMon = MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
			GetMonitorInfo(hMon, &target);
			FULLSCREEN_WIDTH = target.rcMonitor.right - target.rcMonitor.left;
			FULLSCREEN_HEIGHT = target.rcMonitor.bottom - target.rcMonitor.top;
			FULLSCREEN_ASPECT_RATIO = FULLSCREEN_WIDTH / FULLSCREEN_HEIGHT;
			cam->setAspectRatio(FULLSCREEN_ASPECT_RATIO);
			//glViewport(0, 0, FULLSCREEN_WIDTH, FULLSCREEN_HEIGHT);
			VIEWPORT_WIDTH = FULLSCREEN_WIDTH;
			VIEWPORT_HEIGHT = FULLSCREEN_HEIGHT;
			glutFullScreen();
		}
		break;
	case 'p':
	case 'P':
		if (fullScreen){
			if (FULLSCREEN_WIDTH == 1366)
			{
				SOIL_save_screenshot(getScreenShotFileName().c_str(), SOIL_SAVE_TYPE_BMP, 0, 0, 1360, FULLSCREEN_HEIGHT);
			}
			else
			{
				SOIL_save_screenshot(getScreenShotFileName().c_str(), SOIL_SAVE_TYPE_BMP, 0, 0, FULLSCREEN_WIDTH, FULLSCREEN_HEIGHT);
			}
		}
		else
			SOIL_save_screenshot(getScreenShotFileName().c_str(), SOIL_SAVE_TYPE_BMP, 0, 0, VIEW_SIZE_WIDTH, VIEW_SIZE_HEIGHT);
		break;
	}


}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	if (!fullScreen)
	{
		VIEW_SIZE_WIDTH = width;
		VIEW_SIZE_HEIGHT = height;
	}
	VIEW_ASPECT_RATIO = float(width) / height;
	VIEWPORT_WIDTH = width;
	VIEWPORT_HEIGHT = height;
	cam->setAspectRatio(VIEW_ASPECT_RATIO);
}

//function to track the mouse for camera
void mouseMove(int x, int y)
{
	if(prevX == -1)
		prevX = x;
	if(prevY == -1)
		prevY = y;

	cam->cameraUpdateRotX(x-prevX);
	cam->cameraUpdateRotY(y-prevY);
	cam->setupCamera(curr_program);
	prevX = x;
	prevY = y;
}

int main(int argc, char** argv)
{

	glutInit(&argc, argv);
	glutSetOption(GLUT_MULTISAMPLE, 4);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(VIEW_SIZE_WIDTH, VIEW_SIZE_HEIGHT);
	glutCreateWindow("Atmostpheric Scattering");
	glutWarpPointer(VIEW_SIZE_WIDTH/2, VIEW_SIZE_HEIGHT/2);

	cout<<"Video Card in Use : "<<glGetString(GL_RENDERER)<<endl;
	cout<<"Current OpenGL Version : "<<glGetString(GL_VERSION)<<endl;
	cout<<"Current GLSL version :` "<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<endl<<endl;

	glewInit();
	init();

	glutDisplayFunc(render);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);

	//uncomment/comment this to enable/disable camera mouse-movement.
	glutPassiveMotionFunc(mouseMove);

	//comment this to show cursor in the window.
	glutSetCursor(GLUT_CURSOR_NONE);

	glutMainLoop();

	return 0;

}
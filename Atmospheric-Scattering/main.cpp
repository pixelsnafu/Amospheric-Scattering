#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <string>

#include "camera.h"
#include "shaderProgram.h"
#include "Vec3.h"
#include "Sphere.h"
#include "SkyBox.h"
#include "TextureManager.h"

using namespace std;

int VIEW_SIZE_WIDTH = 1024;
int VIEW_SIZE_HEIGHT = 768;

float FULLSCREEN_WIDTH, FULLSCREEN_HEIGHT, FULLSCREEN_ASPECT_RATIO;
float VIEW_ASPECT_RATIO = (float)VIEW_SIZE_WIDTH/(float)VIEW_SIZE_HEIGHT;
int SCENE_WIDTH = 1920, SCENE_HEIGHT = 1080;
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

//Buffer/shader variables
GLuint vao[40];
GLuint vbo[2];
GLuint sceneFbo, sceneMultisampleFbo;
GLuint textureColorBufferMultiSampled;
GLuint vertexAttribute, texAttribute;
GLuint cloudmap_program, skybox_program, normalmap_program, atmosphere_program, fxaa_program, curr_program;
GLuint vao_index = 0;

//camera variables
glm::vec3 up_vector(0, 1, 0);
glm::vec3 camera_position(0.0, 0.0, -25.0);
glm::vec3 camera_lookat(0.0, 0.0, 0.0);

glm::mat4 scale(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f)));
float rotation[3] = {0.0, 0.0, 0.0};
float sphereScale = 10.0f;

glm::vec4 lightColor(1.25, 1.25, 1.25, 1.0);
glm::vec4 lightPosition(100.0, 0.0, 100.0, 1.0);
glm::vec4 cloudLightPosition(50.0, 0.0, 50.0, 1.0);

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

Sphere *s, *cs, *as;
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

/*
GLuint createFBOTexture(int w, int h, bool isDepth = false){

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, (isDepth ? GL_DEPTH_COMPONENT : GL_RGBA),
		w, h, 0, (isDepth ? GL_DEPTH_COMPONENT : GL_RGBA), GL_FLOAT, NULL);
	
	//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 16, GL_RGBA, w, h, GL_TRUE);

	//texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (glGetError()){
		cout << "Error while creating Empty Texture!" << endl;
		cout << gluErrorString(glGetError()) << endl;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}
*/


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

GLuint generateMultiSampleTexture(GLuint samples, GLuint screenWidth, GLuint screenHeight)
{
	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, screenWidth, screenHeight, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	if (glGetError())
	{
		cout << "Error while creating Multisampled Empty Texture: " << gluErrorString(glGetError()) << endl;
	}

	return texture;
}

GLuint generateMultiSampleFramebuffer(GLuint width, GLuint height)
{
	GLuint fbo, depthBuffer;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Create a multisampled color texture attachment
	textureColorBufferMultiSampled = generateMultiSampleTexture(4, width, height);
	cout << "Multisampled Texture ID: " << textureColorBufferMultiSampled << endl;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);

	// Create a render buffer object for depth attachment
	glGenRenderbuffers(1, &depthBuffer);
	glBindBuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
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

void init(){
	cloudmap_program = setUpAShader("shaders/normalmap_shader.vert", "shaders/cloudmap_shader.frag");
	if (!cloudmap_program)
	{
		cerr<<"Error setting up Shaders!";
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
		cerr<<"Error setting up skybox shaders!";
		exit(1);
	}

	fxaa_program = setUpAShader("shaders/quad.vert", "shaders/fxaa.frag");
	if (!fxaa_program)
	{
		cerr << "Error setting up FXAA shaders!";
		exit(1);
	}

	glGenVertexArrays(40, vao);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GLfloat aspect_ratio = (GLfloat) VIEW_SIZE_WIDTH / (GLfloat) VIEW_SIZE_HEIGHT;
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
	textureManager.LoadTexture2D("textures/earth_day_8k.jpg", "earthDay");
	textureManager.LoadTexture2D("textures/earth_night_8k.png", "earthNight");
	textureManager.LoadTexture2D("textures/earth_night_8k_blur.png", "earthNightBlur");
	textureManager.LoadTexture2D("textures/earth_specular.jpg", "earthSpecularMap");
	textureManager.LoadTexture2D("textures/earth_normalmap_8k.jpg", "earthNormalMap");
	textureManager.LoadTexture2D("textures/earth_clouds_8k.jpg", "earthClouds");
	textureManager.LoadTexture2D("textures/clouds_normalmap_8k.jpg", "earthCloudsNormalMap");
	textureManager.GenerateFBOTexture2D("sceneColorTexture", SCENE_WIDTH, SCENE_HEIGHT);
	textureManager.GenerateFBOTexture2D("sceneDepthTexture", SCENE_WIDTH, SCENE_HEIGHT, true);

	sceneFbo = generateFrameBufferObject(textureManager["sceneColorTexture"] , textureManager["sceneDepthTexture"]);

	map<string, string> textureHandles;

	curr_program = normalmap_program;
	
	textureHandles.clear();
	textureHandles.insert(make_pair("earthDay", "day"));
	textureHandles.insert(make_pair("earthNight", "night"));
	textureHandles.insert(make_pair("earthSpecularMap", "specMap"));
	textureHandles.insert(make_pair("earthNormalMap", "bumpMap"));
	textureHandles.insert(make_pair("earthClouds", "clouds"));

	s = new Sphere(vao[vao_index], 1.0f, 50, 50, 0.005f, 0.f);
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


	float cloudScale = sphereScale + 0.1;
	cs = new Sphere(vao[vao_index], 1.0f, 50, 50, 0.0065f, 0.f);
	cs->generateMesh();
	cs->setTextureHandles(textureHandles);
	cs->setScale(cloudScale, cloudScale, cloudScale);
	cs->setDiffuseColor(1.0f, 0.941f, 0.898f);
	cs->initBuffers(curr_program);

	vao_index++;
	curr_program = atmosphere_program;

	as = new Sphere(vao[vao_index], 1.0f, 50, 50, 0.0, 0.0);
	GLfloat atmosphereScale = sphereScale + 0.15f;
	as->setScale(atmosphereScale, atmosphereScale, atmosphereScale);
	as->generateMesh();
	as->initBuffers(curr_program);

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
	glUniform4fv(glGetUniformLocation(curr_program, "lightPosition"), 1, glm::value_ptr(lightPosition));
	glUniform4fv(glGetUniformLocation(curr_program, "lightColor"), 1, glm::value_ptr(lightColor * glm::vec4(0.75, 0.75, 0.75, 1.0)));
	glUniform1f(glGetUniformLocation(curr_program, "yRotation"), diffRotation.y);

	cs->render(curr_program, textureManager);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

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

	curr_program = fxaa_program;
	glUseProgram(curr_program);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*
	if (fullScreen)
	{
		const Screen screen = getScreenDimensions();
		glViewport(0, 0, screen.width, screen.height);
	}
	else
	{
		glViewport(0, 0, VIEW_SIZE_WIDTH, VIEW_SIZE_HEIGHT);
	}
	*/
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	
	glUniform2f(glGetUniformLocation(curr_program, "InverseFBOScreenRatio"), 1.f / SCENE_WIDTH, 1.f / SCENE_HEIGHT);

	textureManager.BindTexture2D("sceneColorTexture", "scene", curr_program);
	
	glBindVertexArray(vao[vao_index]);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	textureManager.unbindAllTextures();

	glutPostRedisplay();
	glutSwapBuffers();
}

string getScreenShotFileName(){
	time_t t = time(0);
	struct tm* now = localtime(&t);
	string underscore = string("_");
	return string("screenshots/Earth") + underscore + to_string(now->tm_year + 1900) + underscore +
		(now->tm_mon < 10 ? (string("0") + to_string(now->tm_mon)) : to_string(now->tm_mon)) + 
		underscore + (now->tm_mday < 10 ? (string("0") + to_string(now->tm_mday)) : to_string(now->tm_mday)) + 
		underscore + (now->tm_hour < 10 ? (string("0") + to_string(now->tm_hour)) : to_string(now->tm_hour)) +
		underscore + (now->tm_min < 10 ? (string("0") + to_string(now->tm_min)) : to_string(now->tm_min)) +
		underscore + (now->tm_sec < 10 ? (string("0") + to_string(now->tm_sec)) : to_string(now->tm_sec)) +
		string(".bmp");

}

void keyboard(unsigned char key, int x, int y){

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
void mouseMove(int x, int y){
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

int main(int argc, char** argv){

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
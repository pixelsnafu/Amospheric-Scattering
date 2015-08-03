#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/GL.h>
#include <iostream>
#include <vector>
#include <ctime>

#include "camera.h"
#include "shaderProgram.h"
#include "Vec3.h"
#include "Sphere.h"
#include "SkyBox.h"

using namespace std;

int VIEW_SIZE_WIDTH = 1024;
int VIEW_SIZE_HEIGHT = 768;

#define FULLSCREEN_WIDTH GetSystemMetrics(SM_CXSCREEN)
#define FULLSCREEN_HEIGHT GetSystemMetrics(SM_CYSCREEN)

const float FULLSCREEN_ASPECT_RATIO = (float)FULLSCREEN_WIDTH/(float)FULLSCREEN_HEIGHT;
const float VIEW_ASPECT_RATIO = (float)VIEW_SIZE_WIDTH/(float)VIEW_SIZE_HEIGHT;

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
GLuint vertexAttribute, texAttribute;
GLuint program, skybox_program, normalmap_program, curr_program;
GLuint vao_index = 0;

//camera variables
glm::vec3 up_vector(0, 1, 0);
glm::vec3 camera_position(0.0, 0.0, 12.0);
glm::vec3 camera_lookat(0.0, 0.0, 0.0);

glm::mat4 scale(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f)));
float rotation[3] = {0.0, 0.0, 0.0};
float sphereScale = 4.0f;

glm::vec4 lightColor(1.25, 1.25, 1.25, 1.0);
glm::vec4 lightPosition(100.0, 0.0, 100.0, 1.0);
glm::vec4 cloudLightPosition(50.0, 0.0, 50.0, 1.0);

//quad variables
// x,y vertex positions
float quad_vertices[] = {
	-0.5, -0.5, 0.0,
	0.5, -0.5, 0.0,
	0.5,  0.5, 0.0,
	0.5,  0.5, 0.0,
	-0.5,  0.5, 0.0,
	-0.5, -0.5, 0.0
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

Sphere* s, *cs;
Skybox* skybox;

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

	++index;
}

void init(){
	program = setUpAShader("shaders/normalmap_shader.vert", "shaders/shader.frag");
	if(!program){
		cerr<<"Error setting up Shaders!";
		exit(1);
	}

	normalmap_program = setUpAShader("shaders/normalmap_shader.vert", "shaders/normalmap_shader.frag");
	if (!normalmap_program){
		cerr << "Error Setting Up Normal Map Shaders!" << endl;
		exit(1);
	}

	skybox_program = setUpAShader("shaders/skybox.vert", "shaders/skybox.frag");
	if(!program){
		cerr<<"Error setting up Shaders!";
		exit(1);
	}

	glGenVertexArrays(40, vao);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GLfloat aspect_ratio = (GLfloat) VIEW_SIZE_WIDTH / (GLfloat) VIEW_SIZE_HEIGHT;
	cam = new camera(camera_position, camera_lookat, up_vector, l, r, t, b, n, f, aspect_ratio);
	
	
	curr_program = normalmap_program;

	//initBuffers(vao_index, program);
	s = new Sphere(vao_index, 1.0f, 50, 50, 0.005f, 0.f);
	s->generateMesh();
	s->loadTexture("textures/earth_day_8k.jpg", "mySampler");
	s->loadTexture("textures/earth_night_8k.jpg", "night");
	s->loadTexture("textures/earth_specular.jpg", "specMap");
	s->loadTexture("textures/earth_normalmap_8k.jpg", "bumpMap");
	s->setScale(sphereScale, sphereScale, sphereScale);
	s->setDiffuseColor(1.0f, 0.941f, 0.898f);
	s->initBuffers(curr_program);

	vao_index++;
	curr_program = program;

	float cloudScale = sphereScale + 0.075;
	cs = new Sphere(vao_index, 1.0f, 50, 50, 0.0065f, 0.f);
	cs->generateMesh();
	cs->loadTexture("textures/clouds_normalmap_8k.jpg", "cloudBumpMap");
	cs->loadTexture("textures/earth_clouds_8k.jpg", "clouds");
	cs->setScale(cloudScale, cloudScale, cloudScale);
	cs->setDiffuseColor(1.0f, 0.941f, 0.898f);
	cs->initBuffers(curr_program);

	vao_index++;
	curr_program = skybox_program;

	vector<const GLchar*> faces;
	faces.push_back("skybox/right.png");
	faces.push_back("skybox/left.png");
	faces.push_back("skybox/top.png");
	faces.push_back("skybox/bottom.png");
	faces.push_back("skybox/front.png");
	faces.push_back("skybox/back.png");


	skybox = new Skybox(vao_index, faces);
	skybox->generateMesh();
	skybox->setScale(5.0, 5.0, 5.0);
	skybox->setDiffuseColor(0.0, 0.0, 0.5);
	skybox->loadTextures("skybox");
	skybox->enableCubemap();
	skybox->initBuffers(curr_program);
}

void render(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	curr_program = normalmap_program;
	glUseProgram(curr_program);
	cam->setupCamera(curr_program);

	glUniform4fv(glGetUniformLocation(curr_program, "lightPosition"), 1, glm::value_ptr(lightPosition));
	glUniform4fv(glGetUniformLocation(curr_program, "lightColor"), 1, glm::value_ptr(lightColor));

	
	s->render(curr_program);
	s->animate();

	curr_program = program;
	glUseProgram(curr_program);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDisable(GL_DEPTH_TEST);

	cam->setupCamera(curr_program); 
	glUniform4fv(glGetUniformLocation(curr_program, "lightPosition"), 1, glm::value_ptr(lightPosition));
	glUniform4fv(glGetUniformLocation(curr_program, "lightColor"), 1, glm::value_ptr(lightColor * glm::vec4(0.75, 0.75, 0.75, 1.0)));

	cs->render(curr_program);
	cs->animate();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	//rendering the skybox
	glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content
	glUseProgram(skybox_program);
	//cast out the translation from the view matrix of the camera, to make the skybox appear at infinity
	glm::mat4 view = glm::mat4(glm::mat3(cam->getCameraViewMatrix()));
	glm::mat4 projection = cam->getCameraProjectionMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	skybox->render(skybox_program);
	glDepthFunc(GL_LESS); // Set the depth function to default after rendering the skybox

	glutPostRedisplay();
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y){

	switch(key){
	case 27:
		glutLeaveMainLoop();
		exit(0);
		break;

	case 'D':
	case 'd':
		cam->moveCameraRight();
		cam->updateCamera(curr_program);
		if (glm::length(cam->getCamPosition()) <= s->getRadius() * 4.4f){
			cam->setCameraPosition(s->getRadius() * 4.4f * glm::normalize(cam->getCamPosition()));
		}
		break;

	case 'a':
	case 'A':
		cam->moveCameraLeft();
		cam->updateCamera(curr_program);
		if (glm::length(cam->getCamPosition()) <= s->getRadius() * 4.4f){
			cam->setCameraPosition(s->getRadius() * 4.4f * glm::normalize(cam->getCamPosition()));
		}
		break;

	case 'w':
	case 'W':
		cam->moveCameraForward();
		cam->updateCamera(curr_program);
		if (glm::length(cam->getCamPosition()) <= s->getRadius() * 4.4f){
			cam->setCameraPosition(s->getRadius() * 4.4f * glm::normalize(cam->getCamPosition()));
		}
		break;

	case 's':
	case 'S':
		cam->moveCameraBackward();
		cam->updateCamera(curr_program);
		if (glm::length(cam->getCamPosition()) <= s->getRadius() * 4.4f){
			cam->setCameraPosition(s->getRadius() * 4.4f * glm::normalize(cam->getCamPosition()));
		}
		break;
	case 'f':
		if(fullScreen){
			fullScreen = false;
			cam->setAspectRatio(VIEW_ASPECT_RATIO);
			cam->setupCamera(curr_program);
			glutReshapeWindow(VIEW_SIZE_WIDTH, VIEW_SIZE_HEIGHT);
		}else{
			fullScreen = true;
			cam->setAspectRatio(FULLSCREEN_ASPECT_RATIO);
			cam->setupCamera(curr_program);
			glutFullScreen();
		}
		break;
	case 'p':
		if (fullScreen)
			SOIL_save_screenshot("screenshots/screenshot.bmp", SOIL_SAVE_TYPE_BMP, 0, 0, 1360, FULLSCREEN_HEIGHT);
		else
			SOIL_save_screenshot("screenshots/screenshot.bmp", SOIL_SAVE_TYPE_BMP, 0, 0, VIEW_SIZE_WIDTH, VIEW_SIZE_HEIGHT);
		break;
	}


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
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
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

	//uncomment/comment this to enable/disable camera mouse-movement.
	glutPassiveMotionFunc(mouseMove);

	//comment this to show cursor in the window.
	glutSetCursor(GLUT_CURSOR_NONE);

	glutMainLoop();

	return 0;

}
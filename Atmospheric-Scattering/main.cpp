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

#define VIEW_SIZE_WIDTH 1024
#define VIEW_SIZE_HEIGHT 768

#define FULLSCREEN_WIDTH GetSystemMetrics(SM_CXSCREEN)
#define FULLSCREEN_HEIGHT GetSystemMetrics(SM_CYSCREEN)

const float FULLSCREEN_ASPECT_RATIO = (float)FULLSCREEN_WIDTH/(float)FULLSCREEN_HEIGHT;
const float VIEW_ASPECT_RATIO = (float)VIEW_SIZE_WIDTH/(float)VIEW_SIZE_HEIGHT;

//camera variables
int prevX = -1, prevY = -1;
camera* cam;
bool fullScreen = false;

float l = -0.25;
float r = 0.25;
float t = 0.25;
float b = -0.25;
float n = 0.5;
float f = 10000.5;

//Buffer/shader variables
GLuint vao[40];
GLuint vbo[2];
GLuint vertexAttribute, texAttribute;
GLuint program, skybox_program;
GLuint vao_index = 0;

//camera variables
glm::vec3 up_vector(0, 1, 0);
glm::vec3 camera_position(0.0, 0.0, 12.0);
glm::vec3 camera_lookat(0.0, 0.0, 0.0);

glm::mat4 scale(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f)));
float rotation[3] = {0.0, 0.0, 0.0};

glm::vec4 lightColor(1.25, 1.25, 1.25, 1.0);
glm::vec4 lightPosition(0.0, 2.0, 12.0, 1.0);

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

Sphere* s;
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
	program = setUpAShader("shaders/shader.vert", "shaders/shader.frag");
	if(!program){
		cerr<<"Error setting up Shaders!";
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

	
	//initBuffers(vao_index, program);
	s = new Sphere(vao_index, 1.0f, 30, 30, 0.01f, 0.f);
	s->generateMesh();
	s->loadTexture("textures/earth_day_large.jpg", "mySampler");
	s->loadTexture("textures/earth_night_large.jpg", "night");
	s->loadTexture("textures/earth_clouds_large.jpg", "clouds");
	s->loadTexture("textures/earth_specular.jpg", "specMap");
	s->setScale(4.0, 4.0, 4.0);
	s->setDiffuseColor(1.0f, 0.941f, 0.898f);
	s->initBuffers(program);

	vao_index++;

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
	skybox->initBuffers(skybox_program);
}

void render(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cam->setupCamera(program);

	glUniform4fv(glGetUniformLocation(program, "lightPosition"), 1, glm::value_ptr(lightPosition));
	glUniform4fv(glGetUniformLocation(program, "lightColor"), 1, glm::value_ptr(lightColor));

	
	s->render(program);
	s->animate();

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
		cam->updateCamera(program);
		break;

	case 'a':
	case 'A':
		cam->moveCameraLeft();
		cam->updateCamera(program);
		break;

	case 'w':
	case 'W':
		cam->moveCameraForward();
		cam->updateCamera(program);
		break;

	case 's':
	case 'S':
		cam->moveCameraBackward();
		cam->updateCamera(program);
		break;
	case 'f':
		if(fullScreen){
			fullScreen = false;
			cam->setAspectRatio(VIEW_ASPECT_RATIO);
			cam->setupCamera(program);
			glutReshapeWindow(VIEW_SIZE_WIDTH, VIEW_SIZE_HEIGHT);
		}else{
			fullScreen = true;
			cam->setAspectRatio(FULLSCREEN_ASPECT_RATIO);
			cam->setupCamera(program);
			glutFullScreen();
		}
		break;
	case 'p':
		SOIL_save_screenshot("screenshots/screenshot.bmp", SOIL_SAVE_TYPE_BMP, 0, 0, 1024, 768);
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
	cam->setupCamera(program);
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
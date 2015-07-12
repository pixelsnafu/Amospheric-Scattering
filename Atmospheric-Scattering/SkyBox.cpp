#include "SkyBox.h"
#include "Vec3.h"

void Skybox::generateMesh(){
	
	//far plane

	Vec3f v1(-1.0f, 1.0f, -1.0f);
	Vec3f v2(-1.0f, -1.0f, -1.0f);
	Vec3f v3(1.0f, -1.0f, -1.0f);
	Vec3f v4(1.0f, 1.0f, -1.0f);

	addTriangle(v1, v2, v3);
	addTriangle(v3, v4, v1);

	//left plane

	v1 = Vec3f(-1.0f, -1.0f, 1.0f);
	v2 = Vec3f(-1.0f, -1.0f, -1.0f);
	v3 = Vec3f(-1.0f, 1.0f, -1.0f);
	v4 = Vec3f(-1.0f, 1.0f, 1.0f);

	addTriangle(v1, v2, v3);
	addTriangle(v3, v4, v1);

	//right plane

	v1 = Vec3f(1.0f, -1.0f, -1.0f);
	v2 = Vec3f(1.0f, -1.0f, 1.0f);
	v3 = Vec3f(1.0f, 1.0f, 1.0f);
	v4 = Vec3f(1.0f, 1.0f, -1.0f);

	addTriangle(v1, v2, v3);
	addTriangle(v3, v4, v1);
	

	//near plane

	v1 = Vec3f(-1.0f, -1.0f, 1.0f);
	v2 = Vec3f(-1.0f, 1.0f, 1.0f);
	v3 = Vec3f(1.0f, 1.0f, 1.0f);
	v4 = Vec3f(1.0f, -1.0f, 1.0f);

	addTriangle(v1, v2, v3);
	addTriangle(v3, v4, v1);

	//top plane
	
	v1 = Vec3f(-1.0f,  1.0f, -1.0f);
    v2 = Vec3f(1.0f,  1.0f, -1.0f);
    v3 = Vec3f(1.0f,  1.0f,  1.0f);
	v4 = Vec3f(-1.0f, 1.0f, 1.0f);

	addTriangle(v1, v2, v3);
	addTriangle(v3, v4, v1);

	//bottom plane

	v1 = Vec3f(-1.0f, -1.0f, -1.0f);
    v2 = Vec3f(-1.0f, -1.0f,  1.0f);
    v3 = Vec3f(1.0f, -1.0f, -1.0f);
	v4 = Vec3f(1.0f, -1.0f,  1.0f);

	addTriangle(v1, v2, v3);
	addTriangle(v3, v2, v4);
}

void Skybox::loadTextures(const GLchar* sampler){

	GLuint texID;
	glGenTextures(1, &texID);
	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
	for(GLuint i = 0; i < textureFaces.size(); i++){
		image = SOIL_load_image(textureFaces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	texIDs.push_back(texID);
	samplers.push_back(sampler);
}
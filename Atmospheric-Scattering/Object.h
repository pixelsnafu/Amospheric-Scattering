#pragma once

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <gl/glew.h>
#include <gl/GL.h>
#include <SOIL.h>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <fstream>
#include <cmath>

#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Vec3.h"
#include "camera.h"

/**
General Object abstract class to calculate vertex, normal and texture coordinates,
pass them into a buffer and render. 
*/

using namespace std;

class Object{
protected:
	float* scale;
	float* translate;
	float* angle;
	float* diffuse;

	//vectors of 3d vectors to store set of points
	vector<Vec3f> points;
	vector<Vec3f> normals;
	vector<Vec3f> tangents;
	vector<Vec3f> binormals;
	vector<Vec3f> uv;
	vector<face*> faces;

	//a map<vertex, vector<normals>> to store the normals which are shared by each vertex
	map<Vec3f, vector<Vec3f>, Vec3Comp> sharedNormals;
	map<Vec3f, vector<Vec3f>, Vec3Comp> sharedTangents;

	GLuint vao;
	GLuint vbo[10];
	//GLuint texID;
	vector<int> texIDs;
	vector<const GLchar*> samplers;

	//array buffers for the final buffer outputs
	float* outVertices;
	float* outNormals;
	float* outTangents;
	float* outBiNormals;
	float* outVertexNormals;
	float* outVertexTangents;
	float* outVertexBiNormals;
	float* outUV;

	//normal and light flags
	bool smooth;
	bool lightSwitch;
	bool cubemap;

public:

	//constructor
	Object(GLuint vao);
	//destructor
	virtual ~Object();
	//function to init the buffers for the 3d object
	void initBuffers(const GLuint& program);
	//render the 3d object
	void render(const GLuint& program);
	//add a triangle to the collection of vertices, calculate its normals and texture coordinates if need be
	void addTriangle(Vec3f v1, Vec3f v2, Vec3f v3);
	//a helper add triangle function
	void addTriangle(Vec3f v1, Vec3f u1, Vec3f v2, Vec3f u2, Vec3f v3, Vec3f u3);
	//function to calculate vertex normals
	virtual void smoothNormals();
	//function to load texture file into opengl framework
	void loadTexture(const char* filename, const GLchar* sampler);
	//used by skybox to load multiple textures
	virtual void loadTextures(const GLchar* sampler){}
	//set rotation of the object
	void setRotation(float x, float y, float z);
	//set the size of the current object
	void setScale(float x, float y, float z);
	//set the position of the object
	virtual void setPosition(float x, float y, float z);
	//return the position of the object
	Vec3f getPosition();
	//set the diffuse color of the object
	void setDiffuseColor(float x, float y, float z);
	//functions to enable/disable lighting on the object
	void enableLighting();
	void disableLighting();
	//functions to enable/disable cubemap texture (in case of a skybox object)
	void enableCubemap();
	void disableCubemap();

	void setSmoothShading(const bool& smooth);
	//function to animate the object
	virtual void animate(){}
	//function to generate the mesh coordinates (uses addTriangle function in the base class)
	virtual void generateMesh() = 0;
};

#endif
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
#include "TextureManager.h"

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
	map<Vec3f, Vec3f, Vec3Comp> sharedTangents;

	GLuint vao;
	GLuint vbo[10];
	//GLuint texID;
	map<string, string> m_textureHandles;

	//array buffers for the final buffer outputs
	float* outVertices;
	float* outNormals;
	float* outTangents;
	float* outVertexNormals;
	float* outVertexTangents;
	float* outUV;

	//normal and light flags
	bool smooth;
	bool lightSwitch;
	bool cubemap;

public:

	Object(GLuint vao);
	virtual ~Object();

	void initBuffers(const GLuint& program);

	void render(const GLuint& program, TextureManager& textureManager);

	void addTriangle(Vec3f v1, Vec3f v2, Vec3f v3);
	void addTriangle(Vec3f v1, Vec3f u1, Vec3f v2, Vec3f u2, Vec3f v3, Vec3f u3);

	virtual void calculateVertexNormals();
	virtual void calculateVertexTangents();

	void setRotation(float x, float y, float z);
	void setScale(float x, float y, float z);
	virtual void setPosition(float x, float y, float z);

	Vec3f getPosition() const;
	Vec3f getSize() const;
	glm::mat4 getRotationMatrix() const;
	glm::vec3 getRotation() const;

	void setDiffuseColor(float x, float y, float z);
	void setTextureHandles(map<string, string>);

	void enableLighting();
	void disableLighting();
	
	void enableCubemap();
	void disableCubemap();

	void setSmoothShading(const bool& smooth);
	virtual void animate(){}
	
	virtual void generateMesh() = 0;
};

#endif
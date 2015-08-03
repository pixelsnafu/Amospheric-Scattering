#pragma once

#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "Object.h"

/**
Derived Class Sphere from Object containing the mesh calculation of a sphere,
using latitutde and longitude
*/

class Sphere : public Object{
	float radius;
	int slices, stacks;
	float sunDistance;
	float theta, rotSpeed, revSpeed;

public:
	//c'tor
	Sphere(GLuint vao, float radius, int slices, int stacks, float rotSpeed, float revSpeed);
	//d'tor
	~Sphere();
	//function to calculate the mesh coordinates
	void generateMesh();
	//function to set the position of the sphere
	void setPosition(float x, float y, float z);
	//function to calculate vertex normals of the sphere (special case)
	virtual void calculateVertexNormals();
	//function to animate the sphere
	void animate();

	float getRadius();
};

#endif
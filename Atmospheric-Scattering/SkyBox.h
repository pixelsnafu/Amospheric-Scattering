#pragma once

#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include <vector>
#include "Object.h"

using namespace std;

class Skybox: public Object{
	vector<string> textureFaces;
public:
	Skybox(GLuint vao): Object(vao){}
	Skybox(GLuint vao, vector<string> textureFaces) : Object(vao) { this->textureFaces = textureFaces; }
	void generateMesh();
};

#endif
#pragma once

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/GL.h>
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\type_ptr.hpp>



#define PI 3.14159
#define MOVE_FACTOR 0.1f

using namespace std; 

/**
Custom Camera Class
*/
class camera{
    float l, r, t, b, n, f;
	glm::vec3 eye, lookAt, up;
	glm::mat4 view, projection;
    int theta, phi;
    float aspectRatio;

public:
    
	//constructor
	camera(glm::vec3 eye, glm::vec3 lookAt, glm::vec3 up, float l, float r, float t, float b, float n, float f, float aspectRatio){

		this->eye = eye;
		this->lookAt = lookAt;
		this->up = up;

        this->l = l * aspectRatio;
        this->r = r * aspectRatio;
        this->t = t;
        this->b = b;
        this->n = n;
        this->f = f;
        this->aspectRatio = aspectRatio;

        theta = 0;
        phi = 0;
    }

	//setup all the camera variable by sending uniform values to the vertex shader
    void setupCamera(GLuint program){

		glUseProgram(program);

		view = glm::lookAt(eye, lookAt, up);
		projection = glm::frustum(l, r, b, t, n, f);
		
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, false, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, false, glm::value_ptr(projection));
    }

	//update cmaera lookat position in the shader
	void updateCamera(GLuint program){
		glUseProgram(program);
		view = glm::lookAt(eye, lookAt, up);
		glUniformMatrix4fv(glGetUniformLocation(program, "viewMatrix"), 1, false, glm::value_ptr(view));
	}

	glm::mat4 getCameraViewMatrix(){
		return view;
	}

	glm::mat4 getCameraProjectionMatrix(){
		return projection;
	}

    void moveCameraForward(){

		glm::vec3 moveDirection = lookAt - eye;
		moveDirection = glm::normalize(moveDirection);

		eye += moveDirection * MOVE_FACTOR;
		lookAt += moveDirection * MOVE_FACTOR;
    }

    void moveCameraBackward(){
		
		glm::vec3 moveDirection = lookAt - eye;
		moveDirection = glm::normalize(moveDirection);

		eye -= moveDirection * MOVE_FACTOR;
		lookAt -= moveDirection * MOVE_FACTOR;
    }

    void moveCameraLeft(){

		glm::vec3 viewDirection = lookAt - eye;
		glm::vec3 moveDirection = glm::cross(viewDirection, up);
		moveDirection = glm::normalize(moveDirection);

		eye -= moveDirection * MOVE_FACTOR;
		lookAt -= moveDirection * MOVE_FACTOR;
    }

	
    void moveCameraRight(){

		glm::vec3 lookDirection = lookAt - eye;
		glm::vec3 moveDirection = glm::cross(lookDirection, up);
		moveDirection = glm::normalize(moveDirection);

		eye += moveDirection * MOVE_FACTOR;
		lookAt += moveDirection * MOVE_FACTOR;
    }

    void cameraUpdateRotX(int x){
        theta += x;
        float radius = sqrt(pow(eye.x - lookAt.x, 2) + pow(eye.z - lookAt.z, 2));
        float angle = theta*2*PI/360;
		lookAt.x = eye.x + radius*cos(angle);
        lookAt.z = eye.z + radius*sin(angle);
    }

    void cameraUpdateRotY(int y){
		
		phi -= y;
        if(phi > -10)
            phi = -10;
        if(phi <= -170)
            phi = -170;
        float radius = sqrt(pow(eye.y - lookAt.y, 2) + pow(eye.z - lookAt.z, 2));
        float angle = phi*PI/180;
        lookAt.y = eye.y + radius*cos(angle);
		
    }



	//update camera eye position
	void setCameraPosition(const glm::vec3 eye){
		this->eye = eye;
    }

	
	//update camera lookAt position
    void setCameraLookAtPosition(float x, float y, float z){
        lookAt.x = x;
		lookAt.y = y;
		lookAt.z = z;
    }

	//update the camera lookAt position
	void setCameraLookAtPosition(glm::vec3 position){
		lookAt = position;
	}
	//change the aspect ratio of the display camera
    void setAspectRatio(float aspectRatio){
        l = l/this->aspectRatio;
        r = r/this->aspectRatio;

        l *= aspectRatio;
        r *= aspectRatio;

        this->aspectRatio = aspectRatio;
    }

	
	//return the camera position array
    glm::vec3 getCamPosition(){
        return eye;
    }
};

#endif
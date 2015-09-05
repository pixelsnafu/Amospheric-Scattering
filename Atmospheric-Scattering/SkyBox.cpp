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
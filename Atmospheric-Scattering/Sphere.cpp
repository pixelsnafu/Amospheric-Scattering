#include "Sphere.h"

//constructor to initialize the 
Sphere::Sphere(GLuint vao, float radius, int slices, int stacks, float rotSpeed, float revSpeed) : Object(vao){
	this->radius = radius;
	this->slices = slices;
	this->stacks = stacks;

	Vec3f origin(0);
	Vec3f position(translate[0], translate[1], translate[2]);

	this->sunDistance = vDistance(origin, position);

	this->theta = 0;
	this->rotSpeed = rotSpeed;
	this->revSpeed = revSpeed;
}

Sphere::~Sphere(){}

//function to set position of the sphere
void Sphere::setPosition(float x, float y, float z){
	this->translate[0] = x;
	this->translate[1] = y;
	this->translate[2] = z;

	Vec3f origin(0);
	Vec3f position(translate[0], translate[1], translate[2]);

	//calculate distance of the planet from the sun
	sunDistance = vDistance(origin, position);
	//calculate the current theta on the x-z plane of rotation (used for storing the current position
	//in case of a pause).
	theta = atan2(z, x);
}

//sphere tessellation function (nothing new here)
void Sphere::generateMesh(){
	
	float deltaPhi = (float)PI/stacks;
	float deltaTheta = (2 * (float)PI)/slices;

	for(float phi = 0; phi < PI ; phi += deltaPhi){
		for(float theta = 0; theta < 2*PI - 0.001; theta += deltaTheta){

			float x1 = -sinf(phi) * sinf(theta) * radius;
			float y1 = -cosf(phi) * radius;
			float z1 = -sinf(phi) * cosf(theta) * radius;

			float u1 = float( atan2(x1, z1) / (2 * PI) + 0.5 );
			float v1 = float( -asin(y1) / (float)PI + 0.5 );

			float x2 = -sinf(theta + deltaTheta) * sinf(phi) * radius;
			float y2 = -cosf(phi) * radius;
			float z2 = -sinf(phi) * cosf(theta + deltaTheta) * radius;

			float u2 = float( atan2(x2, z2) / (2 * PI) + 0.5 );
			float v2 = float( -asin(y2) / ((float)PI) + 0.5 );

			float x3 = -sinf(theta + deltaTheta) * sinf(phi + deltaPhi) * radius;
			float y3 = -cosf(phi + deltaPhi) * radius;
			float z3 = -sinf(phi + deltaPhi) * cosf(theta + deltaTheta) * radius;

			float u3 = float( atan2(x3, z3) / (2 * (float)PI) + 0.5 );
			float v3 = float( -asin(y3) / (float)PI + 0.5 );

			float x4 = -sinf(theta) * sinf(phi + deltaPhi) * radius;
			float y4 = -cosf(phi + deltaPhi) * radius;
			float z4 = -sinf(phi + deltaPhi) * cosf(theta) * radius;

			float u4 = float( atan2(x4, z4) / (2 * (float)PI) + 0.5 );
			float v4 = float( -asin(y4) / (float)PI + 0.5 );
		

			Vec3f p1(x1, y1, z1);
			Vec3f uv1(u1, v1, 0);
			Vec3f p2(x2, y2, z2);
			Vec3f uv2(u2, v2, 0);
			Vec3f p3(x3, y3, z3);
			Vec3f uv3(u3, v3, 0);
			Vec3f p4(x4, y4, z4);
			Vec3f uv4(u4, v4, 0);

			//addTriangle(x1, y1, z1, u1, v1,
			//	x2, y2, z2, u2, v2,
			//	x3, y3, z3, u3, v3);

			//addTriangle(x1, y1, z1, u1, v1,
			//	x3, y3, z3, u3, v3,
			//	x4, y4, z4, u4, v4); 

			addTriangle(p1, uv1, p2, uv2, p3, uv3);
			addTriangle(p1, uv1, p3, uv3, p4, uv4);
		}
	}
}

//function to calculate the smooth normals for a sphere
void Sphere::calculateVertexNormals(){
	vector<Vec3f> vertexNormalList;
	Vec3f center(0, 0, 0);

	for(unsigned i = 0; i < points.size(); i++){
		Vec3f vn = points.at(i) - center;
		vn.normalize();
		vertexNormalList.push_back(vn);
	}

	outVertexNormals = new float[faces.size() * 3 * 3];
	Vec3f vn;
	int index = 0;
	for(unsigned i = 0; i < faces.size(); i++){
		// Because I overloaded operator[] on face to easily access indices stored in it.
		for(unsigned j = 0; j < 3; j++){
			vn = vertexNormalList.at((*faces.at(i))[j]);
			outVertexNormals[index] = vn.x;
			outVertexNormals[index + 1] = vn.y;
			outVertexNormals[index + 2] = vn.z;

			index += 3;
		}
	}
}


//function to animate the sphere
void Sphere::animate(){

	if(theta < 2 * PI)
		theta += revSpeed;
	else
		theta -= 2 * PI;
	this->translate[0] = sunDistance * cos(theta);
	this->translate[2] = sunDistance * sin(theta);
	this->angle[1] += rotSpeed;
}

float Sphere::getRadius(){
	return radius;
}
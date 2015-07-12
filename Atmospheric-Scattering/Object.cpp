#include "Object.h"
#include "camera.h"

//constructor to initialize all arrays and pointer objects
Object::Object(GLuint vao){
	this->vao = vao;
	this->angle = new float[3]();
	this->scale = new float[3]();
	this->translate = new float[3]();
	this->diffuse = new float[4]();
	this->smooth = false;

	for(int i = 0; i < 3; i++){
		scale[i] = 1.0f;
		diffuse[i] = 1.0f;
	}

	diffuse[3] = 1.0f;
	//lighting is enabled by default
	lightSwitch = true;
	cubemap = false;
}

//destructor to delete all pointers and arrays
Object::~Object(){
	delete [] angle;
	delete [] scale;
	delete [] translate;
	delete [] outVertices;
	delete [] outNormals;
	delete [] outUV;
	delete [] outVertexNormals;
}

void Object::setScale(float x, float y, float z){
	this->scale[0] = x;
	this->scale[1] = y;
	this->scale[2] = z;
}

void Object::setPosition(float x, float y, float z){
	this->translate[0] = x;
	this->translate[1] = y;
	this->translate[2] = z;
}

Vec3f Object::getPosition(){
	return Vec3f(translate[0], translate[1], translate[2]);
}

void Object::setRotation(float x, float y, float z){
	this->angle[0] = x;
	this->angle[1] = y;
	this->angle[2] = z;
}

void Object::setDiffuseColor(float x, float y, float z){
	this->diffuse[0] = x;
	this->diffuse[1] = y;
	this->diffuse[2] = z;
	this->diffuse[3] = 1.0;
}

void Object::enableLighting(){
	lightSwitch = 1;
}

void Object::disableLighting(){
	lightSwitch = 0;
}

void Object::enableCubemap(){
	cubemap = true;
}

void Object::disableCubemap(){
	cubemap = false;
}


void Object::addTriangle(Vec3f v1, Vec3f v2, Vec3f v3){

	//adds the three vertices to the points vector if they don't exist there already
	if(find(points.begin(), points.end(), v1) == points.end())
		points.push_back(v1);

	if(find(points.begin(), points.end(), v2) == points.end())
		points.push_back(v2);

	if(find(points.begin(), points.end(), v3) == points.end())
		points.push_back(v3);

	//calculate the index of the vertices in the points vector
	GLuint index1 = find(points.begin(), points.end(), v1) - points.begin();
	GLuint index2 = find(points.begin(), points.end(), v2) - points.begin();
	GLuint index3 = find(points.begin(), points.end(), v3) - points.begin();


	//calculate the face normal
	Vec3f n1 = v2 - v1;
	Vec3f n2 = v3 - v1;
	Vec3f n = cross(n1, n2);

	//insert normal in the normals vector if not there already
	if(find(normals.begin(), normals.end(), n) == normals.end())
		normals.push_back(n);


	//calculate the normal index
	GLuint normalIndex = find(normals.begin(), normals.end(), n) - normals.begin();

	//make a face with three vertex indices and a normal index
	face* f = new face(index1, index2, index3, normalIndex);
	faces.push_back(f);

	vector<Vec3f> triangleVertices;
	triangleVertices.push_back(v1);
	triangleVertices.push_back(v2);
	triangleVertices.push_back(v3);

	//insert the normal index value for all the three vertex keys in the map
	for(unsigned i = 0; i < triangleVertices.size(); i++){
		Vec3f vertex = triangleVertices.at(i);
		if(sharedNormals.find(vertex) == sharedNormals.end()){
			vector<Vec3f> temp;
			temp.push_back(n);
			sharedNormals.insert(pair<Vec3f, vector<Vec3f>>(vertex, temp));
		}else{
			auto& normalList = sharedNormals.at(vertex);
			normalList.push_back(n);
		}
	}
}

//helper addtriangle function to insert texture coordinates too
void Object::addTriangle(Vec3f v1, Vec3f u1, Vec3f v2, Vec3f u2, Vec3f v3, Vec3f u3){
	//add triangle normally
	addTriangle(v1, v2, v3);

	//TODO integrate uv coordinates in face
	//add extra texture coordinates to the uv vector
	uv.push_back(u1);
	uv.push_back(u2);
	uv.push_back(u3);

	Vec3f e1 = v2 - v1;
	Vec3f e2 = v3 - v1;

	float deltaU1 = u2.x - u1.x;
	float deltaV1 = u2.y - u1.y;
	float deltaU2 = u3.x - u1.x;
	float deltaV2 = u3.y - u1.y;

	float f = 1.f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

	Vec3f tangent;
	tangent = f * (deltaV2 * e1 - deltaV1 * e2);
	
	vector<Vec3f> triangleVertices;
	triangleVertices.push_back(v1);
	triangleVertices.push_back(v2);
	triangleVertices.push_back(v3);

	for(unsigned i = 0; i < triangleVertices.size(); i++){
		Vec3f vertex = triangleVertices.at(i);
		if(sharedTangents.find(vertex) == sharedTangents.end()){
			vector<Vec3f> temp;
			temp.push_back(tangent);
			sharedTangents.insert(pair<Vec3f, vector<Vec3f>>(vertex, temp));
		}else{
			auto& tangentList = sharedTangents.at(vertex);
			tangentList.push_back(tangent);
		}
	}	
}

//function to calculate the vertex normals
void Object::smoothNormals(){

	smooth = true;
	vector<Vec3f> vertexNormalList;
	//find the mean of all the normals which are shared by the vertex,
	//and normalize and add them to the vertex normal vector
	for(int i = 0; i < points.size(); i++){
		vector<Vec3f> tempList = sharedNormals.at(points.at(i));
		Vec3f vn;
		for(int j = 0; j < tempList.size(); j++){
			vn += tempList.at(j);
		}

		vn /= tempList.size();
		vn.normalize();

		vertexNormalList.push_back(vn);
	}


	//calculate the output vertex normal buffer array
	outVertexNormals = new float[faces.size() * 3 * 3];
	Vec3f vn;
	int index = 0;
	for(int i = 0; i < faces.size(); i++){
		vn = vertexNormalList.at(faces.at(i)->a);
		outVertexNormals[index] = vn.x;
		outVertexNormals[index + 1] = vn.y;
		outVertexNormals[index + 2] = vn.z;

		index += 3;

		vn = vertexNormalList.at(faces.at(i)->b);
		outVertexNormals[index] = vn.x;
		outVertexNormals[index + 1] = vn.y;
		outVertexNormals[index + 2] = vn.z;

		index += 3;

		vn = vertexNormalList.at(faces.at(i)->c);
		outVertexNormals[index] = vn.x;
		outVertexNormals[index + 1] = vn.y;
		outVertexNormals[index + 2] = vn.z;

		index += 3;
	}
}

//function to load a texture in the shader
void Object::loadTexture(const char* filename, const GLchar* sampler){
	int texID;
	texID = SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
	if(texID == 0){
		cerr<<"SOIL error: "<<SOIL_last_result();
	}

	texIDs.push_back(texID);
	samplers.push_back(sampler);
	//glBindTexture(GL_TEXTURE_2D, texID);
}

//function to pass all the array buffers into the openGL vertex buffers
void Object::initBuffers(const GLuint& program){

	//get the attribute ids
	GLuint vertexAttributeID = glGetAttribLocation(program, "vPosition");
	GLuint normalAttributeID = glGetAttribLocation(program, "vNormal");
	GLuint uvAttributeID = glGetAttribLocation(program, "vTexCoord");

	outVertices = new float[faces.size() * 3 * 3];
	outNormals = new float[faces.size() * 3 * 3];
	Vec3f v;
	Vec3f n;
	int index = 0;
	for(int i = 0; i < faces.size(); i++){
		n = normals.at(faces.at(i)->n);

		for (unsigned j = 0; j < 3; j++){
			v = points.at((*faces.at(i))[j]);
			outVertices[index] = v.x;
			outVertices[index + 1] = v.y;
			outVertices[index + 2] = v.z;

			if(!smooth){
				outNormals[index] = n.x;
				outNormals[index + 1] = n.y;
				outNormals[index + 2] = n.z;
			}

			index += 3;
		}
	}

	glBindVertexArray(vao);

	glGenBuffers(3, vbo);

	//pas the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, faces.size() * 3 * 3 * sizeof(float), outVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexAttributeID);
	glVertexAttribPointer(vertexAttributeID, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//pass the normal buffers (face or vertex)
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	if(smooth)
		glBufferData(GL_ARRAY_BUFFER, faces.size() * 3 * 3 * sizeof(float), outVertexNormals, GL_STATIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, faces.size() * 3 * 3 * sizeof(float), outNormals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(normalAttributeID);
	glVertexAttribPointer(normalAttributeID, 3, GL_FLOAT, GL_FALSE, 0, 0);

	if(uv.size() > 0 && texIDs.size() > 0){

		outUV = new float[uv.size() * 2];
		index = 0;

		for(int i = 0; i < uv.size(); i++){
			outUV[index] = uv.at(i).x;
			outUV[index + 1] = uv.at(i).y;
			index += 2;
		}
		//pass the uv coord buffers
		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, uv.size() * 2 * sizeof(float), outUV, GL_STATIC_DRAW);
		glEnableVertexAttribArray(uvAttributeID);
		glVertexAttribPointer(uvAttributeID, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

}

//function to render stuff
void Object::render(const GLuint& program){

	//Note to self : very very important!!! Otherwise you won't see anything! 
	glUseProgram(program);


	//bind the current object's texture
	for(GLuint i = 0; i < texIDs.size(); i++){
		glActiveTexture(GL_TEXTURE0 + i);
		if (cubemap)
			glBindTexture(GL_TEXTURE_CUBE_MAP, texIDs[i]);
		else
			glBindTexture(GL_TEXTURE_2D, texIDs[i]);
	}

	if(samplers.size()){
		for(GLuint i = 0; i < samplers.size(); i++){
			glUniform1i(glGetUniformLocation(program, samplers[i]), i);
		}
	}
	//set the uniforms
	glUniform1i(glGetUniformLocation(program, "lightSwitch"), lightSwitch);
	glUniform4fv(glGetUniformLocation(program, "diffuseColor"), 1, diffuse);

	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale[0], scale[1], scale[2]));
	glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translate[0], translate[1], translate[2]));
	glm::mat4 rotationMatrix_X = glm::rotate(glm::mat4(1.0f), angle[0], glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotationMatrix_Y = glm::rotate(glm::mat4(1.0f), angle[1], glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotationMatrix_Z = glm::rotate(glm::mat4(1.0f), angle[2], glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 modelMatrix = scaleMatrix * translateMatrix * rotationMatrix_X * rotationMatrix_Y * rotationMatrix_Z;

	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, false, glm::value_ptr(modelMatrix));

	//bind the vao and draw the triangles
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, faces.size() * 3 * 3);
}



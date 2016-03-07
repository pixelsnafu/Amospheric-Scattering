#include "Object.h"
#include "camera.h"

//constructor to initialize all arrays and pointer objects
Object::Object(GLuint vao){
	this->vao = vao;
	this->angle = new float[3]();
	this->scale = new float[3]();
	this->translate = new float[3]();
	this->diffuse = new float[4]();

	// Object class calculates vertex normals by default.
	// Disable this flag to enable flat shading. 
	this->smooth = true;

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

Vec3f Object::getPosition() const
{
	return Vec3f(translate[0], translate[1], translate[2]);
}

Vec3f Object::getSize() const
{
	return Vec3f(scale[0], scale[1], scale[2]);
}

glm::vec3 Object::getRotation() const
{
	return glm::vec3(angle[0], angle[1], angle[2]);
}

glm::mat4 Object::getRotationMatrix() const
{
	glm::mat4 rotationMatrix_X = glm::rotate(glm::mat4(1.0f), angle[0], glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotationMatrix_Y = glm::rotate(glm::mat4(1.0f), angle[1], glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotationMatrix_Z = glm::rotate(glm::mat4(1.0f), angle[2], glm::vec3(0.0f, 0.0f, 1.0f));
	return rotationMatrix_X * rotationMatrix_Y * rotationMatrix_Z;
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

void Object::setTextureHandles(map <string, string> textureHandles)
{
	m_textureHandles = textureHandles;
}

void Object::enableCubemap(){
	cubemap = true;
}

void Object::disableCubemap(){
	cubemap = false;
}

void Object::setSmoothShading(const bool& smooth){
	this->smooth = smooth;
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

	auto normalIter = find(normals.begin(), normals.end(), n);
	//insert normal in the normals vector if not there already
	if (normalIter == normals.end()){
		normals.push_back(n);
		// because we want the index of the last added normal duh
		normalIter = normals.end() - 1;
	}


	//calculate the normal index
	GLuint normalIndex = normalIter - normals.begin();

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

	auto tangentIter = find(tangents.begin(), tangents.end(), tangent);

	// if tangent not in the vector, then add it and set iter to the
	// vector's tail. 
	if (tangentIter == tangents.end()){
		tangents.push_back(tangent);
		tangentIter = tangents.end() - 1;
	}

	GLuint tangentIndex = tangentIter - tangents.begin();

	face* face = faces.back();
	face->t = tangentIndex;

	vector<Vec3f> triangleVertices;
	triangleVertices.push_back(v1);
	triangleVertices.push_back(v2);
	triangleVertices.push_back(v3);
	
	for (unsigned i = 0; i < triangleVertices.size(); i++){
		Vec3f v = triangleVertices.at(i);
		if (sharedTangents.find(v) == sharedTangents.end()){
			sharedTangents.insert(pair<Vec3f, Vec3f>(v, tangent));
		}
		else{
			Vec3f& vt = sharedTangents.at(v);
			vt += tangent;
		}
	}	
}

//function to calculate the vertex normals
void Object::calculateVertexNormals(){

	vector<Vec3f> vertexNormalList;
	//find the mean of all the normals which are shared by the vertex,
	//and normalize and add them to the vertex normal vector
	for(unsigned i = 0; i < points.size(); i++){
		vector<Vec3f> tempList = sharedNormals.at(points.at(i));
		Vec3f vn;
		for(unsigned j = 0; j < tempList.size(); j++){
			vn += tempList.at(j);
		}

		vn.normalize();

		vertexNormalList.push_back(vn);
	}


	//calculate the output vertex normal buffer array
	outVertexNormals = new float[faces.size() * 3 * 3];
	Vec3f vn;
	int index = 0;
	for(unsigned i = 0; i < faces.size(); i++){
		for (unsigned j = 0; j < 3; j++){
			vn = vertexNormalList.at((*faces.at(i))[j]);
			outVertexNormals[index] = vn.x;
			outVertexNormals[index + 1] = vn.y;
			outVertexNormals[index + 2] = vn.z;

			index += 3;
		}
	}
}

void Object::calculateVertexTangents(){
	vector<Vec3f> vertexTangentList;
	//find the mean of all the tangents which are shared by the vertex,
	//and normalize and add them to the vertex tangent vector

	for (unsigned i = 0; i < points.size(); i++){
		Vec3f vt = sharedTangents.at(points.at(i));
		vt.normalize();
		vertexTangentList.push_back(vt);
	}

	outVertexTangents = new float[faces.size() * 3 * 3];
	Vec3f vt;
	int index = 0;
	for (unsigned i = 0; i < faces.size(); i++){
		for (unsigned j = 0; j < 3; j++){
			vt = vertexTangentList.at((*faces.at(i))[j]);
			outVertexTangents[index] = vt.x;
			outVertexTangents[index + 1] = vt.y;
			outVertexTangents[index + 2] = vt.z;

			index += 3;
		}
	}
}

//function to pass all the array buffers into the openGL vertex buffers
//P.S - This function has to be called in the end, once all the attributes for the objects have been set.
void Object::initBuffers(const GLuint& program){

	cout << "Normals Size: " << normals.size() << endl;
	cout << "Tangents Size: " << tangents.size() << endl;

	//get the attribute ids
	GLuint vertexAttributeID = glGetAttribLocation(program, "vPosition");
	GLuint normalAttributeID = glGetAttribLocation(program, "vNormal");
	GLuint uvAttributeID = glGetAttribLocation(program, "vTexCoord");
	GLuint tangentAttributeID = glGetAttribLocation(program, "vTangent");

	outVertices = new float[faces.size() * 3 * 3];
	outNormals = new float[faces.size() * 3 * 3];
	Vec3f v, n, t;
	int index = 0;
	for(unsigned i = 0; i < faces.size(); i++){
		n = normals.at(faces.at(i)->n);
		if (tangents.size() > 0){
			t = tangents.at(faces.at(i)->t);
		}

		for (unsigned j = 0; j < 3; j++){
			v = points.at((*faces.at(i))[j]);
			outVertices[index] = v.x;
			outVertices[index + 1] = v.y;
			outVertices[index + 2] = v.z;

			if(!smooth){
				outNormals[index] = n.x;
				outNormals[index + 1] = n.y;
				outNormals[index + 2] = n.z;

				if (t != Vec3f()){
					outTangents[index] = t.x;
					outTangents[index + 1] = t.y;
					outTangents[index + 2] = t.z;
				}
			}

			index += 3;
		}
	}

	if (smooth){
		calculateVertexNormals();
		if (tangents.size() > 0){
			calculateVertexTangents();
		}
	}

	glBindVertexArray(vao);

	glGenBuffers(4, vbo);

	//pass the vertex buffers
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

	if(uv.size() > 0){

		outUV = new float[uv.size() * 2];
		index = 0;

		for(unsigned i = 0; i < uv.size(); i++){
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

	if (tangents.size() > 0){
		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		if (smooth)
			glBufferData(GL_ARRAY_BUFFER, faces.size() * 3 * 3 * sizeof(float), outVertexTangents, GL_STATIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, faces.size() * 3 * 3 * sizeof(float), outTangents, GL_STATIC_DRAW);
		glEnableVertexAttribArray(tangentAttributeID);
		glVertexAttribPointer(tangentAttributeID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

}

//function to render stuff
void Object::render(const GLuint& program, TextureManager& textureManager){

	//Note to self : very very important!!! Otherwise you won't see anything! 
	glUseProgram(program);

	//bind the current object's texture

	for (auto iter = m_textureHandles.begin(); iter != m_textureHandles.end(); iter++)
	{
		if (cubemap)
		{
			textureManager.BindTextureCubeMap(iter->first, iter->second, program);
		}
		else
		{
			textureManager.BindTexture2D(iter->first, iter->second, program);
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

	glm::mat4 modelMatrix = scaleMatrix * rotationMatrix_X * rotationMatrix_Y * rotationMatrix_Z * translateMatrix;

	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, false, glm::value_ptr(modelMatrix));

	//bind the vao and draw the triangles
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, faces.size() * 3 * 3);

	textureManager.unbindAllTextures();

	//glUseProgram(0);
}



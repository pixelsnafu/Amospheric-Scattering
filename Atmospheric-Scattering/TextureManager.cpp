#include "TextureManager.h"

unique_ptr<TextureManager> TextureManager::m_Instance = nullptr;
once_flag TextureManager::m_onceFlag;

TextureManager::TextureManager()
{
	m_activeTextureCount = 0;
}

TextureManager& TextureManager::GetInstance()
{
	call_once(m_onceFlag,
		[] 
	{
		m_Instance.reset(new TextureManager);
	});

	return *m_Instance.get();
}

void TextureManager::GenerateFBOTexture2D(string texAlias, int width, int height, bool isDepth)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, (isDepth ? GL_DEPTH_COMPONENT : GL_RGBA),
		width, height, 0, (isDepth ? GL_DEPTH_COMPONENT : GL_RGBA), GL_FLOAT, NULL);

	//texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (glGetError()){
		cout << "Error while creating Empty Texture: " << gluErrorString(glGetError()) << endl;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	m_texIDMap.insert(make_pair(texAlias, textureID));
}

void TextureManager::LoadTexture2D(string filename, string textureAlias)
{
	int width, height;
	GLuint textureID;

	glEnable(GL_TEXTURE_2D);

	unsigned char* imgData = NULL;
	imgData = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
	
	if (imgData == NULL)
	{
		cout << "Error loading Image!" << endl;
		exit(1);
	}
	
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (GLEW_EXT_texture_filter_anisotropic)
	{
		GLfloat maxAnisotropySamples;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropySamples);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropySamples);

	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData);

	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(imgData);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	m_texIDMap.insert(make_pair(textureAlias, textureID));
}

void TextureManager::LoadTextureCubeMap(vector<string> textureFaces, string textureAlias)
{
	int width, height;
	GLuint textureID;
	unsigned char* imgData = NULL;

	glEnable(GL_TEXTURE_CUBE_MAP);
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < textureFaces.size(); i++)
	{
		imgData = SOIL_load_image(textureFaces[i].c_str(), &width, &height, 0, SOIL_LOAD_RGB);
		cout << width << " " << height << endl;
		if (imgData == NULL)
		{
			cout << "Error loading Image!" << endl;
			exit(1);
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imgData);
		SOIL_free_image_data(imgData);
		imgData = NULL;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDisable(GL_TEXTURE_CUBE_MAP);

	m_texIDMap.insert(make_pair(textureAlias, textureID));
}

void TextureManager::BindTexture2D(string texAlias, string sampler, GLuint program)
{
	glUseProgram(program);
	glEnable(GL_TEXTURE_2D);
	GLuint currentTextureID = m_texIDMap.at(texAlias);
	glActiveTexture(GL_TEXTURE0 + m_activeTextureCount);
	glBindTexture(GL_TEXTURE_2D, currentTextureID);
	glUniform1i(glGetUniformLocation(program, sampler.c_str()), m_activeTextureCount);
	TextureUnit currTextureUnit = { GL_TEXTURE0 + m_activeTextureCount, GL_TEXTURE_2D, currentTextureID };
	m_activeTextureUnits.push_back(currTextureUnit);
	m_activeTextureCount++;
}

void TextureManager::BindTextureCubeMap(string texAlias, string sampler, GLuint program)
{
	glUseProgram(program);
	glEnable(GL_TEXTURE_CUBE_MAP);
	GLuint currentTextureID = m_texIDMap.at(texAlias);
	glActiveTexture(GL_TEXTURE0 + m_activeTextureCount);
	glBindTexture(GL_TEXTURE_CUBE_MAP, currentTextureID);
	glUniform1i(glGetUniformLocation(program, sampler.c_str()), m_activeTextureCount);
	TextureUnit currTextureUnit = { GL_TEXTURE0 + m_activeTextureCount, GL_TEXTURE_CUBE_MAP, currentTextureID };
	m_activeTextureUnits.push_back(currTextureUnit);
	m_activeTextureCount++;
}

void TextureManager::unbindTexture(string texAlias)
{
	GLuint texID = m_texIDMap.at(texAlias);
	auto currentTextureUnit = find_if(m_activeTextureUnits.begin(), m_activeTextureUnits.end(), [texID](const TextureUnit& t){
		return t.m_ID == texID;
	});

	glActiveTexture(currentTextureUnit->m_activeUnit);
	glBindTexture(currentTextureUnit->m_target, 0);
	glDisable(currentTextureUnit->m_target);
	m_activeTextureUnits.erase(currentTextureUnit);
	m_activeTextureCount--;
}

void TextureManager::unbindAllTextures()
{
	for (auto currentTextureUnit = m_activeTextureUnits.rbegin(); currentTextureUnit != m_activeTextureUnits.rend(); currentTextureUnit++)
	{
		glActiveTexture(currentTextureUnit->m_activeUnit);
		glBindTexture(currentTextureUnit->m_target, 0);
		glDisable(currentTextureUnit->m_target);
	}
	m_activeTextureUnits.clear();
	m_activeTextureCount = 0;
}
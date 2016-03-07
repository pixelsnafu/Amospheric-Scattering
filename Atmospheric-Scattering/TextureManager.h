#pragma once

#ifndef _TEXTUREMANAGER_H_
#define _TEXTUREMANAGER_H_

#include <gl/glew.h>
#include <gl/GL.h>
#include <SOIL.h>
#include <stb_image_aug.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <cassert>


using namespace std;

struct TextureUnit
{
	GLuint m_activeUnit;
	GLuint m_target;
	GLuint m_ID;
};

class TextureManager{

private:
	TextureManager();
	TextureManager(const TextureManager&) = delete;
	const TextureManager& operator=(const TextureManager&) = delete;

	/*---------------------------------------Data Members---------------------------------------*/
	static unique_ptr<TextureManager> m_Instance;
	static once_flag m_onceFlag;

	map<string, GLuint> m_texIDMap;
	//map<GLuint, GLuint> m_textureTargets;
	vector<TextureUnit> m_activeTextureUnits;
	unsigned int m_activeTextureCount; 

public:

	static TextureManager& GetInstance();
	void LoadTexture2D(string filename, string textureAlias);
	void LoadTexture1D(string filename, string textureAlias);
	void LoadTextureCubeMap(vector<string> textureFaces, string textureAlias);
	void GenerateFBOTexture2D(string texAlias, int width, int height, bool isDepth = false);
	void BindTexture2D(string texAlias, string sampler, GLuint program);
	void BindTexture1D(string texAlias, string sampler, GLuint program);
	void BindTextureCubeMap(string texAlias, string sampler, GLuint program);
	void unbindTexture(string texAlias);
	void unbindAllTextures();

	const GLuint& operator[] (const string& texAlias)
	{
		assert(m_texIDMap.count(texAlias) != 0);
		if (m_texIDMap.find(texAlias) == m_texIDMap.end())
			return 0;
		else
			return m_texIDMap.at(texAlias);
	}
};

#endif

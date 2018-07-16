#pragma once
// Std. Includes
#include <vector>
#include <iostream>
// GL Includes
#include <GL/glew.h>
#include <SOIL/SOIL.h>

class __declspec(dllexport) Texture
{
public:
	Texture(const std::string& texturePath);
	Texture(int screenWidth, int screenHeight);

	GLuint getId() { return mId; }

private:
	void initTextureFromFile(const std::string& texturePath);
	void initTexture(int screenWidth, int screenHeight);
	GLuint mId;
};
#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

GLint TextureFromFile(const char* path, std::string directory);

class __declspec(dllexport) Model
{
public:
	Model(GLchar* path);

	void Draw(GLuint shaderID);

	int getMeshSize() const { return m_MeshSet.size(); }
	void setInstanceNumber(unsigned int vNum) { m_InstanceNumber = vNum; m_IsInstanceRender = true; }

	GLuint getMeshVAOByIndex(int vIndex) { return m_MeshSet[vIndex].getVAO(); }

private:
	/*  Model Data  */
	std::vector<Mesh> m_MeshSet;
	std::string m_Directory;
	std::vector<STexture> textures_loaded;
	unsigned int m_InstanceNumber;
	bool m_IsInstanceRender;

	void __loadModel(std::string path);

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void __processNode(aiNode* node, const aiScene* scene);

	Mesh __processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<STexture> __loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

GLint TextureFromFile(const char* path, std::string directory)
{
	//Generate texture ID and load texture data 
	std::string filename = std::string(path);
	filename = directory + '\\' + filename;
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents;
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, &nrComponents, 0);
	// Assign texture to ID
	GLenum format;
	if (nrComponents == 1)
		format = GL_RED;
	else if (nrComponents == 3)
		format = GL_RGB;
	else if (nrComponents == 4)
		format = GL_RGBA;
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}
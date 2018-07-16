#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>

#include "Shader.h"

struct SVertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};

struct STexture {
	GLuint id;
	std::string type;
	aiString path;
};

class __declspec(dllexport) Mesh {
public:

	std::vector<SVertex> m_VertexSet;
	std::vector<GLuint> m_IndexSet;
	std::vector<STexture> m_TextureSet;

	Mesh(std::vector<SVertex> vertexSet, std::vector<GLuint> indexSet, std::vector<STexture> textureSet);

	void Draw(GLuint shaderID, unsigned int instanceNumber, bool isInstanceRender);

	GLuint getVAO() { return m_VAO; }

private:
	GLuint m_VAO, m_VBO, m_EBO;

	void __setupMesh();
};
#include "ShapeRender.h"

ShapeRender::ShapeRender() : m_SphereVAO(0), m_SphereVBO(0), m_SphereEBO(0), m_CubeVAO(0), m_CubeVBO(0), m_QuadVAO(0), m_QuadVBO(0), m_SphereStepXZ(10), m_SphereStepY(10), m_SphereIndexSize(0)
{
}

void ShapeRender::__initCubeVertex() {
	float vertices[] = {
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};
	glGenVertexArrays(1, &m_CubeVAO);
	glGenBuffers(1, &m_CubeVBO);
	// fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// link vertex attributes
	glBindVertexArray(m_CubeVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ShapeRender::__initQuadVertex() {
	float vertices[] = {
		 1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f,   0.0f, 1.0f,
		 1.0f,-1.0f, 0.0f,   1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,   0.0f, 1.0f,
		 1.0f,-1.0f, 0.0f,   1.0f, 0.0f,
		-1.0f,-1.0f, 0.0f,   0.0f, 0.0f
	};
	glGenVertexArrays(1, &m_QuadVAO);
	glGenBuffers(1, &m_QuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(m_QuadVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ShapeRender::__initSphereVertex() {
	std::vector<float> vertexList;
	std::vector<unsigned int> indexList;
	std::vector<float> normalList;
	glm::vec3 position(0.0);
	float radius = 1.0f;
	int xzStep = 5;
	int yStep = 5;
	int yAngle = 0;
	float PI = 2 * 3.14f / 360;
	int index = 0;
	while (yAngle <= 180) {
		float y0 = position.y + radius * std::cos(PI * yAngle);
		float y1 = position.y + radius * std::cos(PI * (yAngle + m_SphereStepY));
		float xzRadius0 = radius * std::sin(PI * yAngle);
		float xzRadius1 = radius * std::sin(PI * (yAngle + m_SphereStepY));

		int xzAngle = 0;
		yAngle += m_SphereStepY;
		bool start = true;
		while (xzAngle <= 360) {

			float x0 = position.x + xzRadius0 * std::cos(PI * xzAngle);
			float x1 = position.x + xzRadius1 * std::cos(PI * (xzAngle + m_SphereStepXZ));
			float z0 = position.z + xzRadius0 * std::sin(PI * xzAngle);
			float z1 = position.z + xzRadius1 * std::sin(PI * (xzAngle + m_SphereStepXZ));

			vertexList.push_back(x0);
			vertexList.push_back(y0);
			vertexList.push_back(z0);
			vertexList.push_back(x1);
			vertexList.push_back(y1);
			vertexList.push_back(z1);
			if (!start) {
				indexList.push_back(index * 2 - 2);
				indexList.push_back(index * 2 - 1);
				indexList.push_back(index * 2);
				indexList.push_back(index * 2);
				indexList.push_back(index * 2 - 1);
				indexList.push_back(index * 2 + 1);
			}
			index++;
			start = false;
			xzAngle += m_SphereStepXZ;
		}
	}
	glGenVertexArrays(1, &m_SphereVAO);
	glGenBuffers(1, &m_SphereVBO);
	glGenBuffers(1, &m_SphereEBO);
	// fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_SphereVBO);
	glBufferData(GL_ARRAY_BUFFER, vertexList.size() * sizeof(float), vertexList.data(), GL_STATIC_DRAW);

	// link vertex attributes
	glBindVertexArray(m_SphereVAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_SphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexList.size() * sizeof(unsigned int), indexList.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	m_SphereIndexSize = indexList.size();
}

void ShapeRender::renderCube()
{
	if (m_CubeVAO == 0)
	{
		__initCubeVertex();
	}
	glBindVertexArray(m_CubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void ShapeRender::renderQuad()
{
	if (m_QuadVAO == 0)
	{
		__initQuadVertex();
	}
	glBindVertexArray(m_QuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void ShapeRender::renderSphere()
{
	if (m_SphereVAO == 0) {
		__initSphereVertex();
	}

	glBindVertexArray(m_SphereVAO);
	glDrawElements(GL_TRIANGLES, m_SphereIndexSize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
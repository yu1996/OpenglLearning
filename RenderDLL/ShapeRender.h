#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

class __declspec(dllexport) ShapeRender
{
public:
	ShapeRender();
	void renderSphere();
	void renderCube();
	void renderQuad();
	void setSphereStepY(int sphereStepY) { m_SphereStepY = sphereStepY; m_SphereVAO = 0; }
	void setSphereStepXZ(int sphereStepXZ) { m_SphereStepXZ = sphereStepXZ; m_SphereVAO = 0; }

	GLuint getQuadVAO() { if (m_QuadVAO == 0) __initQuadVertex(); return m_QuadVAO; }
	GLuint getCubeVAO() { if (m_CubeVAO == 0) __initCubeVertex(); return m_CubeVAO; }
	GLuint getSphereVAO() { if (m_SphereVAO == 0) __initSphereVertex(); return m_SphereVAO; }

	int getSphereIndexSize() { return m_SphereIndexSize; }

private:
	void __initQuadVertex();
	void __initSphereVertex();
	void __initCubeVertex();
	GLuint m_SphereVBO;
	GLuint m_SphereVAO;
	GLuint m_SphereEBO;
	int m_SphereStepY;
	int m_SphereStepXZ;
	int m_SphereIndexSize;

	GLuint m_QuadVBO;
	GLuint m_QuadVAO;

	GLuint m_CubeVBO;
	GLuint m_CubeVAO;
};
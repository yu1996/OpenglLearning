#pragma once
// Std. Includes
#include <vector>

// GL Includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;

class __declspec(dllexport) Window
{
public:
	const GLuint WIDTH;
	const GLuint HEIGHT;

	// Constructor with vectors
	Window(GLuint width, GLuint height);

private:
	GLFWwindow* m_Window;
};
#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "ShapeRender.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLFWwindow* windowInit();

const GLuint WIDTH = 800, HEIGHT = 600;

Camera  camera(glm::vec3(0.0f, 1.0f, 5.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

ShapeRender shapeRender;
const int INSTANCE_NUMBER = 100;
int main() {
	GLFWwindow* window = windowInit();

	glewInit();
	glViewport(0, 0, WIDTH, HEIGHT);

	Shader shader;
	shader.addShader("instanceShader.vs", GL_VERTEX_SHADER);
	shader.addShader("instanceShader.fs", GL_FRAGMENT_SHADER);
	shader.linkShader();

	glm::mat4 translationUBO[INSTANCE_NUMBER];
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			translationUBO[i * 10 + j] = glm::translate(glm::mat4(), glm::vec3(i, j, 0.0));

			translationUBO[i * 10 + j] = glm::scale(translationUBO[i * 10 + j], glm::vec3(0.3));
		}
	}

	GLuint uboBlockIndex = glGetUniformBlockIndex(shader.ID, "uModelMatrixUBO");
	glUniformBlockBinding(shader.ID, uboBlockIndex, 0);

	GLuint modelMatrixUBO;
	glGenBuffers(1, &modelMatrixUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, modelMatrixUBO);
	glBufferData(GL_UNIFORM_BUFFER, INSTANCE_NUMBER * sizeof(glm::mat4), translationUBO, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, modelMatrixUBO, 0, INSTANCE_NUMBER * sizeof(glm::mat4));

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(0.3, 0.3, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		shader.setMat4("uViewMatrix", camera.getViewMatrix());
		glm::mat4 projectionMatrix = glm::perspective(camera.Zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		shader.setMat4("uProjectionMatrix", projectionMatrix);

		glBindBuffer(GL_UNIFORM_BUFFER, modelMatrixUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * INSTANCE_NUMBER, translationUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindVertexArray(shapeRender.getSphereVAO());
		glDrawElementsInstanced(GL_TRIANGLES, shapeRender.getSphereIndexSize(), GL_UNSIGNED_INT, 0, INSTANCE_NUMBER);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

GLFWwindow * windowInit()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glewExperimental = GL_TRUE;

	return window;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void do_movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
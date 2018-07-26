#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
void renderTriangle();
GLFWwindow* windowInit();

const GLuint WIDTH = 800, HEIGHT = 600;

Camera  camera(glm::vec3(0.0f, 0.0f, -2.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main() {
	GLFWwindow* window = windowInit();

	glewInit();
	glViewport(0, 0, WIDTH, HEIGHT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	Shader shader;
	shader.addShader("tessellation.vs", GL_VERTEX_SHADER);
	shader.addShader("tessellation.fs", GL_FRAGMENT_SHADER);
	shader.addShader("tessellation.tcs", GL_TESS_CONTROL_SHADER);
	shader.addShader("tessellation.tes", GL_TESS_EVALUATION_SHADER);
	shader.linkShader();
	/*GLfloat outerLevel[] = {2.0, 2.0, 2.0};
	GLfloat innerLevel[] = {2.0};
	glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, outerLevel);
	glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, innerLevel);*/
	
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		glm::mat4 projectionMatrix = glm::perspective(camera.Zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 viewMatrix = camera.getViewMatrix();
		glm::mat4 modelMatrix;
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, 0.1));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0, 10.0, 10.0));
		shader.setMat4("uProjectionMatrix", projectionMatrix);
		shader.setMat4("uViewMatrix", viewMatrix);
		shader.setMat4("uModelMatrix", modelMatrix);

		renderTriangle();

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

GLuint VAO = 0;
GLuint VBO = 0;

void renderTriangle()
{
	if (VAO == 0)
	{
		float vertices[] = {
			-0.5, 0.0,   0.0,
			0.5,  0.0,   0.0,
			0.0,  0.866, 0.0
		};
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindVertexArray(VAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	glBindVertexArray(VAO);
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	glDrawArrays(GL_PATCHES, 0, 3);
	glBindVertexArray(0);
}
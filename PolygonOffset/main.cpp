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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLFWwindow* windowInit();
void renderCube();

const GLuint WIDTH = 800, HEIGHT = 600;

Camera  camera(glm::vec3(0.0f, 1.0f, 5.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

const std::vector<glm::vec3> boxPosition
{
	glm::vec3(-2.6f, 0.0f, -0.58f),
	glm::vec3(3.6f, 0.0f, 1.51f),
	glm::vec3(0.0f, 0.0f, 1.7f),
	glm::vec3(-1.3f, 0.0f, -3.3f),
	glm::vec3(0.5f, 0.0f, -1.6f),
	glm::vec3(2.0f, 1.0f, 2.0f)
};
int main() {
	GLFWwindow* window = windowInit();

	glewInit();
	glViewport(0, 0, WIDTH, HEIGHT);

	Shader standardShader;
	standardShader.addShader("standardShader.vs", GL_VERTEX_SHADER);
	standardShader.addShader("standardShader.fs", GL_FRAGMENT_SHADER);
	standardShader.linkShader();

	Shader lineShader;
	lineShader.addShader("lineShader.vs", GL_VERTEX_SHADER);
	lineShader.addShader("lineShader.fs", GL_FRAGMENT_SHADER);
	lineShader.linkShader();

	Texture boxTexture("../Resource/textures/container2.png");

	standardShader.use();
	standardShader.setInt("uBoxTexture", 0);

	glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();

		//FILL PASS

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		glLineWidth(2.0);

		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(0.3, 0.3, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		standardShader.use();
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		standardShader.setMat4("uProjectionMatrix", projection);
		standardShader.setMat4("uViewMatrix", view);
		standardShader.setVec3("uViewPos", camera.Position);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, boxTexture.getId());
		for (int i = 0; i < boxPosition.size(); ++i) {
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, boxPosition[i]);
			standardShader.setMat4("uModelMatrix", modelMatrix);
			renderCube();
		}

		//LINE PASS

		glViewport(0, 0, WIDTH, HEIGHT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		//glEnable(GL_POLYGON_OFFSET_FILL);
		//glEnable(GL_POLYGON_OFFSET_POINT);
		glPolygonOffset(-2.0, 1.0);
		lineShader.use();
		lineShader.setMat4("uProjectionMatrix", projection);
		lineShader.setMat4("uViewMatrix", view);
		lineShader.setVec3("uViewPos", camera.Position);
		lineShader.setVec2("uRenderRegion", glm::vec2(400, 800));
		for (int i = 0; i < boxPosition.size(); ++i) {
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, boxPosition[i]);
			lineShader.setMat4("uModelMatrix", modelMatrix);
			renderCube();
		}

		//Z-FIGHTING PASS

		//glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
		//glDisable(GL_POLYGON_OFFSET_POINT);

		lineShader.setVec2("uRenderRegion", glm::vec2(0, 400));
		for (int i = 0; i < boxPosition.size(); ++i) {
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, boxPosition[i]);
			lineShader.setMat4("uModelMatrix", modelMatrix);
			renderCube();
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

GLuint ScreenVAO;
GLuint ScreenVBO;

void renderScreen() {
	if (ScreenVAO == 0)
	{
		float vertices[] = {
			1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
			-1.0f, 1.0f, 0.0f,   0.0f, 1.0f,
			1.0f,-1.0f, 0.0f,   1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,   0.0f, 1.0f,
			1.0f,-1.0f, 0.0f,   1.0f, 0.0f,
			-1.0f,-1.0f, 0.0f,   0.0f, 0.0f
		};
		glGenVertexArrays(1, &ScreenVAO);
		glGenBuffers(1, &ScreenVBO);
		glBindBuffer(GL_ARRAY_BUFFER, ScreenVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindVertexArray(ScreenVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	glBindVertexArray(ScreenVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f, 0.0f, // bottom-left
			1.0f,  1.0f, -1.0f,  1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  1.0f, 0.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f, 1.0f, // top-left
			  // front face
			  -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			  1.0f, -1.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			  1.0f,  1.0f,  1.0f, 1.0f, 1.0f, // top-right
			  1.0f,  1.0f,  1.0f, 1.0f, 1.0f, // top-right
			  -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, // top-left
			  -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, // bottom-left
											   // left face
											   -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, // top-right
											   -1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // top-left
											   -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-left
											   -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-left
											   -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, // bottom-right
											   -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, // top-right
																				// right face
																				1.0f,  1.0f,  1.0f, 1.0f, 0.0f, // top-left
																				1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-right
																				1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // top-right         
																				1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-right
																				1.0f,  1.0f,  1.0f, 1.0f, 0.0f, // top-left
																				1.0f, -1.0f,  1.0f, 0.0f, 0.0f, // bottom-left     
																												// bottom face
																												-1.0f, -1.0f, -1.0f,  0.0f, 1.0f, // top-right
																												1.0f, -1.0f, -1.0f,  1.0f, 1.0f, // top-left
																												1.0f, -1.0f,  1.0f,  1.0f, 0.0f, // bottom-left
																												1.0f, -1.0f,  1.0f,  1.0f, 0.0f, // bottom-left
																												-1.0f, -1.0f,  1.0f,  0.0f, 0.0f, // bottom-right
																												-1.0f, -1.0f, -1.0f,  0.0f, 1.0f, // top-right
																																				  // top face
																																				  -1.0f,  1.0f, -1.0f,  0.0f, 1.0f, // top-left
																																				  1.0f,  1.0f , 1.0f,  1.0f, 0.0f, // bottom-right
																																				  1.0f,  1.0f, -1.0f,  1.0f, 1.0f, // top-right     
																																				  1.0f,  1.0f,  1.0f,  1.0f, 0.0f, // bottom-right
																																				  -1.0f,  1.0f, -1.0f,  0.0f, 1.0f, // top-left
																																				  -1.0f,  1.0f,  1.0f,  0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
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
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
void renderScreen();
void renderCube();
GLuint loadDepthCubemap();

const GLuint WIDTH = 800, HEIGHT = 600;

Camera  camera(glm::vec3(0.0f, 2.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLuint planeVAO;

const std::vector<glm::vec3> boxPosition
{
	glm::vec3(-1.6f, 0.0f, -0.58f),
	glm::vec3(2.6f, 0.0f, 1.51f),
	glm::vec3(0.0f, 0.0f, 1.7f),
	glm::vec3(-0.3f, 0.0f, -3.3f),
	glm::vec3(0.5f, 0.0f, -1.6f),
	glm::vec3(0.5f, 0.0f, 2.0f)
};
int main() {
	GLFWwindow* window = windowInit();

	glewInit();
	glViewport(0, 0, WIDTH, HEIGHT);

	Shader modelIndexShader;
	modelIndexShader.addShader("modelIndexShader.vs", GL_VERTEX_SHADER);
	modelIndexShader.addShader("modelIndexShader.fs", GL_FRAGMENT_SHADER);
	modelIndexShader.linkShader();

	Shader occlusionQueryShader;
	occlusionQueryShader.addShader("occlusionQueryShader.vs", GL_VERTEX_SHADER);
	occlusionQueryShader.addShader("occlusionQueryShader.fs", GL_FRAGMENT_SHADER);
	occlusionQueryShader.linkShader();

	Shader standardShader;
	standardShader.addShader("standardShader.vs", GL_VERTEX_SHADER);
	standardShader.addShader("standardShader.fs", GL_FRAGMENT_SHADER);
	standardShader.linkShader();

	Texture woodTexture("../Resource/textures/wood.png");

	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	GLuint modelIndexMap;
	glGenTextures(1, &modelIndexMap);
	glBindTexture(GL_TEXTURE_2D, modelIndexMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, WIDTH, HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, modelIndexMap, 0);

	GLuint renderBufferObject;
	glGenRenderbuffers(1, &renderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT); // Use a single renderbuffer object for both a depth AND stencil buffer.
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint modelOcclusionQueryMap;
	glGenTextures(1, &modelOcclusionQueryMap);

	GLuint modelOcclusionQueryBuffer;
	glGenBuffers(1, &modelOcclusionQueryBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, modelOcclusionQueryBuffer);
	glBufferData(GL_TEXTURE_BUFFER, boxPosition.size() * 4, NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	glBindTexture(GL_TEXTURE_BUFFER, modelOcclusionQueryMap);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, modelOcclusionQueryBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, 0);

	occlusionQueryShader.use();
	occlusionQueryShader.setInt("uModelIndexMap", 0);

	standardShader.use();
	standardShader.setInt("uWoodTexture", 0);

	glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();

		//createModelIndexMap
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(0.3, 0.3, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		modelIndexShader.use();
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		modelIndexShader.setMat4("uProjectionMatrix", projection);
		modelIndexShader.setMat4("uViewMatrix", view);

		modelIndexShader.setVec3("uViewPos", camera.Position);
		for (int i = 0; i < boxPosition.size(); ++i) {
			modelIndexShader.setFloat("uModelIndex", float(i));
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, boxPosition[i]);
			modelIndexShader.setMat4("uModelMatrix", modelMatrix);
			renderCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//occlusionQueryPass
		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_CULL_FACE);
		occlusionQueryShader.use();
		glBindImageTexture(0, modelOcclusionQueryMap, 0, 0, 0, GL_READ_WRITE, GL_RGBA8);

		glBindBuffer(GL_TEXTURE_BUFFER, modelOcclusionQueryBuffer);
		unsigned char* data = (unsigned char*)glMapBuffer(GL_TEXTURE_BUFFER, GL_READ_ONLY);
		memset(data, 0, boxPosition.size() * 4);
		glUnmapBuffer(GL_TEXTURE_BUFFER);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, modelIndexMap);

		renderScreen();

		//standardShader
		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		standardShader.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture.getId());

		unsigned char index[24];
		data = (unsigned char*)glMapBuffer(GL_TEXTURE_BUFFER, GL_READ_ONLY);
		memcpy(index, data, boxPosition.size() * 4);
		glUnmapBuffer(GL_TEXTURE_BUFFER);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		standardShader.setMat4("uProjectionMatrix", projection);
		standardShader.setMat4("uViewMatrix", view);
		for (int i = 0; i < boxPosition.size(); ++i) {
			if (index[i * 4] == 255.0) {
				glm::mat4 modelMatrix;
				modelMatrix = glm::translate(modelMatrix, boxPosition[i]);
				standardShader.setMat4("uModelMatrix", modelMatrix);
				renderCube();
			}
		}
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
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
void renderScene(const Shader & shader);
GLuint loadDepthCubemap();

const GLuint WIDTH = 800, HEIGHT = 600;
const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

Camera  camera(glm::vec3(0.0f, 2.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];
bool shadows = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

ShapeRender shapeRender;

int main() {
	GLFWwindow* window = windowInit();

	glewInit();
	glViewport(0, 0, WIDTH, HEIGHT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Shader shader;
	shader.addShader("shadowMapping.vs", GL_VERTEX_SHADER);
	shader.addShader("shadowMapping.fs", GL_FRAGMENT_SHADER);
	shader.linkShader();
	Shader simpleDepthShader;
	simpleDepthShader.addShader("shadowMappingDepth.vs", GL_VERTEX_SHADER);
	simpleDepthShader.addShader("shadowMappingDepth.fs", GL_FRAGMENT_SHADER);
	simpleDepthShader.addShader("shadowMappingDepth.gs", GL_GEOMETRY_SHADER);
	simpleDepthShader.linkShader();

	Texture woodTexture("../Resource/textures/wood.png");

	unsigned int depthCubeMap = loadDepthCubemap();

	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shader.use();
	shader.setInt("uDiffuseTexture", 0);
	shader.setInt("uShadowMap", 1);

	glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 lightProjection, lightSpaceMatrix;
		float nearPlane = 1.0f;
		float farPlane = 25.0f;
		float aspect = SHADOW_WIDTH / SHADOW_HEIGHT;
		lightProjection = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);

		std::vector<glm::mat4> shadowTransform;
		shadowTransform.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransform.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransform.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0, 0.0, 1.0)));
		shadowTransform.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0, 0.0, -1.0)));
		shadowTransform.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransform.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0, -1.0, 0.0)));

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glClear(GL_DEPTH_BUFFER_BIT); 
		simpleDepthShader.use();
		for (int i = 0; i < 6; ++i)
			simpleDepthShader.setMat4("uShadowMatrices[" + std::to_string(i) + "]", shadowTransform[i]);
		simpleDepthShader.setFloat("uFarPlane", farPlane);
		simpleDepthShader.setVec3("uLightPos", lightPos);
		renderScene(simpleDepthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		shader.setMat4("uProjectionMatrix", projection);
		shader.setMat4("uViewMatrix", view);

		shader.setVec3("uViewPos", camera.Position);
		shader.setVec3("uLightPos", lightPos);
		shader.setInt("uShadows", shadows);
		shader.setFloat("uFarPlane", farPlane);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture.getId());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
		renderScene(shader);

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

void renderScene(const Shader &shader)
{
	// floor
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(5.0f));
	glDisable(GL_CULL_FACE);
	shader.setMat4("uModelMatrix", model);
	shader.setInt("uReverseNormals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
	shapeRender.renderCube();
	shader.setInt("uReverseNormals", 0);
	glEnable(GL_CULL_FACE);
	// cubes
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("uModelMatrix", model);
	shapeRender.renderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.75f));
	shader.setMat4("uModelMatrix", model);
	shapeRender.renderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("uModelMatrix", model);
	shapeRender.renderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("uModelMatrix", model);
	shapeRender.renderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.75f));
	shader.setMat4("uModelMatrix", model);
	shapeRender.renderCube();
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

GLuint loadDepthCubemap()
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < 6; ++i)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL
		);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
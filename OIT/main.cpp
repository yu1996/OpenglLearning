#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
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

Camera  camera(glm::vec3(0.0f, 0.0f, 2.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

std::vector<glm::vec3> windowPosition
{
	glm::vec3(-1.5f, 0.0f, -0.48f),
	glm::vec3(1.5f, 0.0f, 0.51f),
	glm::vec3(0.0f, 0.0f, 0.7f),
	glm::vec3(-0.3f, 0.0f, -2.3f),
	glm::vec3(0.5f, 0.0f, -0.6f),
	glm::vec3(0.5f, 0.0f, 1.0f)
};

std::vector<glm::vec3> wallPosition
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

	Shader opaqueShader;
	opaqueShader.addShader("OpaqueRender.vs", GL_VERTEX_SHADER);
	opaqueShader.addShader("OpaqueRender.fs", GL_FRAGMENT_SHADER);
	opaqueShader.linkShader();

	Shader createFragmentListShader;
	createFragmentListShader.addShader("CreateFragmentList.vs", GL_VERTEX_SHADER);
	createFragmentListShader.addShader("CreateFragmentList.fs", GL_FRAGMENT_SHADER);
	createFragmentListShader.linkShader();

	Shader blendShader;
	blendShader.addShader("BlendPass.vs", GL_VERTEX_SHADER);
	blendShader.addShader("BlendPass.fs", GL_FRAGMENT_SHADER);
	blendShader.linkShader();

	Texture windowTexture("../Resource/textures/window.png");
	Texture boxTexture("../Resource/textures/container2.png");

	opaqueShader.use();
	opaqueShader.setInt("uWallTexture", 0);

	createFragmentListShader.use();
	createFragmentListShader.setInt("uWindowTexture", 1);
	createFragmentListShader.setInt("uDepthMap", 0);

	blendShader.use();
	blendShader.setInt("uOpaqueColor", 0);

	GLuint FBO_0;
	glGenFramebuffers(1, &FBO_0);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO_0);

	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

	GLuint opaqueColorMap;
	glGenTextures(1, &opaqueColorMap);
	glBindTexture(GL_TEXTURE_2D, opaqueColorMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, opaqueColorMap, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint startOffsetBuffer;
	glGenTextures(1, &startOffsetBuffer);
	glBindTexture(GL_TEXTURE_2D, startOffsetBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, WIDTH, HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	GLuint startOffsetBufferInitializer;
	glGenBuffers(1, &startOffsetBufferInitializer);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, startOffsetBufferInitializer);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, WIDTH * HEIGHT * sizeof(GLuint), NULL, GL_STATIC_DRAW);
	GLuint* data;
	data = (GLuint*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	memset(data, 0xFF, WIDTH * HEIGHT * sizeof(GLuint));
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

	GLuint atomicCounterBuffer;
	glGenBuffers(1, &atomicCounterBuffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_COPY);

	GLuint fragmentListBufferTexture;
	glGenTextures(1, &fragmentListBufferTexture);

	GLuint fragmentListBuffer;
	glGenBuffers(1, &fragmentListBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, fragmentListBuffer);
	glBufferData(GL_TEXTURE_BUFFER, WIDTH * HEIGHT * 2 * sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, fragmentListBufferTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, fragmentListBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, 0);

	ShapeRender shapeRender;

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();

		glBindFramebuffer(GL_FRAMEBUFFER, FBO_0);
		glClearColor(0.1f, 0.5f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//OpaqueRender

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		opaqueShader.use();
		glm::mat4 projectionMatrix = glm::perspective(camera.Zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 viewMatrix = camera.getViewMatrix();
		opaqueShader.setMat4("uProjectionMatrix", projectionMatrix);
		opaqueShader.setMat4("uViewMatrix", viewMatrix);

		glActiveTexture(GL_TEXTURE0);
		for (int i = 0; i < wallPosition.size(); ++i) {
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, wallPosition[i]);
			opaqueShader.setMat4("uModelMatrix", modelMatrix);
			glBindTexture(GL_TEXTURE_2D, boxTexture.getId());
			shapeRender.renderQuad();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//CreateFragmentList

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		createFragmentListShader.use();
		glClearColor(0.1f, 0.5f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, startOffsetBufferInitializer);
		glBindTexture(GL_TEXTURE_2D, startOffsetBuffer);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindImageTexture(0, startOffsetBuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);
		data = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_WRITE_ONLY);
		data[0] = 0;
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

		glBindImageTexture(1, fragmentListBufferTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);
		//以上逐帧重置需重写
		createFragmentListShader.setMat4("uProjectionMatrix", projectionMatrix);
		createFragmentListShader.setMat4("uViewMatrix", viewMatrix);
		createFragmentListShader.setVec3("uViewPos", camera.Position);
		createFragmentListShader.setVec2("uScreenSize", glm::vec2(WIDTH, HEIGHT));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glActiveTexture(GL_TEXTURE1);
		for (int i = 0; i < windowPosition.size(); ++i) {
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, windowPosition[i]);
			createFragmentListShader.setMat4("uModelMatrix", modelMatrix);
			glBindTexture(GL_TEXTURE_2D, windowTexture.getId());
			shapeRender.renderQuad();
		}

		//Blend
		blendShader.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, opaqueColorMap);
		shapeRender.renderQuad();
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
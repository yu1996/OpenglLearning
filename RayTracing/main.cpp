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

enum SphereMaterialType
{
	Diffuse = 0,
	Metal
};

struct Sphere
{
	glm::vec4 Center;
	glm::vec4 Albedo;
	float Radius;
	int Type;
	glm::vec2 ForComplement;

	Sphere::Sphere(const glm::vec4& vCenter, const glm::vec4& vAlbedo, float vRadius, int vType) :Center(vCenter), Albedo(vAlbedo), Radius(vRadius), Type(vType){}
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLFWwindow* windowInit();
void initSphereSet();
void assignValueToArray(const glm::vec4& vValue, float* vArray);

const GLuint WIDTH = 512, HEIGHT = 512;

Camera  camera(glm::vec3(0.0f, 1.0f, 5.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

ShapeRender shapeRender;

glm::ivec3 WorkGroupNum = glm::ivec3(8, 8, 1);
glm::ivec3 WorkGroupSize = glm::ivec3(32, 32, 1);

const int SPHERE_NUM = 3;

std::vector<Sphere> SphereSet;

int main() {
	GLFWwindow* window = windowInit();
	glewInit();
	glViewport(0, 0, WIDTH, HEIGHT);

	Shader screenShader;
	screenShader.addShader("screenShader_VS.glsl", GL_VERTEX_SHADER);
	screenShader.addShader("screenShader_FS.glsl", GL_FRAGMENT_SHADER);
	screenShader.linkShader();

	Shader computeShader;
	computeShader.addShader("computeShader.glsl", GL_COMPUTE_SHADER);
	computeShader.linkShader();

	//SphereUBO init
	initSphereSet();

	GLuint uboBlockIndex = glGetUniformBlockIndex(computeShader.ID, "uSphereUBO");
	glUniformBlockBinding(computeShader.ID, uboBlockIndex, 0);

	GLuint sphereUBO;
	glGenBuffers(1, &sphereUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, sphereUBO);
	glBufferData(GL_UNIFORM_BUFFER, SPHERE_NUM * sizeof(Sphere), SphereSet.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, sphereUBO, 0, SPHERE_NUM * sizeof(Sphere));

	glBindBuffer(GL_UNIFORM_BUFFER, sphereUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Sphere) * SPHERE_NUM, SphereSet.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	Texture ImageForComputeShader(WorkGroupNum.x * WorkGroupSize.x, WorkGroupNum.y * WorkGroupSize.y);

	float NearPlaneHalfHeight = camera.NearPlane * glm::tan(glm::radians(camera.Zoom / 2.0f));
	float NearPlaneHalfWidth = NearPlaneHalfHeight * WIDTH / HEIGHT;

	while (!glfwWindowShouldClose(window)) {

		int maxWidth[2];
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxWidth);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();

		computeShader.use();
		glBindImageTexture(1, ImageForComputeShader.getId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
		computeShader.setVec3("uCameraPosition", camera.Position);
		computeShader.setVec3("uCameraDirection", camera.Front);
		computeShader.setVec3("uCameraUp", camera.Up);
		computeShader.setVec3("uCameraRight", camera.Right);
		computeShader.setFloat("uNearPlaneHalfHeight", NearPlaneHalfHeight);
		computeShader.setFloat("uNearPlaneHalfWidth", NearPlaneHalfWidth);
		computeShader.setInt("uSphereSetSize", SPHERE_NUM);

		std::cout << camera.Position.x << "\t" << camera.Position.y << "\t" << camera.Position.z << std::endl;

		glDispatchCompute(WorkGroupNum.x, WorkGroupNum.y, WorkGroupNum.z);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(0.3, 0.3, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		screenShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ImageForComputeShader.getId());
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
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
	if (keys[GLFW_KEY_Q])
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (keys[GLFW_KEY_E])
		camera.ProcessKeyboard(UP, deltaTime);
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

void initSphereSet()
{
	SphereSet.push_back(Sphere(glm::vec4(8.0, 0.0, 0.0, 1.0), glm::vec4(0.5, 1.0, 0.5, 1.0), 3.0f, SphereMaterialType::Diffuse));
	SphereSet.push_back(Sphere(glm::vec4(0.0, 0.0, 0.0, 1.0), glm::vec4(1.0, 0.5, 0.5, 1.0), 3.0f, SphereMaterialType::Metal));
	SphereSet.push_back(Sphere(glm::vec4(4.0, -15.0, 0.0, 1.0), glm::vec4(0.5, 0.5, 1.0, 1.0), 12.0f, SphereMaterialType::Diffuse));
}

void assignValueToArray(const glm::vec4& vValue, float* vArray)
{
	for(int i = 0; i < 4; ++i)
	{
		vArray[i] = vValue[i];
	}
}
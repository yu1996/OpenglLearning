// Std. Includes
#include <string>
#include <map>
// GLEW
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "ShapeRender.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL\SOIL.h>

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
GLFWwindow* windowInit();
// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 20.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

bool occlusionQueryOn;

glm::mat4 occlusionModelMatrix;


// The MAIN function, from here we start our application and run our Game loop
int main()
{
	GLFWwindow* window = windowInit();

	glewInit();
	glViewport(0, 0, screenWidth, screenHeight);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	// Setup and compile our shaders
	Shader shader;
	shader.addShader("standardShader.vs", GL_VERTEX_SHADER);
	shader.addShader("standardShader.fs", GL_FRAGMENT_SHADER);
	shader.linkShader();

	Shader quadShader;
	quadShader.addShader("quadShader.vs", GL_VERTEX_SHADER);
	quadShader.addShader("quadShader.fs", GL_FRAGMENT_SHADER);
	quadShader.linkShader();
	
	Texture occlusionTexture("../Resource/textures/container2.png");
	Texture windowTexture("../Resource/textures/window.png");

	const int entityNumber = 40;
	std::vector<GLuint> queryID(40 * 40);
	//GLuint queryID[entityNumber * entityNumber];
	glGenQueries(entityNumber * entityNumber, queryID.data());

	occlusionModelMatrix = glm::translate(occlusionModelMatrix, glm::vec3(0.0, 0.0, 3.0));
	occlusionModelMatrix = glm::scale(occlusionModelMatrix, glm::vec3(1.5 * entityNumber));

	ShapeRender shapeRender;

	shapeRender.setSphereStepY(5);
	shapeRender.setSphereStepXZ(5);
	
	glm::mat4 standardMatrix;
	for (int i = 0; i < 4; i++)
		standardMatrix[i][i] = 1;
	glm::mat4 projectionMatrix = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		glm::mat4 viewMatrix = camera.getViewMatrix();
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		occlusionQueryOn = false;
		glfwPollEvents();
		Do_Movement();

		// Clear the colorbuffer
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//occlusion render
		quadShader.use();

		quadShader.setInt("uWindowTexture", 0);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, screenWidth, screenHeight);
		quadShader.setMat4("uProjectionMatrix", projectionMatrix);
		quadShader.setMat4("uViewMatrix", viewMatrix);
		quadShader.setMat4("uModelMatrix", occlusionModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, occlusionTexture.getId());
		shapeRender.renderQuad();

		//occlusion query
		shader.use();
		glDisable(GL_BLEND);
		shader.setVec3("uViewPos", camera.Position);
		shader.setMat4("uProjectionMatrix", projectionMatrix);
		shader.setMat4("uViewMatrix", viewMatrix);
		for (int i = 0; i < entityNumber; i++) {
			for (int j = 0; j < entityNumber; j++) {
				glBeginQuery(GL_SAMPLES_PASSED, queryID[i * entityNumber + j]);
				glm::mat4 modelMatrix;
				modelMatrix = glm::translate(modelMatrix, glm::vec3(i * 2, j * 2, 1.0));
				shader.setMat4("uModelMatrix", modelMatrix);
				shapeRender.renderCube();
				glEndQuery(GL_SAMPLES_PASSED);
			}
		}

		//ball render
		shader.use();
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader.setMat4("uProjectionMatrix", projectionMatrix);
		shader.setMat4("uViewMatrix", viewMatrix);
		for (int i = 0; i < entityNumber; i++) {
			for (int j = 0; j < entityNumber; j++) {
				glm::mat4 modelMatrix;
				modelMatrix = glm::translate(modelMatrix, glm::vec3(i * 2, j * 2, 1.0));
				shader.setMat4("uModelMatrix", modelMatrix);
				if (occlusionQueryOn) {
					glBeginConditionalRender(queryID[i * entityNumber + j], GL_QUERY_WAIT);
					shapeRender.renderSphere();
					glEndConditionalRender();
				}
				else {
					shapeRender.renderSphere();
				}
			}
		}

		//occlusion render
		//为了便于观察，遮挡体绘制放在了这里
		quadShader.use();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_BLEND);
		glViewport(0, 0, screenWidth, screenHeight);
		quadShader.setMat4("uProjectionMatrix", projectionMatrix);
		quadShader.setMat4("uViewMatrix", viewMatrix);
		quadShader.setMat4("uModelMatrix", occlusionModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, windowTexture.getId());
		shapeRender.renderQuad();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteQueries(100, queryID.data());
	glfwTerminate();
	return 0;
}

#pragma region "User input"

GLFWwindow * windowInit()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glewExperimental = GL_TRUE;

	return window;
}

void Do_Movement()
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
	if (keys[GLFW_KEY_RIGHT]) {
		occlusionModelMatrix = glm::translate(occlusionModelMatrix, glm::vec3(0.05, 0.0, 0.0));
	}
	if (keys[GLFW_KEY_UP]) {
		occlusionModelMatrix = glm::translate(occlusionModelMatrix, glm::vec3(0.0, 0.05, 0.0));
	}
	if (keys[GLFW_KEY_LEFT]) {
		occlusionModelMatrix = glm::translate(occlusionModelMatrix, glm::vec3(-0.05, 0.0, 0.0));
	}
	if (keys[GLFW_KEY_DOWN]) {
		occlusionModelMatrix = glm::translate(occlusionModelMatrix, glm::vec3(0.0, -0.05, 0.0));
	}
	if (keys[GLFW_KEY_O]) {
		occlusionQueryOn = true;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
#pragma endregion
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Camera.h"
#include "ShapeRender.h"
#include <chrono>
#include <random>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLFWwindow* windowInit();
void generate3DNoiseTexture();
unsigned int init3DTexture(void* vTextureData);
float CosineInterpolate(float a, float b, float x);
void generate3DNoiseTexture(GLubyte* vNoise3DTexturePtr);
float InterpolatedNoise3(float vX, float vY, float vZ);
float SmoothedNoise(float vX, float vY, float vZ);
float Noise(float vX, float vY, float vZ);
void initPermutation();
float Noise(glm::vec3 vPosition);
float grad(float x, glm::vec3 vPosition);
glm::vec3 sampleG(float x);
float perm(float x);
template<typename T> T lerp(T a, T b, float x);
glm::vec3 fade(glm::vec3 vT);

static float Permutation[256];
static float g[] = {1, 1, 0,  -1, 1, 0,  1, -1, 0,  -1, -1, 0, 
					1, 0, 1,  -1, 0, 1,  1, 0, -1,  -1, 0, -1,
					0, 1, 1,  0, -1, 1,  0, 1, -1,  0, -1, -1,
					1, 1, 0,  0, -1, 1,  -1, 1, 0,  0, -1, -1};

const GLuint WIDTH = 800, HEIGHT = 600;

Camera  camera(glm::vec3(0.0f, 1.0f, 5.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

ShapeRender shapeRender;

const int Noise3DTexSize = 32;
unsigned int noise3DTextureName = 0;

int main() {
	GLFWwindow* window = windowInit();

	glewInit();
	glViewport(0, 0, WIDTH, HEIGHT);

	initPermutation();

	Shader screenShader;
	screenShader.addShader("screenShader.vs", GL_VERTEX_SHADER);
	screenShader.addShader("screenShader.fs", GL_FRAGMENT_SHADER);
	screenShader.linkShader();

	//Texture boxTexture("../Resource/textures/container2.png");
	GLubyte* Noise3DTexturePtr = reinterpret_cast<GLubyte*>(malloc(Noise3DTexSize * Noise3DTexSize * Noise3DTexSize * 4));
	generate3DNoiseTexture(Noise3DTexturePtr);
	unsigned int Noise3DID = init3DTexture(Noise3DTexturePtr);
	Model bunny("..\\Resource\\objects\\bunny.obj");
	screenShader.use();
	screenShader.setInt("Noise", 0);
	while (!glfwWindowShouldClose(window)) {

		int maxWidth[2];
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxWidth);

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
		screenShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, Noise3DID);
		glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(WIDTH) / HEIGHT, 0.1f, 100.0f);
		screenShader.setMat4("uModelMatrix", glm::mat4());
		screenShader.setMat4("uViewMatrix", camera.getViewMatrix());
		screenShader.setMat4("uProjectionMatrix", ProjectionMatrix);
		//shapeRender.renderQuad();
		bunny.Draw(screenShader.ID);

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

float CosineInterpolate(float a, float b, float x){
	float ft = x * 3.1415927;
	float f = (1 - glm::cos(ft)) * 0.5;
	return a * (1 - f) + b * f;
}

float Noise(float vX, float vY, float vZ)
{
	int n = static_cast<int>(vX * 37 + vY * 57 + vZ * 113);
	n = (n << 13) ^ n;
	return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

float SmoothedNoise(float vX, float vY, float vZ) {
	float Corners = (Noise(vX + 1, vY - 1, vZ - 1) +
					Noise(vX - 1, vY + 1, vZ - 1) +
					Noise(vX - 1, vY - 1, vZ + 1) +
					Noise(vX + 1, vY + 1, vZ - 1) +
					Noise(vX + 1, vY - 1, vZ + 1) +
					Noise(vX - 1, vY + 1, vZ + 1) +
					Noise(vX + 1, vY + 1, vZ + 1) +
					Noise(vX - 1, vY - 1, vZ - 1)) / 64.0f;

	float Sides = (Noise(vX, vY - 1, vZ - 1) + 
					Noise(vX, vY - 1, vZ + 1) +
					Noise(vX, vY + 1, vZ - 1) + 
					Noise(vX, vY + 1, vZ + 1) + 
					
					Noise(vX - 1, vY, vZ - 1) + 
					Noise(vX - 1, vY, vZ + 1) + 
					Noise(vX + 1, vY, vZ - 1) +
					Noise(vX + 1, vY, vZ + 1) +
		
					Noise(vX - 1, vY - 1, vZ) + 
					Noise(vX - 1, vY + 1, vZ) +
					Noise(vX + 1, vY - 1, vZ) + 
					Noise(vX + 1, vY + 1, vZ)) / 32.0f;
	
	float Planes = (Noise(vX + 1, vY, vZ) + 
					Noise(vX - 1, vY, vZ) +
					Noise(vX, vY + 1, vZ) +
					Noise(vX, vY - 1, vZ) +
					Noise(vX, vY, vZ + 1) +
					Noise(vX, vY, vZ - 1)) / 16.0f;

	float Center = Noise(vX, vY, vZ) / 8.0f;

	return Corners + Sides + Planes + Center;
}

float InterpolatedNoise3(float vX, float vY, float vZ) {
	float FractX = glm::fract(vX);
	float FractY = glm::fract(vY);
	float FractZ = glm::fract(vZ);
	int IntegerX = static_cast<int>(vX - FractX);
	int IntegerY = static_cast<int>(vY - FractY);
	int IntegerZ = static_cast<int>(vZ - FractZ);

	float n1 = SmoothedNoise(IntegerX + 1, IntegerY, IntegerZ);
	float n2 = SmoothedNoise(IntegerX + 1, IntegerY + 1, IntegerZ);
	float n3 = SmoothedNoise(IntegerX + 1, IntegerY, IntegerZ + 1);
	float n4 = SmoothedNoise(IntegerX + 1, IntegerY + 1, IntegerZ + 1);
	float n5 = SmoothedNoise(IntegerX, IntegerY, IntegerZ);
	float n6 = SmoothedNoise(IntegerX, IntegerY + 1, IntegerZ);
	float n7 = SmoothedNoise(IntegerX, IntegerY, IntegerZ + 1);
	float n8 = SmoothedNoise(IntegerX, IntegerY + 1, IntegerZ + 1);

	float i1 = CosineInterpolate(n5, n1, FractX);
	float i2 = CosineInterpolate(n6, n2, FractX);
	float i3 = CosineInterpolate(n7, n3, FractX);
	float i4 = CosineInterpolate(n8, n4, FractX);

	float j1 = CosineInterpolate(i1, i2, FractY);
	float j2 = CosineInterpolate(i3, i4, FractY);

	return CosineInterpolate(j1, j2, FractZ);
}

float generatePerlinNoise3D(double* vSeed) {
	float result = 0.0f;
	float amplitude = 0.5;
	float frequency = 4;
	int Octaves = 4;
	for (int i = 0; i < Octaves; ++i) {
		amplitude = glm::pow(amplitude, static_cast<float>(i));
		frequency = glm::pow(frequency, static_cast<float>(i));
		result += InterpolatedNoise3(vSeed[0] * frequency, vSeed[1] * frequency, vSeed[2] * frequency) * amplitude;
	}
	return result;
}

void generate3DNoiseTexture(GLubyte* vNoiseTexturePtr)
{
	_ASSERTE(vNoiseTexturePtr);
	int f, i, j, k, inc;
	int startFrequency = 4;
	int numOctaves = 4;
	double ni[3];
	double inci, incj, inck;
	int frequency = startFrequency;
	GLubyte *ptr;
	//GLubyte* noise3DTexturePtr = reinterpret_cast<GLubyte*>(malloc(Noise3DTexSize * Noise3DTexSize * Noise3DTexSize * 4));
	double amp = 0.5;

	if(!vNoiseTexturePtr)
	{
		std::cout << "Bad Memory Allocate" << std::endl;
	}
	for(f = 0, inc = 0; f < numOctaves; ++f, frequency *= 2, ++inc, amp *= 0.5)
	{
		//setNoiseFrequency(frequency);
		ptr = vNoiseTexturePtr;
		ni[0] = ni[1] = ni[2] = 0;

		inci = 1.0 / (Noise3DTexSize / frequency);
		for(i = 0; i < Noise3DTexSize; ++i, ni[0] += inci)
		{
			incj = 1.0 / (Noise3DTexSize / frequency);
			ni[1] = 0;
			for(j = 0; j < Noise3DTexSize; ++j, ni[1] += incj)
			{
				inck = 1.0 / (Noise3DTexSize / frequency);
				ni[2] = 0;
				for(k = 0; k <Noise3DTexSize;++k, ni[2] += inck, ptr += 4)
				{
					*(ptr + inc) = (GLubyte)(((InterpolatedNoise3(ni[0] * frequency, ni[1] * frequency, ni[2] * frequency) + 1.0f)*amp) * 128.0f);
					//*(ptr + inc) = (GLubyte)(Noise(glm::vec3(i, j, k) / static_cast<float>(Noise3DTexSize)));
				}
			}
		}
	}
}

unsigned int init3DTexture(void* vTextureData)
{
	unsigned int TextureID;
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_3D, TextureID);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, Noise3DTexSize, Noise3DTexSize, Noise3DTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, vTextureData);

	glBindTexture(GL_TEXTURE_3D, 0);
	return TextureID;
}

void initPermutation()
{
	for(int i = 0; i < 256; ++i)
	{
		Permutation[i] = static_cast<float>(i);
	}

	unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();

	std::shuffle(&Permutation[0], &Permutation[255], std::default_random_engine(seed));
}

glm::vec3 fade(glm::vec3 vT)
{
	return vT * vT * vT * (vT * (vT * 6.0f - 15.0f) + 10.0f);
}

template<typename T>
T lerp(T a, T b, float x)
{
	return a * (1 - x) + b * x;
}

float perm(float x)
{
	float ModX = glm::mod(x, 256.0f);
	int IntegerModeX0 = static_cast<int>(ModX);
	float FractX = glm::fract(ModX);
	int IntegerModX1 = (IntegerModeX0 + 1) % 256;
	//int IntegerX = static_cast<int>(glm::floor(x * 256.0));
	//return lerp(Permutation[IntegerX], Permutation[IntegerX + 1], x);
	return Permutation[IntegerModeX0] * (1 - FractX) + Permutation[IntegerModX1] * FractX;
}

glm::vec3 sampleG(float x)
{
	float FractX = glm::fract(x);
	float X = FractX * 16.0f;
	FractX = glm::fract(X);
	int ModX = static_cast<int>(X - FractX) % 16;
	glm::vec3 a = glm::vec3(g[ModX * 3], g[ModX * 3 + 1], g[ModX * 3 + 2]);
	ModX = (ModX + 1) % 16;
	glm::vec3 b = glm::vec3(g[ModX * 3], g[ModX * 3 + 1], g[ModX * 3 + 2]);
	return a * (1 - FractX) + b * FractX;
}

float grad(float x, glm::vec3 vPosition)
{
	glm::vec3 g = sampleG(x);
	return glm::dot(glm::normalize(g), glm::normalize(vPosition));
}

float Noise(glm::vec3 vPosition)
{
	glm::vec3 FractPosition = glm::fract(vPosition);
	glm::vec3 IntegerPosition = glm::mod(vPosition - FractPosition, 256.0f);
	glm::vec3 Fade = fade(FractPosition);

	float A = perm(IntegerPosition.x) + IntegerPosition.y;
	float AA = perm(A) + IntegerPosition.z;
	float AB = perm(A + 1) + IntegerPosition.z;
	float B = perm(IntegerPosition.x + 1) + IntegerPosition.y;
	float BA = perm(B) + IntegerPosition.z;
	float BB = perm(B + 1) + IntegerPosition.z;

	float Result = lerp(lerp(lerp(grad(perm(AA), FractPosition), grad(perm(BA), FractPosition + glm::vec3(-1, 0, 0)), Fade.x),
		lerp(grad(perm(AB), FractPosition + glm::vec3(0, -1, 0)), grad(perm(BB), FractPosition + glm::vec3(-1, -1, 0)), Fade.x), Fade.y),
		lerp(lerp(grad(perm(AA + 1), FractPosition + glm::vec3(0, 0, -1)), grad(perm(BA + 1), FractPosition + glm::vec3(-1, 0, -1)), Fade.x),
			lerp(grad(perm(AB + 1), FractPosition + glm::vec3(0, -1, -1)), grad(perm(BB + 1), FractPosition + glm::vec3(-1, -1, -1)), Fade.x), Fade.y), Fade.z);
	return lerp(lerp(lerp(grad(perm(AA), FractPosition), grad(perm(BA), FractPosition + glm::vec3(-1, 0, 0)), Fade.x),
					lerp(grad(perm(AB), FractPosition + glm::vec3(0, -1, 0)), grad(perm(BB), FractPosition + glm::vec3(-1, -1, 0)), Fade.x), Fade.y),
				lerp(lerp(grad(perm(AA + 1), FractPosition + glm::vec3(0, 0, -1)), grad(perm(BA + 1), FractPosition + glm::vec3(-1, 0, -1)), Fade.x),
					lerp(grad(perm(AB + 1), FractPosition + glm::vec3(0, -1, -1)), grad(perm(BB + 1), FractPosition + glm::vec3(-1, -1, -1)), Fade.x), Fade.y),Fade.z);
}
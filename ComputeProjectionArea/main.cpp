#include <iostream>
#include <Windows.h>
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

//TO ADD:
float outputBoundingArea(const glm::mat4 projectionMatrix, const glm::mat4 viewMatrix, const glm::mat4 modelMatrix);
//init
void initOutputBoundingAreaCalculator();
void initBoundingBox();
void initProjectionInfo();
void initSideIndexSet(std::vector<int[3]>& set);
void initFaceIndexSet(std::vector<int[4]>& set);
//calculate
void calFrustumVertex(std::vector<glm::vec3>& frustumVertexSet);
bool calClipVertex(const glm::vec4 vertex0, const glm::vec4 vertex1, int faceIndex, glm::vec4& clipVertex);
bool calFrustumClipVertex(const glm::vec4 vertex0, const glm::vec4 vertex1, const glm::vec4 faceVertex0, const glm::vec4 faceVertex1, const glm::vec4 faceVertex2, glm::vec4& clipPoint);
//judge:
bool isPointInFrustum(const glm::vec3 point);
bool isPointInBoundingBox(const glm::vec3 pointInView, const glm::vec3 xAxisInView, const glm::vec3 yAxisInView, const glm::vec3 zAxisInView, float xExtentInView, float yExtentInView, float zExtentInView);
bool isPointOnSegment(const glm::vec3 point, const glm::vec3 lineVertex0, const glm::vec3 lineVertex1);
bool isPointOnFrustumFace(const glm::vec3 point, int faceIndex);
bool isPointOnQuadFace(const glm::vec3 point, const glm::vec3 faceVertex0, const glm::vec3 faceVertex1, const glm::vec3 faceVertex2);
//ADD END

const float MIN = 0.0001;
const GLuint WIDTH = 800, HEIGHT = 600;

Camera  camera(glm::vec3(0.0f, 1.0f, 7.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

ShapeRender shapeRender;

//TO ADD:
std::vector<glm::vec3> frustumVertexSet;
std::vector<int[3]> sideIndexSet(8);
std::vector<int[4]> faceIndexSet(6);
const double PI = 3.14159265358979323846264338327950288;
const double RADIAN = PI / 180.0;

struct SBox {
	glm::vec3 center;
	glm::vec3 halfExtent;
	std::vector<glm::vec3> axis;
};

struct SProjectionInfo {
	double fov;
	double aspect;
	double nearPlane;
	double farPlane;
	double nearHalfWidth;
	double nearHalfHeight;
};

SBox box;
SProjectionInfo projectionInfo;
//ADD END

int main() {
	GLFWwindow* window = windowInit();

	glewInit();

	Shader shader;
	shader.addShader("vertShader.glsl", GL_VERTEX_SHADER);
	shader.addShader("fragShader.glsl", GL_FRAGMENT_SHADER);
	shader.linkShader();

	Texture boxTexture("../Resource/textures/container2.png");

	shader.use();
	shader.setInt("uTexture", 0);
	
	initOutputBoundingAreaCalculator();

	float timeSum = 0.0f;
	float frame = 0.0;
	while (!glfwWindowShouldClose(window)) {
		frame += 1.0;
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();

		glViewport(0, 0, WIDTH, HEIGHT);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glPointSize(10);
		shader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, boxTexture.getId());
		glm::mat4 modelMatrix;
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, 5.0));
		glm::mat4 projectionMatrix = glm::perspective(projectionInfo.fov, projectionInfo.aspect, projectionInfo.nearPlane, projectionInfo.farPlane);

		float fBegin = glfwGetTime();
		float area = outputBoundingArea(projectionMatrix, camera.getViewMatrix(), modelMatrix);
		float fEnd = glfwGetTime();
		timeSum += fBegin - fEnd;

		float averageTime = timeSum / frame;
		//std::cout << "Time:" << fBegin - fEnd << std::endl;
		std::cout << "Area:" << area << std::endl;

		shader.setMat4("uModelMatrix", modelMatrix);
		shader.setMat4("uViewMatrix", camera.getViewMatrix());
		shader.setMat4("uProjectionMatrix", projectionMatrix);
		shapeRender.renderCube();

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

//TO ADD:
float outputBoundingArea(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, glm::mat4 modelMatrix) {

	//PART1：计算世界空间顶点坐标
	std::vector<glm::vec3> originVertexSet(8);

	glm::vec3 axisLength0 = box.axis[0] * box.halfExtent[0];
	glm::vec3 axisLength1 = box.axis[1] * box.halfExtent[1];
	glm::vec3 axisLength2 = box.axis[2] * box.halfExtent[2];

	originVertexSet[0] = box.center + axisLength0 + axisLength1 + axisLength2;//right front top
	originVertexSet[1] = box.center + axisLength0 + axisLength1 - axisLength2;//right front bottom
	originVertexSet[2] = box.center + axisLength0 - axisLength1 + axisLength2;//right back top
	originVertexSet[3] = box.center + axisLength0 - axisLength1 - axisLength2;//right back bottom
	originVertexSet[4] = box.center - axisLength0 + axisLength1 + axisLength2;//left front top
	originVertexSet[5] = box.center - axisLength0 + axisLength1 - axisLength2;//left front bottom
	originVertexSet[6] = box.center - axisLength0 - axisLength1 + axisLength2;//left back top
	originVertexSet[7] = box.center - axisLength0 - axisLength1 - axisLength2;//left back bottom

	//PART2：在相机坐标下，根据视锥体，重置坐标

	int zCount = 0;//表示在相机近平面后的物体

	std::vector<glm::vec4> viewVertexSet;
	glm::mat4 vmMatrix = viewMatrix * modelMatrix;
	for (int i = 0; i < originVertexSet.size(); ++i) {
		glm::vec4 vertex = vmMatrix * glm::vec4(originVertexSet[i], 1.0f);
		viewVertexSet.push_back(vertex);
		if (vertex.z > -0.1f || vertex.z < -100.0f)	zCount++;
	}
	//全部在近平面后，相机看不到任何东西
	if (zCount == 8) 
		return 0.0;
	//PART3：如果近平面在箱子内，为4
	glm::vec3 centerInView = glm::vec3(vmMatrix * glm::vec4(box.center, 1.0));

	glm::vec3 xVector = glm::vec3(vmMatrix * glm::vec4(box.center + box.halfExtent[0] * box.axis[0], 1.0)) - centerInView;
	glm::vec3 yVector = glm::vec3(vmMatrix * glm::vec4(box.center + box.halfExtent[1] * box.axis[1], 1.0)) - centerInView;
	glm::vec3 zVector = glm::vec3(vmMatrix * glm::vec4(box.center + box.halfExtent[2] * box.axis[2], 1.0)) - centerInView;

	glm::vec3 xAxisInView = normalize(xVector);
	glm::vec3 yAxisInView = normalize(yVector);
	glm::vec3 zAxisInView = normalize(zVector);

	float xExtentInView = glm::length(xVector);
	float yExtentInView = glm::length(yVector);
	float zExtentInView = glm::length(zVector);

	if(isPointInBoundingBox(centerInView - glm::vec3( projectionInfo.nearHalfWidth,  projectionInfo.nearHalfHeight, -projectionInfo.nearPlane), xAxisInView, yAxisInView, zAxisInView, xExtentInView, yExtentInView, zExtentInView)
	&& isPointInBoundingBox(centerInView - glm::vec3(-projectionInfo.nearHalfWidth,  projectionInfo.nearHalfHeight, -projectionInfo.nearPlane), xAxisInView, yAxisInView, zAxisInView, xExtentInView, yExtentInView, zExtentInView)
	&& isPointInBoundingBox(centerInView - glm::vec3(-projectionInfo.nearHalfWidth, -projectionInfo.nearHalfHeight, -projectionInfo.nearPlane), xAxisInView, yAxisInView, zAxisInView, xExtentInView, yExtentInView, zExtentInView)
	&& isPointInBoundingBox(centerInView - glm::vec3( projectionInfo.nearHalfWidth, -projectionInfo.nearHalfHeight, -projectionInfo.nearPlane), xAxisInView, yAxisInView, zAxisInView, xExtentInView, yExtentInView, zExtentInView))
		return 4.0f;

	//EXTRA PART:相机空间下的剪裁
	//判断每个点是否在视锥体内
	std::vector<bool> vertexStateInView(8);
	std::vector<bool> frustumVertexStateInBoundingBox(8);
	std::vector<glm::vec4> clipVertexSet;
	for (int i = 0; i < vertexStateInView.size(); ++i) {
		vertexStateInView[i] = isPointInFrustum(viewVertexSet[i]);
		if (vertexStateInView[i]) clipVertexSet.push_back(viewVertexSet[i]);
	}
	for (int i = 0; i < frustumVertexStateInBoundingBox.size(); ++i) {
		frustumVertexStateInBoundingBox[i] = isPointInBoundingBox(centerInView - frustumVertexSet[i], xAxisInView, yAxisInView, zAxisInView, xExtentInView, yExtentInView, zExtentInView);
	}
	
	//求面与面的交点，如果点在体外，判断相邻三个点是否有在视锥体内的，如果有，求剪裁点
	for (int i = 0; i < vertexStateInView.size(); ++i) {
		if (!vertexStateInView[i]) {
			for (int j = 0; j < 3; ++j) {
				int intersectCount = 0;
				for (int k = 0; k < faceIndexSet.size(); ++k) {
					glm::vec4 clipPoint;
					if (calClipVertex(viewVertexSet[i], viewVertexSet[sideIndexSet[i][j]], k, clipPoint)) {
						intersectCount++;
						clipVertexSet.push_back(clipPoint);
					}
					if (intersectCount == 2) break;
				}
			}
		}
	}
	
	for (int i = 0; i < frustumVertexStateInBoundingBox.size(); ++i) {
		if (!frustumVertexStateInBoundingBox[i]) {
			for (int j = 0; j < 3; ++j) {
				int intersectCount = 0;
				for (int k = 0; k < faceIndexSet.size(); ++k) {
					glm::vec4 clipPoint;
					if (calFrustumClipVertex(glm::vec4(frustumVertexSet[i], 1.0), glm::vec4(frustumVertexSet[sideIndexSet[i][j]], 1.0), viewVertexSet[faceIndexSet[k][0]], viewVertexSet[faceIndexSet[k][1]], viewVertexSet[faceIndexSet[k][2]], clipPoint)) {
						intersectCount++;
						clipVertexSet.push_back(clipPoint);
					}
					if (intersectCount == 2) break;
				}
			}
		}
	}

	if (clipVertexSet.size() <= 2) return 0.0;

	//PART4：投影到屏幕上，取Y最小点为起始点
	std::vector<glm::vec2> vertexSet;

	float minY = 1;
	int minYIndex = 0;
	for (int i = 0; i < clipVertexSet.size(); ++i) {
		glm::vec4 vertex = projectionMatrix * clipVertexSet[i];
		glm::vec3 screenVertex = glm::vec3(vertex / vertex.w);
		vertexSet.push_back(glm::clamp(glm::vec2(screenVertex), glm::vec2(-1.0, -1.0), glm::vec2(1.0, 1.0)));
		if (vertexSet.back().y < minY) {
			minY = vertexSet.back().y;
			minYIndex = vertexSet.size() - 1;
		}
	}

	//DELETED PART5：判断是否在一条线上

	//PART6：构建顺时针多边形顶点
	glm::dvec2 referenceVector = glm::dvec2(-1.0, 0.0);
	std::vector<int> indexSet;
	indexSet.push_back(minYIndex);
	bool existIndexFlag = false;
	do {
		int minAngleIndex = 0;
		double minAngleCos = -1.0;
		for (int i = 0; i < vertexSet.size(); ++i) {
			existIndexFlag = false;
			for (int j = indexSet.back() == indexSet.front() ? 0 : 1 ; j < indexSet.size(); j++) {
				if (i == indexSet[j]) {
					existIndexFlag = true;
					break;
				}
			}
			if (!existIndexFlag) {
				double angleCos = glm::dot(glm::normalize(referenceVector), glm::normalize(glm::dvec2(vertexSet[i]) - glm::dvec2(vertexSet[indexSet.back()])));
				if (angleCos > minAngleCos) {
					minAngleCos = angleCos;
					minAngleIndex = i;
				}
			}
		}
		referenceVector = glm::vec2(vertexSet[minAngleIndex] - vertexSet[indexSet.back()]);
		indexSet.push_back(minAngleIndex);
	} while (indexSet.back() != indexSet.front() && indexSet.size() <= 24);

	//插入陈文倩代码

	//PART7：计算多边形面积（三角形）
	float sumArea = 0.0f;
	for (int i = 1; i < indexSet.size() - 2; ++i) {
		glm::vec3 v1 = glm::vec3(vertexSet[indexSet[i]] - vertexSet[indexSet[0]], 0.0);
		glm::vec3 v2 = glm::vec3(vertexSet[indexSet[i + 1]] - vertexSet[indexSet[0]], 0.0);
		sumArea += 0.5 * glm::length(glm::cross(v1, v2));
	}
	return sumArea;
 }

void calFrustumVertex(std::vector<glm::vec3>& frustumVertexSet) {

	//29.5是针对FOV = 45.0f 调出来的
	double fovTanValue = std::tan(29.5f * RADIAN);

	projectionInfo.nearHalfHeight = projectionInfo.nearPlane * fovTanValue;
	projectionInfo.nearHalfWidth = projectionInfo.nearHalfHeight * projectionInfo.aspect;

	frustumVertexSet.push_back(glm::vec3( projectionInfo.nearHalfWidth,  projectionInfo.nearHalfHeight, -projectionInfo.nearPlane));
	frustumVertexSet.push_back(glm::vec3( projectionInfo.nearHalfWidth, -projectionInfo.nearHalfHeight, -projectionInfo.nearPlane));
	frustumVertexSet.push_back(glm::vec3(-projectionInfo.nearHalfWidth,  projectionInfo.nearHalfHeight, -projectionInfo.nearPlane));
	frustumVertexSet.push_back(glm::vec3(-projectionInfo.nearHalfWidth, -projectionInfo.nearHalfHeight, -projectionInfo.nearPlane));

	float farNearRatio = projectionInfo.farPlane / projectionInfo.nearPlane;
	frustumVertexSet.push_back(glm::vec3( projectionInfo.nearHalfWidth,  projectionInfo.nearHalfHeight, -projectionInfo.nearPlane) * farNearRatio);
	frustumVertexSet.push_back(glm::vec3( projectionInfo.nearHalfWidth, -projectionInfo.nearHalfHeight, -projectionInfo.nearPlane) * farNearRatio);
	frustumVertexSet.push_back(glm::vec3(-projectionInfo.nearHalfWidth,  projectionInfo.nearHalfHeight, -projectionInfo.nearPlane) * farNearRatio);
	frustumVertexSet.push_back(glm::vec3(-projectionInfo.nearHalfWidth, -projectionInfo.nearHalfHeight, -projectionInfo.nearPlane) * farNearRatio);
}

bool isPointInFrustum(const glm::vec3 pointInView) {
	if (pointInView.z < -projectionInfo.farPlane || pointInView.z > -projectionInfo.nearPlane) return false;
	float zRatio = pointInView.z / -projectionInfo.nearPlane;
	if (pointInView.x < -projectionInfo.nearHalfWidth * zRatio || pointInView.x > projectionInfo.nearHalfWidth * zRatio || pointInView.y > projectionInfo.nearHalfHeight * zRatio || pointInView.y < -projectionInfo.nearHalfHeight * zRatio) return false;
	return true;
}

bool isPointInBoundingBox(const glm::vec3 pointInView, const glm::vec3 xAxisInView, const glm::vec3 yAxisInView, const glm::vec3 zAxisInView, float xExtentInView, float yExtentInView, float zExtentInView) {
	if (std::abs(glm::dot(pointInView, xAxisInView)) < xExtentInView && std::abs(glm::dot(pointInView, yAxisInView)) < yExtentInView && std::abs(glm::dot(pointInView, zAxisInView)) < zExtentInView)
		return true;
	else return false;
}

void initBoundingBox() {
	box.center = glm::vec3(0.0, 0.0, 0.0);
	box.halfExtent = glm::vec3(1, 1, 1);
	box.axis.push_back(glm::vec3(1, 0, 0));
	box.axis.push_back(glm::vec3(0, 1, 0));
	box.axis.push_back(glm::vec3(0, 0, 1));
}

void initProjectionInfo() {
	projectionInfo.fov = camera.Zoom;
	projectionInfo.aspect = (float)WIDTH / (float)HEIGHT;
	projectionInfo.nearPlane = 0.1f;
	projectionInfo.farPlane = 100.0f;
}

void initSideIndexSet(std::vector<int[3]>& set){
	set[0][0] = 1;
	set[0][1] = 2;
	set[0][2] = 4;
	set[1][0] = 0;
	set[1][1] = 3;
	set[1][2] = 5;
	set[2][0] = 0;
	set[2][1] = 3;
	set[2][2] = 6;
	set[3][0] = 1;
	set[3][1] = 2;
	set[3][2] = 7;
	set[4][0] = 0;
	set[4][1] = 5;
	set[4][2] = 6;
	set[5][0] = 1;
	set[5][1] = 4;
	set[5][2] = 7;
	set[6][0] = 2;
	set[6][1] = 7;
	set[6][2] = 4;
	set[7][0] = 3;
	set[7][1] = 5;
	set[7][2] = 6;
}

bool calClipVertex(const glm::vec4 vertex0, const glm::vec4 vertex1, int faceIndex, glm::vec4& clipPoint) {
	
	glm::vec3 lineVertex0 = glm::vec3(vertex0);
	glm::vec3 lineVertex1 = glm::vec3(vertex1);
	glm::vec3 planeVector0 = glm::vec3(frustumVertexSet[faceIndexSet[faceIndex][0]] - frustumVertexSet[faceIndexSet[faceIndex][1]]);
	glm::vec3 planeVector1 = glm::vec3(frustumVertexSet[faceIndexSet[faceIndex][0]] - frustumVertexSet[faceIndexSet[faceIndex][2]]);
	glm::vec3 planeNormalVector = glm::cross(planeVector0, planeVector1);

	glm::vec3 lineVector = lineVertex0 - lineVertex1;

	float parameter;
	float R = glm::dot(lineVector, planeNormalVector);

	if (glm::abs(R) < 1e-8)
	{
		return false;
	}
	else
	{
		parameter = glm::dot(frustumVertexSet[faceIndexSet[faceIndex][0]] - lineVertex0, planeNormalVector) / R;
		clipPoint = glm::vec4(lineVertex0 + lineVector * parameter, 1.0);
	}
	return isPointOnSegment(clipPoint, lineVertex0, lineVertex1) && isPointOnFrustumFace(clipPoint, faceIndex);
}

bool calFrustumClipVertex(const glm::vec4 vertex0, const glm::vec4 vertex1, const glm::vec4 faceVertex0, const glm::vec4 faceVertex1, const glm::vec4 faceVertex2, glm::vec4& clipPoint) {

	glm::vec3 lineVertex0 = glm::vec3(vertex0);
	glm::vec3 lineVertex1 = glm::vec3(vertex1);
	glm::vec3 planeVector0 = glm::vec3(faceVertex0 - faceVertex1);
	glm::vec3 planeVector1 = glm::vec3(faceVertex0 - faceVertex2);
	glm::vec3 planeNormalVector = glm::cross(planeVector0, planeVector1);

	glm::vec3 lineVector = lineVertex0 - lineVertex1;

	float parameter;
	float R = glm::dot(lineVector, planeNormalVector);

	if (glm::abs(R) < 1e-8)
	{
		return false;
	}
	else
	{
		parameter = glm::dot(glm::vec3(faceVertex0) - lineVertex0, planeNormalVector) / R;
		clipPoint = glm::vec4(lineVertex0 + lineVector * parameter, 1.0);
	}
	return isPointOnSegment(clipPoint, lineVertex0, lineVertex1) && isPointOnQuadFace(clipPoint, faceVertex0, faceVertex1, faceVertex2);
}

void initFaceIndexSet(std::vector<int[4]>& set) {
	//front
	set[0][0] = 0;
	set[0][1] = 1;
	set[0][2] = 3;
	set[0][3] = 2;
	//left
	set[1][0] = 2;
	set[1][1] = 3;
	set[1][2] = 7;
	set[1][3] = 6;
	//top
	set[2][0] = 0;
	set[2][1] = 2;
	set[2][2] = 6;
	set[2][3] = 4;
	//right
	set[3][0] = 0;
	set[3][1] = 1;
	set[3][2] = 5;
	set[3][3] = 4;
	//bottom
	set[4][0] = 1;
	set[4][1] = 3;
	set[4][2] = 7;
	set[4][3] = 5;
	//back
	set[5][0] = 4;
	set[5][1] = 5;
	set[5][2] = 7;
	set[5][3] = 6;
}

void initOutputBoundingAreaCalculator() {
	initBoundingBox();
	initProjectionInfo();
	initSideIndexSet(sideIndexSet);
	calFrustumVertex(frustumVertexSet);
	initFaceIndexSet(faceIndexSet);
}

bool isPointOnSegment(const glm::vec3 point, const glm::vec3 lineVertex0, const glm::vec3 lineVertex1)
{
	for (int i = 0; i < 3; ++i) {
		if (lineVertex0[i] < lineVertex1[i]) { 
			if(point[i] < lineVertex0[i] || point[i] > lineVertex1[i]) return false; 
		}
		else if (point[i] > lineVertex0[i] || point[i] < lineVertex1[i]) return false;
	}
	return true;
}

bool isPointOnFrustumFace(const glm::vec3 point, int faceIndex) {
	//这个函数针对视锥体
	//0,5是视锥体两个矩形面,进行人为优化
	if (faceIndex == 0) {
		if (point.x > projectionInfo.nearHalfWidth || point.x < -projectionInfo.nearHalfWidth || point.y > projectionInfo.nearHalfHeight || point.y < -projectionInfo.nearHalfHeight) return false;
		else return true;
	}
	else if (faceIndex == 5) {
		float factor = projectionInfo.farPlane / projectionInfo.nearPlane;
		if (point.x > factor * projectionInfo.nearHalfWidth || point.x < -factor * projectionInfo.nearHalfWidth || point.y > factor * projectionInfo.nearHalfHeight || point.y < -factor * projectionInfo.nearHalfHeight) return false;
		else return true;
	}
	//1,3是左右面，x，z成比例
	else if(faceIndex == 1 || faceIndex == 3){
		if (point.y > glm::abs(point.x / projectionInfo.aspect) || point.y < -glm::abs(point.x / projectionInfo.aspect) || point.z > -projectionInfo.nearPlane || point.z < -projectionInfo.farPlane) return false;
		else return true;
	}
	//2,4为上下面
	else{
		if (point.x > glm::abs(point.y * projectionInfo.aspect) || point.x < -glm::abs(point.y * projectionInfo.aspect) || point.z > -projectionInfo.nearPlane || point.z < -projectionInfo.farPlane) return false;
		else return true;
	}
}

bool isPointOnQuadFace(const glm::vec3 point, const glm::vec3 faceVertex0, const glm::vec3 faceVertex1, const glm::vec3 faceVertex2) {
	//to improve
	glm::vec3 vector0 = faceVertex0 - faceVertex1;
	glm::vec3 vector1 = faceVertex2 - faceVertex1;
	float length0 = glm::length(vector0);
	float length1 = glm::length(vector1);
	glm::vec3 axis0 = glm::normalize(vector0);
	glm::vec3 axis1 = glm::normalize(vector1);
	glm::vec3 pointVector = point - faceVertex1;
	if (glm::dot(pointVector, axis0) > length0 || glm::dot(pointVector, axis0) < 0 || glm::dot(pointVector, axis1) > length1 || glm::dot(pointVector, axis1) < 0) return false;
	return true;
}
//ADD END
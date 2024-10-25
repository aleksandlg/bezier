#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.hpp"
#include <string>
#include <vector>
using namespace std;

unsigned WIDTH = 800, HEIGHT = 600;

unsigned numPoints;	
vector<float> controlPointsArr;	
unsigned curveSize, polarSize;
float *curvePoints, *polarPoints, *dummy;	

float precision = 0.005;

bool flagEval = false, flagPolygon = true, flagCurve = true, flagPolar = true;

//calculates the number of all points needed based on the number of control polygon points
unsigned calc(unsigned n) {
	if (n < 2) return 0;
	if (n == 2) return 3;
	return n + calc(n - 1);
}

//maps the taken coordinates from the screen to [-1, 1]
//also taking in account that (0, 0) is in the center of the screen not in the upper left corner
void normalize(float& x, float& y) {
	x = -1 + (x / WIDTH)*2 ;
	y = 1 - (y / HEIGHT)*2 ;
}

//because it would write the point more times the longer the mouse click was being held
bool isUnique(float& x, float& y) {
	normalize(x, y);

	if (numPoints == 0) return true;

	//the most common case put out for faster check
	if (x == controlPointsArr[2 * numPoints - 2] && y == controlPointsArr[2 * numPoints - 1]) return false;

	for (unsigned i = 0; i < 2 * numPoints; i+=2)
		if (x == controlPointsArr[i] && y == controlPointsArr[i + 1])
			return false;

	return true;
}

//draw the control polygon for the Bezier curve
void drawPolygon() {
	if (numPoints < 2) return;

	unsigned indxSize = numPoints + (numPoints - 2);
	GLuint *index = new GLuint[indxSize];
	index[0] = 0;
	index[1] = 1;
	if (indxSize >= 4) {
		for (unsigned i = 2; i < indxSize - 1; i += 2) {
			index[i] = index[i - 1];
			index[i + 1] = index[i] + 1;
		}
	}

	float* controlPoints = new float[2 * numPoints];

	for (unsigned i = 0; i < 2 * numPoints; i++)
		controlPoints[i] = controlPointsArr[i];

	GLuint vertexVbo, colorVbo, ibo, vao;
	glGenBuffers(1, &vertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
	glBufferData(GL_ARRAY_BUFFER, 2 * numPoints * sizeof(float), controlPoints, GL_STREAM_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indxSize * sizeof(GLuint), index, GL_STREAM_DRAW);

	const string vertexShader = "vertex.txt", fragmentShader = "fragment.txt";
	Shader shader(vertexShader, fragmentShader);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
	GLint posAttrib = glGetAttribLocation(shader.getID(), "aPos");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_TRUE, sizeof(float) * 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	shader.activate();

	glBindVertexArray(vao);

	glDrawElements(GL_LINES, indxSize, GL_UNSIGNED_INT, 0);

	glDeleteBuffers(1, &vertexVbo);
	glDeleteBuffers(1, &colorVbo);
	glDeleteBuffers(1, &ibo);
	shader.destroy();
	glDeleteVertexArrays(1, &vao);

	delete[] controlPoints;
	delete[]index;
}

//recursively evaluates using the algorithm of deCasteljau
void evaluateAux(float t, unsigned numControlPoints, unsigned startPos) {
	if (numControlPoints <= 1) return;

	for (unsigned i = startPos; i < startPos + 2 * numControlPoints - 2; i += 2) {
		dummy[i + 2 * numControlPoints] = (1 - t)*dummy[i] + t * dummy[i + 2];
		dummy[i + 2 * numControlPoints + 1] = (1 - t)*dummy[i + 1] + t * dummy[i + 3];
	}

	evaluateAux(t, numControlPoints - 1, startPos + 2 * numControlPoints);
}

//evaluates the points needed to draw the curve
void evaluateCurve(float precision) {
	if (numPoints < 3) return;

	if (curveSize > 0) {
		delete[] curvePoints;
		curveSize = 0;
	}

	curveSize = 2 + 2 * (1 / precision);
	curvePoints = new float[curveSize];

	unsigned dummySize = 2 * calc(numPoints);
	dummy = new float[dummySize];

	unsigned size = controlPointsArr.size();
	for (unsigned i = 0; i < size; i++) 
		dummy[i] = controlPointsArr[i];

	unsigned k = 0; 
	for (float i = 0; i <= 1, k < curveSize; i += precision) {
		evaluateAux(i, numPoints, 0);

		curvePoints[k++] = dummy[dummySize - 2];
		curvePoints[k++] = dummy[dummySize - 1];

		//clean the dummy; leaves only the control polygon points
		for (unsigned i = size; i < dummySize; i++)
			dummy[i] = 0;
	}

	delete[] dummy;
}

void drawCurve() {
	if (numPoints < 3) return;

	unsigned indxSize = (curveSize/2) + ((curveSize/2) - 2);
	GLuint *index = new GLuint[indxSize];
	index[0] = 0;
	index[1] = 1;
	if (indxSize >= 4) {
		for (unsigned i = 2; i < indxSize - 1; i += 2) {
			index[i] = index[i - 1];
			index[i + 1] = index[i] + 1;
		}
	}

	GLuint vertexVbo, colorVbo, ibo, vao;
	glGenBuffers(1, &vertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
	glBufferData(GL_ARRAY_BUFFER, curveSize * sizeof(float), curvePoints, GL_STREAM_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indxSize * sizeof(GLuint), index, GL_STREAM_DRAW);

	const string vertexShader = "vertex.txt", fragmentShader = "curveF.txt";
	Shader shader(vertexShader, fragmentShader);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
	GLint posAttrib = glGetAttribLocation(shader.getID(), "aPos");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_TRUE, sizeof(float) * 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	shader.activate();

	glBindVertexArray(vao);

	glDrawElements(GL_LINES, indxSize, GL_UNSIGNED_INT, 0);

	glDeleteBuffers(1, &vertexVbo);
	glDeleteBuffers(1, &colorVbo);
	glDeleteBuffers(1, &ibo);
	shader.destroy();
	glDeleteVertexArrays(1, &vao);
	
	delete[]index;
}

void evaluatePolar(float precision) {
	if (numPoints < 3) return;

	if (polarSize > 0) {
		delete[] polarPoints;
		polarSize = 0;
	}

	polarSize = 2 + 2 * (1 / precision);
	polarPoints = new float[polarSize];

	unsigned dummySize = 2 * calc(numPoints - 1);
	dummy = new float[dummySize];

	unsigned size = controlPointsArr.size();

	for (unsigned i = 0; i < size - 2; i++) 
		dummy[i] = 0.5*controlPointsArr[i] + 0.5*controlPointsArr[i + 2];

	unsigned k = 0;
	for (float i = 0; i <= 1, k < polarSize; i += precision) {
		evaluateAux(i, numPoints-1, 0);

		polarPoints[k++] = dummy[dummySize - 2];
		polarPoints[k++] = dummy[dummySize - 1];

		//clean the dummy; leaves only the control polygon points
		for (unsigned i = size; i < dummySize; i++)
			dummy[i] = 0;
	}

	delete[] dummy;
}

void drawPolar() {
	if (numPoints < 3) return;

	unsigned indxSize = (polarSize / 2) + ((polarSize / 2) - 2);
	GLuint *index = new GLuint[indxSize];
	index[0] = 0;
	index[1] = 1;
	if (indxSize >= 4) {
		for (unsigned i = 2; i < indxSize - 1; i += 2) {
			index[i] = index[i - 1];
			index[i + 1] = index[i] + 1;
		}
	}

	GLuint vertexVbo, colorVbo, ibo, vao;
	glGenBuffers(1, &vertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
	glBufferData(GL_ARRAY_BUFFER, polarSize * sizeof(float), polarPoints, GL_STREAM_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indxSize * sizeof(GLuint), index, GL_STREAM_DRAW);

	const string vertexShader = "vertex.txt", fragmentShader = "polarF.txt";
	Shader shader(vertexShader, fragmentShader);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
	GLint posAttrib = glGetAttribLocation(shader.getID(), "aPos");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_TRUE, sizeof(float) * 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	shader.activate();

	glBindVertexArray(vao);

	glDrawElements(GL_LINES, indxSize, GL_UNSIGNED_INT, 0);

	glDeleteBuffers(1, &vertexVbo);
	glDeleteBuffers(1, &colorVbo);
	glDeleteBuffers(1, &ibo);
	shader.destroy();
	glDeleteVertexArrays(1, &vao);

	delete[]index;
}

void processInput(GLFWwindow *window){
	//close window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//select point
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		double x1, y1;
		glfwGetCursorPos(window, &x1, &y1);
		float x = x1, y = y1;

		if (isUnique(x ,y)) {
			numPoints++;
			controlPointsArr.push_back(x);
			controlPointsArr.push_back(y);

			flagEval = true;
		}
	}

	//evaluate only when needed 
	if (flagEval) {
		evaluateCurve(precision);
		evaluatePolar(precision);
		flagEval = false;
	}

	//draw polygon
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		if (flagPolygon) flagPolygon = false;
		else flagPolygon = true;
	}

	//draw curve
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		if (flagCurve) flagCurve = false;
		else flagCurve = true;
	}

	//draw polar
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		if (flagPolar) flagPolar = false;
		else flagPolar = true;
	}

	//clear window and reset
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		controlPointsArr.clear();
		numPoints = 0;

		flagCurve = true;
		flagPolar = true;
		flagPolygon = true;
		flagEval = false;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	WIDTH = width;
	HEIGHT = height;
	glViewport(0, 0, width, height);
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Bezier Polar", nullptr, nullptr);
	if (!window) {
		cerr << "Error::Window::Create\n";
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		cerr << "Error::GLAD::Initialize\n";
		return 1;
	}

	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

	numPoints = 0;
	curveSize = 0;
	polarSize = 0;

	while (!glfwWindowShouldClose(window)){
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (flagPolygon) drawPolygon();
		if (flagCurve) drawCurve();
		if (flagPolar) drawPolar();

		glfwSwapInterval(1);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	delete[] curvePoints;
	delete[] polarPoints;
	
	glfwTerminate();
	return 0;
}
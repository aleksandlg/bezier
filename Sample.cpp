/*#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.hpp"
#include <string>
#include <vector>
using namespace std;

//===================================================================================================
unsigned WIDTH = 800, HEIGHT = 600;

unsigned numPoints;		//number of initial control points 
vector<float> controlPointsArr;		//to store the points for the control polygon
unsigned allSize;
float* allPoints;	//where all control points needed for the bezier curve will be stored 

//calculates the number of all points needed based on the current number of initial control points
unsigned calc(unsigned n) {
	if (n == 2) return 3;
	return n + calc(n - 1);
}

//maps the taken coordinates from the screen to [-1, 1] so openGL can draw them
void normalize(float& x, float& y) {
	cout << "given: " << x << ' ' << y << endl;
	
	x = -1 + (x / HEIGHT)*2 ;
	y = 1 - (y / WIDTH)*2 ;

	cout << "new: " << x << ' ' << y << endl;
}

//because it would write the point more times the longer the mouse click was being held
bool isUnique(float& x, float& y) {
	normalize(x, y);

	if (numPoints == 0) return true;

	//because this will be the most common case i put it out for faster check
	if (x == controlPointsArr[2 * numPoints - 2] && y == controlPointsArr[2 * numPoints - 1]) return false;

	for (unsigned i = 0; i < 2 * numPoints; i += 2)
		if (x == controlPointsArr[i] && y == controlPointsArr[i + 1])
			return false;

	return true;
}

//recursively evaluates the points needed to draw the curve
void evaluateAux(float t, unsigned numControlPoints, unsigned startPos) {
	if (numControlPoints <= 1) return;

	for (unsigned i = startPos; i < startPos + 2 * numControlPoints - 2; i += 2) {
		allPoints[i + 2 * numControlPoints] = (1 - t)*allPoints[i] + t * allPoints[i + 2];
		allPoints[i + 2 * numControlPoints + 1] = (1 - t)*allPoints[i + 1] + t * allPoints[i + 3];
		normalize(allPoints[i + 2 * numControlPoints], allPoints[i + 2 * numControlPoints + 1]);
	}
	evaluateAux(t, numControlPoints - 1, startPos + 2 * numControlPoints);
}

//draw the control  polygon for the Bezier curve
void drawPoligon() {

	if (numPoints < 2) return;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	float colors[] = { 1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
						1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f };

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

	cout << "index: ";
	for (unsigned i = 0; i < indxSize; i++)
		cout << index[i] << ' ';
	cout << endl;
	cout << "allPoints: ";
	for (int i = 0; i < allSize; i++)
		cout << allPoints[i] << ' ';
	cout << endl;

	GLuint vertexVbo, colorVbo, ibo, vao;
	glGenBuffers(1, &vertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
	glBufferData(GL_ARRAY_BUFFER, allSize * sizeof(float), allPoints, GL_STREAM_DRAW);

	glGenBuffers(1, &colorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
	glBufferData(GL_ARRAY_BUFFER, 30 * sizeof(float), colors, GL_STREAM_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indxSize * sizeof(GLuint), index, GL_STREAM_DRAW);

	const string vertexShader = "vertex.txt",
		fragmentShader = "fragment.txt";
	Shader shader(vertexShader, fragmentShader);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
	GLint posAttrib = glGetAttribLocation(shader.getID(), "aPos");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
	GLint colorAttrib = glGetAttribLocation(shader.getID(), "aColor");
	glEnableVertexAttribArray(colorAttrib);
	glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_TRUE, sizeof(float) * 3, (void*)0);

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


//evaluates using the algorithm of deCasteljau
void evaluatePoligon(float precision) {
	if (numPoints <= 1) return;

	if (allSize > 0) {
		delete[] allPoints;
		allSize = 0;
	}

	allSize = 2 * numPoints/*calc(numPoints)*/;
/*	allPoints = new float[allSize];

	unsigned size = controlPointsArr.size();
	for (unsigned i = 0; i < size; i++) {
		allPoints[i] = controlPointsArr[i];
	}

	/*for (int i = 0; i < 1; i += precision)
		evaluateAux(i, numPoints, 0);*/
/*
}

//===================================================================================================

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		double x1, y1;
		glfwGetCursorPos(window, &x1, &y1);
		cout << "mouse derived: " << x1 << ' ' << y1 << endl;
		float arr[6] = {400, 300,
						100, 300,
						300, 150};

		for (int i = 0; i < 6; i += 2) {
			if (isUnique(arr[i], arr[i + 1])) {
				numPoints++;
				cout << "push " << arr[i] << ' ' << arr[i + 1];
				controlPointsArr.push_back(arr[i]);
				controlPointsArr.push_back(arr[i+1]);
				evaluatePoligon(0.25);

				drawPoligon();
			}
		}
	}

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	WIDTH = width;
	HEIGHT = height;
	glViewport(0, 0, width, height);
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Bezier Polar", nullptr, nullptr);
	if (!window) {
		cerr << "Error::Window::Create\n";
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cerr << "Error::GLAD::Initialize\n";
		return 1;
	}
	
	glViewport(0, 0, WIDTH, HEIGHT);
/*	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	//===================================================================================================

	numPoints = 0;
	allSize = 0;



	//===================================================================================================

	while (!glfwWindowShouldClose(window)) {

		processInput(window);

		glfwSwapInterval(1);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	delete[] allPoints;

	glfwTerminate();
	return 0;
}*/
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

class Shader {
private:
	unsigned ID;
public:
	Shader(string = " ", string = " ");

	unsigned getID() const { return this->ID; }

	void activate();
	void destroy();
	void setBool(const string&, bool);
	void setInt(const string&, int);
	void setFloat(const string&, float);
};

Shader::Shader(string vertexPath, string fragmentPath) {
	if (vertexPath == " " || fragmentPath == " ") return;
	string vertexSource, fragmentSource;
	ifstream vertexFile, fragmentFile;

	vertexFile.open(vertexPath);
	fragmentFile.open(fragmentPath);
 	if (!vertexFile) {
		cerr << "Error opening vertex\n" << endl;
	}
	if (!fragmentFile) {
		cerr << "Error opening fragment\n";
	}

	stringstream vertexStream, fragmentStream;
	vertexStream << vertexFile.rdbuf();
	fragmentStream << fragmentFile.rdbuf();

	vertexFile.close();
	fragmentFile.close();

	vertexSource = vertexStream.str();
	fragmentSource = fragmentStream.str();

	const char* vSourceCode = vertexSource.c_str();
	const char* fSourceCode = fragmentSource.c_str();

	unsigned vertex, fragment;
	int success;
	char infoLog[1024];

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vSourceCode, nullptr);
	glCompileShader(vertex);

	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 1024, nullptr, infoLog);
		cerr << "Error::shader::vertex::compilation: " << infoLog << endl;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fSourceCode, nullptr);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 1024, nullptr, infoLog);
		cerr << "Error::shader::fragment::compilation: " << infoLog << endl;
	}

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 1024, nullptr, infoLog);
		cerr << "Error::shader::program::linking: " << infoLog << endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::activate() {
	glUseProgram(ID);
}

void Shader::destroy() {
	glDeleteProgram(ID);
}

void Shader::setBool(const string& name, bool value) {
	GLint location = glGetUniformLocation(ID, name.c_str());
	glUniform1i(location, (int)value);
}

void Shader::setInt(const string& name, int value) {
	GLint location = glGetUniformLocation(ID, name.c_str());
	glUniform1i(location, value);
}

void Shader::setFloat(const string& name, float value) {
	GLint location = glGetUniformLocation(ID, name.c_str());
	glUniform1f(location, value);
}

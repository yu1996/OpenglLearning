#include "Shader.h"

Shader::Shader()
{
	__initShader();
}

void Shader::addShader(const GLchar* shaderPath, GLuint shaderType)
{
	std::string shaderCode;
	std::ifstream shaderFile;
	shaderFile.exceptions(std::ifstream::badbit);
	try {
		shaderFile.open(shaderPath);
		std::stringstream shaderStream;
		shaderStream << shaderFile.rdbuf();
		shaderFile.close();
		shaderCode = shaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const GLchar* charShaderCode = shaderCode.c_str();
	GLuint shader;
	GLint success;
	GLchar infoLog[512];
	// Vertex Shader
	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &charShaderCode, NULL);
	glCompileShader(shader);
	// Print compile errors if any
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::" << shaderPath << "::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	glAttachShader(ID, shader);
	m_ShaderSet.push_back(shader);
}

void Shader::linkShader()
{
	glLinkProgram(ID);
	GLint success;
	GLchar infoLog[512];
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(this->ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	for (GLuint shader : m_ShaderSet) {
		glDeleteShader(shader);
	}
}

void Shader::__initShader()
{
	ID = glCreateProgram();

	if (ID == 0) {
		std::cout << "Error creating shader program" << std::endl;
	}
}
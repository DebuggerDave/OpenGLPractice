#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <source_location>
#include <iterator>

#include "shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath, const char* macroPath)
{
	// retrieve source code from files 
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::string macroCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	std::ifstream macroFile;
	// ensure ifstream objects can throw exceptions
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	macroFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// if geometry shader path is present, also load a geometry shader
		if (geometryPath != nullptr)
		{
			gShaderFile.open(geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
		// if macros are present, load them
		if (macroPath != nullptr)
		{
			macroFile.open(macroPath);
			std::stringstream macroStream;
			macroStream << macroFile.rdbuf();
			macroFile.close();
			macroCode = macroStream.str() + '\n';
			num_injected_lines += std::count(
				macroCode.begin(),
				macroCode.end(),
				'\n');
		}
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	// inject macro code in all relevant files
	if (macroPath != nullptr) {
		insertMacros(vertexCode, macroCode);
		insertMacros(fragmentCode, macroCode);
		if (geometryPath != nullptr) {
			insertMacros(geometryCode, macroCode);
		}
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");
	// if geometry shader is given, compile geometry shader
	unsigned int geometry;
	if (geometryPath != nullptr)
	{
		const char * gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY");
	}
	// shader Program
	id = glCreateProgram();
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);
	if (geometryPath != nullptr)
		glAttachShader(id, geometry);
	glLinkProgram(id);
	checkCompileErrors(id, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr)
		glDeleteShader(geometry);

}

void Shader::use()
{
	glUseProgram(id);
}

void Shader::setBool(const std::string &name, bool value) const
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
	glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string &name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
	glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
	glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
	glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::updateLineNumbers(GLchar* log, GLsizei max_size)
{
	size_t pos = 0;
	std::string log_str(log);
	const std::string first_match("0(");
	const std::string second_match(")");
	// for each line number
	for (size_t pos; pos != std::string::npos; pos = log_str.find(first_match, pos)) {
		size_t second_pos = log_str.find(second_match, pos);
		size_t start_pos = pos + first_match.size();
		// get number and update it
		if (second_pos != std::string::npos) {
			std::string line_num_str = log_str.substr(start_pos, second_pos - start_pos);
			int line_num = -1;
			try {
				line_num = std::stoi(line_num_str);
			} catch (std::invalid_argument const& ex) {
				std::cerr << "Couldn't convert string to int in '" << std::source_location::current().function_name() << "'\n";
			}
			std::string new_line_num_str = std::to_string(line_num - num_injected_lines);
			log_str.replace(start_pos, second_pos - start_pos, new_line_num_str);
		}
		pos += start_pos;
	}

	if (log_str.length() > max_size) {
		std::cerr << "max string size reached, not updating log in '" << std::source_location::current().function_name() << "'\n";
	} else {
		const char* out = log_str.c_str();
		memcpy(log, out, log_str.size());
	}
}

void Shader::checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	const GLsizei max_size = 1024;
	GLchar infoLog[max_size];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, max_size, NULL, infoLog);
			updateLineNumbers(infoLog, max_size);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n ------------------------------------------------------- " << "\n";
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, max_size, NULL, infoLog);
			updateLineNumbers(infoLog, max_size);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n ------------------------------------------------------- " << "\n";
		}
	}
}

void Shader::insertMacros(std::string &code, std::string macros)
{
	size_t version_pos = code.find_first_of("#version");
	size_t insertion_pos = 0;
	if (version_pos != std::string::npos) {
		insertion_pos = code.find_first_of("\n");
		if (insertion_pos != std::string::npos) {
			insertion_pos++;
		}
	}
	code.insert(insertion_pos, macros);
}

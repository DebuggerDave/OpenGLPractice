#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <source_location>
#include <iterator>
#include <utility>
#include <string>

#include "shader.h"
#include "utils.h"

Shader::Shader() : id(0) {}

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path, const std::string& injectible_path)
	: id(0), num_injected_lines(0)
{
	if (!setupShader(vertex_path, fragment_path, geometry_path, injectible_path)) {
		utils::err() << "failed to construct shader program, leaving Shader object empty" << utils::endl;
	}
}

Shader::~Shader() {
    glDeleteShader(id);
}

Shader::Shader(Shader&& other) noexcept :
    id(std::exchange(other.id, 0)) {}

Shader& Shader::operator=(Shader&& other) noexcept
{
    glDeleteShader(id);
    id = std::exchange(other.id, 0);

    return *this;
}

unsigned int Shader::getId() const
{
	return id;
}

void Shader::activate() const
{
	glUseProgram(id);
}

void Shader::deactivate() const
{
	glUseProgram(0);
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

bool Shader::setupShader(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path, const std::string& injectible_path) {
	bool success = true;
	std::string vertex_code, fragment_code, geometry_code, injectible_code;
	unsigned int local_num_injected_lines = 0;

	success &= readShaderFile(vertex_path, vertex_code);
	success &= readShaderFile(fragment_path, fragment_code);
	if (!geometry_path.empty()) {
		success &= readShaderFile(geometry_path, geometry_code);
	}

	if (!injectible_path.empty()) {
		success &= readShaderFile(injectible_path, injectible_code);
		local_num_injected_lines += std::count(injectible_code.begin(), injectible_code.end(), '\n');

		injectCode(vertex_code, injectible_code);
		injectCode(fragment_code, injectible_code);
		if (!geometry_path.empty()) {
			injectCode(geometry_code, injectible_code);
		}
	}

	id = glCreateProgram();
	success &= compileAndAttach(vertex_code, Vertex);
	success &= compileAndAttach(fragment_code, Fragment);
	if (!geometry_path.empty()) {
		success &= compileAndAttach(geometry_code, Geometry);
	}

	glLinkProgram(id);
	success &= checkCompileErrors(id, Linker);

	if (success) {
		num_injected_lines = local_num_injected_lines;
	} else {
		glDeleteShader(id);
		id = 0;
	}

	return success;
}

bool Shader::compileAndAttach(const std::string& code, const ProgramType type) const {
	if (id == 0) {
		utils::err() << "no program to attach to" << utils::endl;
		return false;
	} else if (code.empty()) {
		utils::err() << "shader code is empty" << utils::endl;
		return false;
	}

	unsigned int shader_id;
	if (type == Vertex) {
		shader_id = glCreateShader(GL_VERTEX_SHADER);
	} else if (type == Fragment) {
		shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	} else if (type == Geometry) {
		shader_id = glCreateShader(GL_GEOMETRY_SHADER);
	} else {
		utils::err() << "shader program not recognized" << utils::endl;
		return false;
	}

	const char* codeData = code.c_str();
	glShaderSource(shader_id, 1, &codeData, NULL);
	glCompileShader(shader_id);

	glAttachShader(id, shader_id);
	glDeleteShader(shader_id);

	return checkCompileErrors(shader_id, type);
}

bool Shader::readShaderFile(const std::string& path, std::string& out) const {
	std::string code;
	if (path.empty()) {
		utils::err() << "shader code is empty" << utils::endl;
		return false;
	}

	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		file.open(path);
		std::stringstream stream;
		stream << file.rdbuf();
		file.close();
		code = stream.str();
	}
	catch (std::ifstream::failure& e)
	{
		utils::err() << "unable to parse shader file" << utils::endl;
		return false;
	}

	out = code;
	return true;
}

std::string Shader::programTypeToString(const ProgramType type) const {
	if (type == Vertex) {
		return std::string("Vertex");
	} else if (type == Fragment) {
		return std::string("Fragment");
	} else if (type == Geometry) {
		return std::string("Geometry");
	} else if (type == Linker) {
		return std::string("Linker");
	} else {
		return std::string("Unknown");
	}
}

bool Shader::checkCompileErrors(const GLuint shader, const ProgramType type) const
{
	GLint success = true;
	static const GLsizei max_size = 1024;
	GLchar infoLog[max_size];
	if (type != Linker) {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		glGetShaderInfoLog(shader, max_size, NULL, infoLog);
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		glGetProgramInfoLog(shader, max_size, NULL, infoLog);
	}

	if (!success) {
		updateLineNumbers(infoLog, max_size);
		utils::err e;
		e << "\n";
		e << "shader error of type: " << programTypeToString(type) << utils::endl;
		e << infoLog << "\n";
	}

	return success;
}

void Shader::updateLineNumbers(GLchar* log, const GLsizei max_size) const
{
	std::string log_str(log);
	const std::string first_match("0(");
	const std::string second_match(")");

	for (size_t pos = log_str.find(first_match); pos != std::string::npos; pos = log_str.find(first_match, ++pos)) {
		size_t end_pos = log_str.find(second_match, pos);
		// get number and update it
		if (end_pos != std::string::npos) {
			size_t start_pos = pos + first_match.size();
			std::string line_num_str = log_str.substr(start_pos, end_pos - start_pos);
			int line_num = -1;
			try {
				line_num = std::stoi(line_num_str);
				std::string new_line_num_str = std::to_string(line_num - num_injected_lines);
				log_str.replace(start_pos, end_pos - start_pos, new_line_num_str);
			} catch (std::invalid_argument const& ex) {
				utils::err() << "Couldn't convert string to int" << utils::endl;
			}
		}
	}

	if (log_str.length() > max_size) {
		utils::err() << "max string size reached, not updating log" << utils::endl;
	} else {
		memcpy(log, log_str.data(), log_str.size());
	}
}

void Shader::injectCode(std::string &shaderCode, const std::string& injectible) const
{
	if(shaderCode.empty() || injectible.empty()) {
		return;
	}

	size_t version_pos = shaderCode.find_first_of("#version");
	size_t insertion_pos = 0;

	if (version_pos != std::string::npos) {
		insertion_pos = shaderCode.find_first_of("\n");
		if (insertion_pos != std::string::npos) {
			insertion_pos++;
		}
	}

	shaderCode.insert(insertion_pos, injectible + "\n");
}

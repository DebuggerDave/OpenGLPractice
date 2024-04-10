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

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path)
{
	bool success = true;

	success &= setShaderCode(Vertex, vertex_path);
	success &= setShaderCode(Fragment, fragment_path);
	if (!geometry_path.empty()) {
		success &= setShaderCode(Geometry, geometry_path);
	}

	success &= compile();

	if (!success) {
		utils::err() << "failed to construct shader program, leaving Shader object empty" << utils::endl;
	}
}

Shader::~Shader() {
	resetProgram();
}

Shader::Shader(Shader&& other) noexcept :
    id(std::exchange(other.id, 0)),
	vertex_code(std::exchange(other.vertex_code, {})),
	vertex_num_injected(std::exchange(other.vertex_num_injected, 0)),
	fragment_code(std::exchange(other.fragment_code, {})),
	fragment_num_injected(std::exchange(other.fragment_num_injected, 0)),
	geometry_code(std::exchange(other.geometry_code, {})),
	geometry_num_injected(std::exchange(other.geometry_num_injected, 0))
	{}

Shader& Shader::operator=(Shader&& other) noexcept
{
    resetProgram();
    id = std::exchange(other.id, 0);
	vertex_code = std::exchange(other.vertex_code, {});
	vertex_num_injected = std::exchange(other.vertex_num_injected, 0);
	fragment_code = std::exchange(other.fragment_code, {});
	fragment_num_injected = std::exchange(other.fragment_num_injected, 0);
	geometry_code = std::exchange(other.geometry_code, {});
	geometry_num_injected = std::exchange(other.geometry_num_injected, 0);

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

bool Shader::setShaderCode(const ProgramType type, const std::string& code_path, const std::string& injected_path)
{
	resetProgram();

	std::string* code = nullptr;
	unsigned int* num_injected_lines = nullptr;
	switch (type) {
		case Vertex:
			code = &vertex_code;
			num_injected_lines = &vertex_num_injected;
			break;
		case Fragment:
			code = &fragment_code;
			num_injected_lines = &fragment_num_injected;
			break;
		case Geometry:
			code = &geometry_code;
			num_injected_lines = &geometry_num_injected;
			break;
		default:
			utils::err() << "Attempted to set invalid shader code type" << utils::endl;
			return false;
	}

	if (!readShaderFile(code_path, *code)) {
		utils::err() << "Unable to parse " << programTypeToString(type) << " code" << utils::endl;
		return false;
	}

	if (!injected_path.empty()) {
		std::string injected_code;
		if(!readShaderFile(injected_path, injected_code)) {
			utils::err() << "Unable to parse " << programTypeToString(type) << " injectible code" << utils::endl;
			return false;
		}
		if (!injectCode(*code, injected_code, *num_injected_lines)) {
			utils::err() << "Unable to inject code into " << programTypeToString(type) << " program" << utils::endl;
		}

	}

	return true;
}

bool Shader::compile()
{
	resetProgram();

	bool success = true;

	id = glCreateProgram();
	success &= compileAndAttach(vertex_code, Vertex);
	success &= compileAndAttach(fragment_code, Fragment);
	if (!geometry_code.empty()) {
		success &= compileAndAttach(geometry_code, Geometry);
	}

	glLinkProgram(id);
	success &= checkCompileErrors(id, Linker);

	if (!success) {
		utils::err() << "Unable to compile shader program" << utils::endl;
		resetProgram();
	}

	return success;
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

void Shader::resetProgram() {
	glDeleteShader(id);
	id = 0;
}

bool Shader::compileAndAttach(const std::string& code, const ProgramType type) const {
	if (id == 0) {
		utils::err() << "No program to attach to" << utils::endl;
		return false;
	} else if (code.empty()) {
		utils::err() << "Shader code is empty" << utils::endl;
		return false;
	}

	unsigned int shader_id;
	switch (type) {
		case Vertex:
			shader_id = glCreateShader(GL_VERTEX_SHADER); break;
		case Fragment:
			shader_id = glCreateShader(GL_FRAGMENT_SHADER); break;
		case Geometry:
			shader_id = glCreateShader(GL_GEOMETRY_SHADER); break;
		default:
			utils::err() << "Shader program not recognized" << utils::endl;
			return false;
	}

	const char* code_data = code.c_str();
	glShaderSource(shader_id, 1, &code_data, NULL);
	glCompileShader(shader_id);

	glAttachShader(id, shader_id);
	glDeleteShader(shader_id);

	return checkCompileErrors(shader_id, type);
}

bool Shader::readShaderFile(const std::string& path, std::string& out) const {
	out = {};
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
		out = stream.str();
	}
	catch (std::ifstream::failure& e)
	{
		utils::err() << "unable to parse shader file" << utils::endl;
		out = {};
		return false;
	}

	return true;
}

std::string Shader::programTypeToString(const ProgramType type) const {
	switch (type) {
		case Vertex:
			return std::string("Vertex"); break;
		case Fragment:
			return std::string("Fragment"); break;
		case Geometry:
			return std::string("Geometry"); break;
		case Linker:
			return std::string("Linker"); break;
		default:
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
		updateLineNumbers(infoLog, max_size, type);
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		glGetProgramInfoLog(shader, max_size, NULL, infoLog);
	}

	if (!success) {
		utils::err e;
		e << "\n";
		e << "shader error of type: " << programTypeToString(type) << utils::endl;
		e << infoLog << "\n";
	}

	return success;
}

bool Shader::updateLineNumbers(GLchar* log, const GLsizei max_size, const ProgramType type) const
{
	int num_injected_lines = 0;
	switch (type) {
		case Vertex: num_injected_lines = vertex_num_injected; break;
		case Fragment: num_injected_lines = fragment_num_injected; break;
		case Geometry: num_injected_lines = geometry_num_injected; break;
		default:
			utils::err() << "Unable to update log line numbers, program type invalid" << utils::endl;
			return false;
	}

	std::string log_str(log);
	// number is always of the format "0(\d*)"
	const std::string first_match("0(");
	const std::string second_match(")");

	for (size_t pos = log_str.find(first_match); pos != std::string::npos; pos = log_str.find(first_match, ++pos)) {
		size_t end_pos = log_str.find(second_match, pos);
		if (end_pos != std::string::npos) {
			size_t start_pos = pos + first_match.size();
			std::string line_num_str = log_str.substr(start_pos, end_pos - start_pos);
			try {
				int line_num = std::stoi(line_num_str);
				std::string new_line_num_str = std::to_string(line_num - num_injected_lines);
				log_str.replace(start_pos, end_pos - start_pos, new_line_num_str);
			} catch (std::invalid_argument const& ex) {
				utils::err() << "Couldn't convert string to int" << utils::endl;
				return false;
			}
		}
	}

	if (log_str.length() > max_size) {
		utils::err() << "max string size reached, not updating log" << utils::endl;
		return false;
	} else {
		memcpy(log, log_str.data(), log_str.size());
	}
	return true;
}

bool Shader::injectCode(std::string &source_code, const std::string& injected_code, unsigned int& num_injected_lines) const
{
	num_injected_lines = 0;
	if(source_code.empty()) {
		utils::err() << "Unable to inject code, source code is empty" << utils::endl;
		return false;
	}

	size_t version_pos = source_code.find_first_of("#version");
	size_t insertion_pos = 0;

	if (version_pos != std::string::npos) {
		insertion_pos = source_code.find_first_of("\n");
		if (insertion_pos != std::string::npos) {
			insertion_pos++;
		} else {
			utils::err() << "Unable to inject code, source code is invalid" << utils::endl;
			return false;
		}
	} else {
		utils::err() << "Unable to inject code, source code is invalid" << utils::endl;
		return false;
	}

	source_code.insert(insertion_pos, injected_code + "\n");
	// add one for the extra newline added during insertion
	num_injected_lines = std::count(injected_code.begin(), injected_code.end(), '\n') + 1;
	return true;
}

#include "shader.h"

#include "pch.h"
#include "utils.h"
#include "light_block.h"

Shader::Shader() : id(0) {}

Shader::Shader(std::string&& vertex_path, std::string&& fragment_path, std::string&& geometry_path)
{
	setShaderCode(vertex_path, fragment_path, geometry_path);
}

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_pat)
{
	setShaderCode(vertex_path, fragment_path, geometry_path);
}

Shader::~Shader() {
	resetProgram();
}

unsigned int Shader::getId() const
{
	return id;
}

void Shader::activate() const
{
	glUseProgram(id);
}

bool Shader::setShaderCode(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path)
{
	bool success = true;

	success &= setShaderCode(Vertex, vertex_path);
	success &= setShaderCode(Fragment, fragment_path);
	if (!geometry_path.empty()) {
		success &= setShaderCode(Geometry, geometry_path);
	}

	if (!success) {
		LOG("Failed to set shader code")
	}

	return success;
}

bool Shader::setShaderCode(const ProgramType type, const std::string& code_path_in)
{
	resetProgram();

	std::string* code = nullptr;
	std::string* code_path = nullptr;
	unsigned int* num_injected_lines = nullptr;
	switch (type) {
		case Vertex:
			code = &vertex_code;
			code_path = &vertex_path;
			num_injected_lines = &vertex_num_injected;
			break;
		case Fragment:
			code = &fragment_code;
			code_path = &fragment_path;
			num_injected_lines = &fragment_num_injected;
			break;
		case Geometry:
			code = &geometry_code;
			code_path = &geometry_path;
			num_injected_lines = &geometry_num_injected;
			break;
		default:
			LOG("Attempted to set invalid shader code type")
			return false;
	}

	*num_injected_lines = 0;
	*code = {};
	*code_path = {};
	if (!utils::readFile(code_path_in, *code)) {
		LOG("Unable to parse " << programTypeToString(type) << " code")
		return false;
	}

	*code_path = code_path_in;
	return true;
}

void Shader::resetShaderCode(const ProgramType type)
{
	resetProgram();

	std::string* code = nullptr;
	std::string* code_path = nullptr;
	unsigned int* num_injected_lines = nullptr;
	switch (type) {
		case Vertex:
			code = &vertex_code;
			code = &vertex_path;
			num_injected_lines = &vertex_num_injected;
			break;
		case Fragment:
			code = &fragment_code;
			code = &fragment_path;
			num_injected_lines = &fragment_num_injected;
			break;
		case Geometry:
			code = &geometry_code;
			code = &geometry_path;
			num_injected_lines = &geometry_num_injected;
			break;
		default:
			LOG("Cannot reset shader code, Invalid shader type")
			return;
	}

	*code = {};
	*code_path = {};
	*num_injected_lines = 0;
	lit_programs.erase(type);
	if (lit_programs.empty()) {
		light_block.reset();
	}
}

bool Shader::addLights(const ProgramType type, std::shared_ptr<LightBlock> light_block_in)
{

	if (light_block && lit_programs.contains(type) && (light_block_in == light_block)) {
		return true;
	} else if (light_block && !light_block_in) {
		light_block_in = light_block;
	}

	// no data
	if (!light_block && !light_block_in) {
		LOG("Failed to inject light code into " << programTypeToString(type) << " program, light data is empty")
		return false;
	// new data
	} else if (light_block && (light_block_in != light_block)) {
		LOG("Failed to inject light code into " << programTypeToString(type) << " program, this shader program already contains different light code")
		return false;
	// shader program not allocated
	} else if (id != 0) {
		LOG("Failed to inject light code into " << programTypeToString(type) << " program, Shader program is already compiled");
		return false;
	// light data not allocated
	} else if (!light_block_in->isAllocated()) {
		LOG("Failed to inject light code into " << programTypeToString(type) << " program, light block has not been allocated")
	}

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
			LOG("Attempted to set invalid shader code type")
			return false;
	}

	std::string injectible_code{light_block_in->getShaderCode()};
	if (injectible_code.empty()) {
		LOG("Unable to inject light code into " << programTypeToString(type) << " program, light code is empty")
		return false;
	} else if (!injectCode(*code, injectible_code, *num_injected_lines)) {
		LOG("Failed to inject light code into " << programTypeToString(type) << " program")
		return false;
	} else {
		light_block = light_block_in;
		lit_programs.insert(type);
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

	if (light_block) {
		GLuint light_block_index = glGetUniformBlockIndex(id, light_block->getName().c_str());
		GLint actual_light_block_size{0};
		glGetActiveUniformBlockiv(id, light_block_index, GL_UNIFORM_BLOCK_DATA_SIZE, &actual_light_block_size);
		if (light_block->byteSize() != actual_light_block_size) {
			LOG("Light block sizes do not match")
			success = false;
		}
		glBindBufferBase(GL_UNIFORM_BUFFER, light_block_index, light_block->getId());
	}

	if (!success) {
		LOG("Unable to compile shader program")
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
		LOG("No program to attach to")
		return false;
	} else if (code.empty()) {
		LOG("Shader code is empty")
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
			LOG("Shader program not recognized")
			return false;
	}

	const char* code_data = code.c_str();
	glShaderSource(shader_id, 1, &code_data, NULL);
	glCompileShader(shader_id);

	glAttachShader(id, shader_id);
	glDeleteShader(shader_id);

	return checkCompileErrors(shader_id, type);
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
	GLchar info_log[max_size];
	if (type != Linker) {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		glGetShaderInfoLog(shader, max_size, NULL, info_log);
		updateLineNumbers(info_log, max_size, type);
		const std::string* code_path = nullptr;
		const std::string* code = nullptr;
		switch (type) {
			case Vertex:
			code = &vertex_code;
				code_path = &vertex_path; break;
			case Fragment:
			code = &fragment_code;
				code_path = &fragment_path; break;
			case Geometry:
			code = &geometry_code;
				code_path = &geometry_path; break;
			default:
				LOG("Failed to get debug path data, invalid program type")
		}
		if (!success) {
			LOG("\nFailed to compile the " << programTypeToString(type) << " shader at path " << *code_path)
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		glGetProgramInfoLog(shader, max_size, NULL, info_log);
		if (!success) {
			LOG("\nFailed to Link the shader program")
		}
	}

	utils::err() << info_log;

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
			LOG("Unable to update log line numbers, program type invalid")
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
				LOG("Couldn't convert string to int")
				return false;
			}
		}
	}

	if (log_str.length() > max_size) {
		LOG("max string size reached, not updating log")
		return false;
	}

	memcpy(log, log_str.data(), log_str.size());

	return true;
}

bool Shader::injectCode(std::string &source_code, const std::string& injectible_code, unsigned int& num_injected_lines) const
{
	if(source_code.empty()) {
		LOG("Unable to inject code, source code is empty")
		return false;
	}

	size_t version_pos = source_code.find_first_of("#version");
	size_t insertion_pos = 0;

	if (version_pos == std::string::npos) {
		LOG("Unable to inject code, source code is invalid")
		return false;
	}
	insertion_pos = source_code.find_first_of("\n");
	if (insertion_pos == std::string::npos) {
		LOG("Unable to inject code, source code is invalid")
		return false;
	}
	// add one for the final newline character added
	insertion_pos++;

	source_code.insert(insertion_pos, injectible_code + "\n");
	// add one for the extra newline added during insertion
	num_injected_lines += std::count(injectible_code.begin(), injectible_code.end(), '\n') + 1;

	return true;
}

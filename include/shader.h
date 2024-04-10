#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

class Shader
{
public:
	enum ProgramType {
		Linker,
		Vertex,
		Fragment,
		Geometry,
	};

	Shader();
	Shader(const std::string& vertex_path, const std::string& fragment_path,
		const std::string& geometry_path={});
	~Shader();
	Shader(const Shader& other) = delete;
	Shader(Shader&& other) noexcept;
	Shader& operator=(const Shader& other) = delete;
	Shader& operator=(Shader&& other) noexcept;

	unsigned int getId() const;
	// set as active opengl shader
	void activate() const;
	// add glsl source code data, reset program if already compiled
	bool setShaderCode(const ProgramType type, const std::string& code_path, const std::string& injected_code={});
	// compile, attach, and link all shader programs
	bool compile();

	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	// ------------------------------------------------------------------------
	void setVec2(const std::string &name, const glm::vec2 &value) const;
	void setVec2(const std::string &name, float x, float y) const;
	void setVec3(const std::string &name, const glm::vec3 &value) const;
	void setVec3(const std::string &name, float x, float y, float z) const;
	void setVec4(const std::string &name, const glm::vec4 &value) const;
	void setVec4(const std::string &name, float x, float y, float z, float w);
	// ------------------------------------------------------------------------
	void setMat2(const std::string &name, const glm::mat2 &mat) const;
	void setMat3(const std::string &name, const glm::mat3 &mat) const;
	void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
	// cleanup program memory
	void resetProgram();
	// compile and attach shader code
	bool compileAndAttach(const std::string& code, const ProgramType type) const;
	// read file path and return shader code
	bool readShaderFile(const std::string& path, std::string& out) const;
	// check errors after compiling or linking shaders
	bool checkCompileErrors(const GLuint shader, const ProgramType type) const;
	// convert shader program type data
	std::string programTypeToString(const ProgramType type) const;
	// update the line numbers in logs to account for the injected shader code
	bool updateLineNumbers(GLchar* log, const GLsizei max_size, const ProgramType type) const;
	// inject code after '#version' statement in shader
	bool injectCode(std::string &source_code, const std::string& injected_code, unsigned int& num_injected_lines) const;

	// shader program id
	unsigned int id = 0;
	// number of lines of code injected into the glsl files
	unsigned int vertex_num_injected = 0;
	unsigned int fragment_num_injected = 0;
	unsigned int geometry_num_injected = 0;
	// shader source code
	std::string vertex_code;
	std::string fragment_code;
	std::string geometry_code;
};
#endif
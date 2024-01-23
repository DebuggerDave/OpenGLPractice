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
		const std::string& geometry_path = "", const std::string& injectible_path = "");
	~Shader();
	Shader(const Shader& other) = delete;
	Shader(Shader&& other) noexcept;
	Shader& operator=(const Shader& other) = delete;
	Shader& operator=(Shader&& other) noexcept;

	unsigned int getId() const;
	void activate() const;
	void deactivate() const;
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
	// initial setup
	bool setupShader(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path, const std::string& injectible_path);
	// compile and link shader code
	bool compileAndAttach(const std::string& code, const ProgramType type) const;
	// read file path and return shader code
	bool readShaderFile(const std::string& path, std::string& out) const;
	// check errors after compiling or linking shaders
	bool checkCompileErrors(const GLuint shader, const ProgramType type) const;
	// convert shader program type data
	std::string programTypeToString(const ProgramType type) const;
	// update the line numbers in logs to account for the injected shader code
	void updateLineNumbers(GLchar* log, const GLsizei max_size) const;
	// inject code after '#version' statement in shader
	void injectCode(std::string &shaderCode, const std::string& injectible) const;

	// shader program id
	unsigned int id = 0;
	// number of lines of code added to the glsl files
	int num_injected_lines = 0;
};
#endif
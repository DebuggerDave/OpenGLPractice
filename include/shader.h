#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

class Shader
{
public:
	unsigned int id;

	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, const char* macros = nullptr);

	// activate the shader
	void activate();
	void deactivate();
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
	void checkCompileErrors(GLuint shader, std::string type);
	// update the line numbers in logs to account for the injected shader code
	void updateLineNumbers(GLchar* log, GLsizei max_size);
	// inject code after '#version' statement in shader
	void insertMacros(std::string &code, std::string injectible);

	// number of lines of code added to the glsl files
	int num_injected_lines = 0;
};
#endif
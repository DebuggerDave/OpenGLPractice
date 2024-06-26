#ifndef LIGHTBLOCK
#define LIGHTBLOCK

#include "light_uniform_buffer.h"

#include "glm/fwd.hpp"
#include "glad/gl.h"

#include <cstddef>
#include <string>

namespace std {
	template <typename T>
	class shared_ptr;
	template <typename T, typename Allocator>
	class vector;
}

class LightBlock
{
public:
	enum class LightType {
		Directional,
		Spot,
		Point
	};

	~LightBlock();
	// This class is only accessible through shared pointer
	static std::shared_ptr<LightBlock> makeShared(const size_t num_directional_light = 0, const size_t num_spot_light = 0, const size_t num_point_lights = 0);
	// public push back functions
	template<typename T>
	bool pushBack(const T& light);
	template<typename T>
	bool pushBack(T&& light);
	// BufferSubData new direction data into the graphics card
	bool updateDirection(const LightType type, const size_t index, const glm::vec4& direction);
	// BufferSubData new color data into the graphics card
	bool updateColor(const LightType type, const size_t index, const LightColor& color);
	// BufferSubData new position data into the graphics card
	bool updatePosition(const LightType type, const size_t index, const glm::vec4& position);
	// read only data
	const UNIFORM_BUFFER_TYPE& read() const;
	// return generated shader code for injection, only available after allocation
	const char* getShaderCode() const;
	// gl buffer id
	unsigned int getId() const;
	// if the uniform buffer has been allocated
	bool isAllocated() const;
	// name struct/unifrom-buffer in injectible shader code
	std::string getName() const;
	// size of data on disk
	size_t byteSize() const;
	// allocate memory on graphics card for light data
	bool allocate();
	// Free light data memory on graphics card
	// This can only occur if one instance of the shared pointer exists,
	// in case the buffer is actively being used by other instances
	void deallocate();

private:
	LightBlock(const size_t num_directional_light = 0, const size_t num_spot_light = 0, const size_t num_point_lights = 0);
	LightBlock(const LightBlock& other) = delete;
	LightBlock(LightBlock&& other) =delete;
	LightBlock& operator=(const LightBlock& other) = delete;
	LightBlock& operator=(LightBlock&& other) = delete;

	// private push back functions
	void addLight(const DirectionalLight& light);
	void addLight(DirectionalLight&& light);
	void addLight(const SpotLight& light);
	void addLight(SpotLight&& light);
	void addLight(const PointLight& light);
	void addLight(PointLight&& light);
	// generate shader code for injection, should occur at allocation time
	bool genShaderCode();
	// replace macro definitions value in shader code
	bool replaceMacro(std::string& code, const std::string& macro, const std::string& value) const;
	// copy vector data into buffer
	template <typename T>
	size_t vecMemCopy(void* destination, const std::vector<T>& source);
	// TODO move this function to a test
	// verify all data in struct is accounted for
	bool verifyData() const;
	// accumulate size of vector in struct as std::vector and in memory as raw data
	template <typename T>
	bool accumulateVerifyData(const std::vector<T>& vec, size_t& struct_size, size_t& data_size) const;

	// gl buffer id
	GLuint id{0};
	// generated shader code
	std::string shader_code{};
	// light data
	UNIFORM_BUFFER_TYPE uni_buff{};
	// path to injectible shader code
	static const std::string light_uniform_buffer_path;
	// macros in injectible shader code
	static const std::string directional_light_macro;
	static const std::string spot_light_macro;
	static const std::string point_light_macro;
	// name of struct in injectible shader code
	static const std::string block_name;
};

#endif
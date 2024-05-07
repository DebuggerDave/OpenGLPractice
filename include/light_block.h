#ifndef LIGHTBLOCK
#define LIGHTBLOCK

#include "shader_lights.h"

#include "utils.h"
#include "light_uniform_buffer.h"

#include <string>
#include <memory>

#include "glm/glm.hpp"
#include "glad/glad.h"

class LightBlock
{
public:
	enum LightType {
		Directional,
		Spot,
		Point
	};

	~LightBlock();
	static std::shared_ptr<LightBlock> makeShared(const size_t num_directional_light = 0, const size_t num_spot_light = 0, const size_t num_point_lights = 0);
	template<typename T>
	bool pushBack(T&& light);
	//template<typename T>
	//bool pushBack(T&& light);
	// BufferSubData new direction data into the graphics card
	bool updateDirection(const LightType type, const size_t index, const glm::vec4& direction);
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
	void verifyData() const;
	// accumulate size of vector in struct as std::vector and in memory as raw data
	template <typename T>
	void accumulateVerifyData(const std::vector<T>& vec, size_t& struct_size, size_t& data_size) const;

	// gl buffer id
	GLuint id{0};
	// generated shader code
	std::string shader_code{};
	// light data
	UNIFORM_BUFFER_TYPE uni_buff{};
	// paths to injectible shader code
	inline static const std::string light_struct_path{"./glsl/include/light_structs.h"};
	inline static const std::string light_uniform_buffer_path{"./glsl/include/light_uniform_buffer.h"};
	// macros in injectible shader code
	inline static const std::string directional_light_macro{"#define NUM_DIRECTIONAL_LIGHTS"};
	inline static const std::string spot_light_macro{"#define NUM_SPOT_LIGHTS"};
	inline static const std::string point_light_macro{"#define NUM_POINT_LIGHTS"};
	// name of struct in injectible shader code
	inline static const std::string block_name{STRINGIFY(UNIFORM_BUFFER_TYPE)};
};

#endif
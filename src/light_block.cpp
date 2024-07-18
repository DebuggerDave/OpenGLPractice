#include "light_block.h"

#include "light_uniform_buffer.h"
#include "utils.h"

#include "glad/gl.h"
#include "glm/vec4.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/trigonometric.hpp"

#include <cstddef>
#include <string>
#include <vector>
#include <memory>
#include <utility>

LightColor::LightColor(const glm::vec4& ambient, const glm::vec4& diffuse, const glm::vec4& specular) :
	ambient{ambient},
	diffuse{diffuse},
	specular{specular}
	{}

Attenuation::Attenuation() :
	constant{1.0f},
	linear{0.0f},
	quadratic{0.01f}
	{}

DirectionalLight::DirectionalLight() :
	color{},
	dir{glm::vec4(0.0f, -1.0f, 0.0f, 0.0f)}
	{}

SpotLight::SpotLight() :
	color{},
	attenuation{},
	dir{glm::vec4(0.0f, -1.0f, 0.0f, 0.0f)},
	pos{glm::vec4(0.0f)},
	inner_angle_cosine{glm::cos(glm::radians(20.0f))},
	outer_angle_cosine{glm::cos(glm::radians(70.0f))}
	{}

PointLight::PointLight() :
	color{},
	attenuation{},
	pos{glm::vec4(0.0f)}
	{}

LightBlock::LightBlock(const size_t num_directional_lights, const size_t num_spot_lights, const size_t num_point_lights) noexcept :
	uni_buff{
		.directional_lights{std::vector<DirectionalLight>(utils::max(1, num_directional_lights))},
		.spot_lights{std::vector<SpotLight>(utils::max(1, num_spot_lights))},
		.point_lights{std::vector<PointLight>(utils::max(1, num_point_lights))}
	}
{
	if ((num_directional_lights < 1) ||  (num_spot_lights < 1) || (num_point_lights < 1)) {
		LOG("Cannot construct LightBlock with lights of size zero, defaulting to size one");
	}
}

LightBlock::~LightBlock()
{
	deallocate();
}

LightBlock::LightBlock(LightBlock&& other) noexcept
{
	id = std::exchange(other.id, 0);
	shader_code = std::move(other.shader_code);
	uni_buff = std::move(other.uni_buff);
}

template<typename T>
bool LightBlock::pushBack(const T& light) {
	if (id == 0) {
		addLight(light);
		return true;
	} else {
		LOG("Failed to add light, data already allocated on graphics card")
		return false;
	}
}
template
bool LightBlock::pushBack(const DirectionalLight& light);
template
bool LightBlock::pushBack(const SpotLight& light);
template
bool LightBlock::pushBack(const PointLight& light);

template<typename T>
bool LightBlock::pushBack(T&& light) {
	if (id == 0) {
		addLight(light);
		return true;
	} else {
		LOG("Failed to add light, data already allocated on graphics card")
		return false;
	}
}
template
bool LightBlock::pushBack(DirectionalLight&& light);
template
bool LightBlock::pushBack(SpotLight&& light);
template
bool LightBlock::pushBack(PointLight&& light);

bool LightBlock::updateDirection(const LightType type, const size_t index, const glm::vec4& direction)
{
	size_t offset{0};
	switch (type)
	{
		case LightType::Directional:
			if (index >= uni_buff.directional_lights.size()) {
				LOG("Unable to update light direction, invalid index")
				return false;
			}
			uni_buff.directional_lights[index].dir = direction;
			offset = offsetof(decltype(uni_buff.directional_lights)::value_type, dir)
			+ (sizeof(DirectionalLight) * index);
			break;
		case LightType::Spot:
			if (index >= uni_buff.spot_lights.size()) {
				LOG("Unable to update light direction, invalid index")
				return false;
			}
			uni_buff.spot_lights[index].dir = direction;
			offset = sizeof(decltype(uni_buff.directional_lights)::value_type) * uni_buff.directional_lights.size()
			+ offsetof(decltype(uni_buff.spot_lights)::value_type, dir)
			+ (sizeof(SpotLight) * index);
			break;
		case LightType::Point:
			LOG("Unable to update light direction, light type doesn't have a direction")
			return false;
			break;
		default:
			LOG("Unable to update light direction, light type not supported")
			return false;
	}

	if (id != 0) {
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec4), glm::value_ptr(direction));
	}

	return true;
}

bool LightBlock::updateColor(const LightType type, const size_t index, const LightColor& color)
{
	size_t offset{0};
	switch (type)
	{
		case LightType::Directional:
			if (index >= uni_buff.directional_lights.size()) {
				LOG("Unable to update light color, invalid index")
				return false;
			}
			uni_buff.directional_lights[index].color = color;
			offset = offsetof(decltype(uni_buff.directional_lights)::value_type, color)
				+ (sizeof(DirectionalLight) * index);
			return false;
			break;
		case LightType::Spot:
			if (index >= uni_buff.spot_lights.size()) {
				LOG("Unable to update light color, invalid index")
				return false;
			}
			uni_buff.spot_lights[index].color = color;
			offset = sizeof(decltype(uni_buff.directional_lights)::value_type) * uni_buff.directional_lights.size()
				+ offsetof(decltype(uni_buff.spot_lights)::value_type, color)
				+ (sizeof(SpotLight) * index);
			break;
		case LightType::Point:
			if (index >= uni_buff.point_lights.size()) {
				LOG("Unable to update light color, invalid index")
				return false;
			}
			uni_buff.point_lights[index].color = color;
			offset = sizeof(decltype(uni_buff.directional_lights)::value_type) * uni_buff.directional_lights.size()
				+ sizeof(decltype(uni_buff.spot_lights)::value_type) * uni_buff.spot_lights.size()
				+ offsetof(decltype(uni_buff.point_lights)::value_type, color)
				+ (sizeof(PointLight) * index);
			break;
		default:
			LOG("Unable to update light color, light type not supported")
			return false;
	}

	if (id != 0) {
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(LightColor), &color);
	}

	return true;
}

bool LightBlock::updatePosition(const LightType type, const size_t index, const glm::vec4& position)
{
	size_t offset{0};
	switch (type)
	{
		case LightType::Directional:
			LOG("Unable to update light position, light type doesn't have a position")
			return false;
			break;

		case LightType::Spot:
			if (index >= uni_buff.spot_lights.size()) {
				LOG("Unable to update light position, invalid index")
				return false;
			}
			uni_buff.spot_lights[index].pos = position;
			offset = sizeof(decltype(uni_buff.directional_lights)::value_type) * uni_buff.directional_lights.size()
				+ offsetof(decltype(uni_buff.spot_lights)::value_type, pos)
				+ (sizeof(SpotLight) * index);
			break;
		case LightType::Point:
			if (index >= uni_buff.point_lights.size()) {
				LOG("Unable to update light position, invalid index")
				return false;
			}
			uni_buff.point_lights[index].pos = position;
			offset = sizeof(decltype(uni_buff.directional_lights)::value_type) * uni_buff.directional_lights.size()
				+ sizeof(decltype(uni_buff.spot_lights)::value_type) * uni_buff.spot_lights.size()
				+ offsetof(decltype(uni_buff.point_lights)::value_type, pos)
				+ (sizeof(PointLight) * index);
			break;
		default:
			LOG("Unable to update light position, light type not supported")
			return false;
	}

	if (id != 0) {
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec4), glm::value_ptr(position));
	}

	return true;
}

const UNIFORM_BUFFER_TYPE& LightBlock::read() const
{
	return uni_buff;
}

const char* LightBlock::getShaderCode() const
{
	return shader_code.c_str();
}

unsigned int LightBlock::getId() const
{
	return id;
}

bool LightBlock::isAllocated() const
{
	return (id != 0);
}

std::string LightBlock::getName() const
{
	return block_name;
}

size_t LightBlock::byteSize() const
{
	return ((uni_buff.directional_lights.size() * sizeof(DirectionalLight)) +
		(uni_buff.spot_lights.size() * sizeof(SpotLight)) +
		(uni_buff.point_lights.size() * sizeof(PointLight)));
}

bool LightBlock::allocate()
{
	if (!verifyData()) {
		LOG("Failed to allocate light block, data verification failed")
		return false;
	}
	if (!genShaderCode()) {
		LOG("Failed to allocate light block")
		return false;
	}

	glGenBuffers(1, &id);
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	std::unique_ptr<GLvoid, utils::FreeDelete> buffer(malloc(byteSize()));
	if (buffer == nullptr) {
		LOG("Failed to allocate light block")
		return false;
	}

	size_t bytes{0};
	bytes += vecMemCopy((unsigned char*)buffer.get() + bytes, uni_buff.directional_lights);
	bytes += vecMemCopy((unsigned char*)buffer.get() + bytes, uni_buff.spot_lights);
	bytes += vecMemCopy((unsigned char*)buffer.get() + bytes, uni_buff.point_lights);

	// TODO should I verify size here?
	if (bytes != byteSize()) {
		LOG("Unibuff data missing in allocation calculation")
		return false;
	}

	glBufferData(GL_UNIFORM_BUFFER, byteSize(), (const GLvoid*)buffer.get(), GL_DYNAMIC_DRAW);

	return true;
}

void LightBlock::deallocate()
{
	glDeleteBuffers(1, &id);
	id = 0;
	shader_code = {};
}

void LightBlock::addLight(const DirectionalLight& light)
{
	uni_buff.directional_lights.push_back(light);
}

void LightBlock::addLight(DirectionalLight&& light)
{
	uni_buff.directional_lights.push_back(light);
}

void LightBlock::addLight(const SpotLight& light)
{
	uni_buff.spot_lights.push_back(light);
}

void LightBlock::addLight(SpotLight&& light)
{
	uni_buff.spot_lights.push_back(light);
}

void LightBlock::addLight(const PointLight& light)
{
	uni_buff.point_lights.push_back(light);
}

void LightBlock::addLight(PointLight&& light)
{
	uni_buff.point_lights.push_back(light);
}

bool LightBlock::genShaderCode()
{
	std::string light_uniform_buffer_code;
	std::string cur_shader_code;

	if (!utils::readFile(light_uniform_buffer_path, light_uniform_buffer_code)) {
		LOG("Failed to read light uniform buffer code");
		return false;
	}

	cur_shader_code = "\n" + light_uniform_buffer_code + "\n";

	if (!replaceMacro(cur_shader_code, directional_light_macro, std::to_string(uni_buff.directional_lights.size()))) {
		LOG("Failed to replace directional light macro");
		return false;
	}
	if (!replaceMacro(cur_shader_code, spot_light_macro, std::to_string(uni_buff.spot_lights.size()))) {
		LOG("Failed to replace directional light macro");
		return false;
	}
	if (!replaceMacro(cur_shader_code, point_light_macro, std::to_string(uni_buff.point_lights.size()))) {
		LOG("Failed to replace directional light macro");
		return false;
	}


	shader_code = cur_shader_code;
	return true;
}

bool LightBlock::replaceMacro(std::string& code, const std::string& macro, const std::string& value) const
{
	std::size_t start_pos = code.find(macro);
	if (start_pos == std::string::npos) {
		LOG("Failed to find macro")
		return false;
	}
	std::size_t end_pos = code.find("\n", start_pos);
	if (start_pos == std::string::npos) {
		LOG("Failed to find end of macro")
		return false;
	}

	code.replace(start_pos, end_pos - start_pos, macro + " " + value);
	return true;
}

template <typename T>
size_t LightBlock::vecMemCopy(void* destination, const std::vector<T>& source) {
	size_t bytes = source.size() * sizeof(T);
	memcpy(destination, source.data(), bytes);
	return bytes;
}
template
size_t LightBlock::vecMemCopy(void* destination, const std::vector<DirectionalLight>& source);
template
size_t LightBlock::vecMemCopy(void* destination, const std::vector<SpotLight>& source);
template
size_t LightBlock::vecMemCopy(void* destination, const std::vector<PointLight>& source);

bool LightBlock::verifyData() const {
	const size_t expected_struct_size = sizeof(uni_buff);
	size_t actual_struct_size = 0;
	const size_t expected_data_size = byteSize();
	size_t actual_data_size = 0;
	bool success = true;

	success &= accumulateVerifyData(uni_buff.directional_lights, actual_struct_size, actual_data_size);
	success &= accumulateVerifyData(uni_buff.spot_lights, actual_struct_size, actual_data_size);
	success &= accumulateVerifyData(uni_buff.point_lights, actual_struct_size, actual_data_size);

	if ((actual_struct_size != expected_struct_size) || (actual_data_size != expected_data_size)) {
		success = false;
		LOG("ERROR, not all light data is accounted for")
	}

	return success;
}

template <typename T>
bool LightBlock::accumulateVerifyData(const std::vector<T>& vec, size_t& struct_size, size_t& data_size) const {
	struct_size += sizeof(vec);
	data_size += sizeof(T) * vec.size();

	if (vec.size() < 1) {
		LOG("Invalid vector size, cannot be less than 1")
		return false;
	}
	return true;
}
template
bool LightBlock::accumulateVerifyData(const std::vector<DirectionalLight>& vec, size_t& struct_size, size_t& data_size) const;
template
bool LightBlock::accumulateVerifyData(const std::vector<SpotLight>& vec, size_t& struct_size, size_t& data_size) const ;
template
bool LightBlock::accumulateVerifyData(const std::vector<PointLight>& vec, size_t& struct_size, size_t& data_size) const;
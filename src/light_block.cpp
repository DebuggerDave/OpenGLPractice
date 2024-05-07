#include "light_block.h"

#include "utils.h"

#include <cmath>
#include <vector>
#include <utility>
#include <cstddef>
#include <memory>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

LightColor::LightColor() :
	ambient{glm::vec4{0.2f}},
	diffuse{glm::vec4{1.0f}},
	specular{glm::vec4{1.0f}}
	{}

Attenuation::Attenuation() :
	constant{1.0f},
	linear{0.0f},
	quadratic{0.5f}
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
	inner_angle_cosine{glm::cos(glm::radians(15.0f))},
	outer_angle_cosine{glm::cos(glm::radians(25.0f))}
	{}

PointLight::PointLight() :
	color{},
	attenuation{},
	pos{glm::vec4(0.0f)}
	{}

LightBlock::~LightBlock()
{
	deallocate();
}

std::shared_ptr<LightBlock> LightBlock::makeShared(const size_t num_directional_lights, const size_t num_spot_lights, const size_t num_point_lights) {
	return std::shared_ptr<LightBlock>{new LightBlock{num_directional_lights, num_spot_lights, num_point_lights}};
}

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
bool LightBlock::pushBack(const DirectionalLight&& light);
template
bool LightBlock::pushBack(const SpotLight&& light);
template
bool LightBlock::pushBack(const PointLight&& light);
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
		case Directional:
			uni_buff.directional_lights[index].dir = direction;
			offset = offsetof(decltype(uni_buff), directional_lights)
			+ offsetof(decltype(uni_buff.directional_lights)::value_type, dir)
			+ (sizeof(DirectionalLight) * index);
			break;
		case Spot:
			uni_buff.spot_lights[index].dir = direction;
			offset = offsetof(decltype(uni_buff), spot_lights)
			+ offsetof(decltype(uni_buff.spot_lights)::value_type, dir)
			+ (sizeof(SpotLight) * index);
			break;
		case Point:
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

bool LightBlock::updatePosition(const LightType type, const size_t index, const glm::vec4& position)
{
	size_t offset{0};
	switch (type)
	{
		case Directional:
			LOG("Unable to update light position, light type doesn't have a position")
			return false;
			break;
		case Spot:
			uni_buff.spot_lights[index].pos = position;
			offset = offsetof(decltype(uni_buff), spot_lights)
				+ offsetof(decltype(uni_buff.spot_lights)::value_type, pos)
				+ (sizeof(SpotLight) * index);
			break;
		case Point:
			uni_buff.point_lights[index].pos = position;
			offset = offsetof(decltype(uni_buff), point_lights)
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
	verifyData();
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

LightBlock::LightBlock(const size_t num_directional_lights, const size_t num_spot_lights, const size_t num_point_lights) :
	uni_buff{
		.directional_lights{std::vector<DirectionalLight>(num_directional_lights)},
		.spot_lights{std::vector<SpotLight>(num_spot_lights)},
		.point_lights{std::vector<PointLight>(num_point_lights)}
	}
	{}

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
	std::string light_struct_code;
	std::string light_uniform_buffer_code;
	std::string cur_shader_code;

	if (!utils::readFile(light_struct_path, light_struct_code)) {
		LOG("Failed to read light struct code");
		return false;
	}
	if (!utils::readFile(light_uniform_buffer_path, light_uniform_buffer_code)) {
		LOG("Failed to read light uniform buffer code");
		return false;
	}

	cur_shader_code = "\n" + light_struct_code + "\n" + light_uniform_buffer_code + "\n";

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

void LightBlock::verifyData() const {
	const size_t expected_struct_size = sizeof(uni_buff);
	size_t actual_struct_size = 0;
	const size_t expected_data_size = byteSize();
	size_t actual_data_size = 0;

	accumulateVerifyData(uni_buff.directional_lights, actual_struct_size, actual_data_size);
	accumulateVerifyData(uni_buff.spot_lights, actual_struct_size, actual_data_size);
	accumulateVerifyData(uni_buff.point_lights, actual_struct_size, actual_data_size);

	if ((actual_struct_size != expected_struct_size) || (actual_data_size != expected_data_size)) {
		LOG("ERROR, not all light data is accounted for")
	}
}

template <typename T>
void LightBlock::accumulateVerifyData(const std::vector<T>& vec, size_t& struct_size, size_t& data_size) const {
	struct_size += sizeof(vec);
	data_size += sizeof(T) * vec.size();
}
template
void LightBlock::accumulateVerifyData(const std::vector<DirectionalLight>& vec, size_t& struct_size, size_t& data_size) const;
template
void LightBlock::accumulateVerifyData(const std::vector<SpotLight>& vec, size_t& struct_size, size_t& data_size) const ;
template
void LightBlock::accumulateVerifyData(const std::vector<PointLight>& vec, size_t& struct_size, size_t& data_size) const;
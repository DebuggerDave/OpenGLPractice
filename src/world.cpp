#include "world.h"

#include "component.h"
#include "utils.h"

#include "glm/mat4x4.hpp"
#include "glm/mat3x3.hpp"
#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glad/gl.h"
#include "entt/entity/registry.hpp"
#include "entt/entity/snapshot.hpp"
#include "PerlinNoise.hpp"
#include "cereal/archives/binary.hpp"

#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <utility>

World::World() noexcept :
	instancing_model_buffers((unsigned int)BlockId::NumNames),
	instancing_normal_mat_buffers((unsigned int)BlockId::NumNames),
	instancing_models((unsigned int)BlockId::NumNames),
	instancing_normal_mats((unsigned int)BlockId::NumNames)
{
	glCreateBuffers(instancing_model_buffers.size(), instancing_model_buffers.data());
	glCreateBuffers(instancing_normal_mat_buffers.size(), instancing_normal_mat_buffers.data());
	if (loadAll()) {
		initData();
	} else {
		reset();
	}
	connect();
}

World::~World()
{
	const Registry::iterable& it = world_registry.storage();
	if (it.begin() != it.end()) {
		saveAll();
	}
	glDeleteBuffers(instancing_model_buffers.size(), instancing_model_buffers.data());
	glDeleteBuffers(instancing_normal_mat_buffers.size(), instancing_normal_mat_buffers.data());
}

World::World(World&& other) noexcept :
	instancing_model_buffers{std::exchange(other.instancing_model_buffers, {})},
	instancing_normal_mat_buffers{std::exchange(other.instancing_normal_mat_buffers, {})},
	instancing_models{std::move(other.instancing_models)},
	instancing_normal_mats{std::move(other.instancing_normal_mats)},
	connections{}
{
	other.disconnect();
	world_registry = std::exchange(other.world_registry, {});
	connect();
}

void World::reset()
{
	std::remove(world_path.c_str());
	// Disconnect so we can handle all the data initalization in bulk instead of one at a time
	disconnect();
	generateWorld();
	initData();
	connect();
}

void World::setupInstancing(const GLuint VAO, const GLuint vertex_attrib_index, const BlockId id) const
{
	GLuint cur_index = vertex_attrib_index;
	GLuint end_index;
	GLint num_components;
	GLsizei stride;
	void* pointer;

	auto setupAttribArray = [](const GLuint& index, const GLint num_components, const GLsizei stride, const void* const pointer) -> void {
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, num_components, GL_FLOAT, GL_FALSE, stride, pointer);
		glVertexAttribDivisor(index, 1);
	};

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, instancing_model_buffers[id.uint()]);
	num_components = 4;
	stride = sizeof(glm::mat4);
	end_index = cur_index + 4;
    for (int i = 0; cur_index < end_index; cur_index++, i++) {
		pointer = (void*)(sizeof(glm::vec4) * i);
		setupAttribArray(cur_index, num_components, stride, pointer);
    }

    glBindBuffer(GL_ARRAY_BUFFER, instancing_normal_mat_buffers[id.uint()]);
	num_components = 3;
	stride = sizeof(glm::mat3);
	end_index = cur_index + 3;
    for (int i = 0; cur_index < end_index; cur_index++, i++) {
		pointer = (void*)(sizeof(glm::vec3) * i);
		setupAttribArray(cur_index, num_components, stride, pointer);
    }
}

size_t World::numObjects(const BlockId id) const
{
	if (instancing_models[id.uint()].size() != instancing_normal_mats[id.uint()].size()) {
		LOG("Unexpected size of instancing vector")
	}
	
	return instancing_models[id.uint()].size();
}

void World::updateNormalMats(const glm::mat4& view)
{
	if (instancing_models.size() != instancing_normal_mats.size()) {
		LOG("Unexpected size of instancing data")
	}
	using ModelType = decltype(instancing_models)::value_type::value_type;
	using NormalMatType = decltype(instancing_normal_mats)::value_type::value_type;

	for (unsigned int i=0; i < (unsigned int)instancing_models.size(); i++) {
		const std::vector<ModelType>& model_vec = instancing_models[i];
		std::vector<NormalMatType>& normal_mat_vec = instancing_normal_mats[i];

		for (unsigned int j=0; j < (unsigned int)model_vec.size(); j++) {
			normal_mat_vec[j] = glm::transpose(glm::inverse(glm::mat3(view * model_vec[j])));
		}

		const GLsizeiptr size_bytes = normal_mat_vec.size() * sizeof(NormalMatType);
		glNamedBufferSubData(instancing_normal_mat_buffers[i], 0, size_bytes, normal_mat_vec.data());
	}
}

void World::generateWorld()
{
	world_registry.clear();

	const siv::PerlinNoise::seed_type seed = std::random_device()();
	const siv::BasicPerlinNoise<float> perlin(seed);

	for (unsigned int x = 0; x < chunk_size; x++) {
		for (unsigned int z = 0; z < chunk_size; z++) {
			const float perlin_x = x * noise_scale;
			const float perlin_z = z * noise_scale;
			constexpr float max_noise_val = std::tuple_size<decltype(perlin)::state_type>{};
			if ((perlin_x > max_noise_val) && (perlin_z > max_noise_val)) {
				LOG("Perlin noise is outside expected input values")
			}
			// perlin noise is [-1, 1]
			const float noise = perlin.normalizedOctave2D(perlin_x, perlin_z, 3/*octaves*/, 0.5f/*persistence*/);
			const float adjusted_noise = std::roundf(noise * terrain_amplitude + terrain_median_height);
			// add block_half length so that blocks meet at whole numbers
			const float surface_level_position = adjusted_noise + (utils::sign(adjusted_noise) * block_half_length);
			const float bedrock_position = min_height + block_half_length;
			for (float y=bedrock_position; y<=surface_level_position; y++) {
				Registry::entity_type entity = world_registry.create();
				// center around zero, on increments of block_half_length
				const float float_x = (x + block_half_length) - (chunk_size / 2.0f);
				const float float_z = (z + block_half_length) - (chunk_size / 2.0f);
				world_registry.emplace<Position>(entity, float_x, y, float_z);
				if (y < surface_level_position) {
					world_registry.emplace<BlockId>(entity, BlockId::Name::Dirt);
				} else {
					world_registry.emplace<BlockId>(entity, BlockId::Name::Grass);
				}
			}
		}
	}
}

void World::saveAll() const
{
	save<ALLCOMPONENTS>(world_path);
}

bool World::loadAll()
{
	return load<ALLCOMPONENTS>(world_path);
}

template <typename... Component>
void World::save(const std::string& path) const
{
	std::ofstream stream;
	stream.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
	const entt::basic_snapshot<Registry> snapshot(world_registry);
	cereal::BinaryOutputArchive archive{stream};

	snapshot.get<Entity>(archive);
	([&]()
	{
		snapshot.get<Component>(archive);
	}(), ...);
}
template
void World::save<ALLCOMPONENTS>(const std::string& path) const;

template <typename... Component>
bool World::load(const std::string& path)
{
	std::ifstream stream;
	stream.open(path, std::ios::in | std::ios::binary);
	if (!stream) { return false; }
	entt::basic_snapshot_loader<Registry> snapshot_loader(world_registry);
	cereal::BinaryInputArchive archive{stream};

	snapshot_loader.get<Entity>(archive);
	([&]()
	{
		snapshot_loader.get<Component>(archive);
	}(), ...);

	return true;
}
template
bool World::load<ALLCOMPONENTS>(const std::string& path);

void World::onPositionBlockIdConstruct(const Registry& registry, const Entity entity)
{
	// only operate when both components have been removed
	if (!registry.all_of<BlockId, Position>(entity)) return;

	const BlockId id = registry.get<BlockId>(entity);
	const glm::vec3& pos = registry.get<Position>(entity).vec3;
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);

	instancing_models[id.uint()].push_back(std::move(model));
	instancing_normal_mats[id.uint()].push_back(glm::mat3(1.0f));
	// append it to the appropriate buffer
	updateInstancingBuffers(id, true, (instancing_models[id.uint()].size() - 1));
}

void World::onPositionBlockIdDestruct(const Registry& registry, const Entity entity)
{
	// only operate when both components have been removed
	if (!registry.all_of<BlockId, Position>(entity)) return;

	const BlockId id = registry.get<BlockId>(entity);
	const glm::vec3& pos = registry.get<Position>(entity).vec3;
	// TODO do I really need this?
	if ((instancing_models[id.uint()].size() == 0) || (instancing_normal_mats[id.uint()].size() == 0)) {
		LOG("Instancing data size does not match entt registry")
		return;
	}

	using ModelType = decltype(instancing_models)::value_type::value_type;
	using NormalMatType = decltype(instancing_normal_mats)::value_type::value_type;

	std::vector<ModelType>& model_vec = instancing_models[id.uint()];
	std::vector<NormalMatType>& normal_mat_vec = instancing_normal_mats[id.uint()];
	for (unsigned int i=0; i < (unsigned int)model_vec.size(); i++) {
		// row 3 of a 4x4 is the translation
		if (glm::vec3(model_vec[i][3]) != pos) {
			continue;
		} else {
			utils::vecSwapPopBack(model_vec, i);
			utils::vecSwapPopBack(normal_mat_vec, i);
			updateInstancingBuffers(id, true, i);
		}

		return;
	}

	LOG("Failed to remove entity from instance_vector")
}

void World::connect() {
	disconnect();
	connections.push_back(world_registry.on_construct<Position>().connect<onPositionBlockIdConstruct>(this));
	connections.push_back(world_registry.on_destroy<Position>().connect<onPositionBlockIdDestruct>(this));
	connections.push_back(world_registry.on_construct<BlockId>().connect<onPositionBlockIdConstruct>(this));
	connections.push_back(world_registry.on_destroy<BlockId>().connect<onPositionBlockIdDestruct>(this));
}

void World::disconnect() {
	for (size_t i=0; i<connections.size(); i++){
		connections[i].release();
	}
	connections.clear();
}

void World::initInstancingBuffers() {
	for (unsigned int i = 0; i < BlockId::NumNames; i++) {
		updateInstancingBuffers(BlockId(i), false, 0, true);
	}
}

void World::updateInstancingBuffers(const BlockId id, const bool new_data, const size_t index, const bool force_copy) {
	if (!force_copy && new_data && (instancing_models[id.uint()].size() <= index)) { return; }

	for (unsigned int i = 0; i < (unsigned int)num_unique_buffers; i++) {
		auto lambda = [=]<typename T>(const std::vector<T>& vec, GLuint buffer) {
			using ElemType = std::remove_cvref_t<decltype(vec)>::value_type;
			constexpr size_t elem_size = sizeof(ElemType);
			size_t vec_capacity_bytes = vec.capacity() * elem_size;
			size_t vec_size_bytes = vec.size() * elem_size;
			void* const data = (void*)vec.data();

			GLint buffer_size;
			glGetNamedBufferParameteriv(buffer, GL_BUFFER_SIZE, &buffer_size);
			// an empty buffer object causes errors when binding to a VAO
			if ((static_cast<size_t>(buffer_size) != elem_size) && (vec_capacity_bytes == 0)) {
				glNamedBufferData(buffer, elem_size, NULL, GL_DYNAMIC_DRAW);
			// resize following std::vector's amortized complexity
			} else if ((vec_capacity_bytes != 0) && (force_copy || (vec_capacity_bytes != static_cast<size_t>(buffer_size)))) {
				glNamedBufferData(buffer, vec_capacity_bytes, NULL, GL_DYNAMIC_DRAW);
				glNamedBufferSubData(buffer, 0, vec_size_bytes, data);
			} else if (new_data) {
				GLintptr offset = index * elem_size;
				glNamedBufferSubData(buffer, offset, elem_size, (void*)((char*)data + offset));
			}
		};

		lambda(instancing_models[id.uint()], instancing_model_buffers[id.uint()]);
		lambda(instancing_normal_mats[id.uint()], instancing_normal_mat_buffers[id.uint()]);

	}
}

void World::initInstancingData() {
	for (unsigned int i = 0; i < instancing_models.size(); i++) {
		instancing_models[i].clear();
		instancing_models.shrink_to_fit();
		instancing_normal_mats[i].clear();
		instancing_normal_mats[i].shrink_to_fit();
	}

	const auto view = world_registry.view<Position, BlockId>();
	for (Entity entity : view) {
		const glm::vec3& pos = view.get<Position>(entity).vec3;
		const BlockId id = view.get<BlockId>(entity);
		glm::mat4 model = glm::mat4(1.0);
		model = glm::translate(model, pos);

		instancing_models[id.uint()].push_back(std::move(model));
		instancing_normal_mats[id.uint()].push_back(glm::mat3(1.0f));
	}
}

void World::initData() {
	initInstancingData();
	initInstancingBuffers();
}
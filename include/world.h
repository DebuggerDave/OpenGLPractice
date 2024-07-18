#ifndef WORLD_H
#define WORLD_H

struct BlockId;

#include "glm/mat4x4.hpp"
#include "glm/mat3x3.hpp"
#include "glad/gl.h"
#include "entt/entity/registry.hpp"

#include <string>
#include <vector>

class World
{
public:
	using Registry = entt::basic_registry<uint64_t>;
	using Entity = Registry::entity_type;

	World() noexcept;
	~World();
	World(const World& other) = delete;
	World(World&& other) noexcept;
	World& operator=(const World& other) = delete;
	World& operator=(World&& other) = delete;

	void reset();
	// attach instancing buffers to VAO
	void setupInstancing(const GLuint VAO, const GLuint vertex_attrib_index, const BlockId id) const;
	// number of instancing objects for a given ID
	size_t numObjects(const BlockId id) const;
	// update instancing normal matrices for a given view matrix
	void updateNormalMats(const glm::mat4& view);

	// square length of world
	static const int chunk_size = 256;
	static_assert((World::chunk_size % 2) == 0);
	// surface variance from average height
	static const int terrain_amplitude = 10;
	// average surface height
	static const int terrain_median_height = 20;
	// maximum world height
	static const int max_height = 50;
	// minimum world height
	static const int min_height = 0;
	// perlin noise
	inline static const float noise_scale = 0.01f;
	// path to save to disk
	inline static const std::string world_path = "./world.bin";
	// blocks are 1m wide
	inline static constexpr float block_half_length = .5;
private:
	// procedurally generate registry
	void generateWorld();
	// save all registry components to disk
	void saveAll() const;
	// load all registry components from disk
	bool loadAll();
	// save registry component to disk
	template <typename... Component>
	void save(const std::string& path) const;
	// load registry component from disk
	template <typename... Component>
	bool load(const std::string& path);
	// callback for when an entity gains both it's BlockId and Position Components
	void onPositionBlockIdConstruct(const Registry& registry, const Entity entity);
	// callback for when an entity loses both it's BlockId and Position Components
	void onPositionBlockIdDestruct(const Registry& registry, const Entity entity);
	// add all entt callbacks
	void connect();
	// remove all entt callbacks
	void disconnect();
	// copy data from data structures to opengl buffers
	void initInstancingBuffers();
	// copy data into opengl buffers, resize if needed
	void updateInstancingBuffers(const BlockId id, const bool new_data = false, const size_t index = 0, const bool force_copy = false);
	// copy data from entt::registry into external data structures
	void initInstancingData();
	// copy data from entt::registry into external data structures and opengl buffers
	void initData();

	// invariant: model and normal vectors are the same size

	// opengl buffers for instancing model matrices
	std::vector<GLuint> instancing_model_buffers;
	// opengl buffers for instancing normal matrices
	std::vector<GLuint> instancing_normal_mat_buffers;
	// number of opengl buffers to update
	// TODO is there a better way to do this?
	static const size_t num_unique_buffers = 2;
	// instancing model matrices to be copied to opengl buffers
	std::vector<std::vector<glm::mat4>> instancing_models;
	// instancing normal matrices to be copied to opengl buffers
	std::vector<std::vector<glm::mat3>> instancing_normal_mats;
	// Entity component system
	Registry world_registry;
	// connections to ecs
	std::vector<entt::connection> connections;
};

#endif
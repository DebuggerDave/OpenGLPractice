#ifndef WORLD_H
#define WORLD_H

#include "entt/entity/fwd.hpp"
#include <string>

class World
{
public:
	using Registry = entt::basic_registry<uint64_t>;
	using Entity = Registry::entity_type;

	World();
	~World();

	void generateWorld();
	void clean();
	const Registry& getRegistry() const;
	void saveAll();
	bool loadAll();

	static const int chunk_size = 16;
	static const int terrain_aplitude = 4;
	static const int terrain_median_height = 10;
	static const int max_height = 15;

private:
	template <typename... Component>
	void save(const std::string& path) const;
	template <typename... Component>
	bool load(const std::string& path);

	inline static const std::string world_path = "./world.bin";
	Registry registry;
};

#endif
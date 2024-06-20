#include "world.h"

#include "pch.h"
#include "component.h"
#include "utils.h"

World::World()
{
	if (!loadAll()) {
		generateWorld();
	}
}

World::~World()
{
	saveAll();
}

void World::generateWorld()
{
	static_assert((chunk_size % 2) == 0);
	registry.clear();
	std::random_device rd;
	const siv::BasicPerlinNoise<float> perlin;

	for (unsigned int x = 0; x < chunk_size; x++) {
		for (unsigned int z = 0; z < chunk_size; z++) {
			float perlin_x = (((float)x / chunk_size) * std::tuple_size<decltype(perlin)::state_type>{});
			float perlin_z = (((float)z / chunk_size) * std::tuple_size<decltype(perlin)::state_type>{});
			unsigned int surface_level = perlin.normalizedOctave2D(perlin_x, perlin_z, 3, 0.5f) * terrain_aplitude + terrain_median_height;
			for (unsigned int y=0; y<=surface_level; y++) {
				Entity entity = registry.create();
				float float_x = (x + 0.5f) - (chunk_size / 2.0f);
				float float_y = y + 0.5f;
				float float_z = (z + 0.5f) - (chunk_size / 2.0f);
				registry.emplace<Position>(entity, float_x, float_y, float_z);
				if (y == surface_level) {
					registry.emplace<BlockId>(entity, BlockId::Grass);
				} else {
					registry.emplace<BlockId>(entity, BlockId::Dirt);
				}
			}
		}
	}
}

void World::clean()
{
	std::remove(world_path.c_str());
	generateWorld();
}

const World::Registry& World::getRegistry() const
{
	return registry;
}

void World::saveAll() {
	save<ALLCOMPONENTS>(world_path);
}

bool World::loadAll() {
	return load<ALLCOMPONENTS>(world_path);
}

template <typename... Component>
void World::save(const std::string& path) const
{
	std::ofstream stream;
	stream.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
	entt::basic_snapshot<Registry> snapshot(registry);
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
	entt::basic_snapshot_loader<Registry> snapshot_loader(registry);
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
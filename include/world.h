#include "entt/entity/registry.hpp"
#include "PerlinNoise.hpp"
#include <string>

class World
{
public:
	World();
	~World();

	void generate();

private:
	void saveFile();
	void openFile();

	inline static const std::string file_name = "./world/world.bin";
	entt::registry registry;

};

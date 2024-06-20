#ifndef COMPONENT_H
#define COMPONENT_H

#include "glm/ext/vector_float3.hpp"

struct Position
{
	Position() = default;
	Position(const Position& position) = default;
	Position(float x, float y, float z);
	glm::vec3 data;
};

enum class BlockId {
	Grass,
	Dirt,
	Cobblestone
};

template<typename Archive>
void serialize(Archive &archive, Position &position);

#define ALLCOMPONENTS Position, BlockId

#endif
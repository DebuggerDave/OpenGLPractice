#ifndef COMPONENT_H
#define COMPONENT_H

#include "glm/vec3.hpp"

struct Position
{
	Position() = default;
	Position(const Position& position) = default;
	Position(float x, float y, float z);
	glm::vec3 vec3;
};

enum class BlockId : unsigned int
{
	Grass,
	Dirt,
	Cobblestone,
	NoneOrNumIDs
};

struct BoxCollider
{
	BoxCollider() = default;
	BoxCollider(const BoxCollider&) = default;

	glm::vec3 offset = { 0.0f, 0.0f, 0.0f };
	glm::vec3 size = { 1.0f, 1.0f, 1.0f };
};

template <typename Archive>
void serialize(Archive &archive, Position &position);

#define ALLCOMPONENTS Position, BlockId

#endif
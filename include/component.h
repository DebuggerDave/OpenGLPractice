#ifndef COMPONENT_H
#define COMPONENT_H

#include "glm/vec3.hpp"

struct Position
{
	Position() noexcept = default;
	Position(const Position& position) noexcept = default;
	Position(float x, float y, float z) noexcept;
	// serialize
	template <typename Archive>
	void serialize(Archive &archive);

	// data
	glm::vec3 vec3;
};

struct BlockId
{
	enum class Name : unsigned int {
		Grass,
		Dirt,
		Cobblestone,
		None
	};

	BlockId() noexcept;
	BlockId(const unsigned int name) noexcept;
	BlockId(const Name name) noexcept;
	BlockId(const BlockId&) noexcept = default;
	// serialization
	template <typename Archive>
	void serialize(Archive &archive);
	// conversions
	unsigned int uint() const;
	operator Name() const;

	// convenience vars
	static constexpr Name NameFirst = Name::Grass;
	static constexpr Name NameLast = Name::Cobblestone;
	static constexpr unsigned int NumNames = static_cast<unsigned int>(NameLast) + 1;
	// data
	Name name;
};

struct BoxCollider
{
	BoxCollider() noexcept = default;
	BoxCollider(const BoxCollider&) noexcept = default;

	// data
	glm::vec3 offset = { 0.0f, 0.0f, 0.0f };
	glm::vec3 size = { 1.0f, 1.0f, 1.0f };
};

#define ALLCOMPONENTS Position, BlockId, BoxCollider

#endif
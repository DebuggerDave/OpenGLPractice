#include "component.h"

#include "pch.h"

Position::Position(float x, float y, float z) :
data{x, y, z}
{}

template<typename Archive>
void serialize(Archive &archive, Position &position) {
	glm::vec3& pos = position.data;
    archive(pos.x, pos.y, pos.z);
}
template
void serialize(cereal::BinaryInputArchive &archive, Position &position);
template
void serialize(cereal::BinaryOutputArchive &archive, Position &position);
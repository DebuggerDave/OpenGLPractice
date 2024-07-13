#include "component.h"

#include "glm/vec3.hpp"
#include "cereal/archives/binary.hpp"

Position::Position(float x, float y, float z) : vec3{x, y, z}
{}

template<typename Archive>
void serialize(Archive &archive, Position &position) {
	glm::vec3& pos = position.vec3;
    archive(pos.x, pos.y, pos.z);
}
template
void serialize(cereal::BinaryInputArchive &archive, Position &position);
template
void serialize(cereal::BinaryOutputArchive &archive, Position &position);
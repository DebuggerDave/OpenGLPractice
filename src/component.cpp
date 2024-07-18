#include "component.h"

#include "glm/vec3.hpp"
#include "cereal/archives/binary.hpp"

Position::Position(float x, float y, float z) noexcept : vec3{x, y, z} {}

template<typename Archive>
void Position::serialize(Archive &archive) {
    archive(vec3.x, vec3.y, vec3.z);
}
template
void Position::serialize(cereal::BinaryInputArchive &archive);
template
void Position::serialize(cereal::BinaryOutputArchive &archive);


BlockId::BlockId() noexcept : name(NameFirst) {}
BlockId::BlockId(const unsigned int name) noexcept : name(static_cast<Name>(name)) {}
BlockId::BlockId(const Name name) noexcept : name(name) {}

unsigned int BlockId::uint() const
{
	return static_cast<unsigned int>(name);
}

BlockId::operator Name() const
{
	return name;
}

template<typename Archive>
void BlockId::serialize(Archive &archive) {
    archive(name);
}
template
void BlockId::serialize(cereal::BinaryInputArchive &archive);
template
void BlockId::serialize(cereal::BinaryOutputArchive &archive);

template<typename Archive>
void BoxCollider::serialize(Archive &archive) {
    archive(offset.x, offset.y, offset.z, size.x, size.y, size.z);
}
template
void BoxCollider::serialize(cereal::BinaryInputArchive &archive);
template
void BoxCollider::serialize(cereal::BinaryOutputArchive &archive);
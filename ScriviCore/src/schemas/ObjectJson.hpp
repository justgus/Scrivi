#pragma once

#include "scrivi/ObjectTypes.hpp"
#include "scrivi/Result.hpp"

#include <string>
#include <string_view>

namespace scrivi::schemas {

std::string serializeCharacter(const CharacterObject& obj);
Result<CharacterObject> parseCharacter(std::string_view json);

std::string serializeLocation(const LocationObject& obj);
Result<LocationObject> parseLocation(std::string_view json);

std::string serializeItem(const ItemObject& obj);
Result<ItemObject> parseItem(std::string_view json);

std::string serializeRule(const RuleObject& obj);
Result<RuleObject> parseRule(std::string_view json);

std::string serializeTimeline(const TimelineObject& obj);
Result<TimelineObject> parseTimeline(std::string_view json);

// Serialize / parse any WorldObject variant.
std::string serializeWorldObject(const WorldObject& obj);
Result<WorldObject> parseWorldObject(std::string_view json, ObjectKind kind);

} // namespace scrivi::schemas

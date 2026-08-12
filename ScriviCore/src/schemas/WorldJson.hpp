#pragma once

#include "scrivi/Result.hpp"
#include "worlds/WorldTypes.hpp"

#include <string>
#include <string_view>

namespace scrivi::schemas {

// scrivi.world.v1 — the world package's own identity record.
std::string serializeWorld(const worlds::WorldRecord& w);
Result<worlds::WorldRecord> parseWorld(std::string_view json);

// scrivi.world-binding.v1 — the project-local translation + reference.
std::string serializeWorldBinding(const worlds::WorldBindingRecord& b);
Result<worlds::WorldBindingRecord> parseWorldBinding(std::string_view json);

} // namespace scrivi::schemas

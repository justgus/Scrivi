#pragma once

#include "scrivi/ObjectTypes.hpp"
#include "scrivi/Result.hpp"

#include <string>
#include <string_view>

namespace scrivi::schemas {

// The `scrivi.object.<kind>.v1` schema tag for a kind.
std::string objectSchemaTag(ObjectKind kind);

// Builds the WorldObject alternative matching `kind`, carrying `fields` across.
WorldObject makeWorldObject(ObjectKind kind, WorldObjectFields fields);

// Serialize / parse any WorldObject variant.
//
// Per-kind wrappers were retired in SP-095: the 11 kinds share one field block
// and differ only by schema tag, so the kind-keyed pair below is the whole API.
std::string serializeWorldObject(const WorldObject& obj);
Result<WorldObject> parseWorldObject(std::string_view json, ObjectKind kind);

} // namespace scrivi::schemas

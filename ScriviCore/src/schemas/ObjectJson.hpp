#pragma once

#include "scrivi/ObjectTypes.hpp"
#include "scrivi/Result.hpp"

#include <string>
#include <string_view>

namespace scrivi::schemas {

// Serialize a CharacterObject to a JSON string.
std::string serializeCharacter(const CharacterObject& obj);

// Parse a JSON string back into a CharacterObject.
// Returns parseError on malformed JSON, validationError on schema mismatch.
Result<CharacterObject> parseCharacter(std::string_view json);

} // namespace scrivi::schemas

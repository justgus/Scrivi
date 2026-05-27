#pragma once

#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <map>
#include <string>
#include <vector>

namespace scrivi {

// -----------------------------------------------------------------------
// CharacterObject — one character entry in objects/characters/<slug>.json
// -----------------------------------------------------------------------

struct CharacterObject {
    ObjectID         objectID;
    Slug             slug;
    std::string      displayName;
    std::string      status;          // "active" | "archived"

    ISO8601Timestamp createdAt;
    std::string      createdByIdentityID;
    std::string      createdByPersonaID;
    std::string      createdByDisplayName;

    ISO8601Timestamp modifiedAt;
    std::string      modifiedByIdentityID;
    std::string      modifiedByPersonaID;
    std::string      modifiedByDisplayName;

    std::string      notes;
    std::vector<std::string>           tags;
    std::map<std::string, std::string> attributes;  // freeform key/value pairs
};

} // namespace scrivi

#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <string>
#include <vector>

namespace scrivi::schemas {

struct PersonaEntry {
    PersonaID   personaID;
    std::string displayName;
    std::string personaKind;
    std::string controlledByIdentityID;
    ISO8601Timestamp createdAt;
    std::string status;
};

struct ProjectPersonasData {
    std::vector<PersonaEntry> personas;
};

std::string serializeProjectPersonas(const ProjectPersonasData& data);
Result<ProjectPersonasData> parseProjectPersonas(std::string_view json);

} // namespace scrivi::schemas

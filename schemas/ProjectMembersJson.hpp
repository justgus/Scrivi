#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <string>
#include <vector>

namespace scrivi::schemas {

struct MemberEntry {
    IdentityID  identityID;
    std::string role;
    std::string status;
    std::string defaultPersonaID;
    ISO8601Timestamp joinedAt;
};

struct ProjectMembersData {
    std::vector<MemberEntry> members;
};

std::string serializeProjectMembers(const ProjectMembersData& data);
Result<ProjectMembersData> parseProjectMembers(std::string_view json);

} // namespace scrivi::schemas

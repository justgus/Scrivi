#pragma once

#include "scrivi/Types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace scrivi {

struct Comment {
    std::string      commentID;
    std::string      body;
    bool             resolved  = false;
    ISO8601Timestamp createdAt;

    std::string createdByIdentityID;
    std::string createdByPersonaID;
    std::string createdByDisplayName;

    std::optional<ISO8601Timestamp> resolvedAt;
    std::optional<std::string>      resolvedByIdentityID;
    std::optional<std::string>      resolvedByPersonaID;
    std::optional<std::string>      resolvedByDisplayName;
};

// scopeKind: "scene" | "object" (determines subdirectory under comments/)
struct CommentThread {
    std::string          schema;
    std::string          scopeKind;   // "scene" | "object"
    std::string          targetID;    // sceneID or objectID value
    std::vector<Comment> comments;
};

} // namespace scrivi

#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace scrivi::schemas {

struct SceneStoryTime {
    int64_t     offsetMs            = 0;       // derived — not authoritative for "manual" scenes
    std::string offsetSource        = "default";   // "default" | "manual" | "inferred"
    int64_t     gapMs               = 0;       // gap from previousSceneEnd to this scene's start
                                               // canonical for "manual"; 0 for "default"
    int64_t     durationMs          = 3'600'000;   // default: 1 hour
    std::string durationSource      = "default";   // "default" | "manual"
    std::string inferenceHint;                      // empty = null in JSON
    double      inferenceConfidence = -1.0;         // -1.0 = absent in JSON
    std::string bandID;                             // empty = null in JSON
    std::string bandAssignedAt;                     // empty = null in JSON
};

struct SceneMetaData {
    SceneID     sceneID;
    std::string title;
    Slug        slug;
    std::string status;
    ISO8601Timestamp createdAt;
    std::string createdByIdentityID;
    std::string createdByPersonaID;
    std::string createdByDisplayName;
    ISO8601Timestamp modifiedAt;
    std::string modifiedByIdentityID;
    std::string modifiedByPersonaID;
    std::string modifiedByDisplayName;
    std::string contentPath;
    std::size_t wordCount      = 0;
    std::size_t characterCount = 0;
    SceneStoryTime storyTime;
};

std::string serializeSceneMeta(const SceneMetaData& data);
Result<SceneMetaData> parseSceneMeta(std::string_view json);

} // namespace scrivi::schemas

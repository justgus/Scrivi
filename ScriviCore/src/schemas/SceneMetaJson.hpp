#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

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

// One todo item on a scene's todo card (EP-030 SP-091, T-0392).
struct SceneTodoItem {
    std::string text;
    bool        done = false;
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

    // Writing-tool card content (EP-030 SP-091). Per-scene creative material, stored
    // with the scene so it travels with it and stays Git-visible — deliberately NOT in
    // inspector-layout.json, which is view configuration.
    //
    // All three are ADDITIVE: a sidecar written before SP-091 has none of these keys and
    // must parse unchanged, leaving them empty.
    std::vector<std::string>   tags;      // ordered, de-duplicated
    std::string                outline;   // freeform synopsis prose
    std::vector<SceneTodoItem> todo;      // ordered
};

std::string serializeSceneMeta(const SceneMetaData& data);
Result<SceneMetaData> parseSceneMeta(std::string_view json);

} // namespace scrivi::schemas

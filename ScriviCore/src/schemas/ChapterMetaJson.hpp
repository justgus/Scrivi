#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <string>
#include <vector>

namespace scrivi::schemas {

// EP-027 §8.1 (B3, scenes): a scene reference is the BARE FILENAME of the scene's
// `.meta.json`, resolved relative to the owning chapter's own folder. It deliberately
// carries NO `sceneID` (identity lives solely in the scene sidecar) and NO embedded
// chapter-folder prefix (a name that changes on every chapter reorder/split/migrate).
// A bare filename never needs rewriting when its parent chapter folder is renamed, which
// is what dissolves the B3/B4/C6 orphaning class. `scenes[]` stays an ordered, rebuildable
// cache — regenerated from the folder scan when it disagrees with disk.
struct SceneRef {
    std::string metadataFilename;   // e.g. "a0-scene.meta.json" — relative to the chapter dir
};

struct ChapterMetaData {
    ChapterID   chapterID;
    std::string title;
    Slug        slug;
    std::string displayLabel;
    std::string status;
    ISO8601Timestamp createdAt;
    std::string createdByIdentityID;
    std::string createdByPersonaID;
    std::string createdByDisplayName;
    std::vector<SceneRef> scenes;
};

std::string serializeChapterMeta(const ChapterMetaData& data);
Result<ChapterMetaData> parseChapterMeta(std::string_view json);

} // namespace scrivi::schemas

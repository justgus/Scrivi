#pragma once

#include "schemas/SceneMetaJson.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <vector>

namespace scrivi::manuscript {

struct ResolvedScene {
    SceneID      sceneID;
    ChapterID    chapterID;
    std::string  title;
    std::string  chapterTitle;
    Slug         slug;
    std::string  status;
    RelativePath metadataPath;
    RelativePath contentPath;
    RelativePath chapterMetadataPath;

    // EP-039 AC2 (SP-131). `resolve()` already reads and parses every scene sidecar, so
    // the story-time block is ALREADY IN HAND -- carrying it costs nothing here and saves
    // `SceneStoryTimeIndex` a second full walk. ⚠️ Adding a second traversal to populate
    // it would reintroduce the very cost this Epic exists to remove.
    schemas::SceneStoryTime storyTime;
};

// Walks the on-disk project structure (manuscript.meta.json → chapter.meta.json
// → scene.meta.json) and returns scenes in manuscript order.
class ManuscriptOrderResolver {
public:
    explicit ManuscriptOrderResolver(CoreServices& services);

    [[nodiscard]] Result<std::vector<ResolvedScene>> resolve(const AbsolutePath& projectRoot) const;

    // Locates ONE scene without resolving the whole manuscript.
    //
    // ⚠️ [I-0196]. `resolve()` reads AND JSON-PARSES the sidecar of EVERY scene.
    // `openScene` called it to find a SINGLE scene, and the app calls `openScene`
    // once per scene on load -- so a full open was N resolves x N parses. ⚠️ At
    // 1,152 scenes that is ~1.33 MILLION read+parse operations and a MEASURED
    // 263 SECONDS of blocking main-thread work (macOS, local SSD, -O2). ⚠️ The
    // cost is QUADRATIC, so it is invisible at 16 scenes and fatal at 1,152.
    //
    // ✅ This walks chapters in order and asks each chapter's scene listing for
    // the id. `listScenesByOrder` already reads each scene sidecar for its
    // AUTHORITATIVE sceneID and content filename (EP-027 made scenes
    // filesystem-authoritative), ⚠️ so the answer is exactly as trustworthy as
    // `resolve()`'s -- ✅ it simply STOPS at the match instead of building, and
    // fully parsing, an entry for every other scene in the manuscript.
    //
    // ⚠️ It returns only what a single-scene open needs (identity + paths). A
    // caller wanting titles or ordering still uses `resolve()`.
    [[nodiscard]] Result<ResolvedScene> findScene(const AbsolutePath& projectRoot,
                                                  const SceneID& sceneID) const;

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript

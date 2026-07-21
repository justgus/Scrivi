#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

// SceneMerger — join a scene into the scene immediately before it in the SAME chapter
// (EP-028 SP-074, T-0299). The predecessor is the survivor: it keeps its own order-key
// files and gains the merged scene's body (appended), while the merged scene's `.meta.json`
// + `.md` are removed. Identity + location are filesystem-authoritative (EP-027 §8.1): the
// scene is found by scanning on-disk chapters/scenes, never a cache. The affected chapter's
// `scenes[]` cache is rebuilt from disk afterwards.
//
// This is the same-chapter case only. A whole-chapter merge (cross-folder relocation) is
// scrivi_merge_chapter / ChapterMerger (T-0300).
class SceneMerger {
public:
    explicit SceneMerger(CoreServices& services);

    [[nodiscard]] Result<MergeSceneResult> merge(const MergeSceneRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript

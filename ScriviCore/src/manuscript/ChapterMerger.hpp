#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

// ChapterMerger — merge a whole chapter into the chapter immediately before it in
// manuscript order (EP-028 SP-074, T-0300). This is the atomic ScriviCore fix for I-0083.
//
// Every scene FILE in the merged chapter is RELOCATED into the predecessor chapter's folder
// (appended after its last scene with freshly minted order keys, `.meta.json` + `.md` moved
// and the sidecar slug/contentPath rewritten — exactly the SceneReorderer cross-chapter
// move), so no scene body is lost. Only then is the now-empty merged chapter folder + its
// manuscript.meta.json entry removed. Identity + location are filesystem-authoritative
// (EP-027 §8.1): the chapter is located by chapterID scan on disk, not from a cache.
//
// Contrast with the buggy Swift-composed merge (I-0083): that reassigned the merged
// chapter's scenes to the predecessor IN MEMORY and then called deleteChapter, which
// removed the merged chapter's folder wholesale — deleting the scene files that still
// physically lived there. This endpoint moves the files first.
class ChapterMerger {
public:
    explicit ChapterMerger(CoreServices& services);

    [[nodiscard]] Result<MergeChapterResult> merge(const MergeChapterRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript

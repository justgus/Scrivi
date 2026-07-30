#pragma once

// FragmentCutter — EP-029 SP-088 (T-0353): cut-with-merge. Extract a manuscript range that may
// cross scene/chapter boundaries into a structured fragment (like FragmentExtractor), THEN delete
// the spanned text and collapse the spanned scenes/chapters into one continuous scene — exactly
// as ⌘X behaves in a flat document.
//
// Delete-and-fold (design §4.3, ruling 2026-07-27):
//   1. Extract the fragment (so the removed content is available to paste / land in a buffer).
//   2. Fold the survivors into the HEAD scene: its body becomes headPrefix + tailSuffix — the
//      bytes before the span start abutted directly to the bytes after the span end (DIRECT
//      concatenation, no blank-line seam: a cut just closes the gap, like deleting a selection).
//   3. Delete every OTHER scene in the span (interior + the tail scene) by sceneID. Scenes AFTER
//      the span in a partially-covered chapter are left untouched.
//   4. Remove any chapter left empty by step 3.
//
// Why not compose SceneMerger/ChapterMerger: SceneMerger is same-chapter only; ChapterMerger merges
// a WHOLE chapter (would pull in post-span scenes); there is no single-scene-cross-chapter merge.
// Delete-and-fold is uniform across same/cross-chapter spans and leaves post-span scenes untouched.
// It composes the existing SceneWriter / SceneDeleter / ChapterDeleter primitives — no new
// structural code — in one ScriviCore call. The head fold happens BEFORE any deletion, so a
// mid-sequence failure never loses surviving text.
//
// Design: docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md §3, §4.3 (approved 2026-07-27).

#include "manuscript/FragmentExtractor.hpp"  // FragmentSpan / Fragment

#include "scrivi/IDs.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <string>
#include <vector>

namespace scrivi::manuscript {

struct CutFragmentResult {
    Fragment               fragment;           // the extracted content (for buffer / undo)
    SceneID                survivingSceneID;   // the head scene, now = headPrefix + tailSuffix
    std::vector<SceneID>   removedSceneIDs;    // scenes deleted by the cut, in reading order
    std::vector<ChapterID> removedChapterIDs;  // chapters emptied + removed, in reading order
};

class FragmentCutter {
public:
    explicit FragmentCutter(CoreServices& services);

    // `spans` describe the selection, one per touched scene, in manuscript reading order (same
    // contract as FragmentExtractor). invalidArgument on the same span-validation failures as
    // extract; ioError on a read/write failure.
    [[nodiscard]] Result<CutFragmentResult> cut(const AbsolutePath& projectRoot,
                                                const std::vector<FragmentSpan>& spans);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript

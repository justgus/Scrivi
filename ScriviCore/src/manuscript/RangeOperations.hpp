#pragma once

// RangeOperations — EP-053 S1 (SP-186, T-0614): read, paste and cut a Range, each in ONE pass over exactly its files.
//
// ✅ R2 (user, 2026-10-17): "an operation is a range, and its cost is the range's." ✅ R7: "A two scene paste should take 4 file
// reads and 4 file writes." ✅ Budget re-ruled 2026-10-20 (user, planning EP-053): **3 reads + 5 writes**, because the
// chapter sidecar's `scenes[]` list is kept current — the validator and `scanForExternalChanges` still read it, and a stale
// one reports a cut scene as a BLOCKING "Missing scene.meta.json".
//
// How the budget is met (design v0.2 §4.4):
//   • every location, order key and title comes from the PROJECT INDEX — no listing, no sibling sidecar read;
//   • every file is written ONCE with its final content — no empty create then save, no `workspace-state.json`;
//   • the index is PATCHED IN PLACE from what was written (`ProjectIndex::insertScenesAfter` and siblings).
//
// ✅ Scene identity (user, 2026-10-20): a paste re-creates a scene with the Range's own sceneID when that ID is NOT in the
// manuscript (it was cut), and mints a fresh one when it is (it was copied). Chapters likewise. A cut deletes the files but
// does NOT prune relationship edges, so its undo — or a later paste, a move — gets them back; a cut never pasted leaves
// dangling edges, which load-time repair drops as it always has.
// ✅ [SP-191] Q3b: a cut (and a relabel that empties a chapter) puts each removed scene's sidecar and each removed chapter's
// sidecar in the Range it returns, and a paste that keeps an ID writes them back — so an undo restores status, story time,
// tags, outline and todo exactly. A paste under a fresh ID (a copy) starts a new sidecar, as before.
// ✅ [SP-191] Q3e (user, 2026-10-21): the manuscript is itself a Range and every operation works on a sub-range of it. A
// scene merge is a cut of the zero-length sub-range across a break; a split is a paste of an empty two-entry Range; a
// chapter merge moves nothing out of the Range, so it is a RELABEL of a run of whole scenes (`relabel`).

#include "manuscript/ProjectIndex.hpp"
#include "manuscript/Range.hpp"

#include "scrivi/IDs.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace scrivi::manuscript {

// One scene the operation created, rewrote or moved — design v0.2 P3, "the result is the change".
struct RangeSceneChange {
    enum class Kind { Created, Rewritten, Moved };
    Kind          kind = Kind::Rewritten;
    SceneLocation location;        // where the scene now is (chapter, paths, titles); ⚠️ its `ordinal` is not meaningful
    SceneID       afterSceneID;    // the scene before it in reading order now; empty = the manuscript's first
    std::string   text;            // the body as written (Created / Rewritten); empty for Moved (its text is unchanged)
};

// One chapter the operation created.
struct RangeChapterChange {
    ChapterID    chapterID;
    std::string  title;
    RelativePath metadataPath;
    ChapterID    afterChapterID;   // the chapter before it in reading order
};

// The whole change, in reading order — what a platform applies to its in-memory scene list instead of reloading.
struct RangeDelta {
    std::vector<RangeSceneChange>   scenes;
    std::vector<SceneID>            removedSceneIDs;
    std::vector<RangeChapterChange> createdChapters;
    std::vector<ChapterID>          removedChapterIDs;
};

struct RangeOperationResult {
    RangeDelta delta;
    // Paste: the Range it INSERTED, under the scenes' real IDs — its undo is a cut of exactly this.
    // Cut: the Range it REMOVED — the clipboard's content, and its undo is a paste of this at the cut's start.
    Range range;
    std::map<std::string, std::string> writtenTexts;   // sceneID → body, for the history's persisted-text record ([I-0296])
    // True when the index was patched to match the disk. ⛔ False ⇒ the caller drops the index (the disk is still right).
    bool indexCurrent = false;
    // Relabel: the chapter the run LEFT — its inverse is a relabel of `range` back into this chapter.
    ChapterID fromChapterID;
    // Cut: where its inverse pastes `range` — the survivor's seam (the start scene's cut point, or the end scene's start).
    SceneID     undoCaretSceneID;
    std::size_t undoCaretByte = 0;
};

class RangeOperations {
public:
    // ⚠️ `index` must be VALID and describe the project as it is on disk. It is patched only after every write succeeded.
    // ⛔ On ANY failure the caller drops the index: a write that failed part-way leaves it describing a state that is gone.
    RangeOperations(CoreServices& services, AbsolutePath projectRoot, ProjectIndex& index);

    // `scrivi_range_read`: from (startScene, startByte) to (endScene, endByte), walking the index — no search. Reads only
    // the range's scene texts. invalidArgument if a scene is unknown, the end precedes the start, or an offset is outside
    // its scene or inside a UTF-8 sequence.
    // A READ: static, on a const index (the ABI serves it under a read guard, without taking the index).
    [[nodiscard]] static Result<Range> read(CoreServices& services, const AbsolutePath& projectRoot,
                                            const ProjectIndex& index, const SceneID& startScene, std::size_t startByte,
                                            const SceneID& endScene, std::size_t endByte);

    // `scrivi_range_paste` (design §4.4): the caret scene is split at the caret; its head takes the first entry's text;
    // each later entry creates a scene (same chapter as the one before it) or a chapter and its first scene (a different
    // chapter); the tail goes onto the last created scene. ✅ [I-0311]: when the Range opens a chapter, the caret
    // chapter's scenes after the caret move into the last created chapter, so the manuscript reads as a flat insert would.
    // ✅ [SP-191] Q7: at a scene's very START (caret 0, more than one entry) the new scenes go BEFORE the caret scene, which
    // keeps its text, identity and sidecar and takes the LAST entry's text in front of its own.
    // ⛔ Refused (invalidArgument) if `caretSceneText` is not the caret scene's text on disk: a stale editor never writes.
    [[nodiscard]] Result<RangeOperationResult> paste(const Range& range, const SceneID& caretScene, std::size_t caretByte,
                                                     const std::string& caretSceneText, const AuthorshipRef& author);

    // `scrivi_range_cut` (design §4.3, R4): the start scene keeps the text before the range, joined DIRECTLY to the text
    // after it in the end scene; every other scene in the range is deleted; every chapter left empty is deleted; the end
    // chapter's scenes after the range join the start scene's chapter (Q2).
    // ✅ [SP-191] Q7: when the range begins at the start scene's very first character, nothing of it survives, so the END
    // scene keeps its identity instead (with its tail) — the inverse of a paste at a scene's start. The result's
    // `undoCaret…` says where the undo pastes the Range back.
    // ⛔ Refused (invalidArgument) if the Range is not what the index holds — its scenes consecutive in reading order, in
    // the chapters it names — or if its first or last entry's text is not on disk (design P1).
    [[nodiscard]] Result<RangeOperationResult> cut(const Range& range, const AuthorshipRef& author);

    // `scrivi_range_relabel` ([SP-191] Q3e): the run of WHOLE scenes in `range` (one chapter's, consecutive; their text is
    // not read and need not be sent) changes chapter, keeping its reading order:
    //   • into the chapter BEFORE it — the run must open its chapter; it is appended to that chapter's end;
    //   • into the chapter AFTER it — the run must close its chapter; it goes before that chapter's first scene;
    //   • into a chapter the manuscript does not have (empty = a fresh ID) — the run must close its chapter; the chapter is
    //     created right after it, from the first entry's `chapterMeta` when it has one (an undo), else new.
    // On disk it is a MOVE: the scene files go to the target chapter's folder (renamed, and their sidecars' slug and
    // contentPath rewritten, only when their order keys would collide); the texts are never rewritten. A chapter left empty
    // is deleted, folder and all, after its scenes are out ([I-0083]), and its sidecar travels on the result's first entry.
    [[nodiscard]] Result<RangeOperationResult> relabel(const Range& range, const ChapterID& target,
                                                       const AuthorshipRef& author);

private:
    [[nodiscard]] SceneID scenesBefore(std::size_t ordinal) const;

    CoreServices& services_;
    AbsolutePath  root_;
    ProjectIndex& index_;
};

}  // namespace scrivi::manuscript

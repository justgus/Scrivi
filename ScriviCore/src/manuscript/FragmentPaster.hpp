#pragma once

// FragmentPaster — EP-029 SP-087 (T-0352): paste-splice a scrivi.fragment.v1 structured
// fragment into the manuscript at a caret, reconstructing every carried scene/chapter
// boundary — as if inserting into one continuous flat document.
//
// The inverse of FragmentExtractor (SP-086). Composes the existing EP-027 create primitives
// (SceneCreator with afterSceneID order-key placement; ChapterCreator with afterChapterID
// in-place placement) + SceneWriter — it introduces no new folder/order-key logic.
//
// Behaviour (design §4.2):
//   1. Split the target scene at the caret: head = body[0..caret], tail = body[caret..end].
//   2. First piece (opensWith None, the leading/head partial) → its text is appended to `head`;
//      the target scene becomes head + piece0.text.
//   3. Each opensWith Scene piece  → createScene after the running insertion point (same chapter).
//      Each opensWith Chapter piece → createChapter(afterChapterID) + write its text into the new
//      chapter's first scene.
//   4. The target scene's `tail` FOLLOWS THE WHOLE PASTED RUN: it is appended to the running
//      insertion point (the last scene created, or the target scene if the fragment carried no
//      boundaries), after the trailing (tail) piece's text. Flat-document model (user-ruled
//      2026-07-27): text after the caret reads continuously *after* everything pasted.
//   5. Fresh sceneIDs/chapterIDs are minted by the create primitives; order-keys by EP-027.
//
// NOT the editor's job to pre-split: the caret-in-heading refusal + divider-span normalisation
// are editor-side preconditions (SP-089); this primitive assumes a caret inside a real scene body.
//
// Design: docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md §3, §4.2 (approved 2026-07-27).

#include "manuscript/FragmentExtractor.hpp"  // Fragment / FragmentPiece / OpensWith / Partial
#include "manuscript/ManuscriptOrderResolver.hpp"  // ResolvedScene

#include "scrivi/IDs.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace scrivi::manuscript {

// Where to paste, plus the identity/authorship the create primitives need.
struct PasteFragmentRequest {
    AbsolutePath  projectRootPath;
    AbsolutePath  appSupportRoot;
    ProjectID     projectID;
    AuthorshipRef author;

    Fragment      fragment;
    SceneID       caretSceneID;    // the scene the caret sits in
    std::size_t   caretByteOffset = 0;  // scene-local UTF-8 byte offset of the caret
};

struct PasteFragmentResult {
    SceneID                targetSceneID;      // the (split) scene the paste began in
    std::vector<SceneID>   createdSceneIDs;    // scenes minted by the paste, in reading order
    std::vector<ChapterID> createdChapterIDs;  // chapters minted by the paste, in reading order
    // [I-0297]: sceneID → the body this paste WROTE (the target and every created scene), exactly as on disk — so nothing
    // has to find and re-read them (each lookup walked the manuscript; over a network share that froze the app).
    std::map<std::string, std::string> writtenTexts;
    // ✅ [I-0297] (SP-185): every chapter this paste CHANGED on disk — the target scene's (rewritten, scenes added) and each
    // chapter it created. No other chapter is touched (scenes are only ever added after the running scene, chapters only
    // after the running chapter). The C ABI rebuilds just these in the project index (`ChapterRebuildGuard`) instead of
    // dropping it whole — a whole drop cost a full manuscript walk on the next read (1,186 scenes: ~1,250 sidecars).
    std::vector<ChapterID> affectedChapterIDs;
};

// The exact inverse of a paste (EP-029 AC6 / T-0356 — undo of a structured paste, and redo of a
// structured cut). Folds the created scenes back into the target and STRIPS the pasted fragment's
// piece texts, restoring the target's original pre-paste body, then deletes the created
// scenes/chapters. Keyed by IDs + the fragment (never byte spans), so the app does no byte math.
struct UncutPasteRequest {
    AbsolutePath           projectRootPath;
    Fragment               fragment;           // the fragment that was pasted (its piece lengths)
    SceneID                targetSceneID;       // the scene the paste split (fold everything back here)
    std::vector<SceneID>   createdSceneIDs;     // scenes the paste minted, reading order
    std::vector<ChapterID> createdChapterIDs;   // chapters the paste minted, reading order
};

struct UncutPasteResult {
    SceneID survivingSceneID;   // the target scene, now restored to its pre-paste body
    std::string survivingText;  // [I-0297]: that body, exactly as written
    // ✅ [I-0297] (SP-185): every chapter this un-paste CHANGED on disk — the target's (restored), each chapter a deleted
    // scene was in, and each deleted chapter (gone from disk, so the rebuild simply no longer lists it).
    std::vector<ChapterID> affectedChapterIDs;
};

class FragmentPaster {
public:
    explicit FragmentPaster(CoreServices& services);

    // invalidArgument if the fragment is empty, the caret scene is unknown, or the caret byte
    // offset exceeds the target scene body. ioError on a read/write failure.
    [[nodiscard]] Result<PasteFragmentResult> paste(const PasteFragmentRequest& request);

    // ✅ [I-0297] (SP-185): finds ONE scene (identity, paths, chapter); nullopt = not in the manuscript.
    // ⚠️ The overloads above RESOLVE the whole manuscript to find the few scenes they touch — every scene sidecar read
    // twice (2,641 reads for one paste into 1,186 scenes, measured; over a network share most of a frozen undo). The C ABI
    // answers from the project index instead, and from disk when the index has no answer.
    using SceneLookup = std::function<std::optional<ResolvedScene>(const SceneID&)>;
    [[nodiscard]] Result<PasteFragmentResult> paste(const PasteFragmentRequest& request, const SceneLookup& lookup);

    // The inverse of paste (T-0356). invalidArgument if the fragment is empty, the target scene is
    // unknown, or a created scene body is shorter than the piece text it should end with (a state
    // that cannot have come from this fragment's paste). ioError on a read/write failure.
    [[nodiscard]] Result<UncutPasteResult> uncutPaste(const UncutPasteRequest& request);
    [[nodiscard]] Result<UncutPasteResult> uncutPaste(const UncutPasteRequest& request, const SceneLookup& lookup);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript

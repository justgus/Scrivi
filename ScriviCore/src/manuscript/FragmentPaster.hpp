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

#include "scrivi/IDs.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
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
};

class FragmentPaster {
public:
    explicit FragmentPaster(CoreServices& services);

    // invalidArgument if the fragment is empty, the caret scene is unknown, or the caret byte
    // offset exceeds the target scene body. ioError on a read/write failure.
    [[nodiscard]] Result<PasteFragmentResult> paste(const PasteFragmentRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript

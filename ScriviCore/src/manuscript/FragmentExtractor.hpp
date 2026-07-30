#pragma once

// FragmentExtractor — EP-029 SP-086 (T-0351): extract a manuscript range that may
// cross scene/chapter boundaries into a structured `scrivi.fragment.v1` fragment.
//
// The manuscript is presented (on Apple) as one continuous NSTextStorage with divider
// attachments between scenes and non-editable heading runs at chapter starts. A selection
// spanning boundaries cannot be copied as flat text without dragging those UI markers into
// the string. This primitive instead reads the SELECTED scene-local byte spans straight from
// the on-disk scene bodies (which contain neither dividers nor headings — those are UI-only)
// and assembles an ordered, structure-carrying fragment.
//
// READ-ONLY: extract never mutates the project (⌘C is non-destructive). Cut (delete + merge)
// is a separate primitive (SP-088); paste-splice is SP-087.
//
// Input is an ordered list of spans, one per scene the selection touches, in reading order:
//     [ { sceneID, startByte, endByte }, … ]
// Byte offsets are scene-local UTF-8 byte offsets into the scene body (the same convention as
// the EP-019 history engine). The editor derives these from its storage-range → sceneID map.
//
// Output is the fragment model (design §3): an ordered list of pieces, each annotated with the
// boundary that opens it (relative to the previous piece) and whether it is a partial head/tail
// scene, plus a plain-text flattening for the system pasteboard (T2=A).
//
// Design: docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md §3, §4.1 (approved 2026-07-27,
// trades T1=A · T2=A · T3=A · T4=A).

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace scrivi::manuscript {

// One requested selection span within a single scene, in reading order.
struct FragmentSpan {
    SceneID     sceneID;
    std::size_t startByte = 0;  // scene-local UTF-8 byte offset (inclusive)
    std::size_t endByte   = 0;  // scene-local UTF-8 byte offset (exclusive); >= startByte
};

// How a piece opens relative to the piece before it (design §3).
enum class OpensWith {
    None,     // same scene continues — valid only for the FIRST piece (selection began mid-scene)
    Scene,    // this piece begins a new scene (a divider was crossed)
    Chapter,  // this piece begins a new scene AND a new chapter (a heading was crossed)
};

// Whether a piece is a partial scene at the fragment's leading/trailing edge (design §3).
enum class Partial {
    None,  // whole scene (interior pieces are always whole)
    Head,  // first piece started mid-scene (its scene's prefix stays behind on cut)
    Tail,  // last piece ended mid-scene (its scene's suffix stays behind on cut)
};

// One assembled fragment piece: one scene's contribution to the selection.
struct FragmentPiece {
    OpensWith   opensWith    = OpensWith::None;
    std::string chapterTitle;  // present (non-empty semantics) only when opensWith == Chapter
    std::string sceneTitle;    // this scene's title (captured so paste can restore it — T-0357)
    std::string text;          // this scene's selected body (divider/heading-free by construction)
    Partial     partial       = Partial::None;
};

// The assembled structured fragment (schema "scrivi.fragment.v1").
struct Fragment {
    std::vector<FragmentPiece> pieces;
    std::string                plainText;  // pieces' text joined by a blank-line seam (T2 flattening)
};

class FragmentExtractor {
public:
    explicit FragmentExtractor(CoreServices& services);

    // Extract the fragment described by `spans` from the project at `projectRoot`.
    // Fails (invalidArgument) if: spans is empty; a span's sceneID is not in the manuscript;
    // a span is out of order (not in manuscript reading order); startByte > endByte; or a
    // span exceeds its scene body's byte length. ioError if a scene body can't be read.
    [[nodiscard]] Result<Fragment> extract(const AbsolutePath& projectRoot,
                                           const std::vector<FragmentSpan>& spans) const;

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript

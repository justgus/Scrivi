#pragma once

// Range — EP-053 S1 (SP-186, T-0614): ONE contiguous run through the manuscript, as a doubly linked list of scenes.
//
// ✅ R6 (user, 2026-10-17; design v0.2 §4.2): "a linked list of scenes. Each scene object will contain the starting position
// in the file of the first character of the range and the length … a forward link to the next scene … and the previous
// link … a chapter identifier … the chapter to restore to when an undo occurs … the text of the scene from the file."
//   • It is built by ANYONE from its own data — the editor from its in-memory scene list at a selection, ScriviCore from disk
//     given a start and an end (`scrivi_range_read`) — and used EVERYWHERE: the clipboard's structured content, a copy
//     buffer's content, the input to a paste, the output of a read and of a cut, what the core writes.
//   • ⛔ NO break characters. A scene break is the link from one entry to the next; a CHAPTER break is a link between entries
//     whose `chapterID` differs. Every entry after the first begins a scene.
//   • Contiguous by construction: the next scene is a link, never a search.
//   • `std::list` IS the doubly linked list: each entry's previous and next are its neighbours (`std::prev` / `std::next`),
//     the first has no previous and the last no next.
// ⚠️ Offsets (`start`, `length`, `sceneLength`) are UTF-8 BYTES — the unit every platform measures the same way (design §4.2).
// ⚠️ `sceneLength` (the scene's whole length) is one field beyond R6's list: without it an entry cannot say whether it covers
//    its whole scene ("starting position 0 and … the number of characters in the scene").

#include "manuscript/FragmentExtractor.hpp"   // Fragment, FragmentPiece (scrivi.fragment.v1)
#include "scrivi/IDs.hpp"
#include "scrivi/Result.hpp"

#include <cstddef>
#include <list>
#include <optional>
#include <string>
#include <string_view>

namespace scrivi::util { class JsonDoc; }

namespace scrivi::manuscript {

struct RangeEntry {
    SceneID     sceneID;
    ChapterID   chapterID;      // the chapter the scene belongs to — on an undo, the chapter it is restored to
    std::string chapterTitle;   // carried so a paste re-creates the chapter's name (T-0357)
    std::string sceneTitle;     // carried so a paste re-creates the scene's name (T-0357)
    std::size_t start = 0;      // where the covered text begins in the scene's file (UTF-8 bytes)
    std::size_t length = 0;     // how much of it the range covers (UTF-8 bytes)
    std::size_t sceneLength = 0;// the scene's whole text (UTF-8 bytes): the entry is the whole scene when start 0, length = this
    std::string text;           // the covered text, exactly as stored in the file
    // ✅ [SP-191] Q3b (user, 2026-10-21): what an undo needs to bring a REMOVED scene or chapter back exactly. Filled by the
    // core when it removes one (a cut, a relabel that empties a chapter); empty in a Range the editor builds. A paste that
    // keeps the entry's ID (the manuscript no longer has it) writes these back; a fresh ID ignores them.
    std::string sceneMeta;      // the scene's whole sidecar (`scrivi.scene.v1`, serialized), as it was on disk
    std::string chapterMeta;    // on the entry that OPENS a chapter: that chapter's sidecar (serialized), as it was

    [[nodiscard]] bool coversWholeScene() const { return start == 0 && length == sceneLength; }
};

// ✅ [SP-188] Q3 (user, 2026-10-22): a cut BALANCES the join — the survivor's head and tail, joined directly, can leave an
// emphasis span open (`**Alpha be` + `Gamma.`). The cut writes the balanced text and records, on the Range it returns, how to
// turn it back into the plain join: `balanced` at `byte` of the survivor became `plain`. The paste that undoes the cut puts
// `plain` back first, so cut ∘ undo is exact. ⚠️ A Range with a join is an undo record: it pastes only at its survivor, and
// a platform drops the join before the Range goes on the clipboard or into a buffer. A cut IGNORES a join it is given (its
// redo recomputes the same one).
struct RangeJoin {
    SceneID     sceneID;   // the survivor
    std::size_t byte = 0;  // where `balanced` begins in the survivor's text as the cut wrote it (UTF-8 bytes)
    std::string balanced;  // what the cut wrote there
    std::string plain;     // what the plain join had there
};

class Range {
public:
    using Entries = std::list<RangeEntry>;

    [[nodiscard]] bool empty() const { return entries_.empty(); }
    [[nodiscard]] std::size_t size() const { return entries_.size(); }
    [[nodiscard]] const Entries& entries() const { return entries_; }
    [[nodiscard]] Entries& entries() { return entries_; }
    void append(RangeEntry entry) { entries_.push_back(std::move(entry)); }
    [[nodiscard]] const std::optional<RangeJoin>& join() const { return join_; }
    void setJoin(std::optional<RangeJoin> join) { join_ = std::move(join); }

    // True when the entry after `it` begins a new chapter (its chapterID differs).
    [[nodiscard]] static bool opensChapter(Entries::const_iterator previous, Entries::const_iterator it) {
        return previous->chapterID.value != it->chapterID.value;
    }

    // `scrivi.range.v1`: { "schema": "scrivi.range.v1", "scenes": [ entry, … ] } — the array's order IS the links.
    // ✅ `sceneMeta` / `chapterMeta` are optional STRINGS (serialized JSON), so a platform's codec carries them through
    // untouched. ✅ An empty `sceneID` / `chapterID` is allowed (Q1): a paste mints a fresh one.
    // ✅ [SP-188] Q3: an optional top-level `"join": {sceneID, byte, balanced, plain}` (`RangeJoin`).
    [[nodiscard]] util::JsonDoc toJson() const;
    [[nodiscard]] static Result<Range> fromJson(const util::JsonDoc& doc);
    [[nodiscard]] std::string toJsonString() const;
    [[nodiscard]] static Result<Range> fromJsonString(std::string_view json);

    // ✅ Converts with `scrivi.fragment.v1` (design §4.2), so copy buffers and history nodes written before EP-053 keep
    // working: the TEXT, the boundaries and the titles survive both ways. ⚠️ A fragment names no scenes or chapters: from one,
    // each entry gets a SYNTHETIC sceneID and a piece that opens a chapter a synthetic chapterID ("~fragment-…") — only ever
    // a marker that the chapter differs; a paste mints real IDs. ⚠️ A fragment does not say where in its scene a piece
    // began, so a Range made from one covers its texts whole and its `partial` flags are not carried back (no paste reads
    // them).
    [[nodiscard]] Fragment toFragment() const;
    [[nodiscard]] static Range fromFragment(const Fragment& fragment);

private:
    Entries entries_;
    std::optional<RangeJoin> join_;
};

}  // namespace scrivi::manuscript

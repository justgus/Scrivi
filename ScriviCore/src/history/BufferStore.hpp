#pragma once

// BufferStore — durable persistence for the multiple copy buffers (EP-019, SP-056,
// T-0213). Peer to HistoryStore, but far simpler: no log, no tree, no live registry.
//
// Owns one file for one project:
//   history/buffers.json   scrivi.buffers.v1 — a flat array of numbered slots
//
// The copy buffers are vim/emacs-register-style named slots (v1: IDs "1"-"9") that
// the writer loads text into and pastes from at multiple locations, WITHOUT touching
// the system pasteboard (design §9). Persistence lives in C++ (this store); the Swift
// layer holds an in-memory mirror for display. Each C ABI call is a stateless
// read-modify-write (load → mutate → atomic write) — there is no open/close lifecycle
// and no in-memory state to keep in sync, so buffers survive quit/relaunch trivially.
//
// Schema (Appendix A.3):
//   { "schema":"scrivi.buffers.v1",
//     "buffers":[ { "bufferID":"1", "label":null, "text":"…", "updatedAt":"…" }, … ] }
// - bufferID is "1"-"9" (string, to allow named registers later).
// - label is reserved (null in v1).
// - EMPTY SLOTS ARE OMITTED — clearing a slot removes its entry.
// - Written atomically (util::atomicWriteTextFile).
//
// Design: docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md §9, Appendix A.3.

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace scrivi::history {

// One loaded copy-buffer slot.
struct BufferSlot {
    std::string bufferID;   // "1"-"9"
    std::string text;
    std::string updatedAt;  // ISO-8601 of the last load into this slot
    // Optional structured fragment (a serialized scrivi.fragment.v1 object). Present only
    // for cross-boundary copies/cuts (T-0355 / AC4); empty for plain-text slots (SP-056).
    // Additive under scrivi.buffers.v1 (T4=A): old files simply have no "fragment" key.
    std::string fragment;
};

class BufferStore {
public:
    // `historyDir` is <projectRoot>/history. `fs`/`clock` are borrowed (owned by the
    // ABI singleton). Touches disk only on the call it services.
    BufferStore(AbsolutePath historyDir, FileSystem* fs, Clock* clock);

    // The v1 slot-ID space. A bufferID outside "1".."9" is rejected (invalidArgument).
    static bool isValidBufferID(const std::string& bufferID);

    // Load `text` into slot `bufferID` (create-or-replace), stamping updatedAt = now.
    // `fragment` is an optional serialized scrivi.fragment.v1 (empty = plain-text slot).
    // Persists atomically. Returns the slot's updatedAt on success.
    Result<std::string> load(const std::string& bufferID, const std::string& text,
                             const std::string& fragment,
                             const std::string& nowTimestamp);

    // Read one slot. Returns nullopt (in an ok Result) if the slot is empty/absent.
    Result<std::optional<BufferSlot>> get(const std::string& bufferID);

    // Read all non-empty slots, ascending by bufferID.
    Result<std::vector<BufferSlot>> list();

    // Clear one slot (remove its entry). No-op (still ok) if the slot was already empty.
    // Persists atomically. Returns true if a slot was removed.
    Result<bool> clear(const std::string& bufferID);

private:
    AbsolutePath bufferPath() const;
    // Read + parse buffers.json into a slot list (empty if the file is absent). A
    // corrupt/unparseable file is treated as empty rather than an error, so a damaged
    // buffers file never blocks the editor (the slots are a convenience, not content).
    Result<std::vector<BufferSlot>> readAll();
    // Serialize `slots` (sorted, empty ones already excluded) + atomic-write.
    Result<void> writeAll(std::vector<BufferSlot> slots);

    AbsolutePath historyDir_;
    FileSystem*  fs_    = nullptr;
    Clock*       clock_ = nullptr;
};

} // namespace scrivi::history

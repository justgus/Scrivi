# Active Sprint

_No active sprint. SP-056 ✅ closed 2026-07-27 (Human-approved) — see below. EP-019 is **held pending** its final
sprint SP-057 (AC2/AC7/AC8 verify + Epic close). The next work is the newly-opened **EP-029** `[Cross]`
(cross-boundary structured Cut/Copy/Paste) — sprints planned in `../Epics/Epic-active.md`; SP-085 to be activated
on your go-ahead._

---

## SP-056: [Apple] Multiple copy buffers ✅ CLOSED (2026-07-27, Human-approved)

**Status:** ✅ Closed — **AC6 Verified.** T-0213 + T-0214 both ✅ Verified live (2026-07-27). Delivered
vim/emacs-register-style multiple copy buffers. Archived summary below; the sprint record is retained here until
moved to `Closed/Sprint-SP-056.md`.

**Verified behaviour (user, 2026-07-27):** ⌘1–9 copy · ⌃1–9 paste · ⌥1–9 cut into slots 1–9 (buffer 0 = the
untouched system pasteboard); the floating Copy Buffers palette (View ▸ Show Buffers / ⌥⌘B) with a
modifier-sensitive per-row action button (copy/paste/cut icons follow the held modifier) + clear (✕); buffers
persist across relaunch and **per project** (palette reloads to the front project on window/tab switch);
Edit-menu Copy/Paste/Cut To Buffer submenus; Scene/Chapter menu items (New/Merge) for mouse users. Cut-into-buffer
records a `cut` history event **tagged with the bufferID** (backend C ABI + history-node schema extended,
re-verified: ctest + interop green).

> **Gap surfaced during verify (2026-07-27) → new Epic.** Cross-boundary ⌘C/⌘X/⌘V and buffer copy/cut across
> **scene/chapter boundaries** do not work as a writer expects (the manuscript is one `NSTextStorage` with
> divider attachments + non-editable heading runs; a spanning selection can't be cleanly copied, and cut is
> blocked by the heading guard). User decision: treat the manuscript as one monolithic document for
> copy/cut/paste — **structured** buffers (scene/chapter markers), cut-that-merges, paste-that-splits. This is
> explicitly **out of EP-019 scope** (structural editing) and is now **EP-029** `[Cross]`. See
> `../Epics/Epic-active.md`.

**~~Status:~~** ~~🟡 Active~~
**Epic:** EP-019 `[Apple]` Custom Undo/Redo History & Multiple Copy Buffers (resumed 2026-07-24; the 6th of 7
sprints — the undo/redo engine + AC1/AC3/AC4/AC5 are delivered & verified, this sprint adds copy buffers → AC6)
**Goal:** Add **vim/emacs-register-style multiple copy buffers** to the macOS editor. The writer loads text into
numbered slots (`1`–`9`) and pastes from any slot at multiple locations — fast repetitive multi-string
replacement without round-tripping through the single system pasteboard (design §9.a CONOPS: the "Kazd'ul" /
"Kazda'la" scan-and-replace workflow). Buffers are **per-project + persistent** (ScriviCore-owned
`history/buffers.json`, `scrivi.buffers.v1`); the **system pasteboard is never clobbered**; **paste-from-buffer
is an ordinary history event** (undo works with zero special cases). Delivers **EP-019 AC6**.
**Start Date:** 2026-07-24
**Target Close Date:** TBD (est. 2–3 days)
**Capacity:** ~10–12 hours

> **Why now (user decision 2026-07-24):** un-defer EP-019 and finish copy buffers on **Apple first**, so the
> locked Apple implementation is the reference the Linux side (EP-026) mirrors — rather than designing the
> feature twice in parallel.

---

### Design references (source of truth)

- `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` — **§9 Multiple Copy Buffers** (9.a CONOPS,
  9.b model, 9.c presentation), **Trade T3** (copy-into-buffer is **not** a history event — no text change;
  cut-into-buffer **is**, `kind:"cut"` + `bufferID`; paste-from-buffer is an ordinary `paste` event), **Trade
  T4** (presentation = **B + D + A**: keyboard HUD fast path + toggleable palette + Edit-menu items), and
  **Appendix A.3** — the `history/buffers.json` (`scrivi.buffers.v1`) schema:
  `{ "schema":"scrivi.buffers.v1", "buffers":[ { "bufferID":"1", "label":null, "text":"…", "updatedAt":"…" } ] }`
  (IDs `"1"`–`"9"`; `label` reserved/null in v1; empty slots omitted; atomic write).
- **Peer pattern (already built, EP-019 SP-052):** `ScriviCore/src/history/HistoryService.{hpp,cpp}` +
  `HistoryStore.{hpp,cpp}`, the `scrivi_history_*` C ABI (`scrivi.h` 327–407), and `ScriviEngine.swift` history
  wrappers + `ScriviInteropTests`. The buffer store/ABI/wrappers mirror this shape. `scrivi.h` already **reserves
  `scrivi_buffers_*` for SP-056** (comment at line 316–317).

### Confirmed at planning

- **Backend is greenfield but small** — no buffers scaffolding exists yet (`grep` found none). T-0213 adds a
  `BufferStore` (peer to `HistoryStore`), the `scrivi_buffers_*` C ABI, and Swift engine wrappers. This is the
  only backend work; the history engine is untouched.
- **`scrivi.h` grows** by the `scrivi_buffers_*` family (an anticipated, reserved addition — not a boundary
  change). Every new endpoint is documented in the header.
- **History integration is the app's job** — the backend just stores buffer slots; whether a copy/cut/paste
  becomes a history event is decided in the editor (T-0214) using the existing `scrivi_history_record_event`
  path (Trade T3).

---

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0213 | `[ScriviCore]`+`[Apple]` **Copy-buffer store + C ABI + engine wrappers** — a `BufferStore` (peer to `HistoryStore`) owns `history/buffers.json` (`scrivi.buffers.v1`, Appendix A.3: 9 slots `"1"`–`"9"`, `text` + `updatedAt`, empty slots omitted, atomic write). New C ABI `scrivi_buffers_load/_get/_list/_clear` (JSON-over-string envelopes). `ScriviEngine.swift` wrappers + `ScriviInteropTests`. | High | ✅ **Verified (2026-07-27)** — ctest 325/325 (8 new BuffersCApi) + macOS interop GREEN; buffer round-trip + persistence verified live via T-0214. |
| T-0214 | `[Apple]` **Buffer UX — palette + Edit-menu items + chords + history integration** — **DELIVERED as three explicit chords** (revised from the context-sensitive design, user 2026-07-25/27): **⌘1–9 copy · ⌃1–9 paste (replaces selection) · ⌥1–9 cut** into slots 1–9; buffer 0 = the untouched system pasteboard (⌘C/⌘V). Floating **Copy Buffers palette** (View ▸ Show Buffers / ⌥⌘B, app-global, follows the frontmost project) with a **modifier-sensitive per-row action button** (copy/paste/cut icons follow the held ⌃/⌥) + clear (✕). **Edit-menu** Copy/Paste/Cut To Buffer submenus + **Scene/Chapter** menu items (New/Merge, mouse alternatives to ⌘↩/⌘⇧↩/⌘⌫/⌘⇧⌫). **History (Trade T3):** copy = no event; paste = ordinary `paste` event; cut = `cut` event **tagged with bufferID** (backend C ABI `scrivi_history_record_event` + history-node schema extended to persist the tag; +2 ctest, +1 interop). System pasteboard untouched; buffers persist across relaunch + per project. Closes **AC6**. | High | ✅ **Verified (2026-07-27)** — all chords, palette (incl. project-switch reload), menus, cut-tag, persistence verified live by the user. Findings fixed en route: palette-paste focus bug (restore first responder), ✕ tooltip (was dropped under `.disabled`), context-sensitive→explicit chord redesign, ⇧⌘ screenshot-collision avoided (paste moved to ⌃), standard Cut/Copy/Paste icons. |

### Assigned Issues

_None. New defects found during implementation/verify are filed against SP-056._

### Verification (2026-07-24)

**T-0213 (backend + wrappers) — build + test GREEN:**
- ✅ **ScriviCore `ctest`: 325/325** (8 new `BuffersCApi` tests — load→get round-trip, unset-slot present=false,
  load replaces, list ascending + count, clear + no-op-when-empty, persistence across a fresh call, out-of-range
  bufferID rejected, corrupt `buffers.json` treated as empty).
- ✅ **macOS app `xcodebuild build` — BUILD SUCCEEDED** (buffer C ABI compiled into `libScriviCore.a` via the
  CMake build phase; the `scrivi_buffers_*` decls reach Swift through the `ScriviCore` modulemap — no header copy,
  no pbxproj change since no new app-target files were added).
- ✅ **macOS interop `xcodebuild test` — TEST SUCCEEDED, 42/42** (6 new buffer wrapper tests: load→get,
  unset-slot, list-order, clear, persist-across-fresh-call, invalid-ID throws).
- **New files:** `ScriviCore/src/history/BufferStore.{hpp,cpp}` (CMake) + `ScriviCore/tests/integration/
  BuffersCApiTests.cpp` (CMake tests). `scrivi.h` gained the `scrivi_buffers_*` family (documented). Edited
  (already in pbxproj): `ScriviEngine.swift` (wrappers + Result structs + visionOS stubs),
  `ScriviInteropTests.swift`. **No new app-target files → no pbxproj change.**
- ⏳ **T-0214 (buffer UX) is next** — the keyboard HUD + palette + Edit-menu + history integration; that closes
  AC6 and is where the user-facing verify happens.

---

### Acceptance Criteria (EP-019 AC6) — ✅ ALL MET (Verified 2026-07-27)

- [x] **≥ 2 buffers** can be loaded and pasted at multiple locations (design §9.a CONOPS).
- [x] Each **paste-from-buffer is exactly one undo step** (an ordinary `paste` history event; Trade T3).
- [x] **Copy-into-buffer records no history event** (no text change); **cut-into-buffer records a `cut` event**
  (bufferID-tagged, persisted).
- [x] The **system pasteboard is never clobbered** — buffer 0 = the pasteboard; ⌘C/⌘V unchanged.
- [x] Buffers **persist across quit/relaunch** (stored in `history/buffers.json`) **and per project** (palette
  reloads to the frontmost project on window/tab switch).
- [x] **Keyboard fast path** — delivered as three explicit chords (⌘1–9 copy / ⌃1–9 paste / ⌥1–9 cut) rather than
  the originally-designed ⌥⌘C/⌥⌘V HUD. Design refinement approved by the user 2026-07-25/27 (the single
  context-sensitive chord was ambiguous and couldn't paste-over-selection; no HUD needed since the chord IS the
  action). The HUD may return in the future inspector panel.
- [x] The **buffers palette** shows the 9 slots with previews + `updatedAt`; a modifier-sensitive per-row button
  (copy/paste/cut) + clear.
- [x] **Edit-menu items** (Copy / Paste / Cut To Buffer submenus) present and functional; Scene/Chapter New/Merge
  menu items added for mouse users.
- [x] **No regression:** undo/redo (AC1/AC3/AC4/AC5), auto-save, scene navigation, structure ops all unaffected;
  backend `ctest` (327) + `ScriviInteropTests` (43) green.

### Scope & guardrails

- **Source of truth is the design doc** (§9 + Trades T3/T4 + Appendix A.3), approved 2026-07-06. Any deviation
  is surfaced + reconciled before implementation (per CLAUDE.md).
- **Backend owns persistence** (`BufferStore` + `buffers.json`), Swift owns UI + history-integration decisions.
  The system pasteboard is a parallel mechanism — never touched.
- **Apple-only sprint.** macOS editor + ScriviCore. The parallel **Linux** copy-buffer UI is **EP-026**, not
  this sprint — this sprint locks the Apple reference first (user decision).
- **pbxproj rule:** every new Swift source (and any C++ shim compiled into the app) is added to
  `Scrivi.xcodeproj/project.pbxproj` in the same step, before the build. ScriviCore `.cpp`/`.hpp` go in **CMake**
  (the app links prebuilt `libScriviCore.a`), NOT pbxproj (memory: `project_pbxproj_scrivicore_scope`).
- **Build/test = Apple toolchain:** `xcodebuild -scheme ScriviApp -destination 'platform=macOS' build|test`
  (deployment target 27.0) + `ctest` for the ScriviCore side. (Not Docker/VNC — that's the Linux world we just
  left.)

---

*Last Updated: 2026-07-24 (**SP-056 planned + activated** — EP-019 `[Apple]` resumed from the backlog (user
request: lock copy buffers on Apple before Linux). Delivers **AC6**: vim/emacs-register-style multiple copy
buffers — 9 per-project persistent slots (`history/buffers.json`, `scrivi.buffers.v1`), keyboard HUD + palette +
Edit-menu UX (Trade T4 = B+D+A), copy≠event / cut=event / paste=ordinary-event (Trade T3), system pasteboard
untouched. Tasks **T-0213** (`BufferStore` + `scrivi_buffers_*` C ABI + `ScriviEngine` wrappers + interop tests)
and **T-0214** (keyboard HUD + palette + Edit-menu items + history integration). Backend is greenfield but small
(peer to the built `HistoryStore`); `scrivi.h` gains the reserved `scrivi_buffers_*` family; pbxproj updated for
new Swift files. Apple-only (the Linux copy-buffer UI is EP-026). SP-057 (AC2/AC7/AC8 verify + Epic close)
follows. **SP-084 ✅ closed 2026-07-24** (EP-025 AC6b; that closed EP-025). Next available Sprint **SP-085**; next
available Task **T-0350** (T-0213/0214 pre-exist from EP-019 planning).)*

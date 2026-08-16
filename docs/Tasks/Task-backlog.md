# Task Backlog

Tasks listed here are documented and ready for Sprint assignment.

New, unstarted tasks are listed as summary rows. Tasks that have been implemented but returned to the backlog before verification retain their full detail below the index table so no documentation is lost.

---

## Index

| Task | Title | Epic | Status |
| ---- | ----- | ---- | ------ |
| T-0218 | `[Linux]` Qt6/QML CMake skeleton — links `libScriviCore.a` + `scrivi.h` | EP-020 (SP-058) | 🟢 Active → `Task-active.md` |
| T-0219 | `[Linux]` `ScriviBridge` — QML ↔ C ABI marshalling + `scrivi_free` discipline | EP-020 (SP-058) | 🟢 Active → `Task-active.md` |
| T-0220 | `[Linux]` Hello-ScriviCore QML window — `scrivi_ensure_local_identity` round-trip | EP-020 (SP-058) | 🟢 Active → `Task-active.md` |
| T-0221 | `[Linux]` Docker (Qt6 + CMake) + Xvfb + VNC harness | EP-020 (SP-058) | 🟢 Active → `Task-active.md` |
| T-0222 | `[Linux]` CI job — build the Qt/QML app on relevant commits | EP-020 (SP-058) | 🟢 Active → `Task-active.md` |
| T-0118 | Scroll bar fidelity — per-scene character-ratio thumb position and size | EP-011 | 🔵 Backlog |
| T-0175 | Spotlight integration (umbrella) — **superseded by EP-017** | EP-017 | ⚪ Superseded |
| T-0184 | Deep-link: result continuation opens project & selects item | EP-017 (SP-045) | ✅ Verified → `Verified/Task-verified-0184.md` |
| T-0185 | New Spotlight importer app-extension target (Xcode-authored) | EP-017 (SP-046) | ⚪ Descoped (I-0057 — `CSImportExtension` non-functional on macOS) |
| T-0186 | Link ScriviCore into the extension (Option A build graph) | EP-017 (SP-046) | ⚪ Descoped (I-0057) |
| T-0187 | Importer emits Spotlight attributes from facade JSON | EP-017 (SP-046) | ⚪ Descoped (I-0057) |
| T-0188 | Importer handles the `com.caposoft.scrivi.project` UTI; sandbox pass | EP-017 (SP-046) | ⚪ Descoped (I-0057) |
| T-0189 | End-to-end verification (donor search, deep-link tap, donations succeed) | EP-017 (SP-047) | ✅ Verified → `Verified/Task-verified-0189.md` |
| T-0190 | iOS/iPadOS/visionOS Spotlight assessment (implement or defer) + EP-017 verification | EP-017 (SP-047) | ✅ Verified (DEFERRED) → `Verified/Task-verified-0190.md` |
| T-0197 | Enable Core Spotlight donation on iOS/iPadOS (+ iOS deep-link/bookmark consumer); visionOS when backend links | EP-017 (deferred from T-0190) | 🔵 Backlog |
| T-0191 | V1 spike: confirm `WindowGroup(for:)` de-dup/focus-by-value on macOS 26 (throwaway; gates R3) | EP-018 (SP-048) | ✅ Done (2026-06-24) |
| T-0192 | Extract `ProjectSession`; move per-project state + methods off `AppEnvironment` (behavior-preserving) | EP-018 (SP-048) | ✅ Verified → `Verified/Task-verified-0192.md` |
| T-0193 | Introduce `OpenProjectRegistry` in `AppEnvironment` (projectID → session) | EP-018 (SP-048) | ✅ Verified → `Verified/Task-verified-0193.md` |
| T-0194 | Per-window project model — AppKit NSWindow per project (R1/R2/R3) + Welcome; single-instance; File menu | EP-018 (SP-049) | ✅ Verified → `Verified/Task-verified-0194.md` |
| T-0195 | Session manifest persistence + launch restore of all previously-open windows (R4) | EP-018 (SP-049) | ✅ Verified → `Verified/Task-verified-0195.md` |
| T-0196 | Rewrite deep-link handler on new model + scene-`ID` fix (R5); open-flow cross-ref; EP-018 verification | EP-018 (SP-050) | ✅ Verified — archived to Verified/Task-verified-0196.md |
| T-0198 | Undo/redo + copy buffers design doc & trade studies | EP-019 (SP-051) | ✅ Verified → `Verified/Task-verified-0198.md` |
| T-0199 | Spike: ⌘Z/⇧⌘Z + Edit-menu routing mechanism | EP-019 (SP-051) | ✅ Done (2026-07-06) |
| T-0200 | `scrivi.history.v1` / `scrivi.buffers.v1` schema spec + repair-matrix row | EP-019 (SP-051) | ✅ Verified → `Verified/Task-verified-0200.md` |
| T-0201 | `HistoryService` core (record/undo/redo, snapshot-diff, sessions) + unit tests | EP-019 (SP-052) | ✅ Verified → `Verified/Task-verified-0201-0203.md` |
| T-0202 | C ABI: `scrivi_history_*` open/record/undo/redo/close | EP-019 (SP-052) | ✅ Verified → `Verified/Task-verified-0201-0203.md` |
| T-0203 | `ScriviEngine.swift` history wrappers + interop tests | EP-019 (SP-052) | ✅ Verified → `Verified/Task-verified-0201-0203.md` |
| T-0204 | `HistoryCapture` + commit-trigger wiring in the editor | EP-019 (SP-053) | ✅ Verified → `Verified/Task-verified-0204-0206.md` |
| T-0205 | Undo/redo apply path + `allowsUndo=false` + ⌘Z routing | EP-019 (SP-053) | ✅ Verified → `Verified/Task-verified-0204-0206.md` |
| T-0206 | Barriers on structural operations | EP-019 (SP-053) | ✅ Verified → `Verified/Task-verified-0204-0206.md` |
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | EP-019 (SP-054) | ✅ Verified → `Verified/Task-verified-0207-0209.md` |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | EP-019 (SP-054) | ✅ Verified → `Verified/Task-verified-0207-0209.md` |
| T-0209 | Session-boundary warning popup | EP-019 (SP-054) | ✅ Verified → `Verified/Task-verified-0207-0209.md` |
| T-0210 | Tree ops: branching, primary-child, `select_branch`, auto-purge | EP-019 (SP-055) | ✅ Verified → `Verified/Task-verified-0210-0212.md` |
| T-0211 | Inline fork popover (Trade T2 core interaction) | EP-019 (SP-055) | ✅ Verified → `Verified/Task-verified-0210-0212.md` |
| T-0212 | Stale-branch detection + user-confirmed purge | EP-019 (SP-055) | ✅ Verified → `Verified/Task-verified-0210-0212.md` |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | EP-019 (SP-056) | ✅ **Verified (2026-07-27)** → `../Sprints/Closed/Sprint-SP-056.md` |
| T-0214 | Buffer UX: palette + Edit/Scene/Chapter menu items + ⌘/⌃/⌥1–9 chords; paste/cut history integration | EP-019 (SP-056) | ✅ **Verified (2026-07-27)** → `../Sprints/Closed/Sprint-SP-056.md` |
| T-0215 | History panel (Trade T2 management surface) | EP-019 (SP-057) | ⚪ **Superseded → T-0366** (ships as the `history` card, EP-030 SP-092) |
| T-0359 | `InspectorCard` protocol + registry + `CardContext` | EP-030 (SP-090) | ✅ **Verified (2026-08-05)** |
| T-0360 | `inspector-layout.json` schema + load/save + unknown-`typeID` skip | EP-030 (SP-090) | ✅ **Verified (2026-08-05)** |
| T-0361 | Bottom tabs + persisted `selectedTab` + menu hide/show; replace `SceneInspectorView` | EP-030 (SP-090) | ✅ **Verified (2026-08-05)** |
| T-0362 | Card stack: add/remove/reorder, collapse, per-stack sort, "apply to all scenes" | EP-030 (SP-090) | ✅ **Verified (2026-08-05)** |
| T-0363 | `tags` + `todo` cards | EP-030 (SP-091) | ✅ **Verified (2026-08-05)** |
| T-0364 | `outline` card | EP-030 (SP-091) | ✅ **Verified (2026-08-05)** |
| T-0365 | `sources` card + `source` object kind + `cites`/`documented-by` relation type | EP-031 (**split: `cites` type ✅ SP-096; `source` kind ✅ SP-098 as T-0406; card → SP-102**) | 🟡 **Two thirds done** — the `cites`/`documented-by` type (SP-096) and the `source` kind (SP-098 T-0406) are both ✅ Verified. **Only the aggregate `sources` card remains, moved SP-099 → SP-102 at SP-099 planning (R2).** Source→scene deferred to **EP-032** |
| T-0394 | `[ScriviCore]` `scrivi_history_get_tree` — windowed `{aroundNodeID?, maxNodes?}` | EP-030 (SP-092) | 🟠 **Implemented — Not Verified** |
| T-0395 | `[Apple]` `ScriviEngine`/`HistoryCapture` history-tree wrapper + interop | EP-030 (SP-092) | 🟠 **Implemented — Not Verified** |
| T-0366 | `history` card — windowed tree, branches, stale badges, purge (**supersedes T-0215**) | EP-030 (SP-092) | 🟠 **Implemented — Not Verified** |
| T-0367 | Properties tab — field-driven view | EP-030 (SP-092) | 🟠 **Implemented — Not Verified** |
| T-0368 | Card failure isolation + inline warning | EP-030 (SP-092) | 🟠 **Implemented — Not Verified** |
| T-0369 | EP-030 verification + Epic close prep | EP-030 (**SP-094**) | 🔵 Backlog |
| T-0396 | `[Apple]` **Typing-session coalescing** — autosave **defers** the commit (records nothing mid-session); entry seals at a real boundary or a **45 s** idle timer. ⚠️ **§4.d relaxed** (user-approved 2026-08-10): disk may lead history by ≤1 save while a session is open — bounded by `close()` + I-0104's head-hash barrier. Cursor-move, cut/paste, scene switch, terminators all **KEPT**; backspace does not commit. App-side only — `HistoryService` untouched. | EP-019 (SP-093) | 🟠 **Implemented — Not Verified (2026-08-10)** |
| T-0397 | `[Cross]` **Whitespace-kind labels in history** — new `whitespaceKind` tree field (`"newline:2"`) → `whitespaceLabel` (`"⏎ new paragraph"`); `preview` left untouched so the fork popover / stale-branch / purge consumers are unaffected | EP-019 (SP-093) | 🟠 **Implemented — Not Verified (2026-08-10)** |
| T-0398 | `[Cross]` **Distinguish added vs. deleted text in history rows** — `minus.circle` glyph + orange tint + "Deleted …" label prefix; `removedLength` carried in the tree payload (**shipped once**, shared with I-0106) | EP-019 (SP-093) | ✅ **Verified 2026-08-10 (user-confirmed)** |
| T-0400 | `[ScriviCore]` **History log-segment rotation** — `activeSegment_` is hard-fixed to `log-000001.jsonl` (`HistoryStore.hpp:114`); nothing rotates it, so a project accumulates one unbounded segment (a real project is already ~3.4 MB). Capacity/eviction bounds the **tree**, not the **log**. Reader already honours `activeLogSegment` from `state.json`, so rotation is **additive, no format change**. Documented as not-implemented in package-structure §16a | EP-019 (deferred) | 🟢 **Nice to have** (user ruling 2026-08-11) — not a blocker; no sprint assigned |
| T-0399 | `[Apple]` **Card soft-failure isolation** — framework backstop in `CardBodyBoundary` + throwing `makeContent` (default forwards) + failing-card test fixture. **EP-030 AC12 rescoped to soft failures** 2026-08-11 (user-approved): SwiftUI cannot catch a trapping view body, so hard failures are out of scope. **Not live-verifiable** — no UI path makes a card fail | EP-030 (SP-101) | 🟠 **Implemented — Not Verified (2026-08-11)** |
| T-0370 | `ObjectKind` additions (8) + `objectKindSubdir` + schema table; retire `timeline`; world-scoped kinds gated | EP-031 (**SP-095**) | ✅ **Verified (2026-08-12)** |
| T-0371 | `WorldObjectFields`: `subtitle`, `image`, `worldID` | EP-031 (**SP-095**) | ✅ **Verified (2026-08-12)** |
| T-0372 | `objects/index.json` + `findByID` over index | EP-031 (**SP-095**) | ✅ **Verified (2026-08-12)** |
| T-0401 | Index rebuild + corruption coverage (missing / corrupt / stale) — Doc 1 AC2 | EP-031 (**SP-095**) | ✅ **Verified (2026-08-12)** |
| T-0402 | ⚠️ Endpoint-kind resolution via `ObjectIndex` — replaces §5.2's broken ID-prefix rule; amends Doc 1 | EP-031 (**SP-096**) | ✅ **Verified (2026-08-12)** |
| T-0373 | `relation-types.json` + `canonicalDirection` + `symmetric` | EP-031 (**SP-096**) | ✅ **Verified (2026-08-12)** |
| T-0374 | `relationships.jsonl` append-log + tombstones + torn-line recovery | EP-031 (**SP-096**) | ✅ **Verified (2026-08-12)** |
| T-0375 | Canonical normalization + duplicate rejection (asymmetric **and** symmetric) | EP-031 (**SP-096**) | ✅ **Verified (2026-08-12)** |
| T-0376 | Compaction at 30% / 1,000 tombstones | EP-031 (**SP-096**) | ✅ **Verified (2026-08-12)** |
| T-0405 | ⚠️ **I-0113** — `worldID` on `scrivi_create/open/delete_object` (breaking ABI widen) | EP-031 (**SP-098**) | ✅ **Verified (2026-08-12)** |
| T-0377 | Cascade-prune on object **and** scene delete + load-time repair | EP-031 (**SP-098**) | ✅ **Verified (2026-08-12)** |
| T-0378 | `scrivi_list_objects` / `scrivi_list_orphaned_objects` | EP-031 (**SP-098**) | ✅ **Verified (2026-08-12)** |
| T-0379 | `scrivi_promote_object` (item↔artifact) | EP-031 (**SP-098**) | ✅ **Verified (2026-08-12)** |
| T-0380 | ⚠️ Pending-vs-dangling distinction + frozen graph toward unavailable worlds | EP-031 (**SP-098**) | ✅ **Verified (2026-08-12)** |
| T-0406 | `source` object kind (T-0365 ScriviCore half) — **closes EP-031 AC1** | EP-031 (**SP-098**) | ✅ **Verified (2026-08-12)** |
| T-0403 | ⚠️ `FileSystem::createFileExclusive` — exclusive-create primitive Doc 3 §6.5 assumes but that does not exist | EP-031 (**SP-097**) | ✅ **Verified (2026-08-12)** |
| T-0381 | `.scrivworld` package + `world.json` + world index + `scrivi_create_world` | EP-031 (**SP-097**) | ✅ **Verified (2026-08-12)** |
| T-0382 | `binding.json` + `worldID`-verified resolution + relink | EP-031 (**SP-097**) | ✅ **Verified (2026-08-12)** |
| T-0383 | Lock→write→unlock + heartbeat + stale-lock recovery | EP-031 (**SP-097**) | ✅ **Verified (2026-08-12)** |
| T-0384 | Epoch chain (world/timeline/binding) + resolve endpoint | EP-031 (**SP-097**) | ✅ **Verified (2026-08-12)** |
| T-0385 | Cached world index → named pending entries; **world-scoped kinds become creatable** (closes AC1 gated half + AC3 faction clause) | EP-031 (**SP-097**) | ✅ **Verified (2026-08-12)** |
| T-0404 | `rule` relocation to world scope + Package Structure §11 correction (deferred from SP-095) | EP-031 (**SP-097**) | ✅ **Verified (2026-08-12)** |
| T-0412 | **Confirm the I-0121 fix on x86-64** — ScriviCore CI green on **both** matrix legs for the first time since 2026-07-30. ⚠️ The code change is already applied; this is the verification the developer's arm64 machine cannot provide | EP-031 (**SP-106**) | 🔵 Backlog |
| T-0413 | ⚠️ **Sanitizer CI leg** — `-fsanitize=undefined` (+ `address`?) with `-fno-sanitize-recover=all`, so UB fails deterministically instead of by instruction-set accident. **May change the test configuration**; root `CMakeLists.txt` sets no sanitizer/`-Werror`/build-type today | EP-031 (**SP-106**) | 🔵 Backlog |
| T-0414 | **macOS platform coverage** — Apple has **zero** platform-specific tests vs Linux's 7; `platformDefault`'s Apple branch is untested (the shared test only asserts non-empty + ends in `Scrivi`) | EP-031 (**SP-106**) | 🔵 Backlog |
| T-0389 | Pending presentation + Worlds menu + warning view (**⚠️ includes the unbuilt `offline`/`unmounted` status refinement — AC24**) | EP-031 (**SP-102** — split from SP-099, R2) | 🔵 Backlog |
| T-0390 | External Change Repair Matrix — world-package conditions | EP-031 (**SP-100**) | 🔵 Backlog |
| T-0391 | EP-031 verification + Epic close prep | EP-031 (**SP-100**) | 🔵 Backlog |
| T-0216 | Perf/integration fixtures (100k events, 500 KB scene); gitignore migration | EP-019 (SP-057) | ⚪ **Closed — OBE (2026-08-05, user-directed)** |
| T-0217 | Doc updates + EP-019 acceptance-criteria verification | EP-019 (**SP-094**, ⬅ SP-057 superseded) | ✅ **Verified 2026-08-11** — completed in SP-094 and closed with EP-019; record of truth [`Sprint-SP-094.md`](../Sprints/Closed/Sprint-SP-094.md) |
| T-0234 | `[Linux]` **Shell flip** — `main.cpp` → `QApplication`+`QMainWindow` host, landing QML re-hosted via `QQuickWidget`; keep context props + `QFileDialog` picker + Quit; re-verify EP-021 loop over VNC | EP-022 (SP-061) | ✅ Verified 2026-07-14 |
| T-0235 | `[Linux]` **Editor shell + read-only continuous viewport** — one `QPlainTextEdit`/`QTextDocument` (read-only, undo disabled), all scene bodies via `openProject` + `scrivi_open_scene` loop, scene-boundary markers + per-scene start offsets (`sceneStartMap` seed) | EP-022 (SP-061) | ✅ Verified 2026-07-14 |
| T-0236 | `[Linux]` **Scene navigator** — ordered list + chapter grouping + live titles; click selects + scrolls viewport; display/select only (structure editing = EP-023) | EP-022 (SP-061) | ✅ Verified 2026-07-14 |
| T-0237 | `[Linux]` **Verify AC1/AC2 + headless scene-load smoke** — multi-scene fixture asserts all bodies loaded, wired to CI; VNC click-through | EP-022 (SP-061) | ✅ Verified 2026-07-14 |
| T-0238 | `[Linux]` **Editable viewport + dirty tracking** — `setReadOnly(false)`, edits mapped to owning scene via offset map, per-scene dirty flags, non-editable/non-deletable boundary markers | EP-022 (SP-062) | ✅ Verified 2026-07-14 |
| T-0239 | `[Linux]` **Per-scene auto-save** — `scrivi_save_scene` on ~1.5s debounce + scene-switch + close/app-quit (VNC foreground-quit path); real `.md` on disk | EP-022 (SP-062) | ✅ Verified 2026-07-14 |
| T-0240 | `[Linux]` **⌘↩ create scene** in-editor (Linux: **Ctrl+Return**) — save current, create-scene, insert boundary, caret into new segment, update offset map + navigator | EP-022 (SP-062) | ✅ Verified 2026-07-14 |
| T-0241 | `[Linux]` **⌘⇧↩ create chapter** in-editor (Linux: **Ctrl+Shift+Return**) — save current, create-chapter, insert boundary, caret into new chapter's first scene, update offset map + navigator | EP-022 (SP-062) | ✅ Verified 2026-07-14 |
| T-0242 | `[Linux]` **Verify AC3 + headless save smoke** — edit→`save_scene`→reopen asserts new bytes; VNC type/switch/⌘↩/⌘⇧↩/quit-save | EP-022 (SP-062) | ✅ Verified 2026-07-14 |
| T-0243 | `[Linux]` **Scroll → active-scene promotion** — visible region → active scene via offset map; boundary crossing promotes + saves departing scene | EP-022 (SP-063) | ✅ Verified 2026-07-15 |
| T-0244 | `[Linux]` **Navigator ↔ scroll sync** — highlight follows scroll; navigator click takes caret to the clicked scene's start (scrolls into view); no feedback loop | EP-022 (SP-063) | ✅ Verified 2026-07-15 |
| T-0245 | `[Linux]` **Verify AC1/AC4 (scroll)** — VNC scroll-through tracks active scene + highlight; departing scenes saved; navigator click lands caret at the clicked scene's start | EP-022 (SP-063) | ✅ Verified 2026-07-15 |
| T-0246 | `[Linux]` **Cursor placement + focus** — editor takes focus on open, sensible caret, correct caret across boundaries (no jump-to-start), enforce non-editable/non-deletable markers | EP-022 (SP-064) | 🟡 Implemented — Not Verified |
| T-0247 | `[Linux]` **Quit→reopen surface restore** — persist active scene + anchor/focus/scroll on close; apply `openProject` `restored{}` on reopen | EP-022 (SP-064) | 🔵 Backlog |
| T-0248 | `[Linux]` **EP-022 verification + close prep** — full write→save→switch→scroll→quit→reopen loop VNC-verified; `.md` on disk; AC1–AC7 checklist; restore smoke; draft completion summary for user close | EP-022 (SP-064) | 🔵 Backlog |
| T-0249 | `[Linux]` **Manuscript navigation gestures** — Page Forward / Page Backward + jump to absolute manuscript start / end. Gestures/keystrokes **undecided** (laptops & tablets lack extended keyboards — no PageUp/Down/Home/End assumed); pick touch-friendly bindings. Raised during SP-063 VNC verification; unscheduled (likely EP-022 follow-up or EP-026 menus/parity). | EP-022 (unscheduled) | 🔵 Backlog |
| T-0294 | `[Linux]` **`NavigatorTree` chapter drag** — chapter rows draggable (latch `kChapterIDRole` at `startDrag`, CopyAction-only per I-0067/I-0068), chapter-boundary-only drop resolution + insertion-line highlight, no-op guard; emit `chapterDropRequested(chapterID, afterChapterID)` | EP-023 (SP-073) | ✅ Verified → `Verified/Task-verified-0294-0297.md` |
| T-0295 | `[Linux]` **`EditorShell::onChapterDropped` + `SceneDocument::moveChapter`** — save dirty scene, bridge `reorderChapter`, contiguous chapter-block re-splice (bodies/caret/scroll preserved), navigator rebuild, **post-reslug path refresh** (reorder reslugs the chapter folder; envelope has no new path — strategy decided at SP-073 activation) | EP-023 (SP-073) | ✅ Verified → `Verified/Task-verified-0294-0297.md` |
| T-0296 | `[Linux]` **Headless chapter-reorder smoke** — reorder among ≥3 chapters (incl. move-to-front / move-to-last), reopen, assert order + scene bodies intact + no stale paths; wired to CI | EP-023 (SP-073) | ✅ Verified → `Verified/Task-verified-0294-0297.md` |
| T-0297 | `[Linux]` **Full EP-023 verify + Epic close prep** — AC6 create parity, AC7 full create→rename→reorder→delete→quit→reopen loop over VNC with on-disk checks, AC8 regression sweep (ctest both platforms, macOS build, EP-020–EP-022 flows), draft completion summary | EP-023 (SP-073) | ✅ Verified → `Verified/Task-verified-0294-0297.md` |

### ⚠️ Removed 2026-08-16 — seven rows that went stale when their Sprint closed

**T-0386, T-0387, T-0388, T-0407, T-0408** (SP-099) and **T-0409, T-0411** (SP-103) were removed from the
table above. All seven are ✅ **Verified 2026-08-15**; their record of truth is
[`Sprint-SP-099.md`](../Sprints/Closed/Sprint-SP-099.md) and
[`Sprint-SP-103.md`](../Sprints/Closed/Sprint-SP-103.md), and they are indexed in
[`Task-Documentation.md`](Task-Documentation.md).

This file was showing five of them as 🟠 *Implemented - Not Verified (2026-08-13)* and two as 🔵 *Backlog* —
i.e. **T-0409 and T-0411 appeared unstarted after they had shipped and been verified.** ⚠️ **This is the
failure mode `Task-Documentation.md`'s own rebuild header names** (*"`Task-backlog.md` rows were found to go
stale after their Sprint closed"*): the 2026-08-15 rebuild fixed the index but left these rows behind.
**A Task's row must leave this file when its Sprint closes** — the Sprint archive is the record, and a
backlog row that outlives its Sprint can only ever contradict it.

Also corrected in the same pass: **T-0217** was listed 🔵 Backlog under the superseded SP-057, though it was
completed in **SP-094** and closed with EP-019 on 2026-08-11.

---

## Full Detail — Implemented Tasks Returned to Backlog

---

## T-0175: Spotlight Integration (umbrella) — Superseded by EP-017

**Status:** ⚪ Superseded
**Date Requested:** 2026-06-23 | **Superseded:** 2026-06-23
**Epic:** EP-017

Originally logged as a single design-gated task. Per user direction it was expanded into a full
Epic — **EP-017: Spotlight Search Integration** — and decomposed into Sprints SP-044–SP-047 and
Tasks T-0176–T-0190. See `docs/Epics/Epic-backlog.md` (EP-017) and
`docs/Scrivi_Spotlight_Integration_Design_v0_1.md`. This entry is retained as a pointer; no work
is tracked under T-0175.

---

## EP-017 Spotlight Tasks (T-0176 – T-0190)

Design reference for every task: `docs/Scrivi_Spotlight_Integration_Design_v0_1.md`. Sprint-level
acceptance criteria live in `docs/Sprints/Sprint-backlog.md`; Epic-level ACs in
`docs/Epics/Epic-backlog.md` (EP-017).

### SP-044 — Design sign-off & ScriviCore indexing facade — ✅ Closed & Verified (2026-06-23)

T-0176–T-0179 verified and archived. Full detail:
[T-0176](Verified/Task-verified-0176.md) · [T-0177](Verified/Task-verified-0177.md) ·
[T-0178](Verified/Task-verified-0178.md) · [T-0179](Verified/Task-verified-0179.md).
Sprint retrospective: `docs/Sprints/Closed/Sprint-SP-044.md`.

### SP-045 — Layer 1: in-app Core Spotlight donations & deep-link

**T-0180 — Entitlements + project config.** ✅ Verified & archived 2026-06-23 — enabled the App
Sandbox capability (the Core Spotlight prerequisite) + `scrivi://` URL scheme; verified by a clean
open→write→save→close cycle under the sandbox (no Console `deny`). Full detail:
[Task-verified-0180.md](Verified/Task-verified-0180.md). *Note:* whether the original
`CSInlineDonation` console errors are resolved is checked later at EP-017 AC9 / T-0189 (needs live
donations, T-0182).

**T-0181 — `ScriviEngine` indexable-records API.** ✅ Verified & archived 2026-06-23 — added
`ScriviEngine.extractSearchableText` + `SearchableContentResult`/`SearchableItemResult` decode types
(pure decode, no backend logic in Swift); proven by interop test #18 (`xcodebuild test` 22/22 green).
Also wired `ScriviInteropTests` into the `ScriviApp` scheme so the whole suite runs from CLI/CI, and
fixed two latent git tests broken under the sandboxed test host. Full detail:
[Task-verified-0181.md](Verified/Task-verified-0181.md).

**T-0182 — Donate / update / delete.** ✅ Verified & archived 2026-06-23 — `SpotlightDonor` donates
on open/save and deletes by domain on close, wired into `AppEnvironment`; the Swift donation path was
proven correct in the debugger (extract→24 records→donate). **Known environment limitation:** the OS
rejects the donations (`SetStoreUpdateService` -4099) on the dev-signed sandboxed build, so nothing
indexes yet — confirmed not our logic / not a debugger artifact / not a hard sandbox deny; deferred to
T-0189 to confirm on a provisioned/installed build (also where AC9 is judged). Full detail:
[Task-verified-0182.md](Verified/Task-verified-0182.md).

**T-0183 — Markdown→plain-text extraction.** ✅ Verified & archived 2026-06-23 — no new code needed:
the `util/MarkdownStrip` core + facade application shipped under T-0178 (per design §4b it lives in
the ScriviCore facade, shared by both layers; Swift does no Markdown handling). Covered by the
MarkdownStrip unit tests + interop test #18. Full detail:
[Task-verified-0183.md](Verified/Task-verified-0183.md).

**T-0184 — Deep-link continuation.** Each item carries a `scrivi://open?project=…&item=…` URL.
App handles `CSSearchableItemActionType` / URL scheme to open the project and select the item.
*Implemented 2026-06-23 (not verified):* `ScriviDeepLink` (URL parse, 4 unit tests) +
`ProjectBookmarkStore` (projectID→security-scoped bookmark — the bookmark work deferred from T-0180:
recorded on panel-open, resolved/refreshed/released around a deep-link open). `AppEnvironment.handleDeepLink`
(URL-scheme path: select-if-open, else resolve bookmark → reopen → select; clear message if no
bookmark) + `handleSpotlightItem` (best-effort continuation). `ScriviApp` wires `onOpenURL` +
`onContinueUserActivity(CSSearchableItemActionType)`; `ManuscriptEditorView` consumes
`pendingNavigationSceneID` into its existing `navigateToSceneID`. pbxproj updated (2 new files, all
3 app targets). macOS build clean; `xcodebuild test` 26/26.
**Known limits (inherent):** (1) cold-start resolves only projects opened before in this app (sandbox
bookmark requirement); (2) the Spotlight-tap continuation carries only the item `uniqueIdentifier`
(no projectID for scenes) and is unreliable on SwiftUI macOS, so `scene:` taps only select when the
project is already open — the `scrivi://` URL scheme is the complete route.
**Verification (overlaps T-0189):** end-to-end from a tapped Spotlight result needs donations to index
first (blocked by T-0182 env issue). URL-scheme path is testable now via
`open "scrivi://open?project=<id>&item=scene:<id>"` against a previously-opened project on a build that
receives URL events.

**T-0184 resumed & completed on the per-window model (2026-06-25 — Implemented, Not Verified).**
EP-018 unblocked this. The core deep link (open/focus the right window, select scene by `scene_…` ID)
was already verified via EP-018 / T-0196. The remaining Spotlight-result *continuation* path
(`onContinueUserActivity(CSSearchableItemActionType)`) was finished and hardened:
- `handleSpotlightItem` now accepts an optional `relatedURL` and **prefers the full donated deep link**
  (`SpotlightDonor` sets it as `relatedUniqueIdentifier`). Because that URL carries the **projectID**,
  a tapped `scene:` result can now open even a **closed** project (via its bookmark) — removing the old
  inherent limit where scene taps only worked when the project was already open. Falls back to the
  uid-only logic when no related URL is present. (`AppEnvironment.swift:344-385`)
- `ScriviApp` recovers that URL from `activity.contentAttributeSet?.relatedUniqueIdentifier` and passes
  it through. (`ScriviApp.swift:189-199`)
- No new files. macOS `ScriviApp` build clean; interop suite 26/26 (incl. `ScriviDeepLink` parse).
**Still not user-verified:** the live Spotlight-tap continuation depends on donations indexing
(the T-0182 `SetStoreUpdateService` env issue) and is acknowledged unreliable on SwiftUI macOS; the
`scrivi://` URL scheme remains the fully-verified route. Full end-to-end Spotlight-tap verification is
T-0189 (SP-047).

### SP-046 — Layer 2: on-disk `.scrivi` importer extension — Implemented, Not Verified (2026-06-30)

All four implemented; macOS `ScriviApp` scheme builds clean and the OS registered the Spotlight
extension on install. Awaiting live verification (Spotlight finds `.scrivi` content with the app
closed).

**T-0185 — Importer extension target.** ✅ New macOS app-extension target **ScriviSpotlightImporter**
(bundle `com.caposoft.scrivi.ScriviSpotlightImporter`), created via Xcode's **Spotlight Importer**
template (`CSImportExtension`) and embedded in `ScriviApp`. Authored by Xcode, so the target +
`ImportExtension.swift` + `Info.plist` are managed in `project.pbxproj` by Xcode (no hand-edit; the
CLAUDE.md pbxproj rule applies to *hand-created* files, none here).

**T-0186 — Link ScriviCore (Option A).** ✅ Mirrors the app's recipe via Build Settings on the
extension: own `Build ScriviCore (CMake)` run-script phase → `build/ScriviCore/libScriviCore.a`;
`LIBRARY_SEARCH_PATHS=$(SRCROOT)/build/ScriviCore`, `OTHER_LDFLAGS=-lScriviCore -lc++`,
`SWIFT_INCLUDE_PATHS=$(SRCROOT)/ScriviCore/include/scrivi` (so `import ScriviCore` resolves the C
module). **Required `ENABLE_USER_SCRIPT_SANDBOXING=NO`** (matching the app) — the default `YES`
sandboxed the run-script and made CMake fail to read the root `CMakeLists.txt`.

**T-0187 — Emit attributes from facade JSON.** ✅ `ImportExtension.update(_:forFileAt:)` calls
`scrivi_extract_searchable_text(path)` (same facade as Layer 1; `scrivi_free`'d), decodes the
`scrivi.searchableContent.v1` envelope, and emits one `CSSearchableItemAttributeSet`:
`displayName` = project title; `textContent` = every record's title + description folded together
(so any scene/object content surfaces the package); `keywords` = deduped union. **Note:**
`CSImportExtension` indexes the package as a **single** item, so per-scene deep-linking stays Layer 1's
job (the in-app `CSSearchableIndex` donor) — this on-disk layer makes content findable, by design.

**T-0188 — UTI handling + sandbox.** ✅ `Info.plist` `CSSupportedContentTypes` =
`com.caposoft.scrivi.project` (the app exports this UTI), `CSExtensionLabel` = "Scrivi Project
Importer". Extension runs in the App Sandbox with **User Selected File = Read Only**. Perf pass on a
large fixture deferred to verification.

### SP-047 — Verification, cross-platform assessment, Epic close

**T-0189 — End-to-end verification.** With Scrivi closed, Spotlight finds project/scene/object
content; selecting a result opens Scrivi at the item; donations now succeed so the original
`CSInlineDonation … SetStoreUpdateService` errors no longer appear (or are documented benign);
no regression to open/save/close.

**T-0190 — Cross-platform assessment + EP-017 verification.** Assess iOS/iPadOS/visionOS use of
the shared Core Spotlight APIs; implement or explicitly defer with rationale. Run final EP-017
acceptance-criteria verification and prepare the Epic for close.

---

## EP-018 Per-Window / Per-Project Window Model Tasks (T-0191 – T-0196)

Design: `docs/Scrivi_PerWindow_Project_Model_Design_v0_1.md` (✅ approved 2026-06-24). Swift-layer
only; no ScriviCore/C++ changes. The V1 spike (T-0191) gates the R3 mechanism before any
production refactor commits.

### SP-048 — Foundation (spike, ProjectSession, registry)

**T-0191 — V1 spike. ✅ Done 2026-06-24.** Throwaway `#if SPIKE_T0191` block in `ScriviApp.swift`
(built, run, observed, removed). **Result:** macOS 26 `WindowGroup(for:)` de-dups by value **only
against already-established windows; NOT race-safe** — two `openWindow(value:)` for the same value
in quick succession produced two windows; re-opening an already-open value focused it.
**Decision:** `OpenProjectRegistry` (T-0193) is the **authoritative R3 guard** (check-and-focus
before `openWindow`); native de-dup is a steady-state backstop only. Critical for restore-all
(T-0195) and deep links, which open windows concurrently. Recorded in design doc §3.2 and SP-048
notes.

**T-0192 — Extract `ProjectSession`.** Move all per-project state (`openProjectResult`,
`viewportLoader`, `projectPreferences`, `timelineModel`, `pendingNavigationSceneID`,
`spotlightDomainIdentifier`, `deepLinkAccessURL`, per-window UI flags) and the `openProject` /
`closeProject` / `onAppResign` / `donateSpotlight` methods off `AppEnvironment` onto a new
`@Observable @MainActor ProjectSession`. Behavior-preserving — app stays single-window this
sprint. New `.swift` files added to `project.pbxproj` in the same step (CLAUDE.md).

**T-0193 — `OpenProjectRegistry`.** Add an `@Observable` registry to `AppEnvironment` mapping
`projectID → ProjectSession`, with register/deregister/lookup. Powers R3 (already-open check) and
R4 (restore manifest). Not yet wired to windowing.

### SP-049 — Windowing & restore

**T-0194 — `WindowGroup(for:)` + Landing window.** Convert the scene to
`WindowGroup(for: ProjectWindowID.self)`; add a Landing/picker window for the no-project state;
open/focus projects via `openWindow(value:)`. Achieves R1/R2/R3 using the T-0191-chosen
mechanism.

**T-0195 — Manifest + restore-all.** Persist the set of open projectIDs (+ frontmost) to
UserDefaults; on launch, resolve each via `ProjectBookmarkStore` (reused from T-0184) and
`openWindow` to restore **all** previously-open windows (Q1). Per-window scene/cursor/scroll
restored by the existing backend open flow — no backend change.

### SP-050 — Deep-link rewrite & verification

**T-0196 — Deep-link rewrite + EP-018 verification.** Rebuild the paused T-0184 handler on the
new model: resolve projectID → registry → focus existing window or `openWindow` → set
`pendingNavigationSceneID` on that window's session. Fix scene matching to use the `scene_…` ID
(not title). Keep `ScriviDeepLink` / `ProjectBookmarkStore` / `SpotlightDonor`. Cross-reference
the per-window model from `Scrivi_Project_Creation_and_Open_Flow_v0_2.md`. Run EP-018 R1–R5
verification; unblock EP-017 AC5.

### EP-017 follow-up (deferred from T-0190)

**T-0197 — Enable Core Spotlight donation on iOS/iPadOS.** Broaden the `SpotlightDonor` /
`AppEnvironment` guards from `os(macOS)` to `os(macOS) || os(iOS)`, verify Spotlight entitlements on
the iOS target, and wire the iOS deep-link/bookmark **consumer** side so a tapped Spotlight result can
resolve a persisted project across launches (`ProjectBookmarkStore` is currently macOS-only). visionOS
is out of scope until its `ScriviEngine` backend links (I-0053 covered iOS/iPadOS only). Gated on:
(a) T-0189 done ✅, and (b) iOS bookmark/restore parity. Rationale + assessment in
`Verified/Task-verified-0190.md`.

## EP-019 Undo/Redo History & Copy Buffer Tasks (T-0198 – T-0217)

Design reference for every task: `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`
(section numbers below refer to that doc). Sprint-level goals and exit criteria:
`docs/Sprints/Sprint-backlog.md` (SP-051–SP-057). Epic-level ACs: `docs/Epics/Epic-active.md`
(EP-019). Supersedes **I-0019** (⚪ Closed 2026-07-06 — OBE; requirement carried by EP-019 AC1).

### SP-051 — Design sign-off, spike, schemas — ✅ Closed (2026-07-06)

All three tasks complete; sprint retrospective + full spike findings:
`docs/Sprints/Closed/Sprint-SP-051.md`.

**T-0198 — Design doc + trade studies.** ✅ Verified 2026-07-06 (user design sign-off; all six
trades ruled — T1=B, T2=A+refinements, T3=C, T4=B+D+A, T5=C, T6=A). Archived:
[Task-verified-0198.md](Verified/Task-verified-0198.md).

**T-0199 — ⌘Z-routing spike.** ✅ Done 2026-07-06 (two live user-observed rounds). The proposed
`UndoManager` proxy **failed** (menu never consulted it; `NSInternalInconsistencyException` per
keystroke via NSTextView's coalescer even with `allowsUndo=false`); the **adopted mechanism** is
first-responder `undo(_:)`/`redo(_:)` action methods + `validateUserInterfaceItem` — confirmed
receiving menu clicks and ⌘Z/⇧⌘Z. Findings in design §8/§12.6; spike code removed (no `.swift`
diff); governs T-0205.

**T-0200 — Schema spec.** ✅ Verified 2026-07-06 — design doc Appendix A (field-level
`scrivi.history.v1`/`scrivi.buffers.v1` spec), repair matrix §6.21, package structure §4/§16a/§17.
Archived: [Task-verified-0200.md](Verified/Task-verified-0200.md).

### SP-052 — Linear engine core

**T-0201 — `HistoryService`.** New `ScriviCore/src/history/`: record (snapshot-diff via
prefix/suffix trim on scalar boundaries), undo/redo on a linear chain, session markers, per-scene
head-text cache (design §3.a, §4, §5). Catch2 unit tests.

**T-0202 — C ABI.** `scrivi_history_open/record_event/record_barrier/undo/redo/close` in
`scrivi.h` + `scrivi_c_api.cpp` per the envelope conventions (design §7). pbxproj/CMake updated in
the same step for any new files (CLAUDE.md rule).

**T-0203 — Swift wrappers.** `ScriviEngine.swift` methods + decode types (pure decode, no logic);
interop tests round-tripping record→undo→redo.

### SP-053 — In-session undo/redo (macOS) — delivers EP-019 AC1 (ex-I-0019)

**T-0204 — `HistoryCapture` + triggers.** New `@MainActor` component owned by `ProjectSession`;
commit triggers per design §4.a (sentence terminators `.` `!` `?`, Return, cursor-move-with-pending,
paste/cut, scene switch, pre-auto-save flush via `flushThenSave`, resign/close); IME and rebuild
guards.

**T-0205 — Apply path + routing.** Ranged `replaceCharacters` inside `sceneBoundaries[segIdx]`
under `isRebuilding`, cursor restore, immediate `engine.saveScene`; `allowsUndo=false` + the
T-0199-validated ⌘Z routing (design §8).

**T-0206 — Barriers.** Structural ops (create/delete/merge/split/reorder) record barrier nodes;
undo stops at a barrier with a clear notice (design §4.5).

### SP-054 — Persistence, sessions, capacity

**T-0207 — Log + checkpoint.** Append-only JSONL + atomic `state.json` checkpoint, torn-line
truncation, log-tail replay, head-hash validation → `externalChange` barrier (design §6).

**T-0208 — Capacity + settings.** Eviction from root with auto-purge of aged-off branches
(never evict root→current path); `historySettings` per Trade T1 decision + Project Settings UI row.

**T-0209 — Session warning.** `crossedSessionBoundary` → popup once per crossing, showing the
boundary wall-clock time (Trade T5 ✅ approved: project-open span + 8 h idle rollover).

### SP-055 — Branching ✅ closed 2026-07-13 (T-0210/T-0211/T-0212 verified → `Verified/Task-verified-0210-0212.md`)

**T-0210 — Tree ops.** Fork-on-record (new sibling becomes primary), `select_branch` re-primary,
auto-purge on eviction; unit tests for the undo-type-fork-reselect scenario (design §5).

**T-0211 — Fork popover.** Inline transient popover at the caret listing fork children (preview +
timestamp), keyboard selection (Trade T2 option A), driven by the `forkAhead` envelope field.
Implements the approved T2 refinements (design §10 T2): appears when undo *lands on* a fork;
dismisses (never obstructs) when the writer undoes past it; appears immediately when redo reaches a
fork; redoing without selecting follows the primary child.

**T-0212 — Stale branches.** Detection thresholds + user-confirmed purge flow.

## EP-030 Scene Inspector Card Framework Tasks (T-0359 – T-0369)

Design: `../Scrivi_Scene_Inspector_Card_Framework_v0_1.md` ✅ **Approved 2026-08-05** (C1=C · C2=B · C3=A within
tabs · C4=A · C5=C + in-stack creation · C6=B with **sort per-stack**).

### SP-090 — Framework (🟡 ACTIVE 2026-08-05)

**T-0359 — `InspectorCard` protocol + registry.** Per §5: `typeID` (stable, persisted), `title`, associated
`Body`, and `CardContext` (`sceneID`, `engine`, `config`). A registry maps `typeID` → implementation so the
layout file stays decoupled from Swift types. Swift is UI only — cards call ScriviCore via `ScriviEngine`.

**T-0360 — `inspector-layout.json`.** Schema `scrivi.inspector-layout.v1` (§C2): `selectedTab`,
`inspectorHidden`, `defaultStacks` (worldbuilding **empty**, writing = `tags`/`outline`/`todo`), `stackSort`,
per-scene `worldbuilding`/`writing` arrays. Project-level and Git-visible — **not** the scene sidecar (EP-027
keeps those canonical) and **not** workspace state (a card stack is a creative decision that should travel with
the project). Unknown `typeID` → **skip with a notice, never crash**.

**T-0361 — Tabs + persistence + replace the placeholder.** Order `Writing | Worldbuilding | Properties`,
defaulting to **Writing**. Tab selection **does not follow the scene**. Persist `selectedTab` and
`inspectorHidden` at project level — the latter promotes the existing in-memory `session.inspectorVisible`
(`ProjectSession.swift:75`) to persisted state. Replace the fixed 280pt frame with a resizable pane.

**T-0362 — Card stack mechanics.** Add / remove / reorder / collapse; **sort is per-stack, not per-card** (C6);
"apply to all scenes" and "to this chapter" (C1), each scene independently editable afterward.

### SP-091 — Writing-tool cards

**T-0363 — `tags` + `todo` cards.** Scene-sidecar backed.
**T-0364 — `outline` card.** Scene summary/synopsis.
**T-0365 — `sources` card + `source` object kind.** 🟡 **Two thirds Verified; only the card remains (SP-099).**
**OQ-1 is CLOSED**: the design was amended 2026-08-12 and T-0365's scope is fully specified. It was split
across three sprints, because it was never one task:

- ✅ **`[ScriviCore]` relation type → SP-096 (T-0373), Verified 2026-08-12.** The **`cites` / `documented-by`**
  type — the first with `sourceKind: null` **and** `targetKind: null`, since a citation may document any kind
  (Doc 1 §3.4, §5.1). No new machinery: many-to-many falls out of the canonical-edge model unchanged.
- ✅ **`[ScriviCore]` object kind → SP-098 (T-0406), Verified 2026-08-12.** The `source` **object kind**
  (project-scoped at `objects/sources/`; the enum work SP-095 deliberately left out), round-tripping through
  the C ABI with its own schema tag and index participation. A `cites` edge relates a source to any kind
  **across both partitions** — project character and world artifact alike. **This closed EP-031 AC1.**
- **`[Apple]` half → SP-099.** The **one aggregate `sources` card** (Doc 2 §3.1.1) — lists sources reached via
  *this scene's* objects (scene → objects → sources), each entry naming the object(s) it came from, click →
  citation popup. **Worldbuilding-object cards surface their own sources with the same popup.**

> ⚠️ **Design corrected 2026-08-12 (user ruling), superseding the 2026-08-05 text.** Sources were specified as
> "related to **scenes** by ordinary edges." **Withdrawn.** A citation documents an **object**, not a passage
> of text. Two consequences: the card became **one aggregate card** rather than a card per source (so it can be
> shown/hidden as a unit in the picker); and **source-in-manuscript — footnotes and pull quotes — is deferred
> to `EP-032`**, because rendering an object inside scene text is a capability Scrivi does not have (Markdown
> bodies with no reference syntax; `scrivi.fragment.v1` carries no object references; no resolution in the
> editor, renderer, or export). Adding a `source`→`scene` type later is **additive** — Doc 1 §3.4.1.

### SP-092 — History card + Properties

**T-0366 — `history` card** (**supersedes T-0215**). Windowed tree (`scrivi_history_get_tree`), branch
selection, stale badges + purge — the Trade T2 management surface, now a dismissible card in the Writing stack.
**Unblocks EP-019 SP-057.**
**T-0367 — Properties tab.** Author, created/modified, metrics, story-time position. **Not** a card stack.
Built **field-driven**, each field declaring its own editability (§4.8.1) — read-only content today, but not a
read-only architecture.
**T-0368 — Card failure isolation.** One card's failure never blocks the stack; the failed card shows a warning
in place of its content.

### SP-094 — Verification (merged EP-019 + EP-030)

**T-0369 — EP-030 verification (AC1–AC7) + Epic close prep.** User approval required to close.

> **Moved from SP-093 → SP-094 (user ruling, 2026-08-07).** SP-093 is now the **EP-019 history-capture
> sprint** (granularity + presentation), opened from the SP-092 live-verify. T-0369 and EP-019's SP-057 were
> both one-task, no-build-work verification passes gated on the same live session and app build, so they
> merge into a single **SP-094 — "EP-019 + EP-030 verification & Epic close."** Both Epics' ACs are verified
> in one pass, then **closed independently** — each on its own direct user approval, and a failure in one
> Epic's ACs does not block the other's close.

---

## EP-031 Worldbuilding Object Model & Relationship Graph Tasks (T-0370 – T-0391, T-0401)

Design: `docs/Scrivi_Worldbuilding_Object_Model_v0_2.md` (Doc 1) + `docs/Scrivi_World_Data_Separation_v0_1.md`
(Doc 3), both ✅ Approved 2026-08-05. Full sprint detail: `../Sprints/Sprint-active.md`.

### SP-095 — Object kinds + fields + object index (🔵 Planning 2026-08-12)

**T-0370 — `ObjectKind` additions + `objectKindSubdir` + schema table; retire `timeline`.** Adds 8 kinds
(`building`, `vehicle`, `map` live; `artifact`, `chronicle`, `faction` **gated**; `world` declared but not
creatable). ⚠️ **World-scoped kinds are declared but gated** (SP-095 §R1): Doc 1 §3 / Doc 3 §7.2 put
`artifact`/`chronicle`/`faction`/`rule` at `worlds/<worldID>/…`, and world packages don't exist until SP-098 —
so create returns `invalidArgument` naming the world requirement rather than writing to `objects/`. **`rule`
is untouched** (it already ships project-scoped); relocation is T-0381's. Retires `ObjectKind::timeline`
(T4=A) across 5 files. Also fixes `objectKindFromStr`'s silent `character` default (`scrivi_c_api.cpp:350`),
a mis-dispatch that becomes reachable at 12 kinds.

**T-0371 — `WorldObjectFields`: `subtitle`, `image`, `worldID`.** All additive, all defaulting empty; legacy
5-kind files must parse unchanged. **`image` carries `assetID`/`thumbnailAssetID` only — never inline bytes**
(Doc 1 §4.1); images live in `assets/` via `scrivi_import_asset`. `worldID` round-trips but carries no meaning
until SP-098.

**T-0372 — `objects/index.json` + `findByID` over the index.** `scrivi.object-index.v1`; replaces the O(n)
scan-and-parse in `ObjectStore.cpp:20-55`. **Load-bearing, not an optimization** — SP-096's edges store bare
`{id}` endpoints and cannot be rendered without it (Doc 1 §4.2, §5.2). Written atomically alongside every
mutation, **after** the object write succeeds (a phantom entry is silently wrong; a missing entry self-heals).

**T-0401 — Index rebuild + corruption coverage.** Doc 1 §4.2 calls the rebuild path "a correctness
requirement, not a convenience." Five tests: missing index, corrupt index, stale index (hand-edited slug),
idempotent rebuild, unparseable object file skipped-not-fatal. **The project must open in every case.**

### SP-096 — Relationship graph (🔵 Planning 2026-08-12)

**T-0373 — `relation-types.json`.** `scrivi.relation-types.v1`; `canonicalDirection` + `symmetric`;
seeded on project creation with §5.1's **four** types (`appears-in`, `located-at`, `sibling-of`, `cites`) and
re-seeded on first read if absent. Upsert validates that `symmetric: true` implies `lexical` and matching
labels. ⚠️ **Nullable kind constraints:** `JsonDoc::getString` cannot distinguish absent from explicit `null`,
so "any kind" is written as **absent** and read as empty-means-any; `cites` is the first type unconstrained on
**both** ends.

**T-0374 — `relationships.jsonl` append-log.** `rec`/`seq` records via `appendTextFile` + torn-final-line
detection, reusing EP-019's `HistoryStore` pattern. Create = one appended line; delete = a tombstone; load =
linear scan with the full in-memory map retained (§5.4). Ships `scrivi_create_edge` / `scrivi_delete_edge` /
`scrivi_list_edges_for` (bare IDs, no kind params). ⚠️ **`seq` must be assigned once at the append point** —
`HistoryStore.cpp:194` documents a shipped bug where 13 records were written with `seq 1`.

**T-0375 — Canonical normalization + duplicate rejection.** §5.3. Asymmetric types normalize to the declared
direction; symmetric types sort endpoint IDs lexically. Creating a relationship from **either** end yields
**one** edge; the reverse create is rejected with a distinct error. §9 AC4 requires both cases —
⚠️ `faction` is world-scoped and uncreatable until SP-098, so the same-kind symmetric case uses
`sibling-of` (character↔character); the faction-specific test lands in SP-098.

**T-0376 — Compaction + graph settings.** Compacts on open at **30% of records or 1,000 tombstones**,
whichever first, with **both triggers tested independently** (§9 AC9). Temp-write → atomic replace; `seq`
renumbered from 1. Settings follow the `HistorySettings` precedent — **`project.json` canonical + a local
mirror**, *not* inside history's `state.json`.

**T-0402 — ⚠️ Endpoint-kind resolution (design defect found at planning).** Doc 1 §5.2 says endpoints are
distinguished "by ID prefix (`scene_…`)". **That rule is broken**: `SystemUUIDProvider::newObjectID()`
(`:74`) returns **`character_…` for every object kind**, so a `location`'s ID begins `character_`; and
`DeterministicUUIDProvider` (`:16`/`:20`) uses `scene-`/`obj-` — different separator *and* stem, so tests
would take a different branch from production. Both failure modes are **silent**. Replaced by
`resolveEndpoint()`: `ObjectIndex::find` first (authoritative for ID→kind since SP-095), then the EP-027
scene path, else unresolved. **Doc 1 §5.2 is amended in the same task** — the design and code must not
disagree. Do **not** fix the generators instead: ID shape is load-bearing (`objectID` preservation across
promotion, §3.1) and SP-095 already shipped index rows carrying today's IDs.

### SP-097 — World packages (🔵 Planning 2026-08-12) — ⚠️ **content swapped with SP-098**

> **Why the swap.** SP-097 was to be the integrity sprint. Planning verified **two of its five tasks were
> unbuildable**: **T-0379** (`promote_object`) has no destination while `artifact` is refused by SP-095's
> `checkKindStorable` gate, and **T-0380** (pending-vs-dangling) has **no world plumbing to interrogate** — the
> Epic's highest-risk branch could not be reached, let alone tested. Worlds land first; the whole integrity set
> moves to SP-098 and is built once against real worlds. **Sprint IDs stay in sequence.**

**T-0403 — ⚠️ `FileSystem::createFileExclusive` (do first).** Doc 3 §6.5 specifies lock acquisition as
"atomic create-if-absent (`AtomicWrite`'s exclusive-create path)". **No such path exists**:
`util::atomicWriteTextFile` is temp-write → `fs::rename`, and **rename silently overwrites** — two writers would
both believe they hold the lock. Adds a create-if-absent primitive to `FileSystem` + `LocalFileSystem` + mocks,
with a **concurrent-acquire test proving exactly one winner**. **T-0383 depends on it.**

**T-0381 — the `.scrivworld` package.** `world.json` (`scrivi.world.v1`), its own `index.json` (reusing
SP-095's `scrivi.object-index.v1`), and the world-scoped kind directories. `scrivi_create_world` creates the
package **and** its binding atomically. Self-contained: moving the directory loses nothing.

**T-0382 — bindings + resolution + relink.** `worlds/<worldID>/binding.json` with a **platform-neutral**
reference (Doc 3 §4.4.1 — no Apple bookmarks in the model). Resolution tries `lastKnownPath` **relative to the
project** first, then absolute; the first candidate whose `world.json` carries the **matching `worldID`** wins.
⚠️ **A different `worldID` is not the world — resolution stops**, which is what prevents a same-named package
being silently substituted. No search, no registry (ruled).

**T-0383 — write locking.** `.lock` file (not an OS advisory lock — semantics differ across seven targets and
behave poorly on network volumes). Held **per write, never per session**; heartbeat; **60 s stale-lock
recovery** so a crashed writer blocks others for at most a minute; contention **reports and moves on, never
hangs**. `AtomicWrite` does not subsume this — it makes one write atomic, not a cross-process
read-modify-write sequence.

**T-0384 — the epoch chain.** Three layers (Doc 1 §7.0): timeline → world → project. Timeline offsets are
**world-relative, always**, so rebinding changes exactly one number. Editing a binding never mutates
`world.json`.

**T-0385 — cached index + world kinds become creatable.** `binding.cachedIndex` exists for one reason: a
pending entry must read "⟨Midgard: Sword of Dawn⟩", not a bare UUID — **a writer asked whether to clear world
references cannot decide blind**. Never authoritative; overwritten whenever the world is reachable. Then
relaxes `checkKindStorable` so `artifact`/`chronicle`/`faction` are creatable **into world scope**.
⚠️ **Closes two carried-forward items: AC1's gated half, and AC3's faction↔faction symmetric duplicate test**
deferred from SP-096.

**T-0404 — `rule` relocation (deferred from SP-095).** Moves `objects/rules/` → `worlds/<worldID>/rules/` and
flips `objectKindIsWorldScoped`. **No migration code** (Doc 3 §7 — nothing has shipped); fixtures are
regenerated. Also corrects **Package Structure v0.1 §11** and the `TODO(SP-098)` marker in `ObjectTypes.hpp`,
which now names the wrong sprint.

### SP-098 – SP-100 — outline

Detail is written at each sprint's planning. **SP-098** (post-swap) — cascade-prune on object **and** scene
delete, orphan queries, `promote_object`, and ⚠️ **T-0380 pending-vs-dangling, the Epic's highest-risk task**,
the one failure that is *silent and unrecoverable*; **plus T-0365's ScriviCore half** (the `source` object
kind — the `cites` relation type itself already shipped in SP-096). All four are unblocked by SP-097's worlds.
~~**SP-099** object cards on EP-030's framework (T-0386–T-0389) **plus T-0365's aggregate `sources` card**.~~
⚠️ **SPLIT at SP-099 planning, 2026-08-13 (R2, user-approved).** The staged sprint carried 5 tasks plus an
entire unplanned engine layer:

- **SP-099** — **T-0407** (⚠️ new: the 12 graph/world endpoints have **no** `ScriviEngine` wrappers; **do
  first**, tested through `scrivi_*`) + **T-0386** object cards + **T-0387** picker → **AC16/17/21/22**.
- **SP-102** (new) — **T-0388** in-stack create/edit + **T-0389** pending presentation / Worlds menu / warning
  view + **T-0365's aggregate `sources` card** → **AC18/19/20/23/24**. ⚠️ T-0389 also carries the
  **`offline`/`unmounted` status refinement**, which is declared in `WorldTypes.hpp:68` but **produced
  nowhere in `ScriviCore/src`** — AC24 rests on Apple-layer work no staged task named.

**SP-100** repair matrix + verification (T-0390, T-0391) — **keeps its number**, runs last. EP-031 is now a
**7-sprint** Epic.

**T-0407 — ⚠️ `ScriviEngine` graph + world wrappers (do first).** All 12 endpoints — `create_edge`,
`delete_edge`, `list_edges_for`, `list_pending_edges`, `list_objects`, `list_orphaned_objects`,
`promote_object`, `list_worlds`, `get_world_status`, `relink_world`, `remove_world_reference`,
`list_relation_types` — exist and are exported in `scrivi.h` and **grep to zero call sites** in
`ScriviEngine.swift`, which has object CRUD only (`:237–318`). Follow the established `withCString` +
`decodeC` pattern. `list_edges_for` rows must carry `otherPending` / `otherDisplayName` / `otherWorldStatus`
into Swift **even though nothing consumes them until SP-102** — dropping them forces a second pass over the
same decode. `detail == "worldPending:<status>"` must stay distinguishable in `ScriviError`, since SP-102's
frozen-graph refusal reads it. **Tests go through `scrivi_*`**, per the I-0113 standing habit — a Swift-only
test cannot see a boundary gap.

---

### SP-056 — Copy buffers

**T-0213 — Buffer store.** `history/buffers.json` (`scrivi.buffers.v1`), 9 numbered slots,
`scrivi_buffers_list/set/clear` + engine wrappers (design §9).

**T-0214 — Buffer UX + history integration.** Keyboard HUD + toggleable palette + Edit-menu items
(Trade T4); paste-from-buffer = `paste` event; cut-into-buffer = `cut` event; copy-into-buffer not
an event (Trade T3 ✅ approved); system pasteboard untouched.

### SP-057 — Panel, perf, verification

**T-0215 — History panel.** ⚪ **Superseded 2026-08-05 → T-0366 (EP-030 SP-092).** The requirement is unchanged
(windowed tree via `scrivi_history_get_tree`, branch selection, stale badges + purge — Trade T2 option B
management surface); only its *surface* changed. Per the approved
`Scrivi_Scene_Inspector_Card_Framework_v0_1.md` §8 it ships as the **`history` card** in the Writing stack rather
than a standalone panel, so it is dismissible and positionable like every other writing-tool card. **EP-019
AC-relevant note:** SP-057 no longer builds a panel; it verifies AC2/AC7/AC8 and closes the Epic once T-0366
lands.

**T-0216 — Perf + migration.** ⚪ **CLOSED — OBE 2026-08-05 (user-directed).** Original scope: integration
fixtures (100k-event history, 500 KB single scene) + a `.gitignore` migration adding `history/` for existing
projects (design §6.c, §11).

**Why closed:** the `.gitignore` migration is obsolete — **Scrivi has not shipped**, so no projects exist in the
field whose `.gitignore` predates the `history/` entry; new projects get it at creation. The user closed the
task whole rather than splitting it.

**⚠️ What this gives up:** the **perf fixtures were never built**, so EP-019 ships with **no** large-scale
performance coverage — the 100k-event history and 500 KB single-scene cases (design §6.c worst case) are
untested. `HistoryService` has unit + integration coverage at ordinary sizes only. If history performance is
ever suspect at scale, this is the missing evidence; re-open as a new task rather than reviving T-0216.

**T-0217 — Verification.** Update package-structure + repair-matrix docs; run EP-019 AC1–AC8
verification; prepare Epic for close (user approval required).

> **✅ Both design-doc amendments landed 2026-08-11** (ahead of SP-094 activation, so the AC2 live verify runs
> against the amended wording rather than the superseded list):
> - **§4.a** — trigger 6 (auto-save flush) retired; trigger 7 = the ≥ 45 s idle boundary; new **§4.a.1**
>   explains the save/idle split, backspace-does-not-commit, and why record-and-reopen was not implementable.
> - **§4.d** — rewritten as a **relaxed** invariant with the changed-invariant warning, bound, and failure
>   mode; **§12.2** resolved; **§15 AC2** and the doc header amended; `Epic-active.md` **AC2** rewritten
>   (its stale "the §4.d disk invariant is preserved" claim removed) and flagged so that approving EP-019's
>   close is explicitly an approval of the relaxed §4.d.
>
> **✅ Package-structure + repair-matrix updates landed 2026-08-11** (design change user-approved same day):
> - **`Scrivi_Project_Package_Structure_v0_1.md` §16a** — reconciled with the shipped implementation:
>   checkpoint cadence (close *and* every 200 records), the **§4.d relaxation** stated as a package-level
>   property (scene files may lead history by one open session; prose is still canonical and complete —
>   it is *history* that trails), lazy creation, and the pre-EP-019 case retired per T-0216.
> - **`Scrivi_External_Change_Repair_Matrix_v0_2.md` §6.21** — head-hash mismatch causes restated: a
>   **hard crash mid-session is now an expected cause**, so a mismatch is **not by itself evidence of
>   external editing** and the notice must not claim it is; new replayed-purge sub-condition (I-0110);
>   torn-line behaviour corrected to *first* unparseable line; I-0104's false-positive history recorded as
>   a caution for anyone touching the checkpoint.
> - **Design §12.8 resolved** — the scanner cannot flag `history/` because it only walks `manuscript/`; no
>   ignore-set entry exists or is needed (with a caveat if that scope ever widens). §14 table corrected.
>
> **Still open under T-0217:** the EP-019 AC1–AC8 verification pass itself (SP-094).
>
> ⚠️ **Found while documenting (2026-08-11): log-segment rotation is specified but not implemented.**
> `activeSegment_` is hard-fixed to `log-000001.jsonl` (`HistoryStore.hpp:114`); capacity/eviction bounds
> the tree, not the log, so a segment grows unbounded (a real project is already at ~3.4 MB). The reader
> already honours `activeLogSegment` from `state.json`, so rotation is additive and needs no format change.
> Documented as not-implemented in §16a rather than papered over. **Not an EP-019 blocker** — needs its own
> task; do not fold it into the verification sprint.
>
> *Original scope note (2026-08-10), retained for the record:*
> 1. **AC2 + design §4.a** — document the save-time commit and the **idle-session boundary** (45 s). AC2's
>    trigger list is not exhaustive of what actually commits; every trigger it names is kept.
> 2. **Design §4.d — the invariant itself changed.** "Disk never contains text no history node describes" is
>    **relaxed**: while a typing session is open, disk may lead history by at most one save. Bounded by
>    `close()` committing pending text and, on a hard crash, I-0104's head-hash check raising an
>    `externalChange` barrier. This is a **changed invariant**, not a documentation gap — it must be written
>    up as such, with the rationale (`HistoryService::record` always appends; undo walks one node per step, so
>    recording per-save cannot yield one undo step).

---

*Last Updated: 2026-08-16 (**Consistency audit — eight stale rows removed or corrected.** Seven Tasks whose
Sprint had closed were still listed here with pre-close status: **T-0386/T-0387/T-0388/T-0407/T-0408** (SP-099)
as 🟠 *Implemented - Not Verified*, and **T-0409/T-0411** (SP-103) as 🔵 *Backlog* — all seven are ✅ Verified
2026-08-15. **T-0217** was 🔵 *Backlog* under the superseded SP-057 despite being completed in SP-094 and
closed with EP-019 on 2026-08-11. ⚠️ **This is exactly the staleness `Task-Documentation.md`'s rebuild header
warns about**, and it is now the rule to enforce: **a Task's row leaves this file when its Sprint closes.**
See the note above the "Full Detail" section. Prior note follows.)*

*2026-08-09 (**Tracking-doc audit — backlog realigned to the 2026-08-07 renumbering.** The
SP-093 → EP-019 renumbering and the T-0369 → SP-094 merge had been applied to `Task-active.md`,
`Sprint-active.md` and `Epic-active.md` but never propagated here. Corrected: **T-0369** reassigned SP-093 →
**SP-094** (table + detail heading); **T-0396** rewritten — the stale row still said "retire cursor-move as a
hard commit trigger", which the corrected 2026-08-07 diagnosis **reverses** (cursor-move is kept; the cause is
the 1 s autosave debounce), and now records the app-side/30–60 s/backspace rulings; **T-0397/T-0398** retagged
`[Cross]` to match the sprint plan; **T-0365** corrected from "EP-030 (SP-091) 🔵 Backlog" to **⚪ Deferred →
EP-031 SP-094**, since SP-091 is closed. Prior note follows.)*

*2026-07-13 (SP-055 closed: T-0210/T-0211/T-0212 verified & archived → `Verified/Task-verified-0210-0212.md`; EP-019 AC4 + AC5's deferred branch clauses delivered & verified. SP-056 (copy buffers) next.)*

# Active Epics

## EP-022: [Linux] Writing Surface & Scene Navigator

**Codebase:** `[Linux]` (Qt/QML Ubuntu app, `platforms/linux/`) — calls `[ScriviCore]` only via the
existing plain-C ABI through `ScriviBridge`. Any missing endpoint is a Task with a `[ScriviCore]` note
(none expected — the scene read/save/order endpoints already exist).

**Status:** 🟡 Active (drafted & activated 2026-07-14; promoted from the Epic backlog on EP-021 close)
**Goal:** The **core writing loop** on Linux — a **scene navigator** sidebar (manuscript order, live
titles), the **manuscript editing surface** (open a scene, edit its Markdown, auto-save), and
quit/reopen fidelity (restored active scene + cursor/scroll). This is the heart of the app and the Ubuntu
analogue of Apple **EP-009** (Writing Surface & Scene Navigator) + **EP-011** (scroll/cursor/focus
behavior). Builds directly on EP-021's landing → open-project flow: opening a `ready` project now lands in
a **real editor** instead of the placeholder project window.

**Strategy:** Third of the `[Linux]` family (EP-020–EP-026). EP-020 gave the spine, EP-021 the project
lifecycle; EP-022 delivers the first genuinely useful screen — writing. Largest of the family. Verified in
Docker+VNC (developer) and, per milestone, on real Ubuntu (alpha tester). Sprint count decided at planning
(likely 2–3: navigator + open-scene read; editing + auto-save; scroll/cursor/restore polish + verify).

**Design references:**
- `ScriviCore/include/scrivi/scrivi.h` — `scrivi_open_project` (already returns the scene list +
  `activeScene` + restored surface), `scrivi_open_scene`, `scrivi_save_scene` (signatures + envelopes to be
  reconfirmed at planning).
- `docs/Scrivi_Project_Package_Structure_v0_1.md` — on-disk scene layout (`NNN-slug.md` + `.meta.json`).
- Apple analogues (behavior to re-create in C++/QML, **not** port wholesale): the EP-009 scene navigator +
  writing surface, EP-011's scroll/cursor/focus rules, `ProjectSession`/editor auto-save wiring.

**Date Created:** 2026-07-14
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria (draft — refine at first-sprint planning)

- [x] AC1 — **Scene navigator:** the project window shows a navigator listing the manuscript's scenes in
  order (chapter grouping + live scene titles), sourced from the `scrivi_open_project` scene list. Clicking
  a scene selects it. Keyboard + pointer usable. **(SP-061 — verified over VNC 2026-07-14.)**
- [x] AC2 — **Open a scene into the editor:** selecting a scene loads its Markdown (`scrivi_open_scene` or
  the `openProject` active-scene payload) into an editable text surface; the initial active scene + restored
  cursor/scroll from `openProject` are applied. **(SP-061 — read-only continuous viewport shows all bodies;
  editable surface is SP-062. Verified over VNC 2026-07-14.)**
- [x] AC3 — **Edit + save:** typing edits the scene; changes are saved via `scrivi_save_scene` (debounced
  auto-save + save-on-scene-switch + save-on-close), writing the real `.md` on disk. No data loss on scene
  switch or app quit. **(SP-062 — editable viewport + boundary integrity, per-scene auto-save (debounce +
  switch + close/quit), in-editor `Ctrl+Return`/`Ctrl+Shift+Return` scene/chapter creation. Verified over
  VNC 2026-07-14. Two issues found + handled: I-0061 Quit-button regression fixed & verified; I-0062 live
  new-chapter heading label deferred (cosmetic, self-corrects on reload).)**
- [ ] AC4 — **Scroll / cursor / focus behavior:** the editor honors the EP-011-equivalent rules —
  restored scroll position, sensible cursor placement on open, focus on the editor when a scene loads.
- [ ] AC5 — **Quit / reopen fidelity:** closing and reopening the project (recents) restores the last active
  scene and its surface state (cursor/scroll), proving the write→save→reopen loop end-to-end.
- [ ] AC6 — **Verified:** developer confirms edit → auto-save → switch scene → reopen over Docker+VNC; CI
  green (build + a headless scene read/save smoke if feasible). Real `.md` content confirmed on disk /
  from macOS via the shared mount.
- [ ] AC7 — **No regression:** `scrivi.h` unchanged or additive-only; ScriviCore `ctest` green; the macOS
  app + EP-020/EP-021 flows (landing, create, open, close, recents, identity) still build and work.

### Sprints

**Planned as 4 sprints** (decided 2026-07-14). ScriviCore already provides the scene read/save/order
endpoints, so EP-022 is **pure QML/bridge, single-platform, no core work**. The count is 4 (not 3) because
the **continuous all-in-memory viewport** with **scroll-driven scene switching** was chosen (design
decision below) — the ambitious option, which on Apple spanned two sprints (the viewport, then scroll-driven
switching). Four gives the viewport room and keeps each a shippable, VNC-verifiable milestone. IDs/tasks
assigned at each sprint's activation; titles/scope may refine at planning.

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-061 | **Shell flip to Widgets host** (`QApplication`+`QMainWindow`, EP-020/021 QML re-hosted via `QQuickWidget`) + scene navigator (ordered list + select) + **read-only continuous viewport** — all scene bodies in **one `QPlainTextEdit` / `QTextDocument`** with scene-boundary markers; the project window becomes the editor shell (replaces EP-021's placeholder). (AC1, AC2) | ✅ Closed | 2026-07-14 |
| SP-062 | **Editable** viewport + **per-scene auto-save** + **in-editor ⌘↩/⌘⇧↩ Scene/Chapter creation** — `QPlainTextEdit` editing (doc-level undo disabled); edit → `scrivi_save_scene` per scene (debounce ~1.5s + scene-switch + close/quit); ⌘↩ create-scene / ⌘⇧↩ create-chapter (Apple SP-023 parity). Real `.md` on disk. The write loop. (AC3) | ✅ Closed | 2026-07-14 |
| SP-063 | **Scroll-driven scene switching** + active-scene tracking — the visible/caret scene becomes "active" (saving the departing scene); navigator selection ↔ viewport scroll stay in sync (Apple SP-025 parity). (part of AC1/AC4) | 🔵 Planning (next up) | — |
| SP-064 | Cursor / focus behavior + non-deletable separator + quit-reopen **restore** fidelity (Apple SP-033/EP-011 parity) + full EP-022 verify & **Epic close**. (AC4, AC5, AC6, AC7) | 🔵 Planning | — |

**Split rationale:** SP-061 does the **shell flip** (the app becomes a Widgets `QMainWindow` re-hosting
the EP-020/021 QML via `QQuickWidget`) **and** lands the navigator + gets all scene text on screen
read-only in one `QPlainTextEdit` (lowest-risk first editor milestone, and both the shell and the
viewport structure are the foundation everything else sits on — front-loaded here on purpose). SP-062
makes it editable and closes the edit→save→reopen loop per scene (the heart of the Epic). SP-063 adds the
scroll-driven active-scene behavior (its own Apple sprint — kept separate so it doesn't destabilize the
write loop). SP-064 does cursor/focus polish, proves surface-state restore across reopen, runs the
parity/verification pass, and closes EP-022.

> **Shell-flip risk note:** re-hosting the EP-020/021 landing/create/open/close/recents flows inside a
> `QQuickWidget` (instead of a top-level `QQmlApplicationEngine`) is a real refactor of verified EP-021
> code. SP-061 must re-verify the full landing → create → open → close → recents loop over VNC **before**
> building the navigator/viewport on top — a regression here breaks shipped behavior (AC7).

### Design decisions (locked 2026-07-14, pre-SP-061)

- **Editor widget:** **Qt Widgets `QPlainTextEdit`** (the `NSTextView` analogue) — `QTextDocument`/
  `QTextCursor` for the continuous document, overridable `keyPressEvent` for in-editor scene/chapter
  creation, `document()->setUndoRedoEnabled(false)` mirroring Apple's `allowsUndo = false` (so the future
  custom undo/redo owns the ⌘Z path), and override-able `cut`/`copy`/`paste`. Chosen over Qt Quick
  `TextArea` because the user requires the writing surface to be **identical in convention to the Apple
  app** — same Scene/Chapter creation keystrokes, and (eventually) the same undo/redo and cut/copy/paste
  behavior. `TextArea` (a `QQuickTextControl`) cannot host that parity; `QPlainTextEdit` is the direct
  peer of `ManuscriptTextView.swift`'s `NSTextView`. Markdown is plain text; no rich rendering in v1.
  (Resolves Open Question #1 — **reversed** from the earlier `TextArea` pick.)
- **App shell (consequence of the editor choice):** **flip the app to a Qt Widgets host** — a
  `QApplication` + `QMainWindow` shell that embeds the existing EP-020/021 QML surfaces (Landing,
  navigator) via **`QQuickWidget`**, with the editor as a native `QPlainTextEdit` sibling. On the pinned
  **Qt 6.4** a `QWidget` **cannot** be embedded inside a QML `ApplicationWindow` cleanly (the
  `WidgetsInQuick` path is Qt 6.7+; `QGraphicsProxyWidget` is too fragile for a live editor), so the
  integration direction inverts: QML lives inside Widgets, not the reverse. This is also the closest
  structural match to the AppKit app (a native window hosting views). The current
  `QQmlApplicationEngine`-loads-`Landing.qml` bootstrap (EP-020/021) is refactored into the `QMainWindow`
  shell — see SP-061 foundation task. (New decision 2026-07-14.)
- **Scene model:** **continuous all-in-memory viewport** — all scenes rendered in one scrollable surface
  with **scroll-driven scene switching** (the Apple EP-011 model), not one-scene-at-a-time. With
  `QPlainTextEdit` this is **one `QTextDocument`** holding every scene's text with scene-boundary markers
  (mirroring `ManuscriptTextView`'s `sceneStartMap`/`sceneStorageOffsetMap`), not N stacked editors.
  (Resolves Open Question #5 — the ambitious option; hence 4 sprints, with the viewport + scroll-switching
  split across SP-061/SP-063.)
- **Auto-save cadence:** debounce (~1.5s idle) **+ on scene-switch + on close/app-quit** — mirrors Apple's
  `ProjectSession.saveAllDirty`; must survive the Docker/VNC quit path (app is the container's foreground
  process). Per-scene `scrivi_save_scene` (the viewport edits one surface but persists individual `.md`
  files at scene boundaries). (Resolves Open Question #3.)

### Tasks

All four sprints are drafted with tasks assigned (full detail in `Sprints/Sprint-backlog.md` +
`Tasks/Task-backlog.md`). None activated yet.

| Sprint | Tasks | Delivers |
| ------ | ----- | -------- |
| SP-061 | T-0234 shell flip · T-0235 read-only continuous viewport · T-0236 scene navigator · T-0237 verify + scene-load smoke | AC1, AC2 |
| SP-062 | T-0238 editable + dirty tracking · T-0239 per-scene auto-save · T-0240 ⌘↩ create scene · T-0241 ⌘⇧↩ create chapter · T-0242 verify + save smoke | AC3 |
| SP-063 | T-0243 scroll→active-scene · T-0244 navigator↔scroll sync · T-0245 verify | AC1/AC4 (scroll) |
| SP-064 | T-0246 cursor/focus + non-deletable separator · T-0247 quit→reopen restore · T-0248 EP-022 verify + close prep | AC4, AC5, AC6, AC7 |

### Issues

_(none yet)_

### Open Questions

1. ✅ **Editor widget choice** — **Resolved (2026-07-14, revised same day): Qt Widgets `QPlainTextEdit`**,
   hosted in a `QMainWindow` shell that embeds the QML surfaces via `QQuickWidget`. Reversed from the
   initial `TextArea` pick once parity with the Apple writing surface (in-editor ⌘↩ createScene / ⌘⇧↩
   createChapter, custom undo/redo via `setUndoRedoEnabled(false)`, cut/copy/paste hooks — the
   `NSTextView` analogue) became a hard requirement. `TextArea` can't carry that; embedding a `QWidget`
   in QML isn't clean on Qt 6.4, so the app shell flips to Widgets-hosts-QML. See Design decisions above.
2. ✅ **Scene load path** — **Resolved (2026-07-14): fetch each body via `scrivi_open_scene` at load; no
   payload extension, no ScriviCore work.** Verified against source: `scrivi.h:43` +
   `scrivi_c_api.cpp:430` — `scrivi_open_scene(projectRootPath, appSupportRoot, projectID, sceneID)`
   already returns `{ scene{sceneID,chapterID,title,slug,metadataPath,contentPath}, markdown }`. So the
   continuous viewport loads the full ordered `scenes[]` list + `activeScene` body from
   `scrivi_open_project`, then loops `scrivi_open_scene` per remaining `sceneID` for the other bodies —
   the same endpoints the Apple app uses, additive-only, `scrivi.h` untouched (satisfies AC7). `save_scene`
   args (metadata/content paths, selection anchor/focus, scroll, authorship) confirmed present at
   `scrivi.h:49`. **EP-022 requires no new core endpoints.**
3. ✅ **Auto-save trigger model** — **Resolved (2026-07-14):** debounce ~1.5s + on scene-switch + on
   close/quit, per-scene `save_scene`. See Design decisions.
4. **Navigator scope for v1** *(confirm at SP-061 planning).* Read-only ordered list (select → scroll the
   viewport to that scene) is the AC1 floor; **structure editing** (create/delete/rename/reorder) is
   explicitly **EP-023**, not here. Navigator is display + selection only in EP-022.
5. ✅ **Multi-scene viewport vs. one-scene-at-a-time** — **Resolved (2026-07-14): continuous all-in-memory
   viewport with scroll-driven switching** (the ambitious option; drove the 4-sprint plan). See Design
   decisions.

### Scope Notes

- **In scope:** scene navigator (display + selection), the writing surface (open/edit/auto-save a scene),
  scroll/cursor/focus behavior, active-scene + surface restore on reopen. The project window becomes the
  real editor (replacing EP-021's placeholder).
- **Out of scope (later `[Linux]` Epics):** structure editing — create/delete/rename/reorder (EP-023);
  inspector panel (EP-024); timeline (EP-025); undo/redo, menus, settings + parity (EP-026). Rich-text /
  Markdown *rendering* is not required (plain-text editing surface).
- **Bridge discipline (inherited):** every new `ScriviBridge` method parses the `{"ok":...}`/`{"error":...}`
  envelope, `scrivi_free`s every pointer (RAII `ScriviString`), holds **no** backend logic. New
  `.cpp/.hpp/.qml` are CMake-tracked (not in the Apple pbxproj), browsable via the `platforms/linux` Xcode
  folder reference.
- **Verification model (inherited):** CI = build + headless smoke; developer = GUI over Docker+VNC (host
  port 5901, password `scrivi`; persistent app-support + shared `/projects` mounts); alpha tester = real
  Ubuntu.

### Completion Summary

_(filled in when the Epic reaches 🟠 Complete)_

---

*Last Updated: 2026-07-14 (EP-022 `[Linux]` Writing Surface & Scene Navigator drafted & activated after
EP-021 ✅ closed. Draft AC1–AC7. **Design locked pre-SP-061 (editor decision revised same day):** Qt
**Widgets `QPlainTextEdit`** editor (the `NSTextView` analogue — reversed from `TextArea` for Apple
writing-surface parity: in-editor scene/chapter creation, custom undo/redo, cut/copy/paste), which forces
an **app-shell flip** to a `QApplication`+`QMainWindow` host that re-hosts the EP-020/021 QML via
`QQuickWidget` (a `QWidget` can't embed in a QML window cleanly on the pinned Qt 6.4); **continuous
all-in-memory viewport** = one `QTextDocument` with scene-boundary markers + **scroll-driven scene
switching**; auto-save = debounce+switch+close (Open Questions #1/#3/#5 resolved; #2 scene-load-path + #4
navigator-scope for SP-061 planning). **Sized at 4 sprints** (SP-061 **shell flip** + navigator +
read-only viewport; SP-062 edit + per-scene auto-save; SP-063 scroll-driven switching; SP-064 cursor/focus
+ restore + verify/close) — the viewport is the ambitious option and SP-061 now also carries the shell
refactor, hence 4. Tasks assigned at each sprint's activation. EP-023–EP-026 remain 🔵 Draft.)*

# Active Epics

## EP-019: [Apple] Custom Undo/Redo History & Multiple Copy Buffers — RESUMED

> **Un-deferred 2026-07-24 (user request):** moved back from the backlog to Active to **lock the copy-buffer
> implementation on Apple first**, so it becomes the reference the Linux side (EP-026) mirrors rather than
> being designed twice. Resumes at **SP-056** (multiple copy buffers → AC6). The undo/redo *engine* (C++
> `HistoryService` + C ABI + Swift wrappers) is delivered & verified (AC1/AC3/AC4/AC5); remaining:
> **AC6 (copy buffers, SP-056)**, then AC2/AC7/AC8 (SP-057 verify + close).

---

> **Codebase:** `[Apple]` — the undo/redo engine (ScriviCore `HistoryService` + C ABI) is delivered &
> verified; all remaining work (AC2/AC6/AC7/AC8) is Apple-platform UI. The parallel Linux undo/redo UI
> is a separate `[Linux]` Epic.

**Status:** 🟡 Active — **HELD PENDING** its final sprint SP-057 (user decision 2026-07-27). SP-051–SP-056 ✅ all
closed: AC1 (2026-07-07), AC3+AC5 (2026-07-09), AC4+AC5-branch (2026-07-13), **AC6 (2026-07-27, SP-056 — copy
buffers)**. Remaining: **SP-057** verifies AC2/AC7/AC8, adds the history panel (T-0215) + perf fixtures (T-0216),
then closes the Epic. Held (not continued immediately) so the newly-opened **EP-029** `[Cross]` (cross-boundary
structured Cut/Copy/Paste — the gap surfaced while verifying SP-056) can be planned/tackled first, per the user.
**Goal:** Replace the broken native undo (I-0019) with a from-scratch, sentence-granular undo/redo system backed by a tree-structured, per-project, on-disk persistent history (cross-session undo with session-boundary warning; branching with primary-line selection; capacity eviction and stale-branch purge), plus vim/emacs-register-style multiple copy buffers whose pastes are history events.
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (v0.1 ✅ Approved baseline 2026-07-06 — trades ruled: T1=B, T2=A+refinements, T3=C, T4=B+D+A, T5=C, T6=A)
**Supersedes:** I-0019 (Undo/Redo have no effect — ⚪ Closed 2026-07-06, OBE/superseded by this Epic, user-approved; see `docs/Issues/Closed/Issue-closed-0019.md`. AC1 carries the requirement; delivery target SP-053. Re-open I-0019 if EP-019 is cancelled or AC1 descoped.)
**Date Created:** 2026-07-06
**Target Close Date:** TBD (7 sprints)
**Actual Close Date:** —

### Acceptance Criteria

- [x] AC1 — ⌘Z/⇧⌘Z work in the macOS manuscript editor: repeated ⌘Z walks back one history event at a time; ⇧⌘Z re-applies (**delivers the fix formerly tracked as I-0019**). ✅ **Verified live 2026-07-07** (SP-053; `Tasks/Verified/Task-verified-0204-0206.md`).
- [ ] AC2 — Events commit exactly per the design's event model (`.` `!` `?`, Return, cursor-move-with-pending-changes, paste/cut, scene switch, flush); cursor moves/newlines without text changes produce **no** event.
- [x] AC3 — History persists across quit/relaunch; undoing past the session boundary shows a warning (once per crossing) before proceeding. ✅ **Verified 2026-07-09** (SP-054; `Tasks/Verified/Task-verified-0207-0209.md`).
- [x] AC4 — Undo-then-type creates a branch; the new line becomes primary; the old branch is selectable at the fork and becomes primary when selected; abandoned text fully restorable. ✅ **Verified 2026-07-13** (SP-055; `Tasks/Verified/Task-verified-0210-0212.md`).
- [x] AC5 — History capacity configurable (per Trade T1); oldest events fall off at capacity; branches auto-purge when their branch point ages off; stale branches detectable and purgeable with user confirmation. ✅ **Verified 2026-07-09** (SP-054 — capacity config + linear eviction of the root→current path; `Tasks/Verified/Task-verified-0207-0209.md`). ✅ **Branch clauses verified 2026-07-13** (SP-055 — branch-aware auto-purge on eviction + stale-branch detection/user-confirmed purge; `Tasks/Verified/Task-verified-0210-0212.md`).
- [x] AC6 — Copy buffers: ≥ 2 buffers loadable and pasteable at multiple locations (design CONOPS §9.a); each paste is one undo step; system pasteboard unaffected; buffers persist across relaunch. ✅ **Verified live 2026-07-27** (SP-056; T-0213 + T-0214). Delivered as explicit chords **⌘1–9 copy / ⌃1–9 paste / ⌥1–9 cut** (design refinement, user-approved 2026-07-25/27 — the originally-designed single context-sensitive chord + ⌥⌘C/⌥⌘V HUD was replaced; buffer 0 = the system pasteboard). Palette (app-global, per-project reload), Edit-menu + Scene/Chapter menu items, cut-into-buffer `cut` event tagged with bufferID (backend schema extended).
- [ ] AC7 — Structural operations record barriers; undo stops at a barrier with a clear notice; no text corruption.
- [ ] AC8 — No regression: auto-save, scene navigation, structure ops, external-change scan, Git snapshots unchanged; backend `ctest` + interop suites green.

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-051 | Design sign-off, ⌘Z-routing spike, schema spec | ✅ Closed (user-approved) | 2026-07-06 – 2026-07-06 |
| SP-052 | Linear history engine core (C++) + C ABI + Swift wrappers | ✅ Closed (user-approved) | 2026-07-07 – 2026-07-07 |
| SP-053 | In-session undo/redo on macOS (capture, apply, barriers) — AC1 (ex-I-0019) delivered here | ✅ Closed (user-approved) | 2026-07-07 – 2026-07-07 |
| SP-054 | Persistence, sessions, capacity, settings | ✅ Closed (user-approved) — AC3 + AC5 | 2026-07-07 – 2026-07-09 |
| SP-055 | Branching — tree ops, fork popover, purge | ✅ Closed (user-approved) — AC4 + AC5 branch clauses | 2026-07-10 – 2026-07-13 |
| SP-056 | Multiple copy buffers | ✅ Closed (user-approved) — **AC6** | 2026-07-24 – 2026-07-27 |
| SP-057 | History panel, performance fixtures, verification & Epic close | 🔵 Planning (**EP-019 held pending this sprint**) | — |

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0198 | Design doc + trade studies (this Epic's design) | SP-051 | ✅ Verified (2026-07-06) → `Verified/Task-verified-0198.md` |
| T-0199 | Spike: ⌘Z/⇧⌘Z + Edit-menu routing mechanism | SP-051 | ✅ Done (2026-07-06) — proxy rejected; action-method mechanism confirmed live; design §8/§12.6 updated |
| T-0200 | `scrivi.history.v1` / `scrivi.buffers.v1` schema spec + repair-matrix row | SP-051 | ✅ Verified (2026-07-06) → `Verified/Task-verified-0200.md` |
| T-0201 | `HistoryService` core (record/undo/redo, snapshot-diff, sessions) + unit tests | SP-052 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0201-0203.md` |
| T-0202 | C ABI: `scrivi_history_open/record_event/record_barrier/undo/redo/close` | SP-052 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0201-0203.md` |
| T-0203 | `ScriviEngine.swift` history wrappers + interop tests | SP-052 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0201-0203.md` |
| T-0204 | `HistoryCapture` + commit-trigger wiring in the editor | SP-053 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0204-0206.md` |
| T-0205 | Undo/redo apply path + `allowsUndo=false` + ⌘Z routing | SP-053 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0204-0206.md` |
| T-0206 | Barriers on structural operations | SP-053 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0204-0206.md` |
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | SP-054 | ✅ Verified (2026-07-09) |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | SP-054 | ✅ Verified (2026-07-09; linear eviction, full in SP-055) |
| T-0209 | Session-boundary warning popup | SP-054 | ✅ Verified (2026-07-09) |
| T-0210 | Tree ops: branching, primary-child, `select_branch`, auto-purge on eviction | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0211 | Inline fork popover (T2 core interaction) | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0212 | Stale-branch detection + user-confirmed purge | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | SP-056 | ✅ **Verified (2026-07-27)** |
| T-0214 | Buffer UX: palette + Edit/Scene/Chapter menu items + explicit ⌘/⌃/⌥1–9 chords; paste/cut history integration (cut tagged w/ bufferID) | SP-056 | ✅ **Verified (2026-07-27)** |
| T-0215 | History panel (T2 management surface) | SP-057 | 🔵 Backlog |
| T-0216 | Performance/integration fixtures (100k events, 500 KB scene); gitignore migration | SP-057 | 🔵 Backlog |
| T-0217 | Doc updates + EP-019 acceptance-criteria verification | SP-057 | 🔵 Backlog |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| I-0019 | Undo and Redo have no effect in the manuscript editor | ⚪ Closed 2026-07-06 — OBE/superseded by this Epic (user-approved); requirement lives on as AC1 → `docs/Issues/Closed/Issue-closed-0019.md` |

### Scope Notes

- Structural undo (scene/chapter create/delete/merge/reorder) is explicitly **out of scope** — those operations record history *barriers* in v1 (design §4.5); full structural undo is a documented future extension.
- iOS/iPadOS capture layer deferred (editor is a stub); the C++ engine and ABI are platform-neutral by construction.
- All six trade-study decisions **approved by the user 2026-07-06**: T1=B (capacity in Project
  settings), T2=A with interaction refinements (popover on undo-landing-at-fork, suppressed when
  undoing past, immediate on redo, redo-past takes primary, branch relegation — design §10 T2, §5),
  T3=C (copy-into-buffer not an event; cut is), T4=B+D with A for discoverability, T5=C
  (project-open session + idle rollover), T6=A (history inside package, gitignored).

### Completion Summary

_(filled in when the Epic reaches 🟠 Complete)_


---

## EP-029: [Cross] Cross-Boundary Structured Cut / Copy / Paste

**Status:** 🟡 Active (opened 2026-07-27, user request) — **SP-085 ✅ closed 2026-07-27 (Human-approved).** Design
doc `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md` **✅ APPROVED**; trades ruled **T1=A · T2=A · T3=A ·
T4=A**; Open Questions #1–#3 resolved (incl. caret-in-heading paste = **refuse + flash**, user override).
**Next: SP-086** (ScriviCore extract-fragment) — awaiting go-ahead to activate.
**Codebase:** `[Cross]` — the core capability (fragment model, extract, cut-merge, paste-splice) lives in
**ScriviCore** and is reused by every platform; each platform then wires its editor's Cut/Copy/Paste + copy
buffers through it. This Epic delivers the **Apple** wiring first (mirroring EP-019's "Apple as reference"
decision); Linux/Windows wiring follow as they mature.
**Goal:** Make the manuscript behave as **one monolithic document** for Cut/Copy/Paste and the copy buffers.
A selection spanning scene/chapter boundaries copies/cuts as a **structured fragment** (carrying its scene &
chapter boundary markers); **cut deletes and merges** the spanned scenes/chapters (history records it); **paste
reconstructs** all carried boundaries at the destination, **splitting the target scene** if pasted mid-scene —
exactly as if the manuscript were a single continuous document.

**Origin:** Gap surfaced 2026-07-27 while verifying EP-019 SP-056 (T-0214). The manuscript is one
`NSTextStorage` with divider attachments between scenes and non-editable heading runs at chapter starts; today a
cross-boundary selection can't be cleanly copied (dividers/headings pollute the extracted string), ⌘X is blocked
by the heading-edit guard (silent no-op), and the copy buffers store only flat text. EP-019 explicitly scopes
structural editing OUT (barriers only), so this is a **new** Epic, not an EP-019 addendum. User decision
2026-07-27: full structured round-trip (structured buffers, cut-that-merges, paste-that-splits).

**Design:** `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md` — **TO BE WRITTEN** (SP-085 / T-0350). Will
define the fragment schema (`scrivi.fragment.v1`), extract / cut-merge / paste-splice behaviour, history
integration, the copy-buffer schema evolution (`scrivi.buffers.v1` → structured), and the trade studies below.
**Date Created:** 2026-07-27
**Target Close Date:** TBD (est. 5 sprints)
**Actual Close Date:** —

### Acceptance Criteria (draft — finalised in the design doc)

- [ ] AC1 — **⌘C across boundaries** places a structured fragment (scene/chapter markers + text) on an internal
  clipboard; the system pasteboard also gets a clean plain-text flattening (blank-line seams) for external apps.
- [ ] AC2 — **⌘V of a structured fragment** reconstructs every carried scene/chapter boundary at the caret,
  splitting the target scene when pasted mid-scene; identity/order-keys resolved via the ScriviCore model
  (EP-027) and the merge/split primitives (EP-028).
- [ ] AC3 — **⌘X across boundaries** copies the structured fragment, deletes the spanned text, and **merges** the
  spanned scenes/chapters into one; recorded as one undoable structural operation (history barrier + events).
- [ ] AC4 — **Copy buffers hold structured fragments** (not just flat text): ⌘1–9 copy / ⌥1–9 cut across
  boundaries store structure; ⌃1–9 paste reconstructs it. `buffers.json` schema evolves accordingly.
- [ ] AC5 — **Within a single scene**, Cut/Copy/Paste and the buffers keep the existing fast plain-text path
  (no regression to the EP-019 SP-056 behaviour the user verified 2026-07-27).
- [ ] AC6 — **Undo/Redo** of a cross-boundary cut restores the text AND the pre-merge scene/chapter structure;
  undo of a structured paste removes the created scenes/chapters. No text or structure corruption.
- [ ] AC7 — **No regression:** auto-save, scene navigation, external-change scan, Git snapshots, and the EP-019
  history engine are unaffected; ScriviCore `ctest` + interop suites green.

### Sprints (planned — sequencing below; activated one at a time on user go-ahead)

| Sprint | Title | Codebase | Status |
| ------ | ----- | -------- | ------ |
| SP-085 | **Design doc + trade studies + fragment schema spec** | `[Cross]` (docs) | ✅ **Closed 2026-07-27** — doc approved; T1–T4 ruled (all A) → `Closed/Sprint-SP-085.md` |
| SP-086 | **ScriviCore: `scrivi.fragment.v1` model + extract-fragment** (manuscript range → structured fragment) + C ABI + unit/integration tests | `[ScriviCore]` | ✅ **Closed 2026-07-27** — T-0351 Verified; ctest 338/338 → `Closed/Sprint-SP-086.md` |
| SP-087 | **ScriviCore: paste-splice** (insert a fragment at a caret; split target scene mid-scene; create carried scenes/chapters; wire identity/order-keys via EP-027/EP-028 primitives) + tests | `[ScriviCore]` | ✅ **Closed 2026-07-27** — T-0352 Verified; ctest 346/346 → `Closed/Sprint-SP-087.md` |
| SP-088 | **ScriviCore: cut-with-merge** (delete a spanned range + merge spanned scenes/chapters atomically) + history event/barrier shape + tests | `[ScriviCore]` | ✅ **Closed 2026-07-27** — T-0353 Verified; ctest 352/352 → `Closed/Sprint-SP-088.md` |
| SP-089 | **`[Apple]` editor wiring** — route ⌘C/⌘X/⌘V + ⌘/⌃/⌥1–9 through the fragment API when a selection/paste crosses boundaries; keep the single-scene fast path; structured buffer storage; history integration; verify | `[Apple]` | 🟡 **Active** (2026-07-27) — T-0354 |

### Tasks

_Assigned at each sprint's activation. First task **T-0350** (design doc, SP-085). Task IDs continue from T-0350._

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0350 | Design doc: fragment schema (`scrivi.fragment.v1`), extract/cut-merge/paste-splice behaviour, buffer-schema evolution, trade studies (T1–T4 below), milestone breakdown | SP-085 | ✅ **Verified (2026-07-27)** — `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md` **approved**; T1–T4 ruled (all A); Open Questions #1–#3 resolved |
| T-0351 | `[ScriviCore]` **extract-fragment** — `FragmentExtractor` (manuscript range → `scrivi.fragment.v1` via `ManuscriptOrderResolver` + `SceneReader`) + `scrivi_fragment_extract` C ABI + `FragmentExtractTests` | SP-086 | ✅ **Verified (2026-07-27)** — ctest 338/338 (+11); macOS build green; `scrivi.h` additive; no pbxproj |
| T-0352 | `[ScriviCore]` **paste-splice** — `FragmentPaster` (split target scene at caret; create carried scenes via `createScene(afterSceneID)` + chapters via `createChapter(afterChapterID)`; tail-suffix follows the pasted run, flat-doc model; direct in-scene concatenation) + `scrivi_fragment_paste` C ABI + tests | SP-087 | ✅ **Verified (2026-07-27)** — ctest 346/346 (+8); macOS build green; `scrivi.h` additive; no pbxproj |
| T-0353 | `[ScriviCore]` **cut-with-merge** — `FragmentCutter` (extract the fragment + delete the spanned byte ranges + collapse the spanned scenes/chapters into one via **delete-and-fold**: fold head=headPrefix+tailSuffix, delete other span scenes, remove emptied chapters) + `scrivi_fragment_cut` C ABI + tests | SP-088 | ✅ **Verified (2026-07-27)** — ctest 352/352 (+6); macOS build green; `scrivi.h` additive; no pbxproj |
| T-0354 | `[Apple]` **Pass A — system-clipboard cross-boundary Cut/Copy/Paste** — `ScriviEngine` fragment wrappers (extract/paste/cut); coordinator boundary-detection (selection → per-scene byte spans from `sceneBoundaries`); ⌘C/⌘X/⌘V route through the fragment endpoints on boundary-crossing (internal clipboard + flat `plainText` on `NSPasteboard`); single-scene fast path preserved; caret-in-heading paste = flash + refuse; **structural barrier** on cross-boundary cut/paste; reload manuscript from disk after structural ops. AC1/AC2/AC3/AC5/AC7 | SP-089 | 🟡 **In progress** (2026-07-27) |
| T-0356 | `[ScriviCore]`+`[Apple]` **Reversible structured undo (AC6)** — extend the C++ `HistoryService` so undo steps past a `structuredCut`/`structuredPaste` barrier and re-runs the inverse op (undo-cut = paste the fragment back; undo-paste = cut the created span), redo re-runs forward; app-side inverse-op payloads + wiring + tests. **Dedicated task** (user decision 2026-07-27 — deeper than UI wiring; a real history-engine change done right, not rushed) | SP-089 | 🔵 Backlog (after T-0354 verifies) |
| T-0355 | `[Apple]` **Pass B — structured copy buffers** — extend `scrivi.buffers.v1` in place with an optional `fragment` (T4=A); `scrivi_buffers_load` carries it; `BufferService`/palette + ⌘1–9/⌃1–9/⌥1–9 store/reconstruct structured fragments across boundaries (single-scene stays flat). AC4 | SP-089 | 🔵 Backlog (after Pass A verifies) |

### Trade studies — ✅ ALL RULED 2026-07-27 (T1=A · T2=A · T3=A · T4=A)

- **T1 — Fragment format.** How a structured selection is serialised: (A) a JSON fragment
  (`scrivi.fragment.v1`: ordered `[{chapterBoundary?, sceneBoundary, headOrTailPartial?, text}]`) vs (B) a
  lightweight in-band marker string vs (C) reuse the on-disk scene/chapter layout as the transport.
- **T2 — Internal clipboard vs system pasteboard.** Where the structured fragment lives on ⌘C: (A) a
  ScriviCore-owned internal clipboard with the system pasteboard carrying only the flat-text flattening (so
  external paste still works) vs (B) a custom `NSPasteboard` UTI carrying the fragment vs (C) both.
- **T3 — Cut-merge undo granularity.** Whether a cross-boundary cut is one undo step (single structural event
  that restores text+structure) vs a barrier + sub-events, and how it composes with the EP-019 history engine.
- **T4 — Copy-buffer schema evolution.** Whether structured fragments extend `scrivi.buffers.v1` in place
  (add an optional `fragment` alongside `text`) or bump to `scrivi.buffers.v2`; plaintext slots must keep working.

### Scope Notes

- **Reuses, does not duplicate, EP-027 (identity/ordering) and EP-028 (`SceneMerger`/`ChapterMerger`).** Cut-merge
  composes the existing merge primitives across a multi-scene span; paste-splice composes create + the split path.
- **Apple wiring first** (this Epic's AC1–AC7 target `[Apple]`), mirroring the EP-019 decision to lock the Apple
  reference before the Linux mirror. Linux/Windows wiring are follow-on Epics once those editors reach parity.
- **The EP-019 SP-056 single-scene buffer behaviour (Verified 2026-07-27) must not regress** — the fragment path
  engages only when a selection or paste actually crosses a boundary (AC5).

### Completion Summary

_(filled in when the Epic reaches 🟠 Complete)_


---

*Last Updated: 2026-07-27 (**SP-085 ✅ closed (Human-approved) — EP-029 design doc APPROVED.** The EP-029
`[Cross]` design sprint delivered + got approval for `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md`: the
`scrivi.fragment.v1` ordered-pieces schema, extract / paste-splice / cut-merge behaviour (composing EP-027
create/split + EP-028 `SceneMerger`/`ChapterMerger`), buffer-schema evolution (extend `scrivi.buffers.v1` in
place), and a one-reversible-event history shape (`structuredCut`/`structuredPaste`, undo = inverse op). Trades
ruled **T1=A · T2=A · T3=A · T4=A**; Open Questions #1–#3 resolved — no cross-window/cross-project structured
paste in v1; **caret-in-heading paste = refuse + flash the screen** (user override of the drafted silent-retarget);
divider-anchored selection normalised to the adjacent scene body. Task T-0350 Verified; docs-only. **EP-029 → SP-086**
(ScriviCore extract-fragment) next, awaiting go-ahead. Epics Active 1 (EP-029) + held/draft (EP-019 held, EP-026
Linux draft). Next available Sprint **SP-086**, Task **T-0351**. Prior note follows.)*

*2026-07-27 (**SP-056 ✅ closed (Human-approved) — AC6 Verified; EP-019 held pending SP-057; new
Epic EP-029 opened.** T-0213 + T-0214 both ✅ Verified live: multiple copy buffers delivered as explicit
⌘1–9/⌃1–9/⌥1–9 (copy/paste/cut) chords + app-global per-project palette + Edit/Scene/Chapter menu items +
bufferID-tagged cut event (backend schema extended); ctest 327 + interop 43 green. AC6 met. EP-019 → **held
pending** its final sprint SP-057 (AC2/AC7/AC8 verify + history panel + close). **EP-029** `[Cross]`
(cross-boundary structured Cut/Copy/Paste) opened to capture the gap surfaced during SP-056 verification —
manuscript-as-monolithic-document copy/cut/paste with structured buffers, cut-that-merges, paste-that-splits;
5 sprints planned (SP-085 design → SP-086 extract → SP-087 paste-splice → SP-088 cut-merge → SP-089 Apple wiring);
design doc to be written & approved first (T-0350). Epics Active 1 (EP-019, held) + Draft 2 (EP-026 Linux, EP-029);
next available Epic EP-030, Sprint SP-085, Task T-0351. Prior note follows.)*

---

*Last Updated: 2026-07-24 (**EP-025 ✅ CLOSED (Human-approved) — the final sprint SP-084 delivered AC6b and
closed the Epic.** SP-084 (co-located dot **clustering** — aggregate dot: larger core + count + segmented arc
ring + selection arc; hover fan-out; zoom-resolve) + a persistence sweep + full EP-025 verify; T-0346–T-0349 all
Verified live over VNC. Four clustering findings surfaced + fixed + re-verified same session (larger-diameter
aggregate-of-aggregates; fan overlay + grey backing; tighter dismiss; the **phantom double-draw** defect — a
fanned aggregate's members drew twice because the skip-set excluded the fanned aggregate; fixed so all members
are skipped from the baseline loops and the ring is the sole draw). Final container build **green (216/216)** +
new `timeline_cluster_smoke` + 14 regression smokes + app-launch PASS; `scrivi.h` untouched; no pbxproj
(Linux-only). **EP-025 archived to `Closed/Epic-EP-025.md`; Active Epics 1→0.** The full six-sprint completion
summary lives in the archive. Prior note follows.)*

*2026-07-24 (**EP-025 SP-082 ✅ closed (Human-approved) + SP-084 planned + activated** — the final sprint of
EP-025. SP-082 delivered **AC5** (historical events + imported timelines + export + File-menu items,
T-0340–T-0345 all Verified; I-0090/I-0091 fixed + re-verified; container green 211/211 + `timeline_events_smoke`
+ 11 smokes PASS; archived `Closed/Sprint-SP-082.md`). SP-084 then delivered **AC6b** clustering (above) and
closed the Epic. Prior EP-025 history — SP-079 [AC1/AC2], SP-080 [AC3], SP-081 [AC4], SP-083 [AC6a] — lives in
`Closed/Epic-EP-025.md`.)*

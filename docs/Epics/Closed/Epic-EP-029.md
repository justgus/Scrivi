# EP-029 (Closed)

## EP-029: [Cross] Cross-Boundary Structured Cut / Copy / Paste

**Status:** ✅ Closed (Human-approved 2026-08-03)
**Goal:** Make the manuscript behave as **one monolithic document** for Cut/Copy/Paste and the copy buffers.
A selection spanning scene/chapter boundaries copies/cuts as a **structured fragment** (carrying its scene &
chapter boundary markers); **cut deletes and merges** the spanned scenes/chapters (history records it reversibly);
**paste reconstructs** all carried boundaries at the destination, **splitting the target scene** if pasted
mid-scene — exactly as if the manuscript were a single continuous document.
**Design:** `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md` (✅ Approved 2026-07-27; trades T1=A · T2=A ·
T3=A · T4=A).
**Codebase:** `[Cross]` — the core capability (fragment model, extract, cut-merge, paste-splice, uncut-paste)
lives in **ScriviCore** and is reused by every platform; the **Apple** editor wiring was delivered first
(mirroring EP-019's "Apple as reference" decision). Linux/Windows wiring are follow-on Epics.
**Date Created:** 2026-07-27
**Target Close Date:** TBD (est. 5 sprints)
**Actual Close Date:** 2026-08-03

### Origin

Gap surfaced 2026-07-27 while verifying EP-019 SP-056 (T-0214). The manuscript is one `NSTextStorage` with
divider attachments between scenes and non-editable heading runs at chapter starts; a cross-boundary selection
couldn't be cleanly copied (dividers/headings polluted the extracted string), ⌘X was blocked by the heading-edit
guard (silent no-op), and the copy buffers stored only flat text. EP-019 explicitly scopes structural editing OUT
(barriers only), so this was a **new** Epic. User decision 2026-07-27: full structured round-trip (structured
buffers, cut-that-merges, paste-that-splits).

### Acceptance Criteria — ✅ ALL MET

- [x] AC1 — **⌘C across boundaries** places a `scrivi.fragment.v1` on the internal clipboard; the system
  pasteboard also gets a clean plain-text flattening (blank-line seams) for external apps. (T-0354, Verified
  2026-07-29.)
- [x] AC2 — **⌘V of a structured fragment** reconstructs every carried scene/chapter boundary at the caret,
  splitting the target scene mid-scene; identity/order-keys via EP-027, merge/split via EP-028. (T-0354, Verified
  2026-07-29.)
- [x] AC3 — **⌘X across boundaries** copies the fragment, deletes the span, and **merges** the spanned
  scenes/chapters into one atomically; recorded as one reversible structural operation. (T-0354, Verified
  2026-07-29.)
- [x] AC4 — **Copy buffers hold structured fragments** — ⌘1–9 copy / ⌥1–9 cut across boundaries store structure;
  ⌃1–9 paste reconstructs it; `buffers.json` evolved per T4=A (optional `fragment`, additive). (T-0355, Verified
  2026-08-03.)
- [x] AC5 — **Within a single scene**, Cut/Copy/Paste and the buffers keep the existing fast plain-text path (no
  regression to the EP-019 SP-056 behaviour). The fragment path engages only when a selection/paste actually
  crosses a boundary. (T-0354/T-0355, Verified.)
- [x] AC6 — **Undo/Redo** of a cross-boundary cut restores text AND pre-merge structure; undo of a structured
  paste removes the created scenes/chapters. No text/structure corruption. (T-0356, Verified 2026-08-03.)
- [x] AC7 — **No regression:** auto-save, scene navigation, external-change scan, Git snapshots, and the EP-019
  history engine unaffected; ScriviCore `ctest` + interop suites green (369/369 + 45/45 at close). (Verified.)

### Sprints (all closed)

| Sprint | Title | Codebase | Result |
| ------ | ----- | -------- | ------ |
| SP-085 | Design doc + trade studies + fragment schema spec | `[Cross]` (docs) | ✅ Closed 2026-07-27 — T-0350; `Closed/Sprint-SP-085.md` |
| SP-086 | ScriviCore `scrivi.fragment.v1` model + extract-fragment + C ABI | `[ScriviCore]` | ✅ Closed 2026-07-27 — T-0351; ctest 338/338; `Closed/Sprint-SP-086.md` |
| SP-087 | ScriviCore paste-splice (`scrivi_fragment_paste`) | `[ScriviCore]` | ✅ Closed 2026-07-27 — T-0352; ctest 346/346; `Closed/Sprint-SP-087.md` |
| SP-088 | ScriviCore cut-with-merge (`scrivi_fragment_cut`) | `[ScriviCore]` | ✅ Closed 2026-07-27 — T-0353; ctest 352/352; `Closed/Sprint-SP-088.md` |
| SP-089 | `[Apple]` editor wiring — Pass A (clipboard), Pass B (buffers), AC6 undo, verify | `[Apple]` | ✅ Closed 2026-08-03 — T-0354/T-0355/T-0356/T-0357; ctest 369/369, interop 45/45; `Closed/Sprint-SP-089.md` |

### Tasks (all Verified)

| ID | Title | Sprint | Verified |
| -- | ----- | ------ | -------- |
| T-0350 | Design doc: fragment schema, extract/cut-merge/paste-splice behaviour, buffer-schema evolution, trades T1–T4 | SP-085 | 2026-07-27 |
| T-0351 | `[ScriviCore]` extract-fragment (`FragmentExtractor` + `scrivi_fragment_extract`) | SP-086 | 2026-07-27 |
| T-0352 | `[ScriviCore]` paste-splice (`FragmentPaster` + `scrivi_fragment_paste`) | SP-087 | 2026-07-27 |
| T-0353 | `[ScriviCore]` cut-with-merge (`FragmentCutter` + `scrivi_fragment_cut`) | SP-088 | 2026-07-27 |
| T-0354 | `[Apple]` Pass A — system-clipboard cross-boundary Cut/Copy/Paste + barrier history (AC1/2/3/5/7) | SP-089 | 2026-07-29 |
| T-0357 | `[Apple]` Title-capture cut/paste + chapter promotion (cross-chapter cut promotes survivors; fragment captures titles) | SP-089 | 2026-07-29 |
| T-0355 | `[Apple]` Pass B — structured copy buffers (AC4; optional `fragment` in `scrivi.buffers.v1`, T4=A) | SP-089 | 2026-08-03 |
| T-0356 | `[ScriviCore]`+`[Apple]` Reversible structured undo (AC6) — app-runs-inverse; `structural` node kind + `scrivi_fragment_uncut_paste` | SP-089 | 2026-08-03 |

### Completion Summary

EP-029 delivered manuscript-as-monolithic-document Cut/Copy/Paste + copy buffers across five sprints.

**ScriviCore (SP-086–088, + the AC6 endpoint in SP-089):** a `scrivi.fragment.v1` ordered-pieces fragment
model; `FragmentExtractor` (range → fragment), `FragmentPaster` (paste-splice: split target, create carried
scenes/chapters via EP-027 primitives, tail follows the run), `FragmentCutter` (cut-with-merge: extract +
delete-and-fold + chapter promotion T-0357), and `FragmentPaster::uncutPaste` (the exact inverse of paste — folds
created scenes back into target, strips pasted piece texts by length, deletes created scenes/chapters). C ABI:
`scrivi_fragment_extract`/`_paste`/`_cut`/`_uncut_paste` — all additive, pure C ABI.

**Apple (SP-089):** `ScriviEngine` fragment wrappers; coordinator boundary-detection (selection → per-scene byte
spans); ⌘C/⌘X/⌘V + ⌘/⌃/⌥1–9 route through the fragment API on boundary-crossing, single-scene keeps the fast
plain-text path (AC5); caret-in-heading paste refuses + flashes; structured copy buffers (optional `fragment` in
`scrivi.buffers.v1`); and **reversible structured undo (AC6)** via app-runs-inverse — `HistoryService` gains a
reversible `structural` node kind carrying an opaque inverse payload, undo/redo step across it returning a
`structuralInverse` directive, and the app replays the inverse fragment op (`fragmentPaste` / `fragment_uncut_paste`)
+ manuscript reload.

**Key rulings:** T1=A (JSON fragment) · T2=A (internal clipboard; no cross-window structured paste in v1) · T3=A
(one reversible structural event) · T4=A (extend `scrivi.buffers.v1` in place). Caret-in-heading paste = refuse +
flash (user override). AC6 architecture: **app-runs-inverse**, keeping `HistoryService` in-memory/filesystem-free
(user-ruled 2026-08-03); the inverse-of-paste strips pasted piece texts inside core by length (byte-safe, no
app-side byte-span math).

**Issues fixed along the way:** I-0092 (New Project `.scrivi` extension), I-0093 (project title from
`project.json`), I-0094 (empty-array JSON-decode mismatch that sank the structured cut), I-0095 (heading refresh
after rename), I-0096 (⌘↩ at scene start mislabels scenes), I-0097 (full-screen restore lost), I-0098 (Swift 6
Sendable-closure warnings) — all Verified.

At close: ScriviCore `ctest` **369/369**, macOS interop **45/45**, macOS app **BUILD SUCCEEDED**.

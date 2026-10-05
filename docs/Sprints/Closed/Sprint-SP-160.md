---
sprint: SP-160
epic: EP-049
status: Closed
closed: 2026-10-04
activated: 2026-10-04
platform: Linux
created: 2026-10-04
---

# SP-160 — `[Linux]` [EP-049] S1: Linux writes Apple's manuscript format (the escape layer's write half)

**Status:** ✅ **CLOSED 2026-10-04 (user-approved):** *"close SP-160, run the Audit Check, and close EP-049"* ✅ [T-0586] and [I-0276] verified.
needs the user's approval, and it ACTIVATES [EP-049] (backlog → `Epic-active.md`).
**Epic:** [EP-049] → [`../../Epics/Closed/Epic-EP-049.md`](../../Epics/Closed/Epic-EP-049.md) — this Sprint carries ALL of its ACs.
**Task:** [T-0586] → [`../../Tasks/Verified/Task-verified-0586.md`](../../Tasks/Verified/Task-verified-0586.md)
**The maxim:** ✅ *"do what Apple does, the way Apple does it"* — the Apple reference for every step is named below.
**Size:** ⚠️ **MEDIUM–LARGE.** It is small in code, but it changes what Linux WRITES, so every step is guarded by
a byte-level test.

---

## ✅ Read in the code at planning (2026-10-04)

| Linux today (`platforms/linux/src/ManuscriptEditor.cpp`) | Consequence for this Sprint |
| -------------------------------------------------------- | --------------------------- |
| Every typed key goes through `keyPressEvent` (`:120`). The edit GUARD (`:188-194`) refuses anything touching a scene boundary, then hands the key to `QPlainTextEdit` (`:194`) | ✅ ONE hook for typing, Return and Backspace, after the guard, as Apple's overrides sit behind `shouldChangeText` |
| Paste and drop: `insertFromMimeData` (`:197`), guarded the same way | ✅ ONE hook for the escaping of foreign text (Apple: `readSelection`) |
| ⛔ Copy, cut and drag are NOT overridden: `QPlainTextEdit` builds the clipboard data itself (`createMimeDataFromSelection`) | ⚠️ A new override (Apple: `writeSelection`) |
| ⛔ No input-method override (`inputMethodEvent`) | ⚠️ Composed characters (dead keys, IME) bypass `keyPressEvent` → a second typing hook (Apple's `insertText` covers both) |
| ⛔ Undo is OFF (`:25`, deferred to EP-026) | ⚠️ No undo target. ✅ Each gesture is still ONE edit block, so EP-026 inherits Apple's shape |
| ⚠️ **Ctrl+Backspace = merge scene** (`:147-163`) — Linux's word-delete-backward gesture is taken | ⚠️ Pair widening must cover the deletes that DO exist (measure which, M3) |
| ⚠️ **The backslash is VISIBLE on Linux** (no display half), so the caret CAN rest between `\` and `*` | ⛔ Typing there breaks the escape and makes the `*` LIVE. ✅ Apple prevents it with the caret snap; Linux's write half must prevent it at insertion → **AC4b** |
| Scenes are bodies separated by blank gap blocks (`SceneDocument`), protected by `isEditableRange` / `normalizeCaret` | ✅ Return and the ⌫-join run only INSIDE a body, behind the existing guard, exactly as Apple's sit behind `isSeparatorPosition` |

### ✅ MEASURED 2026-10-04 (step 1 — an offscreen probe on Qt 6.4.2, the rig's version)

| # | Question | ⚠️ Belief at planning | ✅ MEASURED |
| - | -------- | --------------------- | ----------- |
| **M1** | What do Shift-Return, keypad Enter and Alt-Return insert in `QPlainTextEdit`? | Shift-Return inserts U+2028 (LINE SEPARATOR), not a newline | ✅ **Confirmed** — Shift-Return AND Shift+keypad Enter insert U+2028, and ⛔ `SceneDocument::bodyText` KEEPS it, so **Linux wrote U+2028 into scene files**. ⚠️ Alt-Return inserted NOTHING (Apple's Option-Return stores one `\n`) |
| **M2** | Do dead-key / IME compositions arrive via `inputMethodEvent` and bypass `keyPressEvent`? | Yes | ✅ **Confirmed** — commits go ONLY to `inputMethodEvent`; a committed `*` was stored unescaped |
| **M3** | Which deletion gestures exist on Linux (Delete, Ctrl+Delete word-forward, Ctrl+K, …), and which bypass `modifiedRangeFor`'s one-character range? | Ctrl+Delete deletes a word inside the base class, so the guard sees only 1 character | ✅ **Worse than believed** — Ctrl+Delete never reached the guard AT ALL (`keyPressEvent` handed it straight to Qt). Gestures: Backspace, Shift+Backspace (1 char), Delete, Ctrl+Delete (word), Cut = Ctrl+X / Shift+Del. Ctrl+K does nothing |
| **M4** | Does `toPlainText()` / the save path turn a block break into exactly one `\n`, so `\n\n` = two block breaks round-trips byte-exact? | Yes (U+2029 → `\n`) | ✅ **Confirmed** — `\n\n` is two block breaks and round-trips byte-exact |

---

## Plan — each step names its Apple reference

1. **Measure M1–M4** (an offscreen Qt smoke in Docker, the repo's existing pattern). Record the results here before any code.
2. **A Linux escape module** mirroring `ManuscriptEscapes.swift`: the 32-mark set, `escape`, and the source↔presented
   map with the hidden-backslash rule (incl. the hard break and Q1 = (a)). ✅ The SAME rules, ported. ⛔ Not a new
   design. ⚠️ The kind-list rule's lesson applies to the mark set: it must be checked against Apple's, not retyped
   from memory → the AC8 corpus.
3. **Typing** (`keyPressEvent` + `inputMethodEvent`): insert `escape(text)` through one `QTextCursor` edit. ⚠️ Apple: `insertText`.
4. **Insertion never splits a pair (AC4b):** an insertion at the boundary between `\` and its mark lands BEFORE the
   backslash. ⚠️ Apple: `presentedToSource` places the caret before the backslash.
5. **Pair deletion (AC4):** widen any deletion that would split a pair to the whole pair. ⚠️ Apple: `snapSelection` in `shouldChangeText`.
6. **Return (AC5)** and **the ⌫ join (AC6):** port `insertNewline` and `paragraphJoin` line for line. Shift-Return and
   keypad Enter → the same as Return (per M1). Alt-Return → per [T-0584] (⚠️ unruled — ship as Apple ships TODAY,
   a single `\n`, until it is ruled).
7. **Copy / cut / drag (AC3)** — override `createMimeDataFromSelection`: plain text = the PRESENTED text. **Paste /
   drop (AC2)**: foreign text escaped; Scrivi's OWN copy restored EXACTLY. ⚠️ Apple remembers the stored source keyed
   to its own pasteboard write (`ownCopy`). ✅ The Qt form of that same idea: the stored source travels ON Scrivi's
   own `QMimeData` write, in a private format, so a paste or drop of Scrivi's own data restores it verbatim.
   ⚠️ Same result, different mechanism → recorded under Q-E2-8's rule (the T-0576 precedent).
8. **The shared corpus (AC8):** `ScriviCore/tests/fixtures/manuscript_format_corpus.json`, with cases of the form
   {starting text, caret/selection, gesture, expected stored text}. ✅ The Linux smoke drives `ManuscriptEditor` with
   it. ✅ Apple's interop suite drives the real `ManuscriptNSTextView` with it. ⚠️ Apple's test host is SANDBOXED, so
   the file is bundled as a test RESOURCE (a `project.pbxproj` change — the Xcode rule applies).
9. **Build stamp:** `bump-build-stamp.sh` before the rig pass (the user updates the rig from git).

## Acceptance Criteria (= EP-049's)

- [x] **AC1** — Typed escaping, all 32 marks, including IME / dead-key input (M2).
- [x] **AC2** — Paste and drop escape foreign text; Scrivi's own copy pasted back is restored exactly (an intended `##` stays markup).
- [x] **AC3** — Copy, cut and drag put the un-escaped text on the clipboard.
- [x] **AC4** — An escape pair is deleted as ONE unit by every deletion gesture Linux has (M3) — never an orphaned `\` or a bare live mark.
- [x] **AC4b** — An insertion never lands between `\` and its mark.
- [x] **AC5** — Return stores `\n\n` (trim to ≤ 1 space; a trailing `\\` collapses to `\`); Shift-Return and keypad Enter the same (M1).
- [x] **AC6** — Backspace at a paragraph start joins with one space (Q3), with Apple's two exceptions.
- [x] **AC7** — Existing text is never rewritten; untouched scenes save byte-identical (M4).
- [x] **AC8** — The shared corpus passes on BOTH platforms: the same gestures produce the same bytes.
- [x] **AC-build** — The Docker build plus every Linux smoke green; the Apple interop suite green (the corpus resource); macOS / iOS / visionOS build.
- [x] **AC-live** — On the rig (build stamp confirmed): type and paste punctuation, Return, Shift-Return and Backspace; the scene file holds Apple's format; copy into another app gives clean text.

⚠️ **Interim, accepted (EP-049):** Linux SHOWS the backslashes until the display half ([SP-159] design → [EP-048]).
⛔ **NOT in this Sprint:** hiding or rendering (→ [SP-159] / [EP-048]); undo (→ EP-026); Find/Replace ([T-0585]).

---

## Progress log

### 🔵 2026-10-04 — Sprint CREATED in Planning; planning completed

✅ Created at the user's request: *"yes, and complete its planning."* ✅ IDs from `next-id.py` (SP-160, T-0586). ✅ The
Linux editor was read in full (`ManuscriptEditor.cpp`, 313 lines) and mapped to Apple's hooks (table above).
⚠️ **Found at planning:** the visible backslash lets the Linux caret sit inside an escape pair → AC4b, a write-half
rule Apple gets from its caret snap. ⚠️ M1–M4 could not be measured (Docker daemon not running) → the Sprint's
first step, with the beliefs marked as beliefs.

### 🟡 2026-10-04 — Sprint ACTIVATED (user-approved); [EP-049] ACTIVATED; [T-0586] → `Task-active.md`

✅ User: *"activate SP-160 and implement it"*

### 🟢 2026-10-04 — implemented ([T-0586]); ⚠️ AC-live needs the user's rig pass

✅ **Built** (`platforms/linux/src/`): `ManuscriptEscapes.{hpp,cpp}`, a port of `ManuscriptEscapes.swift` (the 32-mark
set, `escape`, `map`, `hiddenBackslashes`, Q1 = (a)). In `ManuscriptEditor`: typing, input-method commits,
Return / keypad Enter / Shift-Return (`\n\n` with Apple's trim and collapse), Alt-Return (one `\n`, as Apple's
Option-Return today), the ⌫ join (Q3), pair-widening on every deletion, AC4b, copy / cut / drag
(`createMimeDataFromSelection`: the presented text + the stored source in `application/x-scrivi-manuscript-source`),
and paste / drop (own copy restored exactly; foreign text escaped). Every handled gesture is one `QTextCursor` edit
block, behind the existing boundary guard.
⛔→✅ **Two pre-existing Linux defects found by the measurements and fixed in scope:** (1) **Shift-Return wrote U+2028
into scene files** (M1) — it now stores `\n\n`; (2) **Ctrl+Delete bypassed the boundary guard entirely** (M3,
`keyPressEvent` handed it straight to Qt) — the smoke caught it orphaning a `\`; it now takes the edit path.
✅ **AC8 — the SHARED corpus** `ScriviCore/tests/fixtures/manuscript_format_corpus.json` (18 cases; the `fixtures/`
folder CLAUDE.md names, created): ✅ **Linux** `escape_smoke` (real `ManuscriptEditor` over a real `SceneDocument`) —
all 18 pass, plus Linux-only checks (IME `*`, Ctrl+Delete from every offset, U+2028 never stored, the next scene
untouched). ✅ **Apple** suite "Manuscript format corpus, shared with Linux" (real `ManuscriptNSTextView`, AppKit
dispatch; the corpus bundled as a test RESOURCE — `project.pbxproj`: a new Resources phase for `ScriviInteropTests`) —
all 18 pass. ✅ **Mutation:** changing one expected result fails BOTH platforms; restored.
✅ **Regression:** 26 of the 27 Linux smoke scripts pass, non-root (the 27th, `dumas_world_fixture`, is a fixture
builder that needs a project argument — the generic loop cannot run it, with or without this change). ✅ Apple
168/168; iOS + visionOS BUILD SUCCEEDED. ✅ Build stamp → **55**.
⚠️ **AC-live owed** (rig, build 55): type and paste punctuation; Return, Shift-Return, keypad Enter, Alt-Return;
Backspace at a paragraph start; copy into another app; check the scene file.
✅ **AC-build:** the canonical `docker build --no-cache -f platforms/linux/docker/Dockerfile` SUCCEEDED (362/362 targets).

### ✅ 2026-10-04 — AC-live PASSED; [T-0586] VERIFIED · ⚠️ ADDED TO THIS SPRINT: [I-0276]

✅ User: *"On Linux I accidentally clicked on an item in the launch window while trying to move it.  Now it is opening the file.  We need to arrest this behavior.  Perhaps we should only open on a double click.  and select on single click.  All five checks pass."*
✅ **AC-live MET → all EP-049 ACs met.** [T-0586] Verified (archived with the close).
⚠️ **SCOPE ADDITION, noted explicitly (Sprint guidelines):** **[I-0276]** — the Linux launch window opened a project
on a SINGLE click. Found in this Sprint's live pass, so linked here by the user's standing rule. ✅ Fixed:
click selects, double-click / Return / Enter opens (`Landing.qml`). ✅ Built; no QML errors at launch. ⚠️ Rig check
owed on **build 57**.
⚠️ **Environment note:** Docker's disk is FULL (56/59 GB; 72 images, 43.7 GB reclaimable). The SP-160 no-cache
image was removed; the rest are the user's to prune.

### ✅ 2026-10-04 — [I-0276] VERIFIED; Sprint COMPLETE (awaiting close approval)

✅ User: *"This (I-0276) is confirmed fixed on the rig."* → archived (`../../Issues/Verified/Issue-verified-0271-0280.md`). ✅ Docker cleaned at the user's direction (all images, containers, volumes, build cache).

### ✅ 2026-10-04 — Sprint CLOSED (user-approved); [T-0586] archived

✅ User: *"close SP-160, run the Audit Check, and close EP-049"*

## Retrospective

**Completed:** ✅ EP-049 AC1–AC8 + AC4b ([T-0586]): Linux writes Apple's manuscript format, proven byte-for-byte by
a SHARED corpus that both platforms' tests run. ✅ [I-0276] (added in scope): the launch window opens on a
double-click, not a single click.
**Fixed in scope (pre-existing Linux defects found by measuring first):** Shift-Return wrote U+2028 into scene
files; Ctrl+Delete bypassed the boundary guard.
**Returned to Backlog:** none.
**What went well:** ✅ measuring Qt BEFORE porting (M1–M4) turned two "beliefs" into fixes; ✅ the shared corpus
made "do what Apple does" CHECKABLE, and a mutation proved it bites on both sides.
**What to improve:** ⛔ I twice misattributed state to the user (work "uncommitted" without `git status`; Docker
images "yours" — they were mine) and let my Docker images fill the disk. ✅ Check before attributing; clean up
build images when a Sprint's builds are done.
**Carry-forward notes:** ⚠️ Linux still SHOWS the escape backslashes — the display half is [SP-159]'s design →
[EP-048]. ⚠️ Alt-Return follows Apple's single `\n` until [T-0584] rules.


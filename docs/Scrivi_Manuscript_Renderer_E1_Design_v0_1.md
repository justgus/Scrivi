# Scrivi — Manuscript Renderer **E1: Foundations** — Design v0.1

**Status:** 🟡 **DRAFT — for approval.** ⚠️ **NOT yet an Epic.**
**Date:** 2026-09-29
**Platform:** ⚠️ **`[Apple]` ONLY** — ✅ **`feedback_linux_adopts_apple_shape` is honoured by a SEPARATE
`[Linux]` Epic (E4), ⛔ not by silence.** ✅ **§9.**
**Authority:** ✅ **[`Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md)**
— ⚠️ **all sixteen questions RULED by the user 2026-09-29.** ⛔ **This document DESIGNS; it does not
re-open rulings.**

⚠️ **E1 IS THE FOUNDATION LAYER. ⛔ It renders NOTHING the writer asked to see except the divider.**
✅ **Its job is to make E2's rendering POSSIBLE and to pay a data-loss debt that already exists.**

---

## 0. ⚠️ What E1 is, in one paragraph

✅ **E1 fixes an existing data-loss path, installs the two-coordinate-space model the renderer needs,
puts the escape layer in front of typing, and makes the scene divider visible again.** ⛔ **It does NOT
render bold, italic, or headings — that is E2.** ⚠️ **The one writer-visible outcome is the user's
original complaint: *"the writer can SEE she is at a boundary."***

⛔ **THE TEMPTATION TO ADD E2's RENDERING HERE MUST BE REFUSED.** ✅ **§4A.1 measured that marker hiding
needs storage attributes that the undo path strips — ⚠️ that is a real subsystem, and folding it in
would make E1 unclosable.**

---

## 1. ✅ Acceptance Criteria

⚠️ **Each AC names its authority in the trade study and its evidence obligation.**

| # | Acceptance Criterion | ✅ Authority | ⚠️ Verified by |
| - | -------------------- | ----------- | -------------- |
| **AC1** | ⛔ **Attachments are TYPED.** ✅ `recomputeBoundaries` treats ONLY a divider attachment as a scene boundary; ⚠️ any other attachment is ignored | ✅ **Study §2.3** | ✅ **Unit test: insert a non-divider attachment, assert `sceneBoundaries` is unchanged** |
| **AC2** | ✅ **The scene divider is VISIBLE** in both Light and Dark | ✅ **Study §1.4A** | ⛔ **A LIVE PASS with a screenshot — ⚠️ NOT a suite** (`feedback_live_pass_finds_what_suites_cannot`) |
| **AC3** | ✅ **A SOURCE↔PRESENTED offset mapping exists**, and every caret/selection path goes through it | ✅ **Study §3.4A** (user ruling) | ✅ **Unit test against the §4B.3 oracle over a corpus** |
| **AC4** | ✅ **Typed Markdown reserved characters are ESCAPED** — ⚠️ **all 32 ASCII punctuation marks** | ✅ **Study §3A.0 + §4B.4** (user ruling: *"use the READ set"*) | ✅ **Round-trip test: type → store → parse → presented text equals what was typed** |
| **AC5** | ⛔ **Enter inserts `\n\n`**; ✅ **Backspace at paragraph start deletes ONE `\n`** | ✅ **Study §3A.6** (user ruling) | ✅ **Unit test on the edit path** |
| **AC6** | ✅ **Trailing spaces are normalised to AT MOST ONE on Enter**; ⚠️ a trailing `\\` collapses to `\` | ✅ **Study §4B.6** (user ruling + study amendment) | ✅ **Unit test: `"x␣␣␣"` + Enter → `"x␣\n\n"`** |
| **AC7** | ✅ **Block intents Scrivi does not expose are SUPPRESSED** — ⚠️ `codeBlock`, `blockQuote`, `table` render as ordinary prose | ✅ **Study §4C.4 / §4D.4(a)** (user ruling: *"option 2"*) | ✅ **Unit test: 4-leading-space paragraph renders as prose, ⛔ not monospace** |
| **AC8** | ✅ **ONE parsing mode is chosen and stated** in code and in this document | ⛔ **Study §4C.2 — the study itself mixes them** | ✅ **Code review + a comment naming the mode and why** |
| **AC9** | ✅ **No regression in save fidelity** — ⚠️ a scene's bytes round-trip unchanged through an edit-save-reload cycle | ✅ **Study §2** | ✅ **Integration test against a real temp project** |
| **AC10** | ✅ **The caret path is MEASURED against a 1.85 MB manuscript** | ⚠️ **Study §10.2, [I-0206] re-open condition** | ⛔ **A measurement, recorded — ⚠️ not a pass/fail gate (§7)** |

⚠️ **⛔ NOT IN E1, STATED SO IT IS NOT DRIFTED IN:** ⛔ **marker hiding (E2)** · ⛔ **bold/italic/heading
RENDERING (E2)** · ⛔ **the formatting COMMANDS (E2)** · ⛔ **`paragraphIndent` preference (E3)** ·
⛔ **font choice (E3)** · ⛔ **Linux (E4)**.

---

## 2. ⛔ AC1 — Typed attachments. ✅ **Do this FIRST; it is the only data-loss item.**

### 2.1 ⚠️ The defect as it stands

✅ **READ — `ManuscriptTextView.swift:1555-1558`:**

```swift
storage.enumerateAttribute(.attachment, in: whole, options: []) { value, range, _ in
    if value != nil { dividers.append(range.location) }   // ⛔ EVERY attachment
}
```

⛔ **`sceneBoundaries` is what the save path slices with** (`:811`, `:818`). ⚠️ **So the first
non-divider attachment ANY feature adds splits scenes at the wrong offsets and writes the wrong bytes
to the wrong files.**

### 2.2 ✅ The fix

✅ **Mark the divider attachment, and filter on the mark:**

```swift
// New attribute key, beside .scriviHeading.
extension NSAttributedString.Key {
    static let scriviDivider = NSAttributedString.Key("scriviDivider")
}
```

⚠️ **`makeDividerAttachment()` (`:614-616` call site) attaches it; ✅ `recomputeBoundaries` enumerates
`.scriviDivider` instead of `.attachment`.**

⛔ **⚠️ ONE ORDERING TRAP, AND IT IS THE REASON THIS IS AC1:** ✅ **`recomputeBoundaries` skips
`divider + 2` (`:1590`) — the attachment character PLUS its trailing newline.** ⚠️ **That arithmetic is
correct for the divider and WRONG for any other attachment.** ✅ **Filtering by type keeps it correct;
⛔ leaving it untyped makes it silently wrong the moment E2 or [EP-032] adds one.**

### 2.3 ✅ Why this cannot be deferred

⚠️ **It is cheap now** (⛔ one attribute, two call sites) ✅ **and expensive after three features depend
on today's behaviour** — ⚠️ **Study §2.3 and §8.2's reason [EP-032] cannot be built first.**

---

## 3. ⛔ AC2 — The divider. ⚠️ **THE CAUSE IS STILL UNKNOWN AND THIS DESIGN SAYS SO.**

⛔ **THIS IS THE ONE AC WITH NO DESIGN, BECAUSE THE DIAGNOSIS FAILED TWICE.**

| Attempt | ⚠️ Theory | ⛔ Outcome |
| ------- | -------- | --------- |
| 1 | ⚠️ **[T-0526] dropped [I-0112]'s appearance guard** | ⛔ **DISPROVEN — ✅ measured: the handler resolves correctly (Study §1.3)** |
| 2 | ⚠️ **`separatorColor` is invisible (1.34:1)** | ⛔ **FIXED AND IT CHANGED NOTHING — ✅ user: *"they are still invisible"* (Study §1.4A)** |
| 3 | ⚠️ **`image(for:)` is a TextKit 1 hook; ✅ TextKit 2 wants `NSTextAttachmentViewProvider`** | ⛔ **LEADING HYPOTHESIS, ⚠️ UNTESTED — a harness returned contradictory results across runs** |

✅ **E1's FIRST TASK ON AC2 IS A LIVE DIAGNOSTIC, NOT A FIX** (`feedback_prove_code_is_reached`):
⚠️ **run the real app, screenshot the manuscript, and determine whether the divider draws AT ALL or
draws invisibly.** ⛔ **A 24 pt GAP with no line means hypothesis 3; ✅ a faint line means a colour
problem that the §1.4A fix did not reach.**

⛔ **DO NOT SHIP A THIRD SPECULATIVE COLOUR CHANGE.** ⚠️ **Two have already been spent.**

---

## 4. ✅ AC3 — The two coordinate spaces. ⚠️ **The core of E1.**

### 4.1 ✅ The model

| Space | ⚠️ Counts | ✅ Owned by |
| ----- | -------- | ---------- |
| ✅ **SOURCE** | ⚠️ **every character in storage** — ✅ including escape backslashes and (later) hidden markers | ✅ **the save path, `byteOffset(charOffset:in:)` `:1933`, [EP-019] undo, ScriviCore** |
| ✅ **PRESENTED** | ✅ **only what the writer sees and can land on** | ✅ **caret, selection, arrow keys, click targeting** |

⚠️ **TODAY THEY ARE THE SAME NUMBER AND EVERY CALL SITE ASSUMES IT.** ⛔ **AC3 withdraws that
assumption.**

### 4.2 ✅ The mechanism — ⚠️ a per-fragment escape scanner, NOT a document-wide table

⛔ **A document-wide offset table is the obvious design and it is the WRONG one** — ⚠️ **a 1.85 MB
manuscript would rebuild it on every keystroke, which is exactly [I-0196]'s hang class.**

✅ **INSTEAD — the scanner from Study §4B.3, run PER FRAGMENT, inside the existing viewport-scoped
path:**

```swift
// All 32 ASCII punctuation marks — RULED 2026-09-29 ("use the READ set").
// ⚠️ This is BOTH the write set and the read set, deliberately: one list cannot drift.
private static let escapable = Set(##"!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~"##)

/// presentedIndex -> sourceIndex, for one fragment.
func presentedToSource(_ src: String) -> [Int] {
    var map: [Int] = []
    let a = Array(src)
    var i = 0
    while i < a.count {
        if a[i] == "\\", i + 1 < a.count, Self.escapable.contains(a[i + 1]) {
            map.append(i + 1)          // the escaped char shows; the backslash does not
            i += 2
        } else {
            map.append(i)
            i += 1
        }
    }
    return map
}
```

✅ **MEASURED (Study §4B.3, re-run under the all-32 ruling): ⚠️ this reproduces Apple's parser on
10 / 10 probes.** ⛔ **Ten probes is not a proof — ✅ AC3's test obligation is a CORPUS, per
`feedback_boundary_tests_not_facade`.**

### 4.3 ⚠️ Where it plugs in

✅ **`byteOffset(charOffset:in:)` (`:1933`) ALREADY does UTF-8↔character conversion for the save path.**
⚠️ **AC3 adds a SECOND conversion at a different layer, and they must not be confused:**

```
writer's caret  --[AC3 presented→source]-->  storage offset  --[:1933 existing]-->  UTF-8 byte offset
   PRESENTED                                    SOURCE                                  ScriviCore
```

⛔ **DO NOT MERGE THESE TWO.** ⚠️ **`:1933` is scene-local and byte-oriented; ✅ AC3 is fragment-local
and character-oriented.**

---

## 5. ✅ AC4 — The escape layer

### 5.1 ⚠️ Where escaping happens

✅ **ON INPUT, in the text view's insertion path — ⛔ NOT in the save path.** ⚠️ **Reason: the save path
writes a SUBSTRING of storage (Study §2), so if storage is already escaped, saving is untouched.**

⚠️ **⛔ AND THAT IS THE WHOLE REASON THIS DESIGN IS CHEAP:** ✅ **escape once, at the door; ⛔ everything
downstream — save, undo, [EP-019] history, ScriviCore — sees ordinary characters it already handles.**

### 5.2 ⛔ What is NOT escaped

| ⚠️ Input | ✅ Escaped? | ⚠️ Why |
| -------- | ---------- | ----- |
| ✅ **The writer types `*`** | ✅ **YES → `\*`** | ⚠️ **AC4** |
| ⛔ **A formatting COMMAND writes `**`** | ⛔ **NO** | ✅ **Scrivi is the author; ⚠️ this is the ONLY route to real markup (Study §3A.0)** |
| ⛔ **PASTE** | ⛔ **UNRULED — ✅ see §8** | ⚠️ **The ruling's words were *"that the user types"*** |
| ⛔ **Text already in a scene file** | ⛔ **NO — ✅ see §8** | ⚠️ **Existing manuscripts are unmigrated** |

### 5.3 ⚠️ The accepted cost, restated so nobody "fixes" it later

✅ **`Mr. Smith, in "quotes" — really?` is stored as `Mr\. Smith\, in \"quotes\" — really\?`.**
⛔ **THIS IS INTENTIONAL** (⚠️ user ruling, Study §4B.4). ✅ **It is valid CommonMark and renders
identically; ⚠️ the cost is raw-file legibility, ⛔ not correctness.** ⛔ **A future reader who
"tidies" this to markdown.org's 16 re-introduces the write/read drift the ruling removed.**

---

## 6. ✅ AC5 / AC6 — Enter, Backspace, and trailing whitespace

### 6.1 ✅ The rules, as ruled

| ⚠️ Keystroke | ✅ Behaviour |
| ------------ | ----------- |
| ✅ **Enter** | ⚠️ **insert `\n\n`**; ✅ **first reduce trailing spaces on the line to AT MOST ONE**; ✅ **collapse a trailing `\\` to `\`** |
| ✅ **Backspace at paragraph start** | ⚠️ **delete ONE `\n`** |

⛔ **"AT MOST ONE", NOT "DELETE ONE" — ⚠️ this is the study's amendment (§4B.6) and it matters:**
✅ **CommonMark's hard-break rule is TWO OR MORE spaces**, ⛔ **so "delete one" leaves three spaces
producing a hard break anyway.**

### 6.2 ✅ Why the merge itself needs no work

⛔ **v0.1 of the study warned E1 "must get right" that a single `\n` joins two lines.** ✅ **MEASURED
(§4B.5): Apple's parser ALREADY does — `"a.\nb."` presents as `"a.␣b."`.** ⚠️ **Nothing to build.**

### 6.3 ⚠️ [EP-019] interaction — ⛔ one thing that must not be got wrong

✅ **Enter is ONE gesture that inserts TWO characters (⚠️ sometimes three edits, with the space
normalisation).** ⛔ **These MUST coalesce into ONE undo step**, ⚠️ **or Undo-after-Enter will restore a
space the writer never saw, which reads as corruption.**

---

## 7. ✅ AC7 — Suppressing unexposed block intents

⚠️ **Scrivi's v1 verb list is CLOSED (Study §3A.0): ✅ bold, italic, heading, list, scene break.**
⛔ **So `codeBlock`, `blockQuote` and `table` are intents the renderer NEVER draws.**

✅ **AC7 is therefore a RENDERER decision, not a byte-level one:** ⚠️ **when the parser reports a block
intent Scrivi does not expose, render the run as ordinary prose.** ⛔ **Touch no characters.**

✅ **WHY THIS IS THE CHEAP FIX FOR A REAL HAZARD:** ⚠️ **≥4 leading spaces — or ONE TAB — turns a
paragraph into a `codeBlock` (Study §4C.3, §4D.2).** ⛔ **A novelist indenting by reflex gets monospace
with no emphasis and no wrapping.** ✅ **Suppression catches it wherever it comes from — ⚠️ typed, pasted,
or already in the file — which a preference cannot.**

---

## 8. ⛔ The two rulings E1 must obtain before it finishes

⚠️ **NEITHER blocks the START of E1. ✅ Both block its CLOSE.**

| # | ⚠️ Question | ✅ Why it is the user's | ⚠️ Study's recommendation |
| - | ---------- | ---------------------- | ------------------------- |
| **R1** | ⛔ **PASTE — is pasted text escaped like typed text?** | ⚠️ **The ruling said *"that the user types"*. ✅ Escaping paste is SAFE but surprising to someone pasting real Markdown** | ⚠️ **Escape it — ✅ consistency beats the rarer case; ⛔ but offer "Paste as Markdown" later if asked** |
| **R2** | ⛔ **EXISTING MANUSCRIPTS — is there a one-time escape pass?** | ⚠️ **Scene files already hold unescaped `*`. ⛔ Under the renderer they will silently change appearance** | ⚠️ **NO migration — ✅ AC7's suppression plus E2's per-element rollout makes the change small; ⛔ a bulk rewrite of a writer's prose is the larger risk** |

---

## 9. ⚠️ What E1 does NOT do, and who does it

| ⛔ Not E1 | ✅ Owner | ⚠️ Why |
| --------- | ------- | ----- |
| ⛔ **Marker hiding (Model B)** | ✅ **E2** | ⚠️ **Needs storage attributes that survive `:358-369`, or a layout-fragment subclass (Study §4A.2)** |
| ⛔ **Bold/italic/heading RENDERING** | ✅ **E2** | ⚠️ **E1 builds the seam; E2 uses it** |
| ⛔ **Formatting COMMANDS** | ✅ **E2** | ⛔ **Under §3A.0 these are the feature's ENTIRE input surface — ⚠️ not a toolbar nicety** |
| ⛔ **`paragraphIndent` preference** | ✅ **E3** | ⚠️ **A display preference, F1's sibling (Study §4D.4)** |
| ⛔ **Linux parity** | ✅ **E4** | ⛔ **`feedback_linux_adopts_apple_shape` requires it be SCHEDULED, ⚠️ not assumed** |
| ⛔ **Scene-split endpoint** | ⚠️ **E2 or ScriviCore — ✅ see §10** | ⛔ **VERIFIED ABSENT (§10)** |

---

## 10. ⛔ VERIFIED GAP — there is NO scene-split endpoint

✅ **CHECKED 2026-09-29 against `ScriviCore/include/scrivi/scrivi.h`:**

```
scrivi_merge_scene    ← :533  ✅ EXISTS
scrivi_split_scene    ← ⛔ NO MATCH
```

⚠️ **Study §10.1 ruled the scene break is a COMMAND that splits a scene at the caret.** ⛔ **The core
cannot do it.** ✅ **So the scene-break command needs a NEW `[ScriviCore]` endpoint** — ⚠️ **which is
`[Cross]` work, not `[Apple]`, and must be scoped where the core is scoped.**

⛔ **THIS IS WHY THE BREAK COMMAND IS NOT IN E1.** ✅ **E1 makes the EXISTING divider visible (AC2);
⚠️ authoring a NEW break waits on a core endpoint nobody has written.**

---

## 11. ⚠️ Risks, each with its evidence

| ⚠️ Risk | ✅ Evidence | ⛔ Mitigation |
| ------- | ---------- | ------------ |
| ⛔ **AC2's cause is still unknown** | ✅ **Two fixes already failed (Study §1.4A)** | ✅ **§3 — diagnose live BEFORE fixing; ⛔ no third speculative change** |
| ⚠️ **The caret path is already slow** | ✅ **[I-0206]: `~59 ms`/keystroke, offset-linear on 1.85 MB** | ✅ **AC10 MEASURES it; ⚠️ [I-0206] stays closed, ⛔ a NEW Issue if it bites (user ruling)** |
| ⛔ **`rebuildStorage` is whole-document** | ✅ **`:568-600`; [I-0196] names it *"the prime suspect for the hang"*** | ✅ **AC3's scanner is PER FRAGMENT (§4.2); ⛔ never document-wide** |
| ⚠️ **The oracle is verified, not proven** | ✅ **10/10 probes — ⛔ probe 10 failed until the all-32 ruling** | ✅ **AC3 requires a CORPUS test** |
| ⛔ **Two parsing modes disagree on whitespace** | ✅ **Study §4C.2 — ⚠️ the study itself mixes them** | ✅ **AC8 forces one choice, stated in code** |

---

## 12. ✅ What was read for this design

✅ **READ:** `ManuscriptTextView.swift` — ⚠️ `rebuildStorage` (`:568-668`, ✅ **read in full this pass**),
`recomputeBoundaries` (`:1545-1596`), the undo attribute path (`:355-375`), save path (`:811-818`) ·
`ProjectPreferences.swift` (**whole file**) · `ScriviCore/include/scrivi/scrivi.h` (⛔ **grep for split**).

⛔ **NOT READ / NOT DONE:**
- ⛔ **The divider's draw path was NOT re-read** — ⚠️ **§3 makes diagnosis the first task rather than
  assuming this design knows the cause.**
- ⛔ **No live run of the app in this pass.** ✅ **AC2 requires one.**
- ⚠️ **`ProjectPreferences` PERSISTS TO `UserDefaults`, NOT INTO THE `.scrivi` PACKAGE** (✅ read:
  `key(for:)` → `"scrivi.project.<id>.preferences"`). ⛔ **So E3's `paragraphIndent` would NOT travel
  with the project to another machine** — ⚠️ **that is a real E3 design question, ✅ surfaced here
  because it was found while reading for E1, ⛔ and it is NOT E1's to answer.**

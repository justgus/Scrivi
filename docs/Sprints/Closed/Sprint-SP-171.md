---
sprint: SP-171
epic: EP-050
status: Closed
closed: 2026-10-08
activated: 2026-10-08
platform: Apple
created: 2026-10-08
---

# SP-171 — `[Apple]` [EP-050] **S1**: the translation layer — VoiceOver reads the page at rest ([T-0599], [I-0277])

**Status:** ✅ **CLOSED 2026-10-08 (user-approved):** *"close SP-171"* — AC1, AC2, AC3 and AC5's reading half met; [T-0599] and
[I-0277] VERIFIED and archived. Activated 2026-10-08 (user: *"activate SP-171 and start step 1"*); created 2026-10-08 (user: *"Please
start planning S1"*); rulings Q1–Q4 in planning, Q2 re-ruled after the live pass (dividers read as WORDS).
**Epic:** [EP-050] `[Apple]` Manuscript Accessibility → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md). **EP-050's first Sprint.**
**Carries:** ✅ [T-0599] → [`../../Tasks/Verified/Task-verified-0599.md`](../../Tasks/Verified/Task-verified-0599.md) · ✅ [I-0277] → [`../../Issues/Verified/Issue-verified-0271-0280.md`](../../Issues/Verified/Issue-verified-0271-0280.md).
**Authority:** EP-050 **AC1, AC2, AC3**, and **AC5's reading half** (Q4); rulings **A1** (the page at rest), **A3**.
**Size:** M–L. ✅ Needs no rig: the live pass is on the Mac.

---

## ⚠️ What reading the code found (2026-10-08)

- ✅ **The map already exists as `PresentedText`** (`Scrivi/Views/ManuscriptFind.swift:20`, [SP-164]): presented ⇄ storage, per
  block, from the presenter's cached analysis — escapes, emphasis markers and heading prefixes hidden, list prefixes kept. That is A1
  less two things: ⚠️ **it EXCLUDES chapter titles and dividers** (Find's Q4), which the page shows.
- ✅ **The caret rules exist** — `ManuscriptPresenter.stopTest` (`ManuscriptPresenter.swift:314`) and
  `MarkdownEscapes.snapSelection`. AC2 calls them; it adds no rule.
- ⛔ **No accessibility member is overridden** on `ManuscriptNSTextView` (`ManuscriptTextView.swift:2660`) — everything
  `NSTextView` reports today is storage ([I-0277], measured 2026-10-05).
- ⚠️ **COST: the whole build is 166–178 ms on 1.85 MB** ([SP-164] AC8). Find rebuilds once per edit, on demand. ⛔ VoiceOver queries
  after EVERY keystroke — a whole rebuild would add ~170 ms on top of [I-0275]'s ~90 ms at the end of the manuscript. ✅ So the map
  is PATCHED per edit (re-derive the edited blocks, shift the tail), not rebuilt.
- ⚠️ **Not known — measured in Plan 1, before building:** (a) whether a subclass override is REACHED by the AX server's dispatch
  (`NSTextView` may answer the parameterized attributes through the legacy `accessibilityAttributeValue(_:forParameter:)` path —
  the T-0579 lesson); (b) WHICH members VoiceOver calls per keystroke and per arrow, and how often.

## ✅ Rulings — 2026-10-08 (user, in planning)

| # | Question | Ruling |
| - | -------- | ------ |
| **Q1** | Chapter titles in the accessibility text | ✅ **Included** — they are on the page (A1), and A2's rotor reaches them |
| **Q2** | Scene dividers (U+FFFC attachment) | ✅ **(a) Spoken as "Scene break" / "End of chapter"** (by `DividerRenderState`) |
| **Q3** | One map or two | ✅ **ONE map for Find and accessibility. Change Find:** titles enter the presented string; Find stays out of them through its search CHUNKS (which already end at titles). ⚠️ Find ([SP-164], verified) is re-tested |
| **Q4** | When the VoiceOver live pass runs | ✅ **At the end of S1** — the reading half of AC5; the rotor half is S2's |

## Goal

✅ **With VoiceOver on, the manuscript is read as it reads on the page** — no backslashes, no `##`, no `**`; chapter titles read;
dividers named — and VoiceOver's cursor and Scrivi's caret stay together while she types and moves.

## EP-050 ACs this Sprint meets

| AC | Criterion (Epic wording, short) |
| -- | ------------------------------- |
| **AC1** | One translation layer; every AX member maps through ONE map; corpus test of round trips and member agreement |
| **AC2** | A selection set by VoiceOver lands where Scrivi's caret rules put it; Scrivi's caret reported in presented positions |
| **AC3** | The map is cached and invalidated per edit — measured on 1.8 MB (`numberOfCharacters`, `string(for:)`, a keystroke with VoiceOver's queries); recorded |
| **AC5** *(reading half, Q4)* | Live pass: VoiceOver reads the page without markup at default punctuation verbosity; typing with VoiceOver on behaves |

## Plan

1. **Measure first** (recorded here before Plan 2 begins):
   - (a) **Reach:** one member overridden (`accessibilityValue`, `accessibilityString(for:)`) — is it what the AX dispatch returns?
     Tested through `accessibilityAttributeValue(_:forParameter:)` / the attribute names, ⛔ not by calling the override.
   - (b) **What VoiceOver asks:** a logging build (`[SCRIVI-AX]`, per member, count + time); the user turns VoiceOver on, types,
     arrows, reads a line — the log names the members and their rate.
   - (c) **Patch cost:** re-derive one block + shift the tail of `source` on 1.85 MB.
2. **One map (Q3):** `PresentedText.build` includes chapter titles (Q1) and the divider character; Find's `chunks` still end at both,
   and `firstMatch` / incremental search are confined to chunks. *(Revised after Plan 1:)* held PER BLOCK (local units + offsets,
   block starts in both spaces), covering the whole manuscript; the whole presented string assembled on request and cached per
   version; LAZY — first built at the first AX or Find request. ONE versioned snapshot, owned by the view and shared by the
   `ManuscriptFinderClient` and the AX members, PATCHED per edit (the `storageVersion` bump at `ManuscriptTextView.swift:2793`).
3. **The members** *(revised after Plan 1 — see the log)*: every one in EP-050's list, plus `accessibilityStyleRange(for:)`, overridden
   on `ManuscriptNSTextView` at the NEW-STYLE level (VoiceOver's entry, measured); `AXValue` at the legacy level. Each through the map — value, character count,
   selected text / range(s) (get and set), visible range, insertion-point line, line ⇄ range, `string(for:)`,
   `attributedString(for:)`, range-for-position / -index, `frame(for:)`, RTF. Lines are PRESENTED lines.
4. **Dividers (Q2):** the attachment character reports "Scene break" / "End of chapter" (`DividerRenderState`) to assistive
   technology. ⚠️ The mechanism (attachment cell's AX description vs. the attributed string's attachment attribute) is chosen
   from Plan 1(a)'s finding.
5. **The caret (AC2):** VoiceOver's `setAccessibilitySelectedTextRange(s)` → storage → `snapSelection` with the presenter's stop
   test; Scrivi's selection → presented positions. A posted `selectedTextChanged` / `valueChanged` stays as AppKit posts it.
6. **Tests** (`ScriviInteropTests`, through the real dispatch): a corpus (escapes, headings, emphasis, lists, chapter titles,
   dividers, an edit at each) — round trips; `string(for: r)` = `value[r]`; `line(for:)` ⇄ `range(forLine:)`; set-selection
   never lands inside hidden markup; the patched map equals a fresh build after every edit. **Find re-tested** (Q3): no match inside
   a chapter title; [SP-164]'s suite green. Mutation-checked.
7. **Cost (AC3):** on 1.85 MB dumas — `numberOfCharacters`, `string(for:)`, and a keystroke WITH Plan 1(b)'s query set; compared
   to [I-0275]'s baseline. Recorded here.
8. **Live pass** (user, Mac, VoiceOver) — steps in the chat reply.

⛔ **NOT in this Sprint:** the Headings rotor (AC4 → S2) · Linux (AC6 → S3, needs the rig) · iOS/visionOS · spoken formatting.

## Progress log

- **2026-10-08** — activated; Plan 1 (measure) started.
- **2026-10-08 — Plan 1(a) MEASURED: ⛔ a new-style override is NOT reached.** Suite *"Manuscript accessibility — Plan 1 spike"*
  (`ScriviInteropTests`). `NSTextView` implements the LEGACY `accessibilityAttributeValue:` itself (class-walk), and with a probe
  `accessibilityValue` / `accessibilityStringForRange:` / `accessibilityNumberOfCharacters` swapped onto `ManuscriptNSTextView`,
  the attribute names still returned STORAGE (`AXValue` = `Mr\. Smith said \*no\*.`, 23, `AXStringForRange{0,3}` = `Mr\`).
  ⚠️ So Plan 3 as written ("every member overridden") would pass a direct-call test and change nothing VoiceOver hears — the
  T-0579 trap. ⚠️ `accessibilityValue()` cannot even be overridden from Swift (`NSView` does not declare it). ✅ Consequence:
  the members are served at the LEGACY level (`accessibilityAttributeValue(_:)`, `(_:forParameter:)`, `accessibilitySetValue`)
  unless 1(b) shows VoiceOver's requests arriving elsewhere; tests ask through attribute NAMES.
- **2026-10-08 — Plan 1(c) MEASURED** (Debug; generated dumas-shaped 1.71 MB — 60 chapters × 20 scenes, titles, dividers,
  escapes + emphasis in every paragraph; the test host is sandboxed and cannot read the real fixture):

  | Measure | Result |
  | ------- | ------ |
  | whole `PresentedText.build` (1,571,330 presented units) | **292–315 ms** (real dumas, sparser markup: 166–178 ms, [SP-164]) |
  | re-derive ONE block (~140 units) | **0.04 ms** |
  | splice + shift a FLAT offset array — edit near the START | ⛔ **87–93 ms** |
  | … — edit near the END | 0.18–0.29 ms |
  | immutable snapshot copy | 0.04–0.13 ms |
  | 1,710 `presentedIndex` lookups | 0.9 ms |

  ✅ Consequence: re-deriving is free; ⛔ a FLAT absolute-offset map is not patchable (the tail shift is O(n), ~90 ms at the start).
  ✅ The map is held PER BLOCK (local units + local offsets) with block start positions in both spaces; an edit re-derives its
  blocks and shifts only the block starts after it (~10⁴, not ~10⁶).
- **2026-10-08 — Plan 1(b) logging build in place** (`Scrivi/Views/ManuscriptAccessibility.swift`, DEBUG only, pass-through, added
  to `project.pbxproj`): `[SCRIVI-AX]` tallies per burst at both levels (`L:` legacy attribute names, `N:` new-style members),
  `SET` lines for selection writes, `slow` for any call > 5 ms.
- **2026-10-08 — Plan 1(b) MEASURED (user's VoiceOver run on dumas-prose-timelines, Debug) — ⚠️ AND IT CORRECTS 1(a).**
  - ✅ **VoiceOver enters at the NEW-STYLE members, and a Swift override of them IS reached.** Every parameterized request was
    logged at both levels with EQUAL counts (e.g. `N:stringForRange×49` / `L:AXStringForRange×49`), and the `N:` time ≥ the `L:`
    time in every burst (`N:lineForIndex 359.8` ⊇ `L:AXLineForIndex 359.7`) — so the `N:` call ENCLOSES the `L:` one: the AX server
    calls `accessibilityString(for:)` etc. (our override), and `NSTextView`'s own implementation forwards to the legacy
    `accessibilityAttributeValue:`. ⛔ **1(a)'s conclusion ("not reached") was WRONG for VoiceOver**: it asked through the legacy
    NAMES, which is the inner half of the chain, not VoiceOver's entry. The spike test now asserts only what it measured (legacy
    names bypass a new-style override) and says so. ✅ Tests of the layer call the new-style members — the dispatch VoiceOver uses.
  - ⚠️ **Exceptions:** `AXValue` arrives only at `L:` (`accessibilityValue()` cannot be overridden from Swift — `NSView` does not
    declare it) and is asked ~once per focus, never per keystroke → served at the legacy level. ⚠️ **`AXStyleRangeForIndex`**
    (index → range; at `L:` only because the probe had no `N:` for it) was NOT in EP-050's list — ✅ it is a position member and
    joins the map (`accessibilityStyleRange(for:)`).
  - ✅ **Per keystroke / arrow (one burst ≈ 21–25 ms of VoiceOver activity, our members ≲ 1 ms):** `stringForRange`×7,
    `selectedTextRange`×4, `attributedStringForRange`×2, `numberOfCharacters`×2, `lineForIndex`×1, `rangeForLine`×1,
    `visibleCharacterRange`×1, `textualContext`, `sharedTextUIElements`. ⛔ **Never `AXValue` per keystroke** — no whole-text
    string is ever needed on the typing path. On focus / app switch: ~35–60 numberOfCharacters, ~30–50 stringForRange, one AXValue.
  - ⚠️ **AppKit's LINE members are the cost, today, on STORAGE:** `lineForIndex` **334–360 ms** (twice, at focus far into the
    manuscript), `insertionPointLineNumber` and `rangeForLine` ~24–27 ms each at the end. They count VISUAL (layout) lines from
    the top — the [I-0275] class, not ours. ✅ Design consequence: lines are VISUAL, and a hidden marker is zero-width, so presented
    and stored text have the SAME lines — the layer maps positions in and out and DELEGATES line and frame geometry to AppKit; it
    adds nothing to that cost (and must not be blamed for it in AC3).
  - ✅ **VoiceOver sets the selection** (`SET selectedTextRange {10,0}`, `{40,0}`, `{169,0}`) — AC2's inbound path, measured live.
  - ✅ **What VoiceOver SAYS today** (user): `##` → *"number number"*, `*` → *"star"*, `\` → *"backslash"* — [I-0277] confirmed by
    ear at default verbosity.
  - ✅ **Continuous reading (user, clarified):** on the first click into the manuscript (focus arriving), VoiceOver began reading
    UNPROMPTED from the start of Chapter 2 and read on past the scene until stopped; Read All (VO-A) began at the START of the
    document and would have read it all; a later click (focus already there) spoke only the word at the caret, then waited.
    ⚠️ Not yet known whether reading-on-focus is VoiceOver's normal behaviour for a long text area — the live pass compares a
    long TextEdit document as the control. ✅ **What it means for the map (Plan 2):** (1) VoiceOver reads FAR from the viewport
    and across scene and chapter boundaries, so the map covers the WHOLE manuscript — never only the visible part — and titles
    and dividers (Q1, Q2) are in the reading flow; (2) the WHOLE presented text (`AXValue`, once per focus) must be assembled
    from the per-block map — cached per edit version, built only when asked; (3) the map is LAZY: built at the first
    accessibility or Find request, so a writer without VoiceOver never pays for it.
  - ℹ️ `AXParent` ×2,500–15,000 per burst at focus / app switch (6–18 ms) — AppKit's own tree walk; unchanged by this layer.
- **2026-10-08 — ✅ Plan 1 COMPLETE. Plan 3 REVISED from the measurements:** override the new-style members (reached); serve
  `AXValue` at the legacy level; add `accessibilityStyleRange(for:)`; map positions and ranges in and out, and delegate lines and
  frames to `super` on the mapped ranges; strings come from the map.
- **2026-10-08 — ✅ Plan 2 IMPLEMENTED: the ONE map.** `Scrivi/Views/ManuscriptPresentedMap.swift` (new; in `project.pbxproj`, all
  three targets): `PresentedSegment` / `PresentedLayout` (per-segment map — a block, or the blank run between blocks; a block never
  merges, so a patch re-derives only the blocks an edit touches) · `PresentedText` (immutable snapshot with its own storage copy,
  for Find's background search) · `PresentedMap` (the LIVE map, `ManuscriptNSTextView.presentedMap`: built at first use, PATCHED
  from `NSTextStorage.didProcessEditingNotification`; a wholesale replacement drops it for a lazy rebuild). ✅ Titles and dividers
  IN the presented text (Q1), `excluded` for Find (Q4) — the finder is handed an excluded run MASKED (U+FFFC × its length) so its
  walk stays in step. ✅ The pending pair (⌘B between words) is hidden at rest. ✅ Find reads a snapshot of the live map (Q3) — the
  old whole rebuild per edit is gone; `PresentedText.build(…range:)` stays for the Navigator's scene jump.
  - **Tests** (`PresentedMapTests`, 5): AC1 page at rest — the map hides EXACTLY what the presenter hides on screen with the caret
    away, round trips at every unit; ✅ **300 seeded random storage edits** (titles, dividers, escapes, emphasis, headings, lists,
    hard breaks) — after EACH the patched map equals a fresh build at every position both ways, text and chunks, ✅ with ONE build
    in total; typed edits through the view (escape layer, Return, ⌫); the pending pair; Find never matches inside a title (masked
    run). Find test updated for Q3. ✅ **Full interop suite 253/253** (count checked against the file's 253 `@Test`s).
  - **AC3 (Debug, generated 1.7 MB, 25,378 segments, 1,573,239 presented units):**

    | Measure | Result |
    | ------- | ------ |
    | whole build (first use) | **159 ms** (the old flat builder: 292–315 ms on the same text) |
    | a one-character edit + its undo near the START: without map → with map | 0.16 → **3.80 ms** (≈1.8 ms patch per edit — the segment-start shift) |
    | … near the END | 0.13 → **0.17 ms** |
    | snapshot for Find | 0.23 ms |
    | whole presented string (`AXValue`, once per focus) | 29.8 ms |

  - ⚠️ **Found: [I-0284]** — the AC1 corpus showed `_under_` and `***both***` raw. The map was RIGHT: the screen shows them raw
    too. `MarkdownBlocks.analyze` returns NO markers for a block containing a hard break (`\` + newline). Filed to the Issue
    backlog (not added to this Sprint); the corpus keeps hard breaks and `_`/`***` emphasis in separate paragraphs.
- **2026-10-08 — ✅ Plan 3 IMPLEMENTED: the members** (`Scrivi/Views/ManuscriptAccessibility.swift` — the Plan 1(b) probe became
  the real overrides; DEBUG still times every member into `[SCRIVI-AX]` bursts for AC3 and the live pass). New-style overrides:
  `numberOfCharacters`, `string(for:)`, `attributedString(for:)` (AppKit's attributed text for the covering storage range with the
  hidden characters deleted — fonts and attachments kept), `rtf(for:)`, `selectedText`, `selectedTextRange(s)` get/set,
  `visibleCharacterRange`, `range(for index:)`, `styleRange(for:)` (added from Plan 1(b)), `range(for point:)`, `frame(for:)`,
  `line(for:)`, `range(forLine:)`, `insertionPointLineNumber` (lines VISUAL — `super` on mapped positions). `AXValue` at the legacy
  entry, guarded by `Thread.isMainThread` (not asserted — [I-0202]). A view without a presenter keeps AppKit's behaviour.
  - **Tests** (`ManuscriptAccessibilityTests`, 5): AXValue / count / every `string(for:)` and `attributedString(for:)` over a grid of
    ranges = the presented text; clamping; RTF round trip; the attributed string loses the markers; lines round-trip at every
    presented index (and a wrapped line counts as more than one); index / style ranges hold their index; `frame(for:)` of the
    presented "Smith" = the layout rect of the stored "Smith"; selection reported presented, set → storage. ✅ **Mutation-checked:**
    M1 `string(for:)` unmapped → 49 failures; M2 `range(forLine:)` unmapped → caught; M3 `frame(for:)` unmapped → ⚠️ SURVIVED the
    first test (it only checked a size) → test strengthened → caught. The dynamic 1(a) spike test was removed (its finding is above).
    ✅ **Full interop suite 257/257** (= the file's 257 `@Test`s).
  - ⚠️ **Not testable here — for the live pass:** VoiceOver's TYPING ECHO comes from AppKit's value-changed notification, whose
    payload is not one of these members; whether a typed `*` (stored `\*`) is echoed as "star" or "backslash star" is measured
    by ear in Plan 8.
- **2026-10-08 — ✅ Plan 4 IMPLEMENTED: dividers NAMED (Q2).** ⚠️ MEASURED first: AppKit attaches NOTHING to a TextKit 2
  attachment character — `accessibilityAttributedString` at a divider carries only `AXFont` / alignment, no `AXAttachment` — so
  VoiceOver had a bare U+FFFC with no description. ✅ Mechanism chosen from that: the platform's own — `attributedString(for:)`
  attaches an `AXAttachment` element (`NSAccessibilityElement`, role static text) labelled **"Scene break"** / **"End of chapter"**
  by `DividerRenderState`. The map stays 1:1 (the divider is one presented unit). ⚠️ Whether VoiceOver SPEAKS an attachment's
  label while reading is judged by ear in the live pass; ⛔ if it does not, the fallback is presenting the words themselves
  (the map would carry a replacement segment) — a ruling, not assumed. Test `dividersNamed`.
- **2026-10-08 — ✅ Plan 5 IMPLEMENTED: the caret both ways (AC2).** Outbound: Scrivi's selection reported in presented positions
  (Plan 3). Inbound: VoiceOver's `setAccessibilitySelectedTextRange(s)` → storage → `setSelectedRange`, whose override already
  runs the caret rules ([SP-162] homes, [T-0572] scene gaps). ⚠️ **Found in design and PROVEN by mutation:** that override reads
  the PREVIOUS caret to recognise arrow steps, so VoiceOver re-setting the position it is already at (after a one-character closer)
  read as a → step and moved one on — **M4** (placement snap removed) failed `caretRoundTrip` with *"presented 10 → storage 15 →
  reads back {11, 0}"*. ✅ So a caret VoiceOver sets is first sent HOME as a PLACEMENT (`snapCaret(from: itself)`). ✅ Tests:
  every presented index, forward, backward and re-set in place, lands on a home and reads back as itself — ⚠️ except at a LIST
  PREFIX, which is visible but never a caret home ([SP-163] Q7): a caret set before `- ` lands after it (the only move allowed);
  `caretHomes` checks each home kind (after `## `, before `\`, after an opener, before a closer).
- **2026-10-08 — Plans 6–7 (tests, cost):** ✅ **Full interop suite 260/260** (= the file's 260 `@Test`s). **AC3** (Debug, generated
  1.7 MB, `patchCost`): whole build 160–180 ms at FIRST use only · patch ≈ 2 ms per edit near the start, ≈ 0 at the end ·
  ✅ **VoiceOver's per-keystroke query set on the map (2 counts, 7 strings, 3 selections, 2 attributed strings) 0.13 ms** · whole
  text (once per focus) 30–33 ms · Find snapshot 0.2–0.3 ms. ⚠️ The LINE members are AppKit's work on storage (334–360 ms for
  `lineForIndex` far into the book, measured BEFORE this layer in Plan 1(b)); the live pass's `[SCRIVI-AX]` bursts compare them.
- **Plan 8 — the live pass** (user, VoiceOver, Mac): see below.
- **2026-10-08 — Plan 8 LIVE PASS, first run** (user, dumas-prose-timelines, Debug, default verbosity). User: *"I didn't notice
  any slowness."*

  | Step | Result (user's words where quoted) |
  | ---- | ---------------------------------- |
  | 1 TextEdit control | ⏳ not reported |
  | 2 focus + Read All | VoiceOver *"announced the line number, the word the caret was in, caret position in the word and then began to read the text from the Chapter 1 scene 1"* — the caret was in Chapter 48. ⚠️ Open: normal VoiceOver behaviour or ours (step 1 decides) |
  | 3 no "backslash" / "number number" / "star" | ✅ passes |
  | 4 Control stops reading | ✅ passes |
  | 5 dividers spoken | ⚠️ **not established** — *"I don't remember… It didn't say 'attachment'. I don't remember it saying anything."* (reading was in Chapter 1, out of sight) |
  | 6 VO-→ across punctuation / bold | ✅ passes |
  | 7 ↓ by line | ✅ passes |
  | 8 typing echo of `Mr. *hi*` | ✅ *"it says 'asterix' but without 'backslash'"* — the echo is the TYPED character, not storage |
  | 9 ⌫ | ✅ passes |
  | 10 ⌘B between words, type | ✅ passes — VoiceOver speaks the command (*"Bold"* / *"Italics"*) |
  | 11 Find: a title-only word | ✅ passes (not found — Q4 holds with titles now in the text, Q3) |
  | 12 Find: `Mr. ` | ✅ passes |

  **`[SCRIVI-AX]` on the real 1.85 MB:** ✅ map built ONCE at first focus — **136.5 ms** (13,000 segments, 1,851,283 presented
  units); ✅ the per-keystroke / per-arrow bursts: our members ≤ ~1 ms (strings 0.1–0.3 ms, attributed 0.2–0.4 ms, frames 0.1 ms);
  `AXValue` **6–15 ms** — ⚠️ asked more often than Plan 1(b) suggested (a `numberOfCharacters×4 · AXValue×1` burst, ~10 times in the
  session), still within budget; ⚠️ AppKit's own line work, unchanged by this layer: `insertionPointLineNumber` **383 ms once** (first
  focus — layout to Chapter 48), then **~31 ms** per call there; `lineForIndex` / `rangeForLine` ~29–34 ms (the [I-0275] class).
- **2026-10-08 — Plan 8 retest (user).**
  - ⛔ **Q2 NOT MET by the `AXAttachment` label (Plan 4):** *"It does not speak anything for either the scene break or the chapter
    break."* With plain arrows (Control stops reading; an arrow then reads the character under the caret): a scene divider —
    NOTHING spoken; a chapter divider — the caret skips the *"Chapter 2"* title ([T-0572]: the caret never rests on titles or
    dividers) and VoiceOver says only *"Two"*. ⚠️ So VoiceOver does not read an attachment element's label from the text, while
    reading or while moving. ➡️ The recorded fallback (presenting the WORDS) needs a ruling.
  - ✅ **Reading from the start is NORMAL VoiceOver behaviour** — control app: *"in Ulysses it also began reading from the beginning
    of the document, so it is performing as intended."* (Step 2's open question CLOSED — not a defect.) The user had stated it
    repeatedly; ⚠️ my retest instructions ignored it and named the wrong keys (Control-Option-arrow moves between UI elements,
    not characters) — corrected by the user.
- **2026-10-08 — ✅ Q2 RE-RULED (user): present the WORDS** (*"Present the words"*, to the fallback the Plan 4 entry recorded).
  ✅ IMPLEMENTED: `PresentedSegment.replacement` — a divider is ONE storage character presenting as **"Scene break"** /
  **"End of chapter"** (`PresentedLayout.words(for:)`); every unit of the words maps to the divider character; `excluded`, so Find
  never matches them (its masked run is the words' length). The `AXAttachment` element code was REMOVED. `attributedString(for:)`
  replaces the divider character with its words (fonts kept) and trims to the presented range asked for. Screen and file
  unchanged. ✅ Tests: `dividersNamed` rewritten — AXValue, count, every range of length 1/4/9 starting anywhere (incl. INSIDE the
  words) through `string(for:)` and `attributedString(for:)`, `range(for index:)` inside the words = the whole words, a caret set
  inside the words lands on a home at or before the divider; `pageAtRest` / Find's presented-text test updated; the seeded 300-edit
  oracle covers replacement segments (its corpus has dividers). ✅ **Full interop suite 260/260.** ⏳ Heard-by-ear retest owed.
- **2026-10-08 — ✅ Q2 PASSES by ear; AC5 reading half MET (user).** *"It says 'Scene break' and 'End of chapter' now."* ✅ **RULED
  (user): single-stepping across a divider stays as it is** — *"When I single step it does not say 'Scene break' and at the chapter
  break it only says '2' but I'm not considering that an issue. I believe it is ok to have it not enunciate here in this
  instance."* (The caret skips dividers and titles, [T-0572]; VoiceOver describes the jump itself.)
- ✅ **[T-0599] and [I-0277] USER-VERIFIED 2026-10-08** by the live pass (every step reported passing; the divider retest passing) —
  ➡️ archived with the SP-171 close. ✅ **AC1, AC2, AC3 met; AC5 reading half met** (the rotor half is S2's).

## Acceptance Criteria

- [x] **AC1** one translation layer — `PresentedMap` / `PresentedLayout`, shared with Find (Q3); every member through it; corpus,
      member-agreement and 300-edit oracle tests; mutation-checked
- [x] **AC2** the caret both ways — reported presented; VoiceOver's sets snapped as PLACEMENTS (drift found and killed, M4)
- [x] **AC3** cost recorded — real dumas: map built once at first focus 136.5 ms; per-keystroke members ≲ 1 ms; AXValue 6–15 ms
- [x] **AC5 (reading half)** live pass — ✅ 2026-10-08, every step passing; dividers *"'Scene break' and 'End of chapter' now"*
- [x] Interop 260/260 · iOS + visionOS builds · type-source, stub-parity, TextKit 2 and layout-convergence guards ✅

## Retrospective

- ✅ **Measuring first paid twice.** Plan 1(c) killed the flat-array map (~90 ms per edit) before it was built; Plan 1(b) — the
  user's two minutes with VoiceOver — found the real entry point, the missing `AXStyleRangeForIndex`, and the per-focus `AXValue`.
- ⚠️ **I drew a wrong conclusion from the wrong dispatch.** 1(a) asked through the LEGACY attribute names and concluded overrides
  were "not reached"; VoiceOver enters at the new-style members. The live log corrected it within the hour — but a test of the
  real path (the T-0579 lesson) should have been the first instinct, not a call through the inner half of the chain.
- ⚠️ **A platform mechanism that tests green can still be silent.** The `AXAttachment` label passed every test and VoiceOver ignored
  it; only the ear caught it. The fallback had been recorded in advance, so the re-ruling was one question.
- ⚠️ **My retest instructions ignored what the user had told me** (reading always starts at the top) and named the wrong keys
  (Control-Option-arrow moves between UI elements). Live-pass steps must be written from what the user has reported, not from a
  generic VoiceOver script.
- ✅ **Mutation checks earned their keep:** M3 (frame) survived the first test; M4 demonstrated the exact drift AC2 forbids.
- ✅ **Found on the way, kept out of scope:** [I-0284] (a hard break switches off its paragraph's emphasis) → Issue backlog.
- ➡️ **Carried to S2:** the Headings rotor (AC4) and AC5's rotor half; the DEBUG `[SCRIVI-AX]` timing stays for that live pass.

---
sprint: SP-172
epic: EP-050
status: Closed
closed: 2026-10-09
activated: 2026-10-08
platform: Apple
created: 2026-10-08
---

# SP-172 — `[Apple]` [EP-050] **S2**: the Headings rotor ([T-0600])

**Status:** ✅ **CLOSED 2026-10-09 (user-approved):** *"close SP-172"* — AC4 and AC5's rotor half met; [T-0600] VERIFIED and
archived. Activated 2026-10-08 (user: *"activate SP-172 and start step 1"*); created 2026-10-08 (user: *"begin S2"*); rulings R1–R2
in planning, R3 after Plan 1.
**Epic:** [EP-050] `[Apple]` Manuscript Accessibility → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md). Previous:
[`Sprint-SP-171.md`](Sprint-SP-171.md).
**Carries:** ✅ [T-0600] → [`../../Tasks/Verified/Task-verified-0600.md`](../../Tasks/Verified/Task-verified-0600.md) (the Headings rotor).
**Authority:** EP-050 **AC4**, and **AC5's rotor half**; ruling **A2** (a Headings rotor — Markdown headings and chapter titles).
**Size:** S–M. ✅ Needs no rig: the live pass is on the Mac.

---

## ⚠️ What reading the SDK and the code found (2026-10-08)

- ✅ **The API** (`AppKit/NSAccessibilityCustomRotor.h`, read 2026-10-08): `NSAccessibilityCustomRotor(rotorType: .heading,
  itemSearchDelegate:)` — VoiceOver supplies the label and its own keys; the view vends it from `accessibilityCustomRotors`
  (`NSAccessibilityProtocols.h`). The delegate answers `rotor(_:resultFor:)` with an `ItemResult(targetElement:)` whose
  `targetRange` is, for a text view, *"the area of interest"*, plus an optional `customLabel`. The search parameters carry
  `currentItem`, `searchDirection` (next / previous) and a type-ahead `filterString`.
- ⚠️ **The API's start conflicts with AC4's "from the caret":** with `currentItem` nil, the header says *"the search should begin
  from, and include, the first or last item"*. VoiceOver's rotor LIST (VO-U) is likely built that way — so starting from the caret on
  nil would drop the headings above it. ⛔ How VoiceOver calls the delegate for the list vs. for heading-to-heading moves is NOT
  known → **measured first** (Plan 1).
- ✅ **The headings are already known to the S1 map's builder:** each block's `BlockInfo.analysis.headings` (line, hidden prefix,
  level); chapter titles are their own `excluded` segments (`.scriviHeading`). ✅ So the outline is recorded ON the segments and is
  PATCHED with the map for free; a rotor query scans ~13,000 segments, never re-analyses 1.85 MB.
- ✅ **With "Show chapter titles" OFF the titles are not in storage at all** (`rebuildStorage`, `ManuscriptTextView.swift:785`).

## ✅ Rulings — 2026-10-08 (user, in planning)

| # | Question | Ruling |
| - | -------- | ------ |
| **R1** | Which rotors | ✅ **Headings only** — one rotor, Markdown headings and chapter titles in order (A2). Per-level rotors are not built |
| **R2** | Chapter titles hidden | ✅ **Follow the page** (A1's rule): titles off → only Markdown headings are listed; "End of chapter" is still read |
| **R3** | AC4's "from the caret" (2026-10-08, after Plan 1) | ✅ **From VoiceOver's reading position** — the `currentItem` VoiceOver passes, as the API defines it. No caret heuristic. AC4 amended in `Epic-active.md` |

## Goal

✅ **A writer using VoiceOver opens the Headings rotor and moves heading to heading — chapter titles and Markdown headings, in
order — landing where she would expect to write.**

## EP-050 ACs this Sprint meets

| AC | Criterion (Epic wording, short) |
| -- | ------------------------------- |
| **AC4** | The Headings rotor: Markdown headings and chapter titles, in order, each a `targetRange` in presented positions; next/previous from the caret |
| **AC5** *(rotor half)* | Live pass: the rotor moves heading to heading |

## Plan

1. **Measure first:** a minimal Headings rotor (API semantics: nil → first / last) with a DEBUG log of every search (`currentItem`
   range, direction, filter string, result). The user opens the rotor list (VO-U), moves through it, and moves heading to heading
   with VoiceOver's heading keys from a caret mid-manuscript. ➡️ Decides how AC4's "from the caret" is met, and what VoiceOver does
   with the caret when an item is chosen.
2. **The outline on the map:** each segment records its headings (presented range of the heading TEXT, without the hidden prefix;
   level); a chapter-title segment is a heading. Patched with the map; scanned per query.
3. **The rotor:** `accessibilityCustomRotors` → `[Headings]`; the delegate searches the outline by direction from `currentItem`'s
   `targetRange` (or as Plan 1 decides for nil); `filterString` matched case- and diacritic-insensitively; `customLabel` = the
   presented heading text.
4. **Tests** (`ScriviInteropTests`, through the rotor's delegate as VoiceOver calls it): order; next / previous chains both ways;
   nil start; filter; `string(for: targetRange)` = the heading text; titles OFF → none listed (R2); the outline after edits equals a
   fresh build (the S1 oracle); cost on 1.7 MB. Mutation-checked.
5. **Live pass** (user, Mac, VoiceOver) — steps written from what the user has reported (reading starts at the top; plain arrows
   move the caret), in the chat reply.

⛔ **NOT in this Sprint:** per-level heading rotors (R1) · Linux (AC6 → S3, needs the rig) · [I-0284].

## Progress log

- **2026-10-08** — activated; Plan 1 (measure) started.
- **2026-10-08 — Plan 1 rotor + Plan 2 outline in place** (built together: VoiceOver asks once per heading when it builds its list —
  ~1,200 on dumas — so the measuring rotor needs the cheap outline). `PresentedSegment.headings` (segment-local heading TEXT + level;
  0 = chapter title) recorded by the builder → `PresentedLayout.outline()`; `PresentedMap.outline()` cached per map `version`
  (bumped by every build and patch). `ManuscriptNSTextView.accessibilityCustomRotors()` → one `.heading` rotor (R1); the delegate
  (a `@MainActor` isolated conformance — a plain one fails Swift 6 isolation) uses the API's semantics for now, with a DEBUG
  `[SCRIVI-ROTOR]` line per search (direction, `currentItem`, filter, Scrivi's caret, result) and per selection VoiceOver SETS.
  ✅ Tests: `HeadingsRotorTests.order` (titles + headings in order, as presented text, both ways, filter); the S1 300-edit oracle now
  also compares the outline. ⏳ The user's VoiceOver run is owed.
- **2026-10-08 — Plan 1 MEASURED (user's VoiceOver run, dumas-prose-timelines).**
  - ✅ **How VoiceOver builds the rotor LIST (VO-U):** it walks `next` from `currentItem.targetRange = {0, 0}` (not nil) to the end —
    one call per heading (~1,200) — and again every time the rotor opens and for each type-ahead letter (`filter="h"` narrowed the
    list; `"hh"` → none). ⛔ **The caret is never passed.** So the list starting at Chapter 1 is VoiceOver's design (user: *"The list
    starts at chapter 1"*), and AC4's "from the caret" can only apply to heading-to-heading moves — ⏳ not yet measured: the user was
    sent the wrong key (I wrote "VO-Command-H"; the user pressed Control-Option-H, which is VoiceOver's own help/command menu).
  - ✅ **Choosing an item** set the selection to the heading's start (`SET selection {162594, 0}` — "The counter-example"); user:
    *"The caret was at the beginning of the scene. I was told I could start typing."* A chapter title chosen (`{1842376, 0}`) — the
    caret then sat past the title ([T-0572] caret rules: never on a title).
  - ⚠️ **After choosing:** *"A double rectangle appeared around the header text. Typing arrows flashed the screen. I had to click.
    When I clicked the double rectangle followed the caret; arrow up or down caused the rectangle to surround a single line, which
    VoiceOver read."* — not yet understood (a screen flash is the visual alert for a beep); ⏳ to be narrowed in the retest.
  - ⛔ **Found + FIXED: labels.** Chapter-title labels began with a newline (`"\nChapter 2"` — the title run starts with one) and
    Markdown headings kept trailing spaces (`"The claim stated plainly "`). ✅ Heading ranges are now trimmed of whitespace at both
    ends (`PresentedLayout.trimmed`); `HeadingsRotorTests.order` now uses both measured shapes.
- **2026-10-08 — Plan 1 retest MEASURED (heading keys, VO-Command-H / VO-Command-Shift-H).**
  - ✅ **Heading-to-heading works both ways** (user: *"passes … moves backwards through the manuscript. Heading by heading,
    including the chapter headings"*). Each move SETs the selection to the heading start, and the next call passes the item just
    returned as `currentItem` — the chain is VoiceOver's, not Scrivi's.
  - ⛔ **VoiceOver never passes the caret here either.** The first move passed `currentItem = {0, 10}` with Scrivi's caret at
    1,842,710 — **VoiceOver's own cursor**, which sits at the top of the text (the same reason reading always starts at the top).
    Later first-moves passed `{19960, 0}`-style ranges where the VoiceOver cursor had been left. ➡️ AC4's "from the caret" is not
    something the API lets the app decide; ⏳ **ruling owed**: amend AC4 to "from VoiceOver's position" (recommended), or a heuristic
    (a `currentItem` that is not a heading → search from the caret), which would also break the VO-U list (it starts from `{0, 0}`).
  - ⚠️ **After a VO-U choice:** → moves the caret and reads the letter; any other key (letters, Escape) flashes the screen, and
    after that the arrows flash too. Not Scrivi's red refusal flash (that is paste-only). ✅ DEBUG `[SCRIVI-ROTOR] key …` and
    `[SCRIVI-ROTOR] refused edit …` lines added (`ManuscriptTextView.keyDown`, `shouldChangeText`) to show where those keys go.
  - ⛔ **Found: a U+0015 control character was inserted into the manuscript** at 329300 — the start of Chapter 10's heading "The
    letter as weapon" (the outline after it shifted by +1, so the S1 patch tracked the edit live). U+0015 is what Control-U types;
    most likely VO-U was pressed while VoiceOver was off or not holding the keys. ⏳ To be confirmed and, if Scrivi lets a control
    character into a scene, filed as an Issue.
- **2026-10-08 — flash retest (keys).** After a VO-U choice (`SET selection {11645, 0}`), → and `x` both reached the manuscript
  (`key code=124 … firstResponder=self`, `key code=7 chars="x" firstResponder=self`), and no `refused edit` line appeared: Scrivi
  accepted both keys. ✅ Title labels now arrive trimmed (`"Chapter 2"`). ⚠️ The stray U+0015 is still at Chapter 10's
  "The letter as weapon". ⏳ Whether the screen flashed on this run was not reported.
- **2026-10-08 — flash closed; R3 ruled.** User: *"no flashes this time"* → the earlier flashes were keys VoiceOver caught and
  could not use, not Scrivi (both keys reached `keyDown` and were accepted). ✅ **R3** (user: *"your recommendation"*): next/previous
  from VoiceOver's reading position; AC4 amended. ➡️ Plan 3 needs no change: the delegate already uses the API's semantics.
  ⚠️ **My instruction for the stray U+0015 was wrong:** "caret at the start of the heading, press Delete" — ⌫ deleted the HIDDEN `## `
  prefix before it, turning the heading into body text (the user restored H2). On disk (checked) the file still reads
  `## \x15The letter as weapon` (`chapter-Y.U/Y.Y-scene.md`); the right key is Forward Delete (fn-Delete) at the heading start.
- **2026-10-08 — stray U+0015 removed by Claude, on disk** (user: *"Can't you do it for me?"*; a MacBook keyboard has no Forward
  Delete key). `perl -pi -e 's/\x15//g'` on `chapter-Y.U/Y.Y-scene.md`; the file now begins `## The letter as weapon`; no U+0015 left
  anywhere in the project. Backup is in the session scratchpad. Scrivi was open but had not dirtied that scene.
- **2026-10-09 — Plan 4 (tests) done; it FOUND TWO DEFECTS, both fixed.** Five tests added to `HeadingsRotorTests`, built through
  the app's own `rebuildStorage` (so R2's titles setting is the app's) and walked as VoiceOver walks (`next` from `{0, 0}`).
  - ⛔ **A heading at presented position 0 was left out of the VO-U list:** `next` used `location > cur.location`, and VoiceOver
    starts its list at `{0, 0}`. It hit every manuscript whose page begins with a heading: "Chapter 1" with titles on, the first
    scene's Markdown heading with titles off. ✅ `next` now takes the first heading at or past `NSMaxRange(currentItem)`
    (`ManuscriptAccessibility.swift:268`): a heading item never re-finds itself. ⚠️ A ZERO-length position exactly at a
    heading's start now returns that heading. VoiceOver passed the heading item itself in Plan 1, so this is not expected live.
  - ⛔ **Type-ahead cost 3.3 s per letter on 1.7 MB** (1,200 matches): every call re-extracted and re-matched every label.
    ✅ `PresentedMap.outline(matching:)` (`ManuscriptPresentedMap.swift:397`) caches labels + the filtered list per map
    `version` AND filter → **59 ms** filtered walk, **64 ms** unfiltered (1,260 items).
  - ✅ Tests: `titlesFollowThePage` (R2, both settings) · `fromReadingPosition` (R3: a mid-text position, the caret at the end
    ignored; nil → first/last; none past the last) · `filter` (case, diacritics, matched as presented) · `labelsFollowEdits` ·
    `cost` (1.7 MB, dumas-shaped, < 250 ms per walk). Rotor + presented-map suites green (11 tests).
  - ✅ **Mutation-checked, 6 of 6 caught:** strict `>` (6 failures) · cache ignores filter · cache ignores version · no diacritic
    folding · previous `<=` · no label cache (6.1 s → the cost bound). ➡️ Plan 5 (live pass) next.
- **2026-10-09 — Plan 5 live pass (user, Mac, VoiceOver, dumas): A, B, C, D all PASSED** (user: *"All tests A, B, C, and D
  passed."*) — the list includes the first heading, type-ahead narrows without a pause, heading keys move both ways, titles off
  lists only Markdown headings.
  - ⚠️ **Observed, not yet pinned down** (user): *"sometimes, when voice over is on, and I click in the manuscript, the view jumps
    to a totally different part of the manuscript … if my caret is in Chapter 18 … I might find the display suddenly in Chapter 16
    … when I move the arrow it returns me to where I was."* The caret does not move; only the scroll position does.
  - ✅ **Leading cause found by measurement, and fixed; ⏳ that VoiceOver is the caller is NOT yet proved live.**
    `accessibilityVisibleCharacterRange` is settable (`NSAccessibilityProtocols.h:715`), and its SETTER was the one member the
    S1 map did not translate (the getter is mapped). VoiceOver passes PRESENTED indices, so AppKit scrolled to that number read as
    a STORAGE index: earlier in the book by the hidden markup before it. Measured (`ManuscriptAccessibilityTests.setVisibleRange`,
    2,000 escaped paragraphs): asked for presented 60,390 → the view showed 53,502 (11% early, the "Chapter 18 → 16" shape);
    the caret is untouched, so an arrow key scrolls back, as the user saw.
    ✅ `setAccessibilityVisibleCharacterRange` now maps presented → storage (`ManuscriptAccessibility.swift`), with a DEBUG
    `[SCRIVI-ROTOR] SET visibleCharacterRange` line → 60,185 (within a screen). Mutation-checked: unmapped → 53,502, fails.
    ⚠️ This is a gap in [SP-171]'s S1 map (T-0599, verified), surfaced by this Sprint's live pass.
  - ✅ **Ruled 2026-10-09 (user): NOT filed** — *"Keep it as an untracked fix in the current sprint and lets move on."* The
    visible-range mapping stays in SP-172 as an untracked fix; no Issue. (Likewise the two Plan 4 defects: *"do not file the
    issues."*) ⏳ The DEBUG `SET visibleCharacterRange` live retest was not run.
- **2026-10-09 — ✅ [T-0600] VERIFIED** (user: *"I'm ready to call this verified."*). ✅ AC4 and AC5's rotor half met.
  ➡️ The Sprint stays ACTIVE until the user approves its close; T-0600 is archived in that step.
- **2026-10-09 — ✅ CLOSED (user-approved):** *"close SP-172"*. [T-0600] archived → `Tasks/Verified/Task-verified-0600.md`.

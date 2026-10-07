---
sprint: SP-164
epic: EP-046
status: Closed
closed: 2026-10-06
activated: 2026-10-06
platform: Apple
created: 2026-10-06
---

# SP-164 — `[Apple]` [EP-046] **E2-S4**: Find and Replace over what the writer SEES

**Status:** ✅ **CLOSED 2026-10-06 (user-approved):** *"yes, close SP-164 and run the Audit Check."* — activated 2026-10-06. ✅ Q1–Q6 ruled; all ACs met; [T-0593], [T-0585] VERIFIED and archived.
**Tasks:** ✅ [T-0593] · ✅ [T-0585] → `../../Tasks/Verified/`
**Epic:** [EP-046] `[Apple]` The Manuscript Renderer — Inline Rendering → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md).
⚠️ **EP-046's LAST Sprint** — after it, the Epic's Audit Check and close.
**Carries:** [T-0585] Find/Replace across hidden characters (ruled Q-E2-5: match the PRESENTED text; replacements written
through the escape layer).
**Authority:** [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §7 (Find/Replace),
§0A (Q-E2-5). Previous Sprint: [`Closed/Sprint-SP-163.md`](Sprint-SP-163.md).
**Size:** ⚠️ **M–L — larger than design §7 assumed.**

---

## ⚠️ What reading the code found (2026-10-06)

- ⛔ **There is NO in-manuscript Find today.** No find bar (`usesFindBar` unset), no `NSTextFinder`, no Find items in Scrivi's
  menus. Design §7 assumed an existing Find to re-point; ✅ this Sprint BUILDS one.
- ⚠️ **The Navigator's search** (`SceneNavigatorView` → `loader.searchCaretHint` → `Coordinator.searchMatchOffset`) puts the
  caret on the first match by searching STORED text — so a query with punctuation (`Mr. Smith` → stored `Mr\. Smith`) or across
  a hidden marker (`bold here` → stored `**bold** here`) finds nothing in the scene. The same class as [T-0585].

## Goal

✅ **The writer finds and replaces the text she SEES** — escapes, markers and heading prefixes never get in the way — and a
replacement can never break the formatting or an escape.

## EP-046 ACs this Sprint meets

| AC | Criterion (Epic wording) |
| -- | ------------------------ |
| **AC10** | Find/Replace matches the PRESENTED text; replacements written escaped (Q-E2-5, [T-0585]) |

## Plan

1. **Spike FIRST (design §7's open question): can AppKit's own find bar work on PRESENTED text?** An `NSTextFinder` whose
   `NSTextFinderClient` exposes the manuscript as presented (escape backslashes, emphasis markers and heading/list prefixes
   removed — the presenter already knows each block's hidden units) and maps every range back to storage: `stringAt(index:…)`
   per block (never one 1.85 MB string), `rects(forCharacterRange:)` / `contentView(at:…)` for the find indicator and
   highlights, `replaceCharacters(in:with:)` for Replace. Measure on 1.85 MB dumas: open-the-bar cost, incremental-search cost
   per keystroke, Replace All cost. ⚠️ If it cannot, the fallback is a Scrivi find bar over the same presented model.
2. **The presented model**: per block, presented ↔ source offsets (E1's escape map + E2's hidden units); a match maps to a
   SOURCE range snapped so it never splits an escape pair or a marker.
3. **Replace**: the replacement is ESCAPED (it is typed text) and goes through the balanced-edit path, so a match that crosses a
   span edge stays balanced (AC12) and takes the style of its first character (the type-over rule).
4. **Replace All**: every match in the manuscript, as ONE undoable step even across scenes (grouped history, as [I-0270]'s
   cross-scene delete).
5. **The Navigator's search jump** matches presented text too (same model).
6. **Menus**: Edit ▸ Find — Find… ⌘F · Find and Replace… ⌥⌘F · Find Next ⌘G · Find Previous ⇧⌘G · Use Selection for Find ⌘E.
   ⚠️ Shortcuts to be checked against the app's and macOS's before activation.
7. **Tests** (exact matches over a corpus of escaped text with markers; Replace bytes; one history event for Replace All;
   mutation-checked) and **AC8** re-check (incremental search on 1.85 MB).
8. **Live pass** (user) — steps in the chat reply; Scrivi autosaves.

## ✅ Questions — Q1–Q6 ALL RULED 2026-10-06

| # | Question | Ruling / recommendation |
| - | -------- | -------------- |
| **Q1** | Mechanism: AppKit's find bar (`NSTextFinder` + a presented-text client) or a Scrivi find bar | ✅ **RULED: AppKit's, if plan step 1's spike proves it**; Scrivi's only if it cannot |
| **Q2** | Replace All: one undo step for the whole manuscript, even across scenes? | ✅ **RULED: yes** — one undo step, grouped across scenes |
| **Q3** | A replacement's formatting | ✅ **RULED: the style of the match's first character** (the type-over rule) |
| **Q4** | Does Find search CHAPTER TITLES (shown by the view, stored as chapter metadata, not scene text)? | ✅ **RULED: no** — chapter titles are not searched |
| **Q5** | The Navigator's search jump uses the same presented matching | ✅ **RULED: yes** |
| **Q6** | Scope options: Ignore Case (on by default), Whole Words, Starts With — whatever the find bar offers | ✅ **RULED: what AppKit's bar offers** — Contains / Starts With / Whole Word, Ignore Case on by default |

⛔ **NOT in this Sprint:** project-wide (multi-project) search · the Markup Hints toggle [T-0589] · Linux ([EP-048] L8).

## Acceptance Criteria

- [x] Plan step 1 measured and recorded; Q1 ruled from it — ✅ AppKit's find bar works through the presented-text client
- [x] AC10 — Find matches presented text across escapes, markers and prefixes; Replace writes escaped, balanced text ✅
- [x] Replace All is one undo step (Q2) — ✅ grouped in the coordinator (⏳ live: it needs a project's history)
- [x] The Navigator's search jump lands on presented matches (Q5) ✅
- [x] AC8 re-checked — presented text 166–178 ms to build for 1.85 MB (once, and again only after an edit); searching it is fast ✅
- [x] ✅ **Live pass (user)** — passed 2026-10-06 (pass 2, after the step 9 + 11 fixes)

---

## Progress log

### 🔵 2026-10-06 — Sprint CREATED in Planning

✅ User: *"create E2-S4"*. ✅ ID SP-164 issued by `next-id.py`. ✅ Read first: there is NO in-manuscript Find (searched for
`usesFindBar`, `NSTextFinder`, `performTextFinderAction`, Find menu items — none); the Navigator's search jump matches stored
text (`searchMatchOffset`). ✅ [T-0585] carried.

### 🔵 2026-10-06 — Q1–Q5 RULED (as recommended); Q6 open

✅ User: *"Q1: Recommended. Q2: Yes. Q3: recommended. Q4: no. Q5: recommended."* ⏳ Q6 (search options) not yet answered.

### 🔵 2026-10-06 — Q6 RULED; all questions ruled

✅ User: *"Yes Q6 is ruled er you recommendation."* ✅ Ready to activate on the user's approval.

### 🟡 2026-10-06 — Sprint ACTIVATED (user-approved); [T-0593] allocated

✅ User: *"activate and then implement SP-164"* ✅ [T-0593] issued by `next-id.py`. ✅ Left `Sprint-backlog.md`.

### 🟠 2026-10-06 — IMPLEMENTED (T-0593 + T-0585 Implemented - Not Verified); ⏳ live pass owed

✅ User: *"activate and then implement SP-164"*

**Spike (plan step 1) — 1.85 MB dumas with emphasis, harness with a minimal view stand-in:**
| Measure | Result |
| ------- | ------ |
| presented text built (1,850,583 of 1,883,463 units; 1,186 chunks, one per scene) | **166–178 ms** cold, 143 ms warm |
| `NSTextFinder` Find Next for `nation. Conceived` (stored `nation\. Conceived`) | ✅ selects `nation\. Conceived` |
| … for `writes itsel` (across a hidden closing `**`) | ✅ selects `writes** itsel` |
| … for `The claim stated plainly` (a heading) | ✅ selects the heading text, after its hidden `## ` |
⚠️ Found: the find bar loads the find pasteboard only when its window is KEY (an inactive test host never does — the tests use Use
Selection for Find instead). ✅ **Q1 settled on evidence: AppKit's find bar.**

**What was built:**
- `ManuscriptFind.swift` (new; 3 targets) — `PresentedText` (the manuscript as the writer sees it: no escapes, markers or heading
  prefixes, no chapter titles (Q4); list prefixes stay — they are visible; one chunk per scene, so a match never crosses a scene
  break; `storageRange` / `presentedRange` maps; `firstMatch` for the Navigator, Q5) and `ManuscriptFinderClient` (the
  `NSTextFinderClient`). ⚠️ AppKit runs incremental search on a BACKGROUND queue, so the text it reads is an immutable snapshot
  behind a lock, rebuilt on the main thread only after an edit; everything touching the view runs on the main thread.
  `ManuscriptFindCommand` (cross-platform) carries the menu's commands.
- `ManuscriptTextView.swift` — the view owns an `NSTextFinder` (find bar in the manuscript's scroll view, incremental search on);
  every character edit tells AppKit first (`noteClientStringWillChange`) and invalidates the snapshot; `replaceFound` = typed text
  (escaped, balanced, Q3); the coordinator's `replaceAll` (Q2: replacements with history suppressed, then ONE grouped edit across
  scenes, each touched scene saved); `searchMatchOffset` (the Navigator's jump) matches presented text (Q5).
- `ScriviApp.swift` — **Edit ▸ Find**: Find… ⌘F · Find and Replace… ⌥⌘F · Find Next ⌘G · Find Previous ⇧⌘G · Use Selection for
  Find ⌘E. ⚠️ The Find and Format menus moved into their own `@CommandsBuilder` (a Commands builder takes at most ten entries).
- `ProjectSession.findAction`; `project.pbxproj` — `ManuscriptFind.swift` ×3 targets.

**Tests:** ✅ **202/202 in 23 suites** (+4 in "Find and Replace (EP-046 E2-S4)", the system find pasteboard saved and restored).
✅ **Mutations bite:** presented text hiding nothing → 2 tests red; Replace not escaping → red. ✅ `check-textkit2.sh` clean; ✅ iOS +
visionOS BUILD SUCCEEDED.
⚠️ **Not covered by tests (live only):** incremental search as you type (the background reads); Replace All as ONE undo step (it
needs a project's history); the find bar's own Replace All count.

### 🟠 2026-10-06 — LIVE PASS 1 (user): steps 1–8 + 10 pass; ⚠️ step 9 (Replace All) and step 11 (Navigator) FIXED, re-check owed

✅ User: *"1. passes … 8. passes."* · *"10. passes."*
- ⚠️ **Step 9 — Replace All, worst case (1,172 replacements on dumas):** the replace and the undo both worked, but **the app
  was busy for about a minute** and **the find bar showed no count.**
  - **Cause (busy):** every replacement went through the TYPING path (`replaceFound` → `insertText` → `shouldChangeText` →
    `didChangeText`, ~50 ms each × 1,172); and the UNDO moved the caret and SCROLLED to each of the ~880 scenes in turn (the
    console's `cursorScene` walk 1184 → 0).
  - **Cause (no count):** `shouldReplaceCharacters(inRanges:with:)` did the work itself and returned **false**, so AppKit
    believed nothing was replaced.
  - ✅ **Fix:** `ManuscriptNSTextView.applyReplacements` applies every replacement in ONE editing pass — each escaped,
    escape-snapped and balanced exactly as a single Replace. The client now returns **true** and applies the batch once on
    the first `replaceCharacters` of the batch (the rest are no-ops; `didReplaceCharacters` ends it), so AppKit reports its
    count. A multi-scene undo/redo step places the caret ONCE (no per-scene caret/scroll). The coordinator also updates live
    titles and timeline dot titles, as the cross-scene delete does, and logs `[SCRIVI-FIND] replaceAll … apply= history=`.
  - ✅ **Measured (test): 1,200 replacements in 106 ms.**
- ⚠️ **Step 11 — the Navigator's FILTER** (the list narrowing as you type, not the jump): a query with a comma matched, one
  with a **period did not** — the filter (`SceneNavigatorView.scenesMatching`) still compared STORED text (`Mr\.`). ✅ The
  jump itself landed correctly. ✅ **Fix:** `MarkdownEmphasis.searchable` (a scene as the writer sees it: no markers, no
  heading prefixes, no escape backslashes; list prefixes stay; text with none of `\ * _ #` is returned as is), used by the
  filter.
- ✅ **Tests: 204/204 in 23 suites** (+2: Replace All of 1,200 matches fast and correct; the Navigator's searchable text;
  the Replace test now drives Replace All in AppKit's order should → replace ×N → did). ✅ Mutation: batch without escaping
  → red. ✅ iOS + visionOS BUILD SUCCEEDED; `check-textkit2.sh` clean.

### ✅ 2026-10-06 — LIVE PASS 2 (user): ALL PASS; [T-0593] + [T-0585] VERIFIED

✅ User: *"1 through 6 pass."* Replace All on dumas, twice: `[SCRIVI-FIND] replaceAll 1173 edits / 1171 scenes: apply=83 ms
history=220 ms` (was ~1 minute); the find bar shows its count; one ⌘Z restores every scene.
✅ Step 7: clicking a filtered scene now lands the caret ON the searched text (the Q5 jump), not the scene's start — user:
*"Its a wash. I'll take it! the exigent behavior is approved."*
✅ A pass the user reports is the instruction to verify: [T-0593] and [T-0585] → ✅ Verified. ⏳ Sprint close awaits approval.

### ✅ 2026-10-06 — Sprint CLOSED (user-approved)

✅ User: *"I pushed. yes, close SP-164 and run the Audit Check."* ✅ [T-0593] → `Verified/Task-verified-0593.md`; [T-0585] →
`Verified/Task-verified-0585.md` (same step). ✅ EP-046's last Sprint — its Audit Check follows.

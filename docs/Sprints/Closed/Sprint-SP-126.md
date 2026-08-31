# SP-126: `[Linux]` EP-035 — The three-tab Scene Inspector shell ✅ CLOSED

**Closed:** 2026-08-30 (user-approved) · **Activated:** 2026-08-29
**Epic:** [EP-035](../../Epics/Epic-active.md) — `[Linux]` Object Foundations · **sprint 2**
**Outcome:** ✅ **All six Tasks Verified 2026-08-30** after a live pass on the REAL RIG (build 8).
**Closes:** **AC10**

---

## ✅ What this sprint delivered

✅ **The Scene Inspector's real three-tab shell** — **Writing · Worldbuilding · Properties**, in Apple's
display order, defaulting to Writing.

⚠️ **"Scene Entities" is finally retired.** It was the name of Apple's **SP-090 placeholder**, which
Apple itself deleted — ⚠️ **EP-024 copied the placeholder rather than its successor, and SP-125 then
wired the copy.** ✅ **Found by the user**, not by any process.

- ✅ **Writing tab** — tags, outline, todo, round-tripped through the core.
- ✅ **Properties tab** — derived stats, ⚠️ **read-only IN FACT** (`QLabel`s, which cannot be edited at
  all — I-0148 recorded a *disabled* Apple `TextEditor` that was still editable).
- ✅ **Worldbuilding tab** — SP-125's object list, moved intact.
- ✅ **`inspector-layout.json` PATCHED, never reconstructed** — ⚠️ **17/17 round-trip checks**, including
  a key this build has never heard of.

✅ **571/571 ctest + 19/19 Linux smokes**, natively on the rig at Qt 6.10.2.

## ⚠️ THREE defects found by LOOKING — none by any suite

| | |
| - | - |
| **Properties unreachable** | Three labels do not fit 200px; Qt fell back to ~8px scroll arrows — ⚠️ **a gesture-only affordance in all but name.** Fixed by elision; ✅ **the panel was then WIDENED to 240/400 by user ruling**, with the window grown to 1220 so the manuscript did not pay for it |
| ⚠️ **Outline + To-do NEVER SAVED** | `QTextEdit` has no `editingFinished`. ⚠️ **My own header comment claimed a focus-out hook I had never written** — ⚠️ **a comment asserting behaviour that does not exist is worse than no comment.** Fixed with an `eventFilter`; verified by REOPEN |
| **I-0179 — the relationship label** | ⚠️ **Took THREE rounds; the first two fixed symptoms** |

## ⚠️ I-0179 — where the user found what I kept missing

1. ⚠️ **The message quoted the wrong string** — it recovered a name by string-surgery on the ROW TEXT.
   ⚠️ **A row's visible text is a PRESENTATION; parsing it back apart to recover data is the bug.**
2. ⚠️ **The row itself read wrong** — *"Myton at 23 — features"* implies Myton features something. The
   edge is *Myton **appears in** scene*; the core projects the inverse for the queried endpoint, so
   ⚠️ **the label describes what the SCENE does.**
3. ⚠️ **THE REAL DEFECT: it was REDUNDANT.** A scene relates to its objects the same way every time, so
   one word repeated down the whole list while distinguishing nothing.
   ✅ **User ruling: hoist it to the group header** — `characters (2) (features)`.

⚠️ **My supporting analysis was ALSO wrong and the user corrected it:** I claimed labels varied per
kind, having counted `cites` edges — ⚠️ **but `cites` runs source→object and never touches a scene.**
✅ Only `appears-in` and `located-at` constrain to a scene, so labels are collected from the rows.

## ✅ The dividend — an APPLE defect found by porting

**I-0180**: `ObjectCard.swift:1032-1036` carries the same wrong label and the same duplication,
⚠️ **shipping on macOS since EP-031 and never noticed.** ✅ **Building the surface a SECOND TIME is what
exposed it** — worth remembering with four ports still to come.

---

## SP-126 — `[Linux]` ⚠️ **The three-tab Scene Inspector shell**

**Status:** ✅ **CLOSED 2026-08-30 — user-approved**
**Date Activated:** 2026-08-29
**Epic:** [EP-035](../Epics/Epic-active.md) — `[Linux]` Object Foundations · **sprint 2**
**Codebase:** `[Linux]` — Qt **Widgets** (`QTabWidget`, already the host)
**Closes:** **AC10** · **Tasks:** **T-0485 – T-0490** (six) · **Next available:** T-0492 (T-0491 = unscheduled sort/reorder)

⚠️ **Exists because the user found a GAP in EP-035's plan** (Epic §3a): the Epic never specified the
tab shell at all, and ⚠️ **SP-125 wired a PLACEHOLDER Apple had already deleted.**

### 1. Sprint Goal

**Give the Linux Scene Inspector its real three-tab shell — Writing · Worldbuilding · Properties —
and populate it in APPLE'S PROVING ORDER.**

⚠️ **"Scene Entities" is not a tab and never was in the finished Apple app.** It is the name of Apple's
SP-090 placeholder — *"a single segmented tab over a stub 'Scene Entities' body"* — which
`SceneInspectorView.swift:6` records SP-090 as REPLACING. ⚠️ **Linux copied the placeholder, not its
successor**, and SP-125 then wired the copy.

### 2. ⚠️ Two corrections found at PLANNING (2026-08-29)

#### ⚠️ 2.1 The tab ORDER in this sprint's own draft was WRONG

⚠️ **The draft said `Writing | Properties | Worldbuilding`.** ✅ **Apple's actual on-screen order is
`Writing | Worldbuilding | Properties`** — the tab bar renders `InspectorTab.allCases`
(`SceneInspectorView.swift:100`), which follows the enum's DECLARATION order
(`InspectorCard.swift:19-22`). ⚠️ **Caught by reading the enum rather than trusting the draft.**

✅ **`Writing` is the DEFAULT tab** (`InspectorLayoutStore.swift:78`).

⚠️ **The PROVING order and the DISPLAY order are different things.** ✅ **Display: Writing ·
Worldbuilding · Properties.** ✅ **Build/prove: Writing → Properties → Worldbuilding** (cheap surfaces
first, per the user). ⚠️ **Do not conflate them.**

#### ✅ 2.2 The layout schema ALREADY EXISTS and is ALREADY POPULATED

⚠️ **This is not a format Linux gets to invent.** `inspector-layout.json` sits at the project root,
carries `scrivi.inspector-layout.v1`, and ✅ **the user's real projects already contain it** — verified
in `the-lone-golem.scrivi`:

```
schema        : scrivi.inspector-layout.v1
selectedTab   : worldbuilding          ← project-level, NOT per-scene
inspectorHidden, defaultStacks, stackSort, scenes{ …4 scenes… }
```

⚠️ **It is APP-SIDE, not core-owned** — there is **no `scrivi_*` endpoint** for it; Apple reads and
writes the file directly (`InspectorLayoutStore.swift:155`). ⚠️ **`SceneMetaJson.hpp:53` explicitly
calls it "view configuration"** and keeps it out of scene metadata.

### 3. ✅ USER RULING (2026-08-29) — Linux READS AND WRITES it

> ✅ **Linux honours `scrivi.inspector-layout.v1` exactly, preserving unknown keys on write.**

⚠️ **PATCH, NEVER RECONSTRUCT** (Porting Outline §4.3, T-0436/T-0437). ⚠️ **Linux does not model
`stackSort` or per-scene stacks this sprint, so it MUST round-trip them untouched** — reconstructing
the document from a Qt struct would silently delete the writer's Apple-side card layout, and
⚠️ **the loss would be invisible until she opened the project on the Mac again.**

⚠️ **ACCEPTED RISK, recorded deliberately:** ⚠️ **two app-side implementations of one schema, with no
core to arbitrate.** ⚠️ **If a third platform needs it, MOVE IT INTO ScriviCore rather than writing a
third parser** — that is the point at which the duplication stops being affordable.

### 4. Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0485** | **The three-tab shell** — `Writing \| Worldbuilding \| Properties`, ⚠️ **in that DISPLAY order**, defaulting to **Writing** | **High** | ✅ **Verified 2026-08-30** |
| **T-0486** | ⚠️ **`inspector-layout.json` read + write** — ⚠️ **round-trip `stackSort`, `defaultStacks` and `scenes` UNTOUCHED**; persist `selectedTab` at PROJECT level | **High** | ✅ **Verified 2026-08-30** |
| **T-0487** | **Writing tab** — tags, outline, todo via `setSceneTags`/`setSceneOutline`/`setSceneTodo` | **High** | ✅ **Verified 2026-08-30** |
| **T-0488** | **Properties tab** — ⚠️ **read-only DERIVED stats** from `getSceneNotes`; ⚠️ **read-only IN FACT, not merely styled** (I-0148) | **High** | ✅ **Verified 2026-08-30** |
| **T-0489** | ⚠️ **MOVE SP-125's object list into Worldbuilding** — ⚠️ **no behaviour change; "Scene Entities" RETIRES** | **High** | ✅ **Verified 2026-08-30** |
| **T-0490** | ⚠️ **LIVE pass** (EP-035 AC9) + `ctest` non-root — ⚠️ **file every Issue found** | **High** | ✅ **Verified 2026-08-30** — ⚠️ **on the REAL RIG**; 3 Issues found (I-0179, I-0180, tab truncation) |

### 5. ✅ Blocked on nothing

`getSceneNotes` (`ScriviBridge.hpp:429`) · `setSceneOutline`/`setSceneTags`/`setSceneTodo` (453–455) are
**already bridged**. ✅ **Properties needs NO new call** — it is derived from the same `getSceneNotes`
result. ✅ **The layout file needs no endpoint at all** — it is app-side by design.

### 6. Definition of Done

- [x] ✅ **Three tabs exist** in Apple's display order, defaulting to Writing; ✅ **"Scene Entities" is
      GONE.** ⚠️ **All three had to be made reachable at 200px** — see §6a.1
- [x] ✅ **`selectedTab` persists at PROJECT level** — verified on disk (`worldbuilding` after selecting it)
      and by reopen; ✅ **it does not follow the scene**
- [x] ✅ **A project laid out on APPLE round-trips INTACT** — ⚠️ **17/17 checks in the new
      `inspector_layout_smoke`**, including ⚠️ **a key this build has never heard of.** ✅ **A CORRUPT file
      is left untouched rather than overwritten** — destroying the evidence is worse than losing the layout
- [x] ✅ **Writing tab round-trips** tags / outline / todo, ⚠️ **verified by REOPEN** — ⚠️ **two of the
      three did NOT save at first; see §6a.2**
- [x] ✅ **Properties shows derived stats** (title, words, characters, created/modified with attribution),
      ⚠️ **read-only IN FACT** — they are `QLabel`s, which cannot be edited at all
- [x] ✅ **The object list behaves EXACTLY as SP-125 left it**, now in Worldbuilding — 11 kinds, same read
      path, same two open affordances, same four explained states
- [x] ✅ **No `ObjectKind` name is hardcoded** — grep-proven clean across `platforms/linux/src/`
- [x] ✅ `ctest` **571/571 non-root, tests ON** — ⚠️ **and 19/19 Linux smokes**, including the new one
- [x] ✅ **A LIVE click-through happened on the REAL RIG** (AC9) — ⚠️ **the first SP-126 pass on real
      hardware and real projects.** ✅ **Three defects found and fixed**, all invisible to the suite;
      ⚠️ **one of them was an APPLE defect (I-0180) exposed by building the surface a second time.**
- [x] ✅ **User verification GRANTED 2026-08-30** — *"all verified in app. no findings."* (build 8)

### 6a. ⚠️ FINDINGS — two defects the suite could not see

⚠️ **Both were found by LOOKING AT and TYPING INTO the panel.** ⚠️ **The build was green and all 19
smokes passed with both present**, which is the AC9 argument in miniature.

#### ⚠️ 6a.1 The Properties tab was UNREACHABLE at the default width

⚠️ **Three full tab labels do not fit in the 200px panel**, and `QTabWidget`'s answer is a pair of
~8px scroll arrows. ⚠️ **Properties was reachable ONLY by a tiny arrow** — ✅ **a gesture-only
affordance in all but name**, failing the same rule as T-0482
(`project_linux_vnc_input_constraints`: a remote pass may drop precision as well as modifiers).

✅ **Fixed:** `setUsesScrollButtons(false)` + `setElideMode(Qt::ElideRight)` — all three tabs stay
visible and clickable at any width, shortening rather than disappearing.

#### ⚠️ 6a.2 Outline and To-do were NEVER SAVED

⚠️ **`QTextEdit` has no `editingFinished` signal** — unlike `QLineEdit`. ⚠️ **So Tags saved and the
other two silently did not**: they were only flushed on a scene change, and a writer who types a note
then clicks into the manuscript never changes scene. ⚠️ **The note sat in the widget and was discarded
on the next load.**

⚠️ **My own header comment claimed "commit happens here and on focus-out" — and I had never wired the
focus-out.** ⚠️ **A comment asserting behaviour that does not exist is worse than no comment.**

✅ **Fixed** with an `eventFilter` on `QEvent::FocusOut`. ⚠️ **Deliberately NOT `textChanged`** (a disk
write per keystroke) ⚠️ **and not scene-change alone** (the case that lost the note).

✅ **Verified by REOPEN, not by trusting the write** — tags, outline and todo all come back, with
`[x]` correctly round-tripping as `done: true`.

---

### 7. Explicitly OUT of scope

| Item | Where |
| ---- | ----- |
| ⚠️ **Per-stack sort + card drag-reorder** | ⚠️ **T-0491 (filed 2026-08-29, user-ruled OUT)** — ⚠️ **`stackSort` is still ROUND-TRIPPED, just not edited** |
| **Project-wide object browser** | Later EP-035 sprint (AC2's other half) |
| **Object CRUD / world binding UI** | Later EP-035 sprint (AC3/AC4) |
| **The Detail Sheet** | **EP-036** |
| **Relationships, sources** | **EP-037** |
| ⚠️ **Moving the layout schema into ScriviCore** | ⚠️ **Only when a THIRD platform needs it** — see §3 |

---

---

*Closed 2026-08-30 by user approval, with its six Tasks verified in the same step.*

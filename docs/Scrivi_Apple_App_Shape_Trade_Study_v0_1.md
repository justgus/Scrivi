# Scrivi — What Kind of App Is This? A Top-Down Structural Trade Study v0.1

**Status:** 🟡 **DRAFT — for ruling.** ⚠️ **NOT an approved design document.**
**Date:** 2026-09-14
**Codebase:** `[Apple]` — `Scrivi/Views/`, `Scrivi/App/`
**Companion:** [`Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md`](Scrivi_Apple_UI_Conformance_Trade_Study_v0_1.md)
— ⚠️ **that study asked "does the app follow the rules?"** ✅ **THIS one asks "WHAT IS IT, and what
should it be built as?"**
**Occasioned by:** ✅ **user request 2026-09-14** — ⚠️ ***"determine what kind of app this is overall
and restructure it from the top down as that kind of app."***

---

## 0. Sourcing rule (inherited)

✅ **Every macOS API/guideline claim carries a fetched URL** (⚠️ **author's cutoff is May 2026**).
✅ **Apple doc pages are JS-rendered — use `developer.apple.com/tutorials/data/documentation/<path>.json`.**
⛔ **UNVERIFIED** marks anything unsourced. ⚠️ **Claims about SCRIVI are marked ✅ read / ⛔ not read.**

---

## 1. ⚠️ The user's framing, and the one correction it needs

✅ **The user describes:** ⚠️ *"a three pane layout with an Inspector panel, a Detail panel (i.e. the
ManuscriptView), and a 'master panel' the Scene Navigator. Although it is not a standard
Master/Detail because the Manuscript View remains visible regardless of what Scene is selected."*

✅ **THAT OBSERVATION IS EXACTLY RIGHT, AND IT IS THE KEY TO THE WHOLE QUESTION.** ✅ **§2 names why.**

⚠️ **ONE CORRECTION, AND IT MATTERS FOR THE TOOLBAR DESIGN.** ⚠️ **IT IS NOT THREE PANES. IT IS FIVE
SURFACES IN ONE WINDOW** — ✅ **all read in `EditorView.swift`:**

| # | surface | ✅ site | shown when |
| - | ------- | ------ | ---------- |
| 1 | Scene Navigator | `EditorView.swift:211` | always |
| 2 | Manuscript | `:284` | always |
| 3 | World-warning banner | `:299` | ⚠️ a world is unavailable |
| 4 | Timeline strip | `:322` | `session.timelineVisible` |
| 5 | Scene Inspector | `:339` | `session.inspectorVisible` |
| **6** | ⚠️ **Object Detail Sheet** | `:345+` | ⚠️ **a card asks** — ✅ **a NON-MODAL pane BESIDE the manuscript** |

⚠️ **Six, counting the Detail Sheet — which the code comment explicitly calls *"an editor-level,
NON-MODAL pane… beside the manuscript rather than over it"*.** ⚠️ **A "three-pane" mental model will
under-build the toolbar**, ✅ **because three of these six are toggleable and the toolbar is where a
writer should toggle them.**

---

## 2. ✅ WHAT KIND OF APP THIS IS — the answer

### 2.1 The disqualifying evidence: it is NOT master/detail

✅ **READ IN THE CODE — the Inspector follows `viewportSceneID`, NOT `selectedSceneID`**
(`EditorView.swift`, `inspector(loader:)`):

```swift
SceneInspectorView(
    sceneID: loader.viewportSceneID ?? loader.segments.first?.sceneID,
```

⚠️ **`viewportSceneID` is WHAT YOU ARE SCROLLED TO. `selectedSceneID` is WHAT YOU CLICKED.**
✅ **THE INSPECTOR TRACKS THE FORMER.** ⚠️ **In a master/detail app those are the same thing by
definition — the selection IS what is displayed.** ✅ **Here they are DIFFERENT VARIABLES that
deliberately drift apart, with an explicit loop-breaker between them** (`EditorView.swift:240-252`,
`isMirroringViewportToSelection`).

✅ **THAT IS THE STRUCTURAL PROOF.** ⚠️ **An app that needs a loop-breaker between "selected" and
"displayed" is NOT a master/detail app that happens to misbehave** — ✅ **it is an app whose
navigator is a SCROLL POSITION INDICATOR, and whose selection is a NAVIGATION VERB, not a filter.**

### 2.2 ✅ THE ANSWER: a single-document editor with a navigable outline and a contextual inspector

✅ **Scrivi is ONE DOCUMENT — the manuscript — presented CONTINUOUSLY.** ✅ **Confirmed at three layers:**

| layer | ✅ evidence |
| ----- | ---------- |
| **on disk** | ⚠️ **scenes are separate `NNN-<slug>.md` files** (`SceneIndex.cpp:27`, `ChapterCreator.cpp:99`) |
| **in memory** | ✅ **ONE `NSTextView` holding ALL `1,823,706` characters** ([I-0200], [I-0206]) |
| **on screen** | ✅ **one continuous scroll; scenes are divider attachments, not pages** |

⚠️ **So the scene is a STRUCTURAL UNIT OF THE DOCUMENT, not a document.** ✅ **The navigator navigates
WITHIN one document. That is an OUTLINE, not a master list.**

✅ **THE CLOSEST SYSTEM ANALOGUES — and they are all the same shape:**
⚠️ **Xcode** (navigator / editor / inspector, ✅ **and its navigator is also a jump-to, not a filter**),
⚠️ **Pages and Word** in outline mode, ⚠️ **Scrivener**, ⚠️ **Keynote** (⚠️ *slide* navigator, but the
deck is the document). ✅ **Every one of them: a source-list that MOVES you, a continuous canvas, and
an inspector reflecting WHERE YOU ARE.**

### 2.3 ⚠️ And it is a PROSE editor, not a rich-text editor — ✅ THIS CONSTRAINS THE TOOLBAR

⚠️ **THIS IS THE FINDING THAT CUTS AGAINST PART OF THE USER'S PROPOSAL, AND IT IS BETTER SAID NOW
THAN AFTER A TOOLBAR IS BUILT.**

✅ **READ:** `ManuscriptTextView.swift:32` — **`textView.isRichText = false`**
✅ **READ:** the canonical body is **Markdown**: *"The `.md` file is canonical prose"*
(`Scrivi_Project_Package_Structure_v0_1.md` §9.1), ✅ **and `"format": "markdown"` in the manifest (§9)**.
✅ **READ:** the ONLY font work in the whole view is ONE derived heading font (`:637`); ⚠️ **there is
NO `NSFontManager`, NO font panel, NO `typingAttributes` styling, NO bold/italic command — anywhere.**
✅ **Verified by grep across `Scrivi/Views` + `Scrivi/App`.**

⚠️ **THEREFORE: *"string attribution controls like font size/selection/style"* AS LITERALLY PROPOSED
DOES NOT FIT THIS APP.** ⚠️ **A font-size/typeface control implies character-level styling persisted
with the text.** ✅ **This app's text model CANNOT persist that** — ⚠️ **`isRichText = false` and a
Markdown file have nowhere to put "14pt Helvetica on this run".** ⚠️ **Adding such controls would
either (a) silently discard the styling on save, or (b) force a document-format change — ✅ which is
an ARCHITECTURE decision, not a toolbar decision.**

✅ **WHAT DOES FIT, and gives the user what they actually want:**

| ⚠️ proposed | ✅ the fitting form | why |
| ----------- | ------------------ | --- |
| ⚠️ font size / typeface | ✅ **a VIEW preference** (⚠️ **display-only, per-device, NOT saved in the `.md`**) | ✅ **writers do want bigger text; it is a lens, not content** |
| ⚠️ "style" (bold/italic) | ✅ **MARKDOWN SEMANTIC MARKS** — ⚠️ **insert `**`/`*`, do not style runs** | ✅ **round-trips through the canonical format** |
| ✅ scene/chapter actions | ✅ **TAKE AS PROPOSED — ⚠️ ALREADY EXIST in the menu bar** | ✅ **§3.2 — they need SURFACING, not building** |
| ✅ file export | ✅ **TAKE AS PROPOSED** | ✅ **document-level, toolbar is right** |

⚠️ **IF the user wants TRUE rich text, that is a legitimate but MUCH larger decision** — ✅ **it
changes the on-disk format, the C++ schema, cross-platform parity (Qt/Linux renders the same files)
and the TextKit 2 work now in flight.** ⚠️ **It must be ruled deliberately, NOT arrived at by adding
a font popup to a toolbar.**

---

## 3. ✅ What the app ALREADY has — the toolbar is mostly a SURFACING job

### 3.1 ⚠️ There is no toolbar at all

✅ **`NSToolbar` appears ZERO times in `Scrivi/Views` + `Scrivi/App`** (grep, verified).
✅ **The project window is a bare `NSHostingView` in a `.titled` window** (`ProjectWindowManager.swift:88-93`).
✅ **`.toolbar` on macOS appears in exactly TWO places, BOTH sheets.**

### 3.2 ✅ …but the ACTION VOCABULARY IS ALREADY COMPLETE, in the menu bar

✅ **READ in `ScriviApp.swift:141-306`** — the app already defines:

| menu | actions |
| ---- | ------- |
| **File** | ✅ New / Open / Close Project |
| **Project** | ✅ Project Settings… |
| **Worlds** | ✅ Manage Worlds… |
| **Scene** | ✅ **New (⌘↩), Merge with Previous (⌘⌫), Go to Scene Start/End** |
| **Chapter** | ✅ **New (⇧⌘↩), Merge with Previous (⇧⌘⌫), Go to Chapter Start/End** |
| **View** | ✅ **Show Scene Inspector / Timeline / World Warnings / Buffers** |
| **Edit** | ✅ **multiple copy Buffers** (EP-019) |

⚠️ **THIS CHANGES THE SHAPE OF THE WORK.** ✅ **A toolbar is NOT new functionality — it is a SECOND,
DISCOVERABLE SURFACE onto commands that already exist and already have shortcuts.**
✅ **The actions are routed through `focusedSession?.<verb>Action?()`**, ⚠️ **so a toolbar button can
call the SAME closure the menu item calls.** ✅ **No new plumbing; that is the cheap part.**

⚠️ **AND IT EXPLAINS A REAL DEFECT:** ✅ **the four View toggles are the ONLY way to reveal the
Inspector and Timeline.** ⚠️ **[I-0203] shows a writer losing the navigator with *"no affordance to
bring it back"*** — ✅ **because the affordance is in a menu, not on screen.**

---

## 4. ✅ The restructure, top down

### 4.1 ⚠️ The correct macOS container for this app type

✅ **A three-column source-list/content/inspector app on macOS 26+ is expressed as:**

| layer | ✅ correct API | ⚠️ Scrivi today |
| ----- | ------------- | --------------- |
| window | ✅ **`NSWindow` + `NSToolbar`** | ⚠️ **no toolbar** |
| columns | ✅ **`NSSplitViewController` / `NavigationSplitView`** | ✅ **`NavigationSplitView` (2 cols) + ⚠️ a HAND-ROLLED 3rd** |
| bars attached to a column | ✅ **`NSSplitViewItemAccessoryViewController`** (macOS 26.0+) / **`safeAreaBar`** (macOS 26.0+) | ⚠️ **plain `VStack` siblings** |
| title | ✅ **rendered by the toolbar** | ⚠️ **`.navigationTitle` with nothing to render into** |

✅ **Sources:** [`NSSplitViewItemAccessoryViewController`](https://developer.apple.com/documentation/appkit/nssplitviewitemaccessoryviewcontroller)
(macOS 26.0+) · [`safeAreaBar`](https://developer.apple.com/documentation/swiftui/view/safeareabar(edge:alignment:spacing:content:)) (macOS 26.0+)
· [WWDC25 §310](https://developer.apple.com/videos/play/wwdc2025/310/) — ✅ **the scroll edge effect is
applied automatically beneath toolbar items, titlebar accessories and split item accessories.**

⚠️ **THE INSPECTOR IS A HAND-ROLLED THIRD COLUMN** (`SceneInspectorView.swift:39-52`: an `HStack`
with a `resizeHandle` and `.frame(width: paneWidth)`), ⚠️ **conditionally inserted into an `HStack`
by `EditorView.swift:338`.**

✅ **SwiftUI HAS THE EXACT API FOR THIS, AND IT IS NOT NEW:** **`.inspector(isPresented:content:)`,
macOS 14.0+**
([source](https://developer.apple.com/documentation/swiftui/view/inspector(ispresented:content:))) —
⚠️ **six major versions BELOW Scrivi's 27.0 deployment target, so availability is a non-issue.**
✅ **Apple documents it as presenting a TRAILING COLUMN, ✅ AUTOMATICALLY RESTORING its presentation
state, and ✅ pairing with `InspectorCommands` for the default toggle command and keyboard shortcut.**

⚠️ **THREE THINGS SCRIVI HAND-BUILT ARE THEREFORE ALREADY PROVIDED:** ⚠️ **the trailing column, ✅ the
show/hide persistence (`session.inspectorVisible` + the View-menu toggle), and ✅ the menu command
itself.** ✅ **`@AppStorage("inspectorPaneWidth")` may still be wanted for width; ⛔ whether `.inspector`
exposes resizing to the same degree was NOT sourced and must be checked before S4 is specified.**

### 4.2 ✅ The proposed toolbar, derived from §2 and §3

⚠️ **PRINCIPLE: the toolbar surfaces DOCUMENT-LEVEL and STRUCTURE-LEVEL verbs.**
⚠️ **It does NOT become a formatting bar (§2.3), and it does NOT duplicate the whole menu bar.**

| group | items | ✅ source |
| ----- | ----- | -------- |
| **Structure** | ✅ New Scene · New Chapter · Merge | ✅ **exists** — `focusedSession` closures |
| **Panes** | ✅ **Toggle Navigator · Inspector · Timeline** | ✅ **exists** — View menu toggles; ⚠️ **fixes [I-0203]'s missing affordance** |
| **Document** | ⚠️ **Export…** | ⛔ **does not exist yet** |
| **View** | ✅ **text-size lens** (§2.3) | ⛔ **does not exist yet** |
| **(trailing)** | ✅ **the window title/subtitle, in the toolbar where they belong** | ⚠️ **currently a stray in-content band** |

⚠️ **ORDERING NOTE:** ✅ **the Structure and Panes groups are FREE (they re-use existing closures).**
⚠️ **Export and the text-size lens are NEW FEATURES and should not be bundled into a structural
restructure** — ✅ **they are what the toolbar makes ROOM for, in a later sprint.**

### 4.3 ✅ Sequence — ⚠️ and what must NOT move yet

| step | work | ⚠️ risk |
| ---- | ---- | ------ |
| **S1** | ✅ **Add `NSToolbar` to the project window; move title/subtitle into it** | ✅ **LOW** — ⚠️ **additive; fixes conformance F1/F2** |
| **S2** | ✅ **Populate it from EXISTING closures** (Structure + Panes) | ✅ **LOW** — ✅ **no new logic** |
| **S3** | ✅ **Convert the three bars to `safeAreaBar`** | ✅ **MEDIUM** — ✅ **fixes [I-0203] structurally** |
| **S4** | ⚠️ **Make the Inspector a REAL third column** | ⚠️ **MEDIUM-HIGH** |
| **S6** | ⚠️ **Resolve the Object Detail Sheet's hosting + its HAND-BUILT toolbar** (✅ **§4.4 below**) | ⚠️ **MEDIUM-HIGH** — ⚠️ **must follow S4 (same trailing edge)** |
| **S5** | ⚠️ **`NSSplitViewController` rebuild** | ⛔ **HIGH — NOT NOW (§5)** |

⚠️ **S1+S2 ALONE ANSWER THE USER'S ASK AND FIX A SHIPPED DEFECT.** ✅ **They are the recommended
first sprint.**

### 4.4 ⚠️ S6 — the Object Detail Sheet, ✅ **the THIRD instance of the same class**

⚠️ **ADDED 2026-09-14 after user review.** ⚠️ **v0.1 enumerated this surface in §1 as #6 and then gave
it NO STEP.** ✅ **That was an omission in the study, not a miscount by the reader** — ⚠️ **and the
surface turns out to carry the study's own thesis more clearly than any other.**

✅ **READ — `ObjectDetailSheet.swift` has a `private var toolbar`** that is an `HStack` of
`.borderless` buttons:

```swift
private var toolbar: some View {
    HStack(spacing: 8) {
        Button { requestStep(back: true)  } label: { Image(systemName: "chevron.backward") }
        Button { requestStep(back: false) } label: { Image(systemName: "chevron.forward")  }
        Spacer()
        Button("Cancel") { revert() }
        Button("Save")   { save() }.keyboardShortcut("s", modifiers: .command)
        Button { requestClose() } label: { Image(systemName: "xmark") }
    }
    .buttonStyle(.borderless).padding(8)
}
```

⚠️ **THAT IS WINDOW CHROME, HAND-BUILT INSIDE A CONTENT PANE:** ✅ **navigation history (back/forward),
a save affordance, and a CLOSE BUTTON.** ✅ **It is hosted at `EditorView.swift:346` as a conditional
`HStack` member with `.frame(minWidth: 420, idealWidth: 520, maxWidth: 720)` and a `.transition`.**

✅ **SO THE CLASS IS NOW THREE-DEEP, and this is the study's central finding restated:**

| surface | ⚠️ hand-builds | ✅ the API that exists |
| ------- | -------------- | -------------------- |
| bars (banner/timeline/tab bar) | ⚠️ `VStack` siblings | ✅ `safeAreaBar` (26.0+) |
| Scene Inspector | ⚠️ `HStack` + `resizeHandle` + `.frame(width:)` | ✅ `.inspector` (14.0+) |
| **Object Detail Sheet** | ⚠️ **a toolbar `HStack`, incl. a close button** | ⚠️ **§4.4 — NOT A SIMPLE SWAP** |

⚠️ **UNLIKE S3 AND S4, S6 HAS NO ONE OBVIOUS TARGET API, ✅ AND THAT IS WHY IT IS ITS OWN STEP.**
✅ **The hosting question was ALREADY RULED ONCE and must not be silently re-litigated** — ✅ **the code
records it (`EditorView.swift:56-64`):** ⚠️ **D1-C (push into the 280pt inspector) was REJECTED because
that width is wrong for long-form notes; ⚠️ D1-B (a separate window) was DEFERRED because Scrivi has
no auxiliary window type and [EP-018] documents what adding one costs.** ✅ **D1-E (the non-modal
beside-pane) is what shipped.**

⚠️ **THREE OPTIONS, TO BE RULED — ✅ NOT decided by this study:**
✅ **(a) KEEP D1-E, fix only the chrome** — ⚠️ **the buttons become a `safeAreaBar(edge: .top)` so the
pane stops hand-rolling a bar. ✅ Cheapest; ⚠️ a close button still lives in content.**
✅ **(b) PROMOTE to a real auxiliary window** — ✅ **D1-B's successor; ✅ gets a REAL `NSToolbar`,
real close, real title.** ⚠️ **Costs the window type [EP-018] warned about.**
✅ **(c) PROMOTE to a second `.inspector`-style trailing column** — ⚠️ **conflicts with S4's inspector
for the same edge; ✅ must be reconciled, not stacked.**

⚠️ **NOTE THE COUPLING: (c) and S4 CONTEND FOR THE SAME TRAILING EDGE.** ✅ **S6 must therefore be
sequenced AFTER S4, or the two will be designed against each other.**

---

## 5. ⚠️ The constraint that governs sequencing

⚠️ **[SP-133] IS MID-FLIGHT ON THE MANUSCRIPT SURFACE (TextKit 2).** ⚠️ **[I-0206] is open on the same
view.** ✅ **S1–S3 do NOT touch `ManuscriptTextView`'s internals** — ⚠️ **they touch the window, the
shell and the bars around it.** ✅ **That is deliberate and it is why the sequence is ordered this way.**

⚠️ **S4/S5 DO touch the shell that [I-0132]/[I-0161] show is delicate** (⚠️ **selection/viewport loop,
focus transfer**). ⚠️ **They must NOT be attempted while TextKit 2 is unlanded.**

---

## 6. ⛔ What this study does NOT establish

- ⛔ **Whether `.inspector` supports Scrivi's USER-RESIZABLE width** (`@AppStorage("inspectorPaneWidth")`,
  220–560pt). ✅ **The modifier itself IS sourced (macOS 14.0+, §4.1); ⚠️ its resize affordance is not.**
- ⛔ **Whether a true rich-text model is wanted** (§2.3) — ⚠️ **a ruling, not a finding.** ⚠️ **It would
  change the on-disk format, the C++ schema AND Linux/Qt parity.**
- ⛔ **Export's scope** — ⚠️ **named as a toolbar slot only; no format, no design.**
- ⛔ **iOS/iPadOS/visionOS.** ⚠️ **The iOS branch ALREADY has `.toolbar` (`EditorView.swift:189`) and a
  different shape; ✅ it needs its own pass.**
- ⛔ **Whether the Detail Sheet should become a real window** — ⚠️ **[EP-018] documents what an
  auxiliary window type costs; ✅ deferred deliberately, not overlooked.**

---

## 7. ✅ Summary for ruling

1. ✅ **Scrivi is a SINGLE-DOCUMENT EDITOR with a navigable OUTLINE and a CONTEXTUAL INSPECTOR** —
   ⚠️ **the Xcode/Scrivener shape, NOT master/detail.** ✅ **Proven by the Inspector tracking
   `viewportSceneID` rather than `selectedSceneID`.**
2. ✅ **A proper toolbar is the RIGHT call and is mostly a SURFACING job** — ⚠️ **the verbs already
   exist in the menu bar and are already routed through callable closures.**
3. ⚠️ **Font size/typeface/style controls as proposed DO NOT FIT** — ✅ **`isRichText = false`, canonical
   Markdown.** ✅ **Offer a display-only text-size lens + Markdown semantic marks instead**, ⚠️ **or rule
   the rich-text question separately and knowingly.**
4. ✅ **It is SIX surfaces, not three** — ✅ **and three are toggleable, which is what the toolbar is for.**
5. ⚠️ **The Inspector hand-builds what `.inspector` (macOS 14.0+) has provided since before this app
   targeted 27.0** — ✅ **trailing column, state restoration and the menu command, all for free.**
6. ✅ **Do S1–S3 now; ⚠️ hold S4–S5 until TextKit 2 lands.**

# Sprint SP-236 — `[Linux]` [EP-059] C: relationships, sources and navigation in the detail view (C1–C4)

**Status:** ✅ **CLOSED** 2026-11-22 (user, 2026-11-22: *"Step 2 passes.  Close SP-236."*). Live pass: steps 1, 3–15 passed first time; step 2 after the scene-name fix. 🟡 Activated
2026-11-22 (user, 2026-11-22: *"Q1-Q6 as recommended, implement SP-236"*); ✅ Q1–Q6 as recommended. Created 2026-11-22 (user, 2026-11-22: *"close SP-235, then plan C1-C4"*).
**Carries:** [T-0698] (C1–C3), [T-0699] (C4).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md) — part **C**, on [SP-235]'s detail view.
**Size:** L — two sections, three pickers and a stack in an existing dialog, and one card. Every endpoint is bridged (`listEdgesFor`,
`listRelationTypes`, `createEdge`, `deleteEdge`, `createObject`, `listObjects`, `openObject` in `ScriviBridge.hpp`).

---

## What reading the code found (2026-11-22)

**The Mac's Related section** (`Scrivi/Views/Detail/ObjectRelationsSection.swift`, under Sources in the sheet):
- Header **"Related"** with **Relate…** (disabled when read-only: *"This object's world is unavailable, so its relationships are
  frozen."*). Empty: **"Not related to anything yet."**
- Rows from `listEdgesFor(objectID)`, **grouped by the edge's label as the core gives it from THIS end** (C1: never recomputed; one
  edge reads `forwardLabel` from one end and `inverseLabel` from the other). Groups and rows sorted by name. Each row: the other
  end's name (or "Untitled <kind>", or the scene's name), its kind under it.
- **A pending far end** (its world away) stays listed: ⚠, greyed, "Held pending — “<world>” is <status>."; it cannot be shown or
  removed.
- Double-click or **Show** opens the other object **in the same sheet** (C3); on a **scene** row it takes the manuscript to that
  scene instead. **Remove Relationship** deletes the edge. ⚠️ It does not filter out `cites` edges: a source also shows here.
- **Relate…** opens `ObjectRelationPicker.swift`: **"Relate to…"**, a relationship menu (only the types whose source accepts this
  object's kind and whose target is an object — one type shows as text), **"Search all worlds…"**, *"Objects from every world in
  this project are listed."*, the objects grouped by world (project-scoped first), this object left out, an already-related one ticked
  and disabled, a kind the type cannot point at disabled with a reason. Empty: *"This project has no relation type that can link two
  objects."* / *"There are no other objects in this project yet."* / *"No match."* Choosing one runs `createEdge` and the row
  appears; the other object's sheet shows it from its own end (C2).

**The Mac's navigation** (`ObjectDetailSheet.swift`, `ObjectDetailHistory.swift`):
- A `NavigationStack`: opening a related object **pushes** it; the title is the current object's name; **Back** (the platform's
  chevron) returns. ⚠️ **No Forward — retired by the user, 2026-09-23:** *"Forward buttons are not required. We have navigate
  forward by double clicking the object reference."* [EP-059] C3 still says "with back and forward" (from EP-037 AC3) — stale (Q2).
- **Pushing with unsaved changes** asks Save · Discard · Cancel (the close prompt). **Back with unsaved changes** asks only Save ·
  Discard, after the pop — the user's ruling, 2026-09-23: *"This confirmation dialog shouldn't attempt to 'Cancel' the navigation …
  rather it should require either a save or a revert."* A push of the object already showing does nothing.

**The Mac's Sources section** (`ObjectSourcesSection.swift`, between Image and Related):
- Header **"Sources"** with **Cite…** and **New Source…** (both off when read-only). Empty: **"No sources cited yet."** (read-only:
  "No sources cited.").
- Rows: the `cites` edges whose other end is a `source`. Click: the **citation popover** — name, subtitle, "Cited by <this>, and by
  <others>", the citation fields that are set (Author, Year, Publisher, Page, URL, Accessed, from the source's `attributes`), notes.
  Double-click / **Show Source**: the source pushed in the sheet. **Remove Citation** deletes the edge.
- **New Source…**: "New source" / "Documenting “<name>”." / a "Title of the work" field / *"Author, year and the rest are edited on
  the source itself — double-click it once it appears."* / Cancel · Create. It creates a project-scoped `source` and a
  `source —cites→ object` edge. If the edge fails, the source is kept and an orange note says to use Cite….
- **Cite…**: "Cite an existing source", "Search sources…", the project's sources, one already cited disabled ("Already cited
  here"). Empty: *"This project has no sources yet. Use “New Source…” to create one."*

**The Mac's scene Sources card** (`Scrivi/Views/Inspector/SourcesCard.swift`, typeID `sources`, the Writing stack): the sources
reached through **this scene's objects** (scene → objects → `cites` → source), one row per source with "via <objects>". Click: the
same citation popover; double-click / **View Detail**: the source's detail; **Show Citation**. Empty: "No sources cited by this
scene's objects." It is read-only.

⚠️ **Two things C4 names that the Mac does not have** (Q4):
- **Editing the citation fields.** The popover shows Author, Year …, and New Source… says they are *"edited on the source itself"*,
  but the detail sheet shows no field for them — nothing in the app writes them (`CitationField` is used only to display;
  `git log -S` finds no editor ever). The create form's sentence is untrue on the Mac.
- **Footnote text.** Nothing in the app has it; `SourcesCard.swift:19` defers footnotes to [EP-032].

**Linux today:**
- `ObjectDetailDialog` ([SP-235]) has no Related or Sources section and no navigation; it opens one object.
- The Writing stack's + menu lists no Sources card (`SceneInspector.cpp:352`, *"The Mac's Sources card is C4's, not built on Linux
  yet"*); a layout naming `sources` shows it as a card this version lacks.
- `EditorShell::selectNavigatorScene` (`EditorShell.cpp:1652`) is how a scene is shown, for a scene row's Show.

## Plan

1. **Navigation** (C3, [T-0698]): the dialog holds a trail of entries. A push re-opens the dialog's content on the new object (the
   same `reload`); **Back** — a toolbar action at the leading end, Alt+Left, tooltip "Back to <name>", hidden at the root — pops.
   The window title follows the current object. The prompts as the Mac's (Q3).
2. **Related** (C1, C2, [T-0698]): the section under Sources, as the Mac's: rows grouped by the core's label, pending rows explained,
   double-click / Show, Remove Relationship, scene rows show the scene. **Relate…** opens the picker, as the Mac's. All on a worker.
3. **Sources** (C4, [T-0699]): the section above Related, as the Mac's: Cite…, New Source…, the citation popover, Show Source,
   Remove Citation.
4. **The scene Sources card** (C4, [T-0699]): typeID `sources` in the Writing stack's + menu, as the Mac's.
5. A write reloads the Inspector (I-0155), as [SP-235]'s.
6. **Tests** (AC6) and the **live pass** (AC7), written for `dumas-prose-timelines-history` on the VM.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **One Sprint or two.** C1–C3 and C4 are separable; C4's Show Source needs C3's navigation. | **One Sprint, two Tasks** ([T-0698] C1–C3, [T-0699] C4) — one live pass on the same dialog. | ✅ As recommended (2026-11-22) |
| **Q2** | **Forward.** C3 says "back and forward"; the Mac retired Forward (user, 2026-09-23). | **Back only, as the Mac;** double-click is forward. Amend C3's wording. | ✅ As recommended (2026-11-22) |
| **Q3** | **Back's prompt.** The Mac can't stop a pop, so it asks Save · Discard after it. Linux owns its Back button and *could* offer Cancel. | **As the Mac's ruling:** Back with unsaved changes asks Save · Discard (no Cancel); a push asks Save · Discard · Cancel. | ✅ As recommended (2026-11-22) |
| **Q4** | **Citation fields and footnote text** — C4 names both; the Mac has neither. | **As the Mac:** the citation fields shown (read-only) in the popover; no footnote text ([EP-032]); C4's wording amended. File an `[Apple]` Issue: the fields can't be entered anywhere, and New Source…'s sentence says they can. Editing them is a later Mac-first Sprint, then Linux. | ✅ As recommended (2026-11-22) |
| **Q5** | **`cites` edges in Related.** The Mac lists a source in both Sources and Related. | **As the Mac** (both). | ✅ As recommended (2026-11-22) |
| **Q6** | **Where the pickers open.** The Mac uses popovers under the buttons. | **A small window-modal dialog over the detail view** for Relate…, Cite… and New Source…, and a popup for the citation — as [SP-235]'s Choose Existing…. | ✅ As recommended (2026-11-22) |

## What was built (2026-11-22)

- **Navigation (C3, [T-0698]):** `ObjectDetailDialog` keeps a trail. A related object or a source opens **in the same window**
  (`navigateTo` → `showEntry`, which starts the content over and re-reads); the title follows. **Back** (`detailBack`) sits at the
  toolbar's leading end, Alt+Left, tooltip "Back to <name> (Alt+Left)", shown only when there is somewhere to go. ✅ Q2: no
  Forward. ✅ Q3: opening another object with unsaved changes asks **Save · Discard · Cancel**; **Back** asks **Save · Discard**
  ("Save your changes to “<name>”?" / "Going back without saving will discard them."). Save then moves; a failed save stays.
  A re-visit of the object showing does nothing.
- **Related (C1, C2, [T-0698]):** under Sources: **Related** with **Relate…**. Rows from one `listEdgesFor` (in the dialog's read),
  grouped by the label the core gives from this end and sorted by name; the kind beside each. A pending far end stays listed:
  "⚠ <name>", "Held pending — “<world>” is <status>."; it does not open and cannot be removed. Double-click / Return / **Show**
  opens the object in place; on a **scene** row the manuscript goes to that scene (`showSceneRequested` → `EditorShell`, the
  navigator's path). **Remove Relationship** deletes the edge. Empty: "Not related to anything yet." ✅ Q5: a source shows here too.
- **Relate… (C2):** a small window over the detail view (✅ Q6), `relatePicker`: "Relate to…", the relationship menu (only types from
  this kind to an object; one type shows as text), "Search all worlds…", the objects grouped by world (project scope first), this
  object left out, an already-related one ticked and disabled, a kind the type cannot point at disabled with the Mac's reason.
  Choosing one runs `createEdge`; the Mac's three empty sentences.
- **Sources (C4, [T-0699]):** between Image and Related: **Sources** with **Cite…** and **New Source…**. A click shows the **citation
  window** (`citationPopup`: name, subtitle, "Cited by <this>, and by <others>", the citation fields that are set — Author, Year,
  Publisher, Page, URL, Accessed — and notes); a double-click (the click waits out the double-click interval, so one gesture does one
  thing) or **Show Source** opens the source in place; **Remove Citation**. **New Source…** creates a project-scoped source and its
  `source —cites→ object` edge; if the edge fails, the source is kept and the message says to use Cite…. **Cite…** lists the
  project's sources, one already cited disabled ("— Already cited here"). ✅ Q4: the fields are shown, not edited; no footnote text;
  the form does not repeat the Mac's untrue sentence → [I-0370].
- **The scene's Sources card (C4):** typeID `sources` in the Writing stack's + menu (`SceneInspector`). The citations ride the
  scene's one read (`loadSceneInspector(…, includeCitations)`) only when the card is in the scene's stack, as the Mac's
  (`SceneInspectorView.wantsCitations`); adding the card re-reads. One row per source, "via <objects>"; click: the citation;
  double-click / **View Detail**: the source's detail; **Show Citation**. Empty: "No sources cited by this scene's objects."
- **Read-only:** Relate…, Cite…, New Source…, Remove Relationship and Remove Citation are off while the object's world is away, with
  "This object's world is unavailable, so its relationships are frozen." / "…its citations are frozen."
- Every read and write runs on a worker; a write reloads the Inspector (I-0155). Lists inside the dialog take their rows' height
  (I-0159: one scroll region).

**Tests (AC6):** `object_detail_smoke` §8–§12 (new) through a real project window: §8 Related grouped by the core's label, Relate…
offers only fitting types and kinds, the edge read from the other end; §9 in-place navigation, Back named, Back's two-way prompt
(no Cancel), the three-way prompt on opening another (Cancel stays, Save writes first), a scene row asks for the scene, Remove
Relationship; §10 New Source…, the citation's fields and "Cited by Cara, and by Dov", Cite… with one disabled, Remove Citation, a
double-click opens the source and does not also show the citation; §11 the Sources card "via" the scene's objects, opening the
source; §12 the graph verbs off while the world is away. A mutation (the type filter removed, in a copy of the repo) fails §8.
`inspector_stack_smoke`'s "card this build lacks" is now `futureCard` (it was `sources`). §5 now waits for the replaced asset's
removal (a race the new fixture objects widened). Docker (Qt 6.4, non-root): ctest **884/884**, smokes **49/49**; the detail smoke
5 runs in a row.

## Live pass (AC7) — the Linux VM

On `dumas-prose-timelines-history` (world Dumas-Characters-Places), with Abbé Faria and Edmond Dantès.

1. Pull, build (`-j3`), launch Inkwyn, open `dumas-prose-timelines-history`. Inspector ▸ Worldbuilding ▸ the Characters card.
2. Double-click Abbé Faria. Under Image: **Sources** (Cite…, New Source…, "No sources cited yet."), then **Related** (Relate…, a
   group "appears in" listing his scenes). No Back arrow at the top left.
3. Relate…: "Relate to…", a menu with "sibling of" and "cites", "Search all worlds…", objects grouped by world; Abbé Faria not listed;
   with "sibling of", non-characters greyed. Type `Edmond`, click Edmond Dantès: Related gains "sibling of" ▸ Edmond Dantès.
4. Double-click a scene under "appears in": the manuscript behind moves to it; the detail window stays.
5. Double-click Edmond Dantès: the window shows him; a Back arrow appears ("Back to Abbé Faria"); his Related has "sibling of" ▸
   Abbé Faria.
6. Type `X` in his Notes, click Back: "Save your changes to “Edmond Dantès”?" with only Save and Discard. Discard: Abbé Faria, no
   Back arrow.
7. Type `Test.` in Faria's Notes, double-click Edmond Dantès: "Save your changes?" — Save, Discard, Cancel. Cancel stays. Again, Save:
   Edmond Dantès. Back (Alt+Left): Abbé Faria without a question, "Test." in Notes.
8. Right-click Edmond Dantès under "sibling of": Show, Remove Relationship. Remove: the group goes.
9. New Source…: "New source", "Documenting “Abbé Faria”.", Create greyed. Type `Test Source`, Create: Sources lists it.
10. Click Test Source once: a window "Test Source", "Cited by Abbé Faria". Close.
11. Cite…: "Cite an existing source", Test Source "— Already cited here", greyed. Esc.
12. Double-click Test Source: the window shows it; "Source", no World line; Back.
13. Close. In the Inspector's Writing tab (the caret is in his scene from step 4): + ▸ Sources. The card: "Test Source / via Abbé
    Faria". Click: its citation ("via Abbé Faria"). Double-click: Test Source's detail. Close.
14. `cd /mnt/scrivi-net/projects/Scrivi-Dumas-Fixture`, `mv Dumas-Characters-Places.inkwynworld Dumas-away`; open Abbé Faria, click
    in: Relate…, Cite…, New Source… greyed; Relate…'s tooltip says relationships are frozen. `mv Dumas-away
    Dumas-Characters-Places.inkwynworld`, click in: they come back.
15. Clean up: right-click Test Source ▸ Remove Citation ("No sources cited yet."); delete "Test." from Notes, Ctrl+S; close; ✕ the
    Sources card if wanted. (Test Source stays in the project: Linux has no Delete Object yet — A9.)

## Acceptance criteria

- [x] AC1 — **C1:** the detail view lists the object's relationships, grouped by the label the core gives from this end; a pending
      far end is listed, named and explained; Remove Relationship deletes the edge.
- [x] AC2 — **C2:** Relate… creates a relationship of a type that accepts both ends; the other object's detail shows it from its end.
- [x] AC3 — **C3:** a related object or source opens in the same window; Back returns, named; unsaved changes are asked about as Q3;
      a scene row shows the scene.
- [x] AC4 — **C4:** New Source… creates a source documenting the object; Cite… cites an existing one; the citation popover shows the
      fields that are set and who else cites it; Remove Citation; the scene's Sources card lists the sources its objects cite.
- [x] AC5 — Read-only (its world away): Relate…, Cite…, New Source… and Remove are off, with the reason.
- [x] AC6 — Tests: a smoke through a real project window for AC1–AC5; ctest and every smoke green in Docker; CI green. ✅ Docker:
      ctest **884/884**, smokes **49/49**; ✅ CI green on `2640388` (run 45103996428).
- [x] AC7 — Live pass on the Linux VM, on an existing project there. ✅ `dumas-prose-timelines-history` (user, 2026-11-22: *"Step 2 passes.  Close SP-236."*)

## Progress log

- **2026-11-22** — ✅ **CLOSED** (user, 2026-11-22: *"Step 2 passes.  Close SP-236."*): step 2 passes after the fix; T-0698, T-0699 ✅ Verified, archived.
- **2026-11-22** — Built: AC1–AC5 met; AC6 green in Docker (884/884, 49/49); T-0698, T-0699 → 🟠; [I-0370] filed (Q4). ⏳ AC7 live pass, CI.
- **2026-11-22** — Live pass 1: steps 1, 3–15 pass. ⚠️ Step 2: every scene under "appears in" read "Untitled scene" — the Dumas scenes have no stored title, so the core's name is empty, and Linux skipped Apple's fallback to the Navigator's name (`ObjectRelationsSection.swift` `displayName(for:)`). Fixed: the shell hands the window its `sceneName` (`EditorShell.cpp` `onOpenObjectRequested`, `ObjectDetailDialog::edgeName`); §8's fixture scene is now untitled and checks the name (fails without the fix). 884/884, 49/49. ⏳ step 2 again.
- **2026-11-22** — 🟡 **ACTIVE** (user, 2026-11-22: *"Q1-Q6 as recommended, implement SP-236"*); Q1–Q6 as recommended; T-0698, T-0699 → active.
- **2026-11-22** — 🔵 **Created in Planning** via `next-id.py` (user, 2026-11-22: *"close SP-235, then plan C1-C4"*); carries [T-0698],
  [T-0699] (new). Q1–Q6 to rule.

*Last Updated: 2026-11-22 (✅ CLOSED.)*

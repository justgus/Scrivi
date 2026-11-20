# Verified Task — T-0657

**Sprint:** [SP-226] · **Epic:** [EP-055] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-20** (user, 2026-11-20: *"Also all these steps pass on all platforms."*)
**Archived:** 2026-11-20.

---

## ✅ T-0657 — `[Cross]` Navigator: collapsible chapters, the current one open, the writer's choices remembered per device

**Epic:** [EP-055] (AC6; Q11, 2026-11-19) · **Status:** ✅ **Verified 2026-11-20** (user) · **Sprint:** [SP-226] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-226.md`](../../Sprints/Closed/Sprint-SP-226.md) · **Date Requested:** 2026-11-03 (user, after the
[SP-199] measurement: *"the Dumas example is a real pain to scroll through the Navigator. Finding any particular scene by
scrolling is rather difficult and getting from the back of the document to, say, the middle of the manuscript requires many
many scroll gestures."*)

**Why:** two problems with one cause, a flat list of every scene. (1) Finding a scene in a long book takes many scroll
gestures. (2) Showing the Navigator is slow: on dumas (1,175 scenes, ~2,350 rows with chapter headers) the iPad's main
thread blocked 3.6 s on the first show and ~1.2 s on a later one. A simulator profile put ~75% in SwiftUI's List walking
every row; a bare List of the same rows takes 12 ms, ours ~75 ms ([SP-199] progress log, 2026-11-03). Collapsed chapters cut
the rows to roughly the chapter count (58 chapter folders in dumas) plus the open chapters' scenes.

**Rulings (user, 2026-11-03):**
- The **current chapter** (the viewport's) is always expanded, so the writer sees which scene she is in.
- The writer can **expand and collapse any chapter** to find a scene; she may keep several open (flipping between two
  scenes).
- A chapter opened **only because she was in it** closes when she leaves it; one she **opened herself** stays open until
  she closes it.
- Her choices are **remembered per device** and restored when the project opens: app-local `workspace-state.json` (Minimum
  Schema Set §10), owned by the core, read and written through the C ABI. Leaving them all open is her choice.
- **Expand All and Collapse All** controls are included.
- Possible later: rules for auto-collapsing chapters (the user's to define).

**Scope:** Apple (Mac and iOS) and Linux, the same shape (Linux adopts Apple's). The core gains the expanded-chapter list in
`scrivi.workspaceState.v1` and its endpoints; reorder (drag) and search (T-0569, which keeps a matching chapter's scenes)
must be decided against collapsed chapters before building.

**Measuring (how SP-199 did it):** an iOS test that copies dumas to a temp folder, opens it with `ProjectSession.loadAsync`,
hosts `SceneNavigatorView` in a `UIHostingController` in a 320 × 1000 `UIWindow` and times `layoutIfNeeded` over a few
runs; `sample <pid>` on the simulator process during a loop gives the profile. Baseline 2026-11-03: ~75 ms (simulator),
bare List 12 ms.

**Implemented ([SP-226], 2026-11-19):** the core stores the set beside the last writing surface (`expandedChapterIDs` in
`scrivi.workspaceState.v1`; `scrivi_set_expanded_chapters`; the open returns it; a scene save keeps it,
`SceneWriter.cpp` `storedOrEmpty`). Apple: `ViewportSceneLoader` holds the set and writes it off the main thread;
`SceneNavigatorView.flatRows` drops a closed chapter's scenes; `ChapterChevron` (Q5). Linux: `EditorShell::applyChapterExpansion`
in place of `expandAll()`, `onChapterExpanded` / `onChapterCollapsed`, the follow in `selectNavigatorScene`. Expand All and
Collapse All in the header's menu and View on both. Measured (AC5): first show on dumas-prose in the simulator 65 → 23 ms.

**Amended in the live pass (user, 2026-11-20):** a filled triangle marks a chapter she keeps open; titles italic = the chapter she is in,
bold = kept open, both = bold italic (Apple and Linux). A scene dropped under a CLOSED chapter's header becomes its last scene (Q4 re-ruled).

**Tested:** ctest macOS 879/879, Linux 884/884; all 44 Linux smokes (`chapter_titles_smoke` §6); iOS 36/36; Mac interop 352/352;
the guard scripts; CI green. ✅ the Mac, iPad, iPhone and Linux VM live passes, both rounds (user, 2026-11-20).

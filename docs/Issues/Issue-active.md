# Active Issues

Issues awaiting **user verification**. An Issue leaves this file only when the user verifies it
(→ `Verified/Issue-verified-XXXX-YYYY.md`, batched in decades of ten) or approves its closure
(→ `Closed/`).

**Claude may mark an Issue `Resolved - Not Verified`. Only the user can mark it Verified.**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| I-0133 | `[Apple]` **`restoredScrollFraction` is written, cleared, and never read — the saved scroll position is dead state.** Found 2026-08-18 while instrumenting [[I-0131]]. `ViewportSceneLoader.restoredScrollFraction` is assigned in `loadAll` from the backend's `restored.scroll` (`:139`), cleared in `restoreWritingSurface` (`ManuscriptTextView.swift:1422`), and **read nowhere** — a grep for every reference returns only the declaration, the write, the clear, and a comment. The scroll fraction is faithfully persisted by `saveScene` on every quit (`scroll: scrollFraction`) and faithfully returned by `scrivi_open_project`, so ScriviCore's half of I-0058 works; the app simply drops it. ⚠️ **Low impact BY DESIGN as of the Current Scene Model:** restore now *centres* the restored scene (I-0131's fix), which is a better outcome than reapplying a document-wide fraction from the previous session — the two would in fact contradict each other. So this is **dead code to remove or a feature to deliberately retire**, not a defect to fix by wiring it up. ⚠️ Do not "fix" it by applying the fraction after centring; that would reintroduce the exact edge-vs-centre disagreement I-0131 exists to eliminate. | Low | **unassigned** | 🔴 **Open (2026-08-18)** — needs a ruling: delete the dead state, or keep it for a future "restore exact scroll" behaviour |
| I-0131 | `[Apple]` **The resume scene is only persisted as a side effect of saving a DIRTY scene, so navigating to a clean scene and quitting resumes on the old one.** Reported by the user 2026-08-17, who isolated it precisely by accident: he clicked *"Six Bells Above the Clouds"* in the navigator and quit → reopened on *"The House on Chicken Wheels"*; **repeated it while editing some text → the new scene stuck.** That difference is the whole diagnosis. **Root cause — two guards that both decline:** on quit, `saveAllDirtyBlocking` calls (a) `saveSceneBlocking(currentIndex)`, guarded by `segments[index].isDirty` (`ViewportSceneLoader.swift:206`) — declines for a clean scene; then (b) `stampWritingSurfaceBlocking`, guarded by `vpID != cursorSceneID` (`:273-275`) — which exists precisely for *"a scene the writer scrolled to but never edited"*, but **declines whenever the viewport scene IS the cursor scene**. Clicking a scene in the navigator moves the caret into it (SP-063's ruling), making viewport == cursor, so **both paths decline and nothing is written** — `WorkspaceState.lastWritingSurface` keeps its old value. The `vpID != cursorSceneID` guard assumes the current-scene save already stamped it, which is true only when that scene is dirty. ⚠️ **Pre-existing EP-019 / I-0058 defect, unrelated to SP-102's world work** — surfaced here only because world testing involved many quit/restart cycles. **Fix (not yet written):** the stamp must not be conditional on dirtiness — either drop the `vpID != cursorSceneID` guard when the current-scene save was skipped, or stamp the viewport scene unconditionally on close. | Medium | **SP-102** (adopted) | 🟠 **Resolved - Not Verified (2026-08-17, SECOND fix)** — ⚠️ **The first fix was aimed at the wrong cause and the user's re-test disproved it** (*"When I went to Scene 15 it came back in scene 10. When I went to 22 it came back in scene 14."*). **Actual root cause — a race in `ManuscriptTextView.navigateToScene`:** it sets `viewportSceneID` to the clicked scene, but `scrollRangeToVisible` then emits scroll notifications, and the scroll handler recomputes the viewport from the **centre of the visible rect** after a **120 ms debounce** and overwrites it. Cancelling `scrollTask` there is useless — it cancels the *old* task; the scroll being caused schedules a **new** one that lands afterwards and wins. It also lands *short*, because `scrollRangeToVisible` scrolls **minimally**: the target sits at the viewport edge while the centre still shows earlier scenes — which is exactly why 15→10 and 22→14, the gap being however many scenes fit on screen. **Fix:** `navigationLockUntil` (0.5 s) makes an explicit navigation authoritative, so the scroll it causes cannot retarget it; hand-scrolling after the window is unchanged. ⚠️ **The first fix is RETAINED, on its own merits** — removing the `vpID != cursorSceneID` guard closes a genuine hole where a clean viewport scene was never stamped at all; it simply was not this bug. ⚠️ **My comment claiming "navigator clicks move the caret (SP-063)" was WRONG** — `navigateToScene` deliberately does not move the caret; the comment is corrected in place. Interop **93/93 macOS arm64**, `ctest` **520/520 macOS arm64**, **BUILD SUCCEEDED**. ⚠️ **Needs the live re-run:** click a distant scene, quit **without typing**, reopen — it must resume there. |
| I-0132 | `[Apple]` **Clicking a scene in the Scene Navigator leaves keyboard focus in the navigator, and there is no keyboard route to the manuscript.** Reported by the user 2026-08-17 while verifying [[I-0131]]: *"When I click a scene in Scene Navigator the focus stays on the navigator... Arrow keys only change the selected scene and typing text updates what is selected. I have to click in the Manuscript View to get focus there... Tab has no effect, although perhaps it should advance the focus to the Manuscript View."* Navigating by click therefore requires a **second, manual click** in the manuscript before the writer can type, and typing while focus is still in the navigator is interpreted as list type-select rather than as text. ⚠️ **This is a deliberate design point colliding with an incomplete one:** `navigateToScene` intentionally does not move the caret (scroll-without-caret is correct — a writer surveying her manuscript should not have her insertion point dragged around), but **nothing offers the writer a way to then commit to that scene**, and Tab — the conventional focus advance — does nothing. `ViewportSceneLoader.takeFocus()` already exists and is wired (`onTakeFocus:`), so the capability is present and only the trigger is missing — the same shape as I-0117/I-0123. ⚠️ **WIDENED 2026-08-18 — the navigator also never scrolls its own list to the selected row.** Found when the user verified [[I-0131]]'s fix: *"Only I had to scroll the Scene Navigator to be able to see the selected entry."* Distinct trigger from the click case above — the selection had been set by **restore**, with no click, focus or caret involved — but the same underlying gap: **the navigator highlights a row and stops there.** `SceneNavigatorView` had no `ScrollViewReader` at all (grep: zero matches), so a selection outside the visible slice of the list was invisible and the writer had to hunt for it by hand. Kept here rather than filed separately because both halves are *"the navigator does not finish the job of going to a scene"*, and §1/§3 of the Current Scene Model govern both. **Fix (this half only):** wrap the List in a `ScrollViewReader`, tag scene rows with `.id(row.rowID)`, and `scrollTo(anchor: .center)` on `viewportSceneID` change **and** `onAppear` — the latter because restore completes before the navigator is on screen, so the very first highlight would otherwise be unreachable. **The focus/caret half above is still unimplemented and still needs the ruling.** | Medium | **unassigned** | 🟠 **Partly resolved - Not Verified (2026-08-18)** — ⚠️ **scroll-to-selection fixed and building; the keyboard/focus model still needs the user's ruling** (the answers given 2026-08-17 cover it, and are recorded in `Scrivi_Current_Scene_Model_v0_1.md` §3/§4 — they are **not yet implemented**) |

---

## Full entries

> The ten Issues above are all `Resolved - Not Verified`; their detail lives in the table rows and in
> the sprint records cited there. No separate full entries remain in this file.

---

## I-0118 — design ruling (user, 2026-08-14)

*Retained here as an active design ruling, not an open defect. I-0118 itself is ✅ Verified and archived
to [`Verified/Issue-verified-0111-0120.md`](Verified/Issue-verified-0111-0120.md); the ruling text below
governs ongoing world-scope work and is kept in front of the reader deliberately.*

All four open questions ruled. **The ruling is simpler than any option offered**, and one answer removed a
whole subsystem I had assumed was required.

**Q1 — Whose index owns a shared world's contents? → WORLD-BOUND, and never reference-counted.**

The user's question reframed the problem: *"If I then delete all the projects on my system, how does that
affect Spotlight's search indexes for the characters in the world?"* — answered **"Let them persist."**

> **A world's search entries belong to the world, not to any project. They are never deleted as a side
> effect of anything a project does — only on explicit instruction.**

⚠️ **This dissolves the problem that made the question hard.** I had framed Q1 as a lifecycle problem
(unbinding from one project must not wipe another's results) and assumed the fix was refcounting bindings.
**If entries are never auto-deleted, there is no lifecycle to track**: no refcount, no unbind hook, no
"last project closed" detection, and no risk of one project's teardown destroying another's search. The
world's `domainIdentifier` is simply `world_<worldID>`, disjoint from every project domain, and
`SpotlightDonor.deleteProject` keeps deleting *project* domains only — untouched.

The residue is deliberate and accepted: a world package deleted outside Scrivi leaves entries behind until
the writer clears them. **That is the correct trade** — the same principle as I-0115, where guessing that
something is gone is worse than admitting we cannot know. Orphaned hits are recoverable; destroyed indexes
of a world still on disk are not.

**Q2 — Deep link shape → WORLD-SCOPED.** `scrivi://open?world=<worldID>&item=<kind>:<id>`, not
`project=`. A character bound by three projects has no single owning project, so a project-scoped link
would have to pick one arbitrarily and would break when that project is deleted. **`ScriviURL` must gain a
`world=` form** — today it parses `project=` only (`ScriviURL.swift:3`).

**Q3 — Offline/unmounted worlds → STALE ENTRIES STAY.** *"If Spotlight offers hits that can't open, so be
it until the world reference is restored."* Consistent with Q1 and with I-0115: a disconnected volume must
never make a writer's cast vanish from search. The open path reports the world's status honestly instead.

**Q4 — Scope → THE WHOLE WORLD PACKAGE**, not just object kinds. That includes `historical-events`,
`historical-timelines`, and `assets`. **Verified buildable without new schema work** — parsers already
exist (`HistoricalEventJson`, `ExternalTimelineJson`, `AssetMetaJson`).

**Implementation shape (not yet built):** `collectObjects` takes a base directory instead of assuming
`objects/`; `extractSearchableText` resolves bound worlds via `WorldStore` and scans **available** ones;
`SearchableItem` carries a per-item domain so world items donate under `world_<id>` while project items
keep `projectID`; the app donates world domains separately and never deletes them on project close;
`ScriviURL` learns `world=`. Then flip the two `SearchableContentTests` expectations, which are written to
fail loudly when this lands.

⚠️ **Deferred to EP-033 (2026-08-14, user).** *What* the "express instruction" to delete a world's entries
actually is has **no answer in the product today**. The user deferred it pending a larger decision —
**whether world management is a view inside Scrivi or a dedicated world-management application** — and
that fork is now tracked as **EP-033 `[Cross]` World Lifecycle Management** (Epic backlog, 🔵 Proposed).
It is larger than this Issue: nothing in Scrivi can delete a world at all today (by design —
`scrivi_remove_world_reference` unbinds only), and a world with no project bound to it is unreachable,
because Scrivi opens projects rather than worlds.

**This does NOT block I-0118's implementation.** Everything ruled above can be built now; only the
*removal* affordance waits, and under the Q1 ruling nothing removes entries automatically anyway — so
shipping the indexing half lands the system exactly where the ruling describes. The gap to state plainly:
until EP-033 rules, a world's search entries are **write-only** from the writer's point of view.

---

---

## Archive map

| Range | Location |
| ----- | -------- |
| I-0001–I-0050 | `Verified/Issue-verified-0001-0010.md` … `-0041-0050.md` |
| I-0051–I-0119 | `Verified/Issue-verified-0051-0060.md` … `-0111-0120.md` |
| I-0121–I-0122 | `Verified/Issue-verified-0121-0130.md` |
| I-0019 | `Closed/Issue-closed-0019.md` |
| I-0072, I-0073, I-0085, I-0103 | `Closed/Issue-closed-0072-0103.md` |

---

*Last Updated: 2026-08-17, later same day (**I-0114–I-0117 ✅ Verified (user-approved) and archived** to
`Verified/Issue-verified-0111-0120.md` in the same step — verified live during the SP-102 / T-0415
world-availability runs, which exercised those exact surfaces. ✅ **They are now usable as evidence for
SP-100's AC pass**, which the prior note said they were not. **Also filed and fixed the same day:
I-0123–I-0129**, all from the user's live SP-102 runs. Active count: 10 → **13**. Prior note follows.)*

*2026-08-17 (**SP-106 closed — I-0121 and I-0122 ✅ Verified and archived** to the new
`Verified/Issue-verified-0121-0130.md` decade file, and their full entries removed from `Issue-backlog.md` in
the same step. Neither was ever listed in this file — both were tracked in `Issue-backlog.md` and the SP-106
sprint record. **This file is unchanged otherwise: the same 10 `Resolved - Not Verified` Issues remain
active**, including I-0114–I-0117, which are **not** evidence for any EP-031 AC until verified. Prior note
follows.)*

*2026-08-15 (docs cleanup — 48 verified Issues archived to decade files, 4 closed Issues
archived; 6 stale full entries (I-0064, I-0067–I-0071) reconciled against their authoritative table rows.
10 `Resolved - Not Verified` Issues remain active.)*

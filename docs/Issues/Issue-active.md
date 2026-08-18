# Active Issues

Issues awaiting **user verification**. An Issue leaves this file only when the user verifies it
(→ `Verified/Issue-verified-XXXX-YYYY.md`, batched in decades of ten) or approves its closure
(→ `Closed/`).

**Claude may mark an Issue `Resolved - Not Verified`. Only the user can mark it Verified.**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| I-0132 | `[Apple]` **Clicking a scene in the Scene Navigator does not move keyboard focus to the manuscript.** ⚠️ **RETURNED FROM VERIFIED 2026-08-18 — archived in error.** The **reveal half is genuinely verified** (the navigator scrolls the restored row into view on launch, and no longer shifts on click) and stays recorded in [`Issue-verified-0131-0140.md`](Verified/Issue-verified-0131-0140.md). **The focus half was not verified and did not work:** the user's verification covered focus *on app launch*, which he said explicitly, and I recorded it as also covering click-to-focus. His re-test: *"When I click a Scene in Scene Navigator it does not correctly change keyboard focus to the ManuscriptView. The Issues I verified all specifically stated that focus changes 'when the app loads' not 'when a scene is clicked'."* ⚠️ **Cause — a responder race, not a missing call.** `navigate(to:)` did call `loader.takeFocus()`, but that called `window.makeFirstResponder(textView)` **synchronously** from inside `onTapGesture`. SwiftUI's `List` is backed by an `NSTableView` which claims first responder while completing its own mouse-down handling, so the table took focus straight back. **This is precisely why it worked on launch and not on click** — launch has no competing responder change, so the synchronous call was never contested. **Second fix:** `takeFocus` now hops to the next runloop pass (`DispatchQueue.main.async`, `ManuscriptTextView.swift:54-70`) so the table finishes first and the focus transfer is the last word. Verified safe for the other eight `takeFocus()` callers — each sets its selection *before* calling and none reads focus state afterward. | Medium | **SP-102** (adopted) | 🟠 **Resolved - Not Verified (2026-08-18, SECOND fix)** — **BUILD SUCCEEDED**, interop **93/93 macOS arm64**. ⚠️ **Needs the live re-run: click a scene in the navigator → the caret must appear in the manuscript and typing must go into the text, not the list.** |
| I-0133 | `[Apple]` **`restoredScrollFraction` was written, cleared, and never read — dead state.** Found 2026-08-18 while instrumenting [[I-0131]]. `ViewportSceneLoader.restoredScrollFraction` was assigned in `loadAll` from the backend's `restored.scroll`, cleared in `restoreWritingSurface`, and **read nowhere**. The fraction is faithfully persisted by `saveScene` on every quit and returned by `scrivi_open_project`, so ScriviCore's half of I-0058 works; the app simply dropped it. ⚠️ **Low impact BY DESIGN as of the Current Scene Model:** restore *centres* the restored scene (I-0131's fix), which is a better outcome than reapplying a document-wide fraction — the two contradict each other. **✅ RULED 2026-08-18 (user): delete the Apple-side dead state; leave Linux alone.** Removed the property, its `loadAll` parameter, the write, the clear and the `ProjectSession` plumbing; each removal site carries a comment recording that the omission is *by design*, so it does not read as a bug later. ⚠️ **The schema field is NOT dead and was deliberately left in place** — `scrivi.h`, the open envelope and `saveScene` are untouched. **Investigating this turned up something the original report missed, now filed as [[I-0134]]: `[Linux]` actively consumes the fraction and applies it *after* `centerCursor()`, deliberately overriding the centring** — i.e. the two platforms now disagree about what "restore where I was" means. That is a parity question, not a dead-code question, so it was **not** settled inside this cleanup. | Low | **SP-102** (adopted) | 🟠 **Resolved - Not Verified (2026-08-18)** — **BUILD SUCCEEDED**, interop **93/93 macOS arm64**; grep for `restoredScrollFraction` returns **zero** references. ⚠️ **Nothing user-visible should change** — that is the verification: restore still lands centred on the right scene. |
| I-0134 | `[Cross]` ⚠️ **Apple and Linux disagree on what "restore where I was" means.** Found 2026-08-18 while ruling [[I-0133]]. **Apple** centres the restored scene and ignores `restored.scroll` entirely (I-0131's ruling, §1 of the Current Scene Model — a scene parked at the viewport edge makes the scroll handler read a *different* scene at the centre, and the navigator highlight follows that). **Linux** does the opposite: `EditorShell.cpp:364` calls `centerCursor()` and then **overrides it** with the saved fraction — *"Apply the saved scroll fraction over the whole document (overrides the centerCursor scroll when a real fraction was persisted)"*. ⚠️ **Both behaviours are deliberate and each was verified on its own platform** — Linux's is T-0247, VNC-verified 2026-07-15 and closed with EP-022; Apple's is I-0131, verified 2026-08-18. Neither is a defect in isolation; the defect is that **the same project reopens differently depending on which app you use**, which EP-026 (cross-platform parity) exists to prevent. ⚠️ **Do not "fix" this by reviving Apple's fraction** — I-0131 documents why edge-restore is wrong there. If parity resolves toward centring, Linux's override is what changes, and that means re-verifying a shipped, user-verified EP-022 behaviour over Docker+VNC. **Needs a ruling before either platform is touched.** | Low | **unassigned** (EP-026 candidate) | 🔴 **Open (2026-08-18)** — parity ruling needed; no code changed |

---

## Full entries

> The Issues above carry their detail in the table rows and in the sprint records cited there. No
> separate full entries remain in this file. ⚠️ **Not all rows are `Resolved - Not Verified`** — read
> each row's Status column; **I-0134 is 🔴 Open and awaiting a ruling**, with no code written.

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
| I-0121–I-0122, I-0130 | `Verified/Issue-verified-0121-0130.md` |
| I-0131 | `Verified/Issue-verified-0131-0140.md` |
| I-0132 | ⚠️ **split** — reveal half in `Verified/Issue-verified-0131-0140.md`; **focus half is active above** |
| I-0019 | `Closed/Issue-closed-0019.md` |
| I-0072, I-0073, I-0085, I-0103 | `Closed/Issue-closed-0072-0103.md` |

---

*Last Updated: 2026-08-18, third pass (⚠️ **I-0132 RETURNED FROM VERIFIED — I archived it on a claim
that was not true.** The user verified focus changing **on app launch**, and said so explicitly; I
recorded that as verifying **click-to-focus** as well. Clicking a scene still left focus in the
navigator. **Cause was a responder race, not a missing call:** `takeFocus` ran
`makeFirstResponder` synchronously from inside `onTapGesture`, and the `NSTableView` backing SwiftUI's
`List` reclaimed first responder while finishing its own mouse-down. Launch had no competing responder
change, which is exactly why the two cases diverged — **the evidence I verified against and the
failing case were different code paths.** Second fix defers the transfer one runloop pass. The
**reveal half stays verified** and remains archived. **Lesson recorded:** when a fix has two halves,
verify each half against its own trigger — a verification of one is not evidence for the other.
Active count: 2 → **3**. Prior note follows.)*

*2026-08-18, later same day (**I-0131 + I-0132 ✅ Verified (user-approved) and
archived** to the new `Verified/Issue-verified-0131-0140.md` decade file, and removed from this file
in the same step. **I-0133 ruled and resolved:** the user chose *delete Apple's dead state, leave
Linux alone* — the property, its `loadAll` parameter, the write, the clear and the `ProjectSession`
plumbing are gone, each site commented so the omission reads as deliberate; ⚠️ **the schema field
stays** because Linux consumes it. ⚠️ **Ruling I-0133 surfaced a finding the original report missed,
now filed as I-0134 (🔴 Open):** Linux applies the scroll fraction *after* `centerCursor()`,
deliberately overriding it — so **Apple and Linux now disagree about what "restore where I was"
means.** Deliberately **not** settled inside a dead-code cleanup: it changes shipped, VNC-verified
EP-022 behaviour and belongs to EP-026 parity. **BUILD SUCCEEDED**, interop **93/93 macOS arm64**.
⚠️ **Active count is now 2** — I-0133 (Resolved - Not Verified) and I-0134 (Open). The prior note's
"Active count: 10 → 13" was already stale before this pass: those Issues had been verified and
archived without this line being updated. Prior note follows.)*

*2026-08-18 (**I-0132 both halves now 🟠 Implemented - Not Verified.** ⚠️ **The
reveal-on-selection-change half was REMOVED, not tuned** — the user's re-test found it scrolled the
navigator "a little bit up or down" on **every** click, because `scrollTo` **re-anchors an
already-visible row** rather than no-opping as my comment had claimed. Reveal now fires **`onAppear`
only**, which is the one moment it is needed (restore sets the selection before the view exists).
The **focus half is implemented**: `navigate(to:)` calls `loader.takeFocus()`, so a click or Return
hands the keyboard to the manuscript and the caret is visible. ⚠️ **Accepted trade, user-ruled:**
this ends arrow-key list browsing after the first click — *"Arrow browsing isn't strictly necessary.
Mouse Wheel and Trackpad Scrolling are still available."* **Tab-as-focus-advance is no longer needed
for this Issue.** Also filed **T-0417** (Scene/Chapter boundary navigation) — adopted into SP-102,
shipping as menu items because ⚠️ **no free macOS key combination exists.** **BUILD SUCCEEDED**,
interop **93/93 macOS arm64**. Prior note follows.)*

*2026-08-17, later same day (**I-0114–I-0117 ✅ Verified (user-approved) and archived** to
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

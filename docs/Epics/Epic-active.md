# Active Epics

## EP-034: `[Cross]` Object Detail & Media — Detail Sheets & Project↔World Interaction

**Status:** 🟡 **ACTIVE** (promoted from backlog 2026-08-20; ⚠️ **widened in place** by the design doc)
**Codebase:** `[Cross]` — `[Apple]` first, `[Linux]` for parity, plus a **small, well-scoped ScriviCore
change** (world assets + asset path + kind-scope endpoint).
**Goal:** Let a writer **see and edit what an object actually is** — description, notes, imagery,
**and its relationships** — through a navigable **Detail Sheet**, and attribute it with sources.
**Date Created:** 2026-08-18 · **Promoted to Active:** 2026-08-20
**Target Close Date:** TBD — **3 of 8 sprints closed** (SP-115 ✅ · SP-116 ✅ · SP-117 ✅, all by 2026-08-21)
**Actual Close Date:** —

**Design document:** [`../Scrivi_Object_Detail_Sheets_Design_v0_1.md`](../Scrivi_Object_Detail_Sheets_Design_v0_1.md)
— 🟢 **APPROVED FOR PLANNING**: ✅ all nine trades **D1–D9** and all three decisions **Q-a/Q-b/Q-c** ruled
2026-08-20.

**Depends on:** EP-031 ✅ closed (object model, graph, world partition).
**Unblocks:** **T-0365** (the `sources` card cannot show content until R6 ships) and ⚠️ **EP-032**, whose
AC5 depends on source creation — **which is why Q-a ruled this Epic runs FIRST.**

---

### ⚠️ The finding that inverts this Epic's expected shape

**The backend and its Swift binding ALREADY EXIST.** Verified against code 2026-08-20:
`scrivi_save_object` takes **full object JSON** (so `subtitle`/`notes`/`image` are already writable), all
edge endpoints ship, and `ScriviEngine`/`ScriviEngineGraph` already wrap every one of them.

> ⚠️ **This is `project_capability_without_surface` at its largest scale yet** — the *entire* object-editing
> capability shipped and verified while the app exposes a single `TextField("Name")`
> (`ObjectCard.swift:647`).
>
> **The lesson, stated correctly (user):** *"Ensure all core features have a UI the writer can reach!"*
> ⚠️ **NOT "ship less core"** — the core here is sound; the missing surface is the defect.

⚠️ **A first draft of the design claimed world-asset storage was "the single largest piece of backend work."
That was WRONG and the user caught it.** `AssetStore` is **entirely root-agnostic** (`AssetStore.cpp:12-17`)
— pass a world root and it works. **"No caller does X" is not evidence the core cannot do X.**

---

### ✅ Approved trade rulings (design doc §5)

| Trade | Ruling | Binding consequence |
| ----- | ------ | ------------------- |
| **D1** | **E** — non-modal editor-level pane; window = successor | ⚠️ **Build host-independent** |
| **D2** | **B** — explicit back **and** forward history | D1-agnostic |
| **D3** | **A** `TextEditor` now, Markdown later; ⚠️ **C ruled OUT** | C would pull EP-019's undo engine in |
| **D4** | **A** — inline, reusing `ObjectPickerView` | Widen its scene-target filter; ✅ no ABI change |
| **D5** | **A** — one layout; ⚠️ **ABI kind-scope endpoint IN SCOPE** | Retires `ObjectCard.swift:46` |
| **D6** | **A** — assets in the world package **+ heartbeat during copy** | ⚠️ `kStaleSeconds` **not** raised |
| **D7** | **D** — `assetPath` in `list_assets` + app-side thumbnails | ⚠️ **No image codecs in C++** |
| **D8** | **A** — thumbnail only when an image exists | ⚠️ Async; never blocks a card |
| **D9** | **A** — `lastKnownPackagePath`, distinctly named | ⚠️ `packagePath` keeps meaning "verified" |

**Decisions:** ✅ **Q-a** — EP-034 precedes EP-032 (starts **SP-115**). ✅ **Q-b** — ⚠️ **the Detail Sheet
is ADDITIVE**; the inline editor and list item **remain**, and **I-0139 is a real fix**. ✅ **Q-c** —
iOS/visionOS **explicitly deferred**; `[Linux]` parity stays in scope.

---

### Acceptance Criteria

Written to be verifiable **by use** — per the lesson EP-031 paid for five times.

- [x] ✅ **AC1** (verified by use, SP-117) — From a Kind Card list item, **double-click** and **right-click → "View Detail"** both open
      the Detail Sheet. ⚠️ **A single click still does what it does today** (Q-b).
- [ ] **AC2** — The sheet **views and edits** `displayName`, `subtitle`, `notes`, `tags`; edits persist
      across app restart.
- [ ] **AC3** — An image can be **imported, displayed, replaced, removed**; ⚠️ for a **world-scoped** object
      it is stored **in the world package** and is present when that world is opened **from a different
      project**.
- [ ] **AC4** — Card list items show a **thumbnail when an image exists**, unchanged when none does.
- [x] **AC5** — The related-objects section lists this object's edges with labels reading correctly from
      this endpoint. ✅ **CLOSED by SP-118**, verified by live use 2026-08-23.
- [x] **AC6** — A relationship can be **created from the sheet**; it appears from both endpoints and is
      rejected as a duplicate from the second. ✅ **CLOSED by SP-118** — verified by live use: a `sibling-of` edge
      created from the sheet appeared from both endpoints and the second was refused as a duplicate.
- [x] **AC7** — **Double-click** and **right-click → "Show"** push-navigate to a related object's sheet;
      **back and forward** move through that history. ✅ **CLOSED by SP-118** — ⚠️ **both halves verified
      separately**: back/forward in SP-117, and push-from-the-related-list here, which needed four Issues
      (I-0151, I-0153, I-0157, I-0161) before it actually worked.
- [ ] **AC8** — A **source can be created from the documented object** and a `cites` edge attached;
      ⚠️ the `sources` card **then shows content for the first time**.
- [~] ⚠️ **AC9 — HALF met (SP-117).** ✅ A **pending** object opens **read-only, explained, never written** —
      **verified by use**, including drive removal *and* return, and the read-only state was hardened by
      **I-0148**. ⚠️ **The remaining half — a world going unavailable WHILE A SHEET IS OPEN, handled
      without data loss — is not yet demonstrated** and belongs with SP-119.
- [x] ✅ **AC10** (SP-115) — All five Issues **I-0135–I-0139** Resolved, ⚠️ **each verified against its own trigger**
      (`feedback_verify_each_half_separately`).
- [ ] **AC11** — `[Linux]` parity for AC1–AC9 in Docker+VNC. ⚠️ **No gesture-only affordances.**
- [ ] **AC12** — `ctest` green macOS arm64 **and** x86-64 + sanitizers; interop green; Linux container
      green; app **BUILD SUCCEEDED**.

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| **SP-115** | ⚠️ **The five Issues + I-0142 (user-found)** — all ✅ Verified | ✅ **Closed** | 2026-08-20 |
| **SP-116** | ⚠️ **`[Cross]`** World assets + `assetPath` + kind-scope endpoint (D6, D7, D5) — retired **I-0140, I-0141, I-0143, I-0144, I-0145, I-0146** | ✅ **Closed** | 2026-08-21 |
| **SP-117** | ⚠️ **`[Cross]`** Detail Sheet shell (D1, D2, D3) — ⚠️ **first writer-facing surface**; paid **T-0420's debt**; retired **I-0148** | ✅ **Closed** | 2026-08-21 |
| SP-118 | ⚠️ **`[Cross]`** Related objects + relationship creation (D4) — ⚠️ **T-0416 fixed as T-0441 + I-0149** | ✅ **Closed 2026-08-23** — [record](../Sprints/Closed/Sprint-SP-118.md) | ✅ **AC5, AC6, AC7** — ⚠️ **13 Issues, all from the live pass** |
| SP-119 | `[Apple]` Images: import, display, card thumbnails (D8) | ⚪ Planned | — |
| SP-120 | `[Apple]` Source creation + footnote text — ⚠️ **closes T-0365's write half** | ⚪ Planned | — |
| SP-121 | `[Linux]` Parity (AC11) | ⚪ Planned | — |
| SP-122 | AC verification + ⚠️ **live-use pass on the real rig** + Epic close prep | ⚪ Planned | — |

⚠️ **Eight sprints is an estimate made BEFORE implementation.** EP-031 planned 6 and delivered 11 — **four
of the five additions came from USE, not planning.**

⚠️ **SP-107–SP-114 are RESERVED to EP-032 and stay reserved.** This Epic starts at **SP-115**, so
⚠️ **sprint IDs run OUT OF SEQUENCE** — precedented (SP-083 before SP-082; SP-102 before SP-100).

### Tasks

| ID | Title | Sprint | Priority | Status |
| -- | ----- | ------ | -------- | ------ |
| **T-0419** | ⚠️ `lastKnownPackagePath` (I-0137) | SP-115 | **High** | ✅ **Verified** |
| T-0420 | `formatVersion` comparison (I-0136) | SP-115 | Medium | ✅ **Verified** |
| T-0421 | Inline editor exit (I-0139) | SP-115 | Medium | ✅ **Verified** |
| T-0422 | Corrupt `world.json` coverage (I-0135) | SP-115 | Low | ✅ **Verified** |
| T-0423 | Disabled **and explained** (I-0138) | SP-115 | Low | ✅ **Verified** |
| **T-0424** | ⚠️ **FILE** the two kind-list findings → I-0140, I-0141 | SP-115 | Medium | ✅ **Verified** |
| **T-0426** | **D6** — `worldID` on asset requests; resolve, refuse, lock, ⚠️ **heartbeat during copy** | SP-116 | **High** | ✅ **Verified** |
| **T-0427** | **D7** — emit `assetPath` from `scrivi_list_assets` | SP-116 | **High** | ✅ **Verified** |
| **T-0428** | ⚠️ **I-0143** — route the `list_assets` array through `JsonDoc` | SP-116 | **High** | ✅ **Verified** |
| **T-0429** | **D5** — `scrivi_list_object_kinds`; ⚠️ **Swift adopts it, `ObjectCard.swift:46` deleted** | SP-116 | **High** | ✅ **Verified** |
| **T-0430** | ⚠️ **I-0141** — `scrivi.h` states the rule **by reference** | SP-116 | Low | ✅ **Verified** |
| **T-0431** | ⚠️ **I-0144** — lock **every** world-package write path | SP-116 | **High** | ✅ **Verified** |
| **T-0432** | ⚠️ **Streaming/block transfer** + per-block watchdog + partial cleanup | SP-116 | **High** | ✅ **Verified** |
| **T-0433** | ⚠️ **I-0146** — stale-lock sweep of abandoned `*.partial` files | SP-116 | **Medium** | ✅ **Verified** |
| **T-0434** | **D1-E** — Detail Sheet pane, ⚠️ **host-independent** | SP-117 | **High** | ✅ **Verified** |
| **T-0435** | **D2-B** — back/forward history | SP-117 | **High** | ✅ **Verified** |
| **T-0436** | ⚠️ **Typed Swift object model** — none exists today | SP-117 | **High** | ✅ **Verified** |
| **T-0437** | **D3-A** — fields; ⚠️ **save by PATCH** | SP-117 | **High** | ✅ **Verified** |
| **T-0438** | **R7** — double-click **and** right-click | SP-117 | **High** | ✅ **Verified** |
| **T-0439** | **R9** — pending: read-only, explained, never written | SP-117 | Medium | ✅ **Verified** |
| **T-0440** | ⚠️ **T-0420's owed surface** | SP-117 | Medium | ✅ **Verified** |
| ✅ **T-0441** | ⚠️ **T-0416** — reconcile seeded relation types on open | SP-118 | **High** | ✅ **Verified** |
| ✅ **T-0442** | **R3** — related-objects section | SP-118 | **High** | ✅ **Verified** |
| ✅ **T-0443** | **D4-A** — inline creation + picker widening | SP-118 | **High** | ✅ **Verified** — ⚠️ **deviation ruled IN FAVOUR** |
| ✅ **T-0444** | **R5/R7** — push-navigation from the related list | SP-118 | **High** | ✅ **Verified** |
| ✅ **T-0445** | ⚠️ **Pending far-endpoints** — named and explained | SP-118 | Medium | ✅ **Verified** |

### Issues

| ID | Sev | Outcome |
| -- | --- | -------- |
| **I-0137** | **High** | ✅ **Verified** — `lastKnownPackagePath`; ⚠️ **on the real rig, drive ejected** |
| I-0136 | Medium | ✅ Verified at the **CORE ONLY** — ⚠️ **no writer-facing surface; owed** |
| I-0139 | Medium | ✅ Verified — exit named for what it does |
| I-0135 | Low | ✅ Verified — `unavailable`, never `missing` |
| I-0138 | Low | ✅ Verified — disabled **and explained** |

| **I-0142** | **High** | ✅ **Verified** — ⚠️ **found by the USER, not a suite**; world shown as a label, **moves disallowed** |
| **I-0143** | **Medium** | ✅ **Verified** — the array routes through `JsonDoc` now |
| **I-0140** | **Medium** | ✅ **Verified** — Swift **derives** scope from the new endpoint |
| **I-0141** | Low | ✅ **Verified** — the header's kind list is **gone, not corrected** |
| **I-0145** | **Medium** | ✅ **Verified** — ⚠️ **pre-existing**: `AssetStore::remove` could strand unreclaimable bytes in a shared world; found by self-review, fixed in **T-0426** |
| **I-0144** | **High** | ✅ **Verified** — every world write path now locks via `WorldWriteGuard`; ⚠️ **one deliberate exception**: the index rebuild, because `WorldLock` is not reentrant |

✅ **All six archived 2026-08-20** → `../Issues/Verified/Issue-verified-0131-0140.md` and
`../Issues/Verified/Issue-verified-0141-0150.md`.

⚠️ **Two more were FILED by SP-115** (design doc §10): the Swift scope restatement
(`ObjectCard.swift:46`) and the stale header comment (`scrivi.h:97-99`) — ⚠️ **occurrence eight** of the
restated-kind-list class — **both 🔴 Open, assigned to SP-116**, where D5's endpoint cures them.

### Scope Notes

- ⚠️ **Additive, not replacing** (Q-b): list item + inline editor + Detail Sheet coexist.
- ✅ **T-0416 is FIXED (T-0441, SP-118, 2026-08-22)** — reconciliation on open, seeded types only, with the
  negative control actually run. ⚠️ **Not user-Verified**, and ⚠️ **the drifted-project live pass is owed**:
  `tintagael` is clean and will not reproduce it.
- ⚠️ *(historical)* **T-0416 may surface as a live blocker** in SP-118 — seeded relation types never reach existing
  projects, so a writer relating objects in an existing project sees only that project's vocabulary.
- **Not in scope:** iOS/iPadOS/visionOS (Q-c), manuscript reference syntax (EP-032), world lifecycle
  (EP-033), any `item`/`artifact` re-ruling.

#### ⚠️ What SP-116's IMPLEMENTATION turned up (2026-08-21)

- ⚠️ **I-0144 (High, unassigned): `WorldLock` has never been called in production.** Object writes into a
  shared world take **no lock**, so two projects with the same world bound can silently lose each other's
  edits. The lock was built in SP-097 for exactly this and never wired in — ⚠️ **`capability_without_surface`
  again, and this Epic's own T-0426 is the first code ever to acquire it.** **Not fixed here**: it touches
  every object write path.
- ⚠️ **D6's "heartbeat during the copy" is not fully achievable** — `FileSystem` has no streaming write to
  interleave with, so the heartbeat brackets each write instead of running inside it. ⚠️ **The ruling
  described behaviour the interface cannot currently provide**; `kStaleSeconds` was **not** raised.
- ⚠️ **S11 is incomplete** — the Linux container leg could not run (Docker unavailable here).

#### ⚠️ SP-116 planning changed two things this table originally assumed (2026-08-21)

1. ⚠️ **SP-116 is `[Cross]`, not `[ScriviCore]`.** D5's endpoint is **adopted in Swift in the same sprint**
   — `ObjectCard.swift:46` is deleted, not left for SP-117. **Shipping the endpoint and stopping would
   leave I-0140 open with its cure sitting unused on the shelf** — ⚠️ `project_capability_without_surface`,
   committed by the sprint curing occurrence *eight* of its sibling defect. **User-ruled.** ⚠️ **No UI
   ships**; the reach is a derivation swap at one property.
2. ⚠️ **A third Issue, I-0143, was found at planning** — `scrivi_list_assets` concatenates JSON with **no
   escaping**, and **T-0427 puts a filesystem path into that array.** Filed *and* fixed in SP-116 by user
   ruling. ⚠️ **It was found by READING the code D7 modifies** — not by the design doc, and not by any
   suite; `AssetTests.cpp` tests the facade and would pass regardless.

### Completion Summary

*(Filled in when the Epic reaches 🟠 Complete.)*

---

## ⚠️ EP-034 verification & closeout procedure (per `Epic-GUIDELINES.md`)

**At each Sprint close:** archive Verified Tasks/Issues **in the same step** the Sprint closes; remove the
Sprint from `../Sprints/Sprint-backlog.md`; update `Epic-Documentation.md`.

**At Epic completion, in order:**

1. ⚠️ **Run the AUDIT CHECK FIRST** — read-only, mechanical (greps + counts), per
   [`../Audits/Audit-Guidelines.md`](../Audits/Audit-Guidelines.md). It runs **before** the ACs are judged.
   ⚠️ **A Check is NOT an Audit; an Epic close does not trigger one.** Findings are ruled as part of this
   close; something large or systemic → **recommend** a full Audit, which begins only on user request.
2. **Judge AC1–AC12 against evidence BY USE**, never by suite name. ⚠️ **No AC is marked Verified on
   fixture evidence alone** — EP-031's AC24 passed a fixture supplying an input the real product never
   supplies (I-0137).
3. Mark 🟠 **Complete**; draft the completion summary.
4. ⚠️ **User reviews and explicitly approves the close. Claude cannot close an Epic.**
5. On approval: move to `Closed/Epic-EP-034.md`, mark ✅ Closed, update `Epic-Documentation.md`, and
   ⚠️ **strip this file's EP-034 detail to a pointer.**

⚠️ **Claude may NOT:** mark ✅ Closed, defer the Epic, or remove an acceptance criterion.

---

### Original backlog rationale (retained — it carries the user findings that opened this Epic)

### Why this is an Epic

**The finding (user, 2026-08-18):** trying to use the new `sources` card, *"I can show the sources card,
but I can't create any sources. None of the card interfaces allow it."* True, and the cause is broader
than sources:

> ⚠️ **Object cards edit exactly ONE field — `displayName`.** There is a single `TextField("Name")` in
> `ObjectCard.swift:647`, and rename is the only mutation the app performs on an object.
> **`subtitle`, `notes` and `image` shipped in SP-095 (T-0371, ✅ Verified 2026-08-12) and the app reads
> and writes none of them.**

This is the same defect shape EP-031 has produced repeatedly — *capability shipped in the core, surface
never built* (the SP-099 R4 audit, I-0117, and `listPendingEdges` with zero call sites). It is the
largest remaining instance.

**The writer's case, in his words:** *"A Chronicle must have an actual chronicle, that is, a story that
it chronicles. Characters need more than just a name. A description and maybe even a picture, drawing,
sketch. A Location needs a description to set the mood and maybe an image to set the right mood. A map
is an image."*

**And the attribution consequence, which is what ties sources to this Epic:** *"If we allow images into
the App for worldbuilding, then we must also be able to cite the correct attribution of those images.
Also, a chronicle may be sourced from another project, or another author's text. That text must be able
to be attributed via a source."*

### ⚠️ Why source creation belongs here and not on the `sources` card

Adding a source *from* the aggregate `sources` card was considered and **rejected by the user**:
*"it would muddy the fact that the source must be associated with an object in the world."*

A citation documents an **object** — a map's image attribution belongs to the map, a chronicle's
provenance belongs to the chronicle. The `sources` card is scene-scoped and aggregate, so creating from
there inverts the relationship the model is built on (Doc 1 §3.4). **Creation belongs where the thing
being documented lives: the object's own detail surface.**

### ⚠️ EP-032 does not cover this

Checked at opening. EP-032 is **reference syntax inside manuscript text** — footnotes and pull quotes,
depending on the fragment model. Object detail has no home in any existing Epic; this work was
genuinely unplanned.

### Rough scope

- An **object detail surface** — view and edit `displayName`, `subtitle`, `notes`; reachable from an
  object card and from the object picker.
- **Long-form text** for `notes`, sized for a chronicle's actual text rather than a one-line field.
- **Image display and import** — `image.assetID` / `thumbnailAssetID` through the existing
  `scrivi_import_asset`; thumbnails on cards, full image in detail. **A `map` is an image**, so this is
  what makes that kind meaningful at all.
- **Source creation + `cites` attachment from the documented object**, closing T-0365's write half and
  making the `sources` card reachable.
- **Attribution for imported images** — the case that forces sources and media into one Epic.
- `[Linux]` parity.

**Depends on:** EP-031 (object model, graph, world partition — `subtitle`/`notes`/`image` and the
`source` kind all land there).
**Unblocks:** **T-0365**, whose `sources` card ships in SP-102 read-only and **cannot show content until
this Epic provides a way to create a source.**

---

## ⚠️ EP-032's sprint IDs remain RESERVED

**SP-107–SP-114 stay reserved to EP-032** (`Epic-backlog.md`) and must **NOT** be reissued.
✅ **Q-a ruled 2026-08-20: EP-034 runs FIRST**, starting at **SP-115** — so ⚠️ **sprint IDs execute out of
sequence.** Precedented: SP-083 ran before SP-082; SP-102 before SP-100.

**Why EP-034 precedes EP-032:** EP-032's AC5 renders footnotes for `source` objects, and nothing in the app
can create a source until EP-034's R6 ships. Running EP-032 first would make its AC5 verifiable only on
hand-authored fixtures — ⚠️ **the exact `capability_without_surface` failure both Epics exist to avoid.**

---

*Last Updated: 2026-08-20 (**SP-115 ✅ CLOSED, user-approved — EP-034's first of 8 sprints done.** Seven
Tasks + six Issues Verified and archived. ⚠️ **Carried out of SP-115 and NOT delivered:** T-0420's
writer-facing surface (**unowned**), and **I-0140/I-0141** → **SP-116**. EP-034 remains 🟡 Active; **SP-116
is next** (D6, D7, D5). Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 → 🟠 REVIEW — all seven Tasks + six Issues ✅ Verified and archived.**
⚠️ **I-0142 was found by the USER during verification**, not a suite: the object editor never showed an
object's own world, and **renaming a world object was broken outright**. ✅ **Ruled: objects cannot move
between worlds** — the control is a label. ⚠️ **Carried out of SP-115: T-0420 has no writer-facing
surface**; **I-0140/I-0141 stay open for SP-116**. Suites: ctest **525/525** · interop **103/103** · app
**BUILD SUCCEEDED**. Next: **SP-116** (D5, D6, D7). Prior note follows.)*

*Last Updated: 2026-08-20 (**EP-034 `[Cross]` PROMOTED backlog → 🟡 ACTIVE and widened in place**, per user
ruling, from "object fields beyond `displayName`" to the full **Detail Sheet + relationship surface +
Project↔World interaction**. Design doc `Scrivi_Object_Detail_Sheets_Design_v0_1.md` 🟢 **APPROVED FOR
PLANNING** — ✅ **all nine trades D1–D9 and all three decisions Q-a/Q-b/Q-c ruled.** AC1–AC12 written;
8 sprints estimated; **SP-115 🟡 ACTIVE — the five Issues I-0135–I-0139 and nothing else.** ⚠️ **Q-a: this
Epic precedes EP-032**, whose SP-107–SP-114 stay reserved, so IDs run out of sequence. ⚠️ **Q-b: the Detail
Sheet is ADDITIVE** — inline editor and list item remain, so **I-0139 is a real fix, not a disposition.**
⚠️ **Q-c: iOS/visionOS explicitly deferred**; `[Linux]` parity stays in scope. Active Epics 0 → 1.)*

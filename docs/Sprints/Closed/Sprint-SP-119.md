# Sprint SP-119 (CLOSED)

## SP-119: `[Cross]` Images, tags, and the open-sheet outage

**Status:** ✅ **CLOSED 2026-08-24 (user-approved)**
**Epic:** [EP-034: `[Cross]` Object Detail & Media](../Epics/Epic-active.md) — its **fifth** sprint
**Goal:** Give the writer the **last unreachable half of the object model** — imagery and tags — and prove
the sheet survives a world going away **while she is looking at it**.
**Start Date:** 2026-08-23
**End Date:** 2026-08-24
**Trades implemented:** ✅ **D8** (conditional, async, non-blocking thumbnails) · **R2**'s remaining half · **R8**
**Closes:** **AC2**, **AC3**, **AC4**, and ⚠️ **AC9's second half**

---

### The capability is ready; this sprint is the surface

**Checked against code 2026-08-23, not assumed:**

| Capability | Core | C ABI | Swift | View |
| ---------- | ---- | ----- | ----- | ---- |
| `importAsset` | ✅ | ✅ `scrivi_import_asset` | ✅ `ScriviEngine:334` | — |
| `listAssets` (+ `assetPath`, T-0427) | ✅ | ✅ `scrivi_list_assets` | ✅ `ScriviEngine:365` | — |
| `removeAsset` | ✅ | ✅ `scrivi_remove_asset` | ✅ `ScriviEngine:380` | — |
| world-scoped asset storage (D6) | ✅ SP-116 | ✅ | ✅ | — |

> ### ✅ RULED 2026-08-23 — this sequence is CORRECT, not a defect
>
> ⚠️ **The earlier framing of this as `capability_without_surface` was WRONG and the user corrected it.**
> Scrivi is a **cross-platform application**, not an app: **ScriviCore is the capability layer and the
> Scrivi App is one surface over it.** ⚠️ **In almost every case the capability must be implemented and
> verified BEFORE a surface can be built on it** — so "core ships first, UI follows" is the intended
> order, not a gap.
>
> ⚠️ **Where the real tension lives is the DESIGN PHASE, and it points the other way:** the surface must be
> designed to the capability that exists, ⚠️ **not to the "lowest common denominator UI"** — which is the
> user's stated observation about how an LLM tends to design: *"easiest and most common, which is not
> necessarily true."*
>
> **This sprint's guard against that is S11** — every field the core exposes for images must be accounted
> for in the design, and anything deliberately not surfaced must be named and justified, not silently
> dropped.

⚠️ **The one thing that IS a gap:** `ObjectDetail` parses `imageAssetID` (`ObjectDetail.swift:46-49`) and
nothing renders it — a read with no reader, which SP-117 already flagged and deferred.

---

### Assigned Tasks

| ID | Title | Codebase | Priority | Status |
| -- | ----- | -------- | -------- | ------ |
| **T-0446** | ⚠️ **Carry `image` in the object index** — the one `[ScriviCore]` change | `[ScriviCore]` | **High** | ✅ **Verified** |
| **T-0447** | **AC3** — import, display, replace, remove an object image in the Detail Sheet | `[Apple]` | **High** | ✅ **Verified** |
| **T-0448** | **AC4 / D8** — card list-item thumbnails, ⚠️ **conditional, async, never blocking** | `[Apple]` | **High** | ✅ **Verified** |
| **T-0449** | **AC2** — `tags` editing; ⚠️ **closes R2 and AC2, owed since SP-117** | `[Apple]` | **High** | ✅ **Verified** |
| **T-0450** | ⚠️ **AC9's second half** — a world goes away **while a sheet is open** | `[Apple]` | **High** | ✅ **Verified** |
| **T-0451** | ⚠️ **I-0164** — attach an asset ALREADY in the world (user-ruled option 1) | `[Apple]` | **High** | ✅ **Verified** |
| **T-0452** | ⚠️ **I-0167** — **Cancel** + save/discard prompt on exit (user-designed) | `[Apple]` | **High** | ✅ **Verified** |

### Assigned Issues

**None open.** ⚠️ **I-0147 remains an Accepted limitation** and is not SP-119's.

---

### Task detail

#### T-0446 — carry `image` in the object index ⚠️ the only `[ScriviCore]` work

✅ **User-ruled 2026-08-23.** `ObjectIndexEntry` (`ObjectIndex.hpp`) carries `objectID`, `kind`, `slug`,
`displayName`, `worldID` — ⚠️ **and no image**, so the card list has no way to know an object has one.

**Add the image to the entry and to `scrivi_list_objects`.** One read then serves the whole list.

⚠️ **The alternative was REJECTED:** a per-row `openObject` is N file reads in a 280pt pane, and D8 is
explicit that an image may live *"on an unavailable world's volume, i.e. a slow or failing read on the USB
rig"* — ⚠️ **"a thumbnail that hangs the inspector would be a worse defect than no thumbnail at all."**

##### ⚠️ What to carry — NOT the assetID alone

> ✅ **RULED 2026-08-23 (user):** *"`imageAssetID` is likely not a useful value to display on the UI. The
> image or thumbnail of the image would be better as a display element. The writer is not going to care
> what the computer calls the image on the inside."*

⚠️ **An ID is a key, not a picture.** Carrying only `imageAssetID` would force every row to do a second
lookup to turn it into something renderable — reintroducing the per-row read this Task exists to avoid.

**The entry must carry what the view can actually draw from:** the **resolved `assetPath`** (as
`scrivi_list_assets` already discloses per T-0427), plus `imageAssetID` for identity/removal. ⚠️ **The ID is
plumbing, never shown to the writer.**

⚠️ **`assetPath` is RESOLVED, never stored.** A path is volume-dependent; persisting one into
`objects/index.json` would go stale the moment the drive mounts elsewhere
(`project_test_rig_tintagael_eskandar`). ⚠️ **It must be empty — not wrong — when the world is away.**

##### ✅ First: make `rebuild()` use the helper that already exists

⚠️ **Corrected 2026-08-23 — the earlier plan said "two near-identical rebuild paths must both change."
That was imprecise.** `scanDir(baseDir, worldScoped)` (`ObjectIndex.cpp:100`) **already is** the unified
scan, with `worldScoped` as the discriminator. ⚠️ **But `rebuild()` (`:151`) does not call it** — it
carries a hand-inlined copy of the same loop. Only `loadWorldIndex()` (`:254`) uses the helper.

✅ **So the answer to "can we unify them" is YES, and it is small:** `rebuild()` becomes
`scanDir(objectsDir, /*worldScoped=*/false)`. ⚠️ **Do this FIRST, as its own step, before adding the image
field** — then the new field is added in **one** place and cannot be added to one path and forgotten in the
other.

⚠️ **This is the derive-never-restate rule applied to control flow.** The abstraction was written; a caller
ignored it, and the copy silently drifted into a hazard.

#### T-0447 — AC3: the image in the Detail Sheet

Import, display, **replace**, remove. ⚠️ **Replace is a distinct operation from remove-then-import** and
must be tested as its own trigger (`feedback_verify_each_half_separately`).

⚠️ **A world-scoped object's image goes in the WORLD package** (D6, shipped SP-116) — pass the object's
`worldID`, exactly as `createObject` does. ⚠️ **Passing `""` is I-0151's defect**: `kindDirFor` fails
outright for a world-scoped kind with an empty worldID.

⚠️ **Patch, never reconstruct** (`ObjectDetail.applyingEdits`), and ⚠️ **re-read before patching**
(I-0155) — `image` is a sub-object and a rebuilt document would drop it.

#### T-0448 — AC4 / D8: card thumbnails

✅ **RULED (D8-A):** thumbnail **only when an image exists**; **no reserved slot** in the 280pt pane.

⚠️ **Async and non-blocking, without exception** (Doc 2 card-independence). ⚠️ **A pending object's row is
UNCHANGED** — name + ⚠ badge — it has no reachable image by definition.

#### T-0449 — AC2: tags

⚠️ **Owed since SP-117**, where it was deferred by user ruling for want of a chip-editor precedent. **R2 and
AC2 cannot close without it.**

⚠️ **`tags` does NOT round-trip as plain strings.** It is `[{"v": "tag"}]` (`ObjectDetail.swift:89-90`) —
⚠️ **the write path must re-emit that shape**, or every tag is silently dropped on the next read.

⚠️ **No chip editor exists in Scrivi.** ⚠️ **Look for an existing pattern before inventing one**
(`feedback_look_for_existing_pattern_first`) — SP-118 shipped four defects that each reimplemented
something the app already had. If nothing fits, a comma-separated `TextField` is acceptable and honest.

#### T-0450 — ⚠️ AC9's second half — ✅ **ALREADY TESTED LIVE; scope is now NARROW**

> ### ✅ TESTED BY THE USER 2026-08-23, before this sprint was planned
>
> *"I opened a Detail Sheet, edited it, and then pulled the drive. If the drive gets restored before the
> app quits, the fields become editable and the Save button returns to active and so saving saves the
> edits typed."*

⚠️ **The recover-and-save path ALREADY WORKS** — the sheet goes read-only on the outage, returns to
editable on remount, and the writer's typing survives the round trip intact. ⚠️ **This Task is therefore
NOT "build outage handling"**; that half is done and was demonstrated by use.

**What remains is one honestly-scoped gap, identified by the user in the same pass:**

> *"I do believe that if edits are made, the drive gets pulled, and the App is quit, then those edits will
> be lost. That is worth surfacing a warning about when the drive is pulled and edits have been made. I
> don't think we should try and save those edits for another session."*

##### ✅ RULED — warn, do not persist

**Implement:** when a world goes unavailable **and the open sheet has unsaved edits**, say so plainly — the
edits are held in memory only and **will be lost if Scrivi quits before the world returns.**

⚠️ **Explicitly OUT OF SCOPE: persisting those edits for a later session.** ✅ **User-ruled.** A draft
cache surviving quit would be a new durability contract — where does it live, when is it reconciled
against a file that may have changed, what happens if the world never returns — ⚠️ **and inventing one
inside an image sprint is how a persistence model gets set by accident** (the lesson of I-0144).

⚠️ **The warning must distinguish itself from the ordinary read-only banner.** *"This object's world is
unavailable"* is already shown and is **not** what this says. This one says **your typing is at risk** —
different fact, different urgency, and the existing banner would swallow it.

⚠️ **Verify the CONDITION, not just the banner:** it must appear only when there are **actual unsaved
edits** (`hasChanges`). A warning shown on an untouched sheet trains the writer to ignore it.

---

### Success criteria

| # | Criterion |
| - | --------- |
| **S1** | An image can be **imported, displayed, replaced and removed** from the Detail Sheet — ⚠️ **replace tested as its own trigger**, not as remove-then-import. |
| **S2** | ⚠️ A **world-scoped** object's image is written **inside the `.scrivworld` package**, confirmed on disk. |
| **S3** | ⚠️ **THE AC3 PROOF:** that image is visible from a **SECOND project** bound to the same world. ⚠️ **The user is creating that project** — it is a precondition, not sprint work. |
| **S4** | Card list items show a thumbnail **only** when an image exists; rows without one are **pixel-unchanged**. |
| **S5** | ⚠️ Thumbnails **never block the card**. Verified with the **drive ejected** — the inspector stays responsive and a pending row renders exactly as today. |
| **S6** | `tags` can be added, edited and removed; ⚠️ **they survive an app restart** — the `{"v":…}` shape round-trips. |
| **S7** | ⚠️ **Unsaved edits + world goes away → an explicit warning that the typing is at risk**, visibly distinct from the ordinary read-only banner, and ⚠️ **shown ONLY when edits actually exist.** ✅ The recover-and-save path is already proven by the user's live test and must not regress. |
| **S8** | ✅ `rebuild()` calls `scanDir()` — ⚠️ **one scan, so the image field is added once.** Proven by a test that rebuilds a **world** index as well as a project one. |
| **S9** | ⚠️ **LIVE CLICK-THROUGH on the real rig**, including the second Eskandar project and a physical eject. |
| **S10** | `ctest` green (arm64 · x86-64 · sanitizers) · Linux green · interop green · app **BUILD SUCCEEDED**. |
| **S12** | ⚠️ **An asset already in the world can be ATTACHED without re-importing it** (I-0164, T-0451). ⚠️ **Verified on the rig against `myton-portrait.png`**, which is the case that exposed it. ⚠️ **The attached assetID must be the EXISTING one** — a new ID means a re-import happened, which is the defect. |
| **S11** | ⚠️ **DESIGN-TENSION CHECK (user-ruled).** Before the image UI is built, every field the core exposes for an asset (`assetID`, `filename`, `category`, `title`, `assetPath`) is **listed with a decision**: surfaced, or deliberately not and why. ⚠️ **Not surfacing something is allowed; not having considered it is not.** This is the guard against designing to the lowest common denominator. |

⚠️ **S3 is the criterion that actually proves T-0446.** Unifying the scan (S8) removes the *risk* of the
world path drifting from the project path — ⚠️ **but only S3 proves the world path WORKS**, by reading the
image from a second project bound to the same world.

⚠️ **S11 is the one that is easy to skip and shouldn't be.** It costs a short written list and is the only
criterion here aimed at the failure the user named: designing to the easiest common denominator rather
than to the capability that exists.

---

### Sprint Notes

- ✅ **AC2, AC3, AC4 and AC9 all close here** — ⚠️ **if and only if S9 passes.** **AC5/AC6/AC7 closed in
  SP-118.** Remaining after this: **AC8** (sources, SP-120), **AC11** (Linux), **AC12**.
- ⚠️ **SP-118 raised THIRTEEN Issues, none found by any suite.** Budget for the live pass finding more, and
  ⚠️ **do not read a green suite as "it works."**
- ⚠️ **Four SP-118 defects were "an existing correct pattern the new code did not follow."** Before writing
  a control, grep for how Scrivi already does it.
- ⚠️ **SwiftUI events are asynchronous** (user guidance, SP-118 close): target the callback where the state
  is *actually current*, not the intuitive one at trigger time. Image loading is async by nature — ⚠️ **the
  likeliest place for this sprint to repeat I-0161.**
- ⚠️ **`xcodebuild test` LAUNCHES the app** (I-0150). Guarded now, ⚠️ **but checksum the rig projects before
  and after anyway** — this sprint writes real files to a real world package.
- ⚠️ **Every new `.swift` file MUST be added to `project.pbxproj` in the same step** (CLAUDE.md).
- ⚠️ **`pgrep Scrivi` before `xcodebuild test`**, and ⚠️ **re-check `SCRIVI_BUILD_TESTS` before trusting
  `ctest`** — the Xcode build flips it OFF; this cost time three times in SP-118.

### Retrospective

*(Filled in at close.)*

---

---

## ⚠️ S11 — the design-tension check for T-0447 (written BEFORE the UI)

**Every field the core exposes for an asset, with an explicit decision.** ⚠️ **Not surfacing something is
allowed; not having considered it is not.** This exists because the named risk is designing to *"the
easiest and most common"* rather than to the capability that exists.

### `ListedAsset` / `ImportAssetResult` — what the core hands the app

| Field | Decision | Reasoning |
| ----- | -------- | --------- |
| `assetPath` | ✅ **SURFACED** — as the image itself | The picture is the point. ⚠️ Rendered, never shown as text. |
| `assetID` | ⚠️ **DELIBERATELY NOT SHOWN** — held as state | Identity for replace/remove. *"The writer is not going to care what the computer calls the image on the inside."* |
| `filename` | ✅ **SURFACED** — small, secondary | ⚠️ **The one the LCD instinct would drop.** It is the only human-recognisable link back to the file she chose ("portrait-final-2.png"), and it is what lets her tell two similar images apart. Shown quietly under the image, not as a heading. |
| `title` | ⚠️ **NOT SURFACED IN SP-119** — set to the filename on import | The core accepts a title, but a second editable name beside `displayName`/`subtitle` invites "what is this for?" ⚠️ **Recorded as a deliberate omission, not an oversight** — if a writer later wants captions, the field is already there. |
| `category` | ⚠️ **NOT SHOWN; always `"image"`** | The sheet imports pictures. Audio/video/document are real categories with no surface yet — ⚠️ **that is remaining scope, not a gap to paper over here.** |
| `sidecarPath` | ⚠️ **NEVER SURFACED** | Pure storage plumbing. |
| `count` | ⚠️ **NOT SURFACED** | An object has one image; a count is meaningless at this surface. |

### `ObjectImageRef` — what the object file stores

| Field | Decision | Reasoning |
| ----- | -------- | --------- |
| `image.assetID` | ✅ **WRITTEN** | The link itself. |
| `image.thumbnailAssetID` | ⚠️ **READ, NOT WRITTEN in SP-119** | ⚠️ **The core supports a separate thumbnail and T-0446 already prefers it for card rows.** Nothing generates one yet — image downscaling is real work and not in this sprint. ✅ **The read path is built so a later sprint can populate it with no schema change.** ⚠️ **Recorded so "we have no thumbnails" is a known state, not a discovery.** |

### Capabilities the core has that this sprint does NOT surface

⚠️ **Named rather than silently skipped:**

- **Multiple assets per object** — `listAssets` returns many; `ObjectImageRef` holds one. ⚠️ **The sheet is a
  one-image surface by design**; a gallery is a separate trade nobody has ruled.
- **Non-image categories** — audio/video/document import all work at the core. No surface. Future scope.
- **Project-scoped assets** — an image on a project-scoped object (`source`) works via `worldID: ""`.
  ⚠️ **Supported and tested, just less visible** than the world case AC3 exercises.

---

## Execution status — 2026-08-23

| Task | State |
| ---- | ----- |
| **T-0446** | ✅ **Verified** |
| **T-0447** | ✅ **Verified** — ✅ S11 written BEFORE the UI |
| **T-0448** | ✅ **Verified** |
| **T-0451** | ✅ **Verified** |
| **T-0452** | ✅ **Verified** |
| **T-0449** | ✅ **Verified** |
| **T-0450** | ✅ **Verified** |

⚠️ **S9 (live click-through) and S3 (the second Eskandar project) are the user's** and cannot be marked by
Claude.

---

---

## Retrospective

### What SP-119 delivered

✅ **Seven Tasks and seven Issues Verified 2026-08-24.** The writer can now give an object an image,
tag it, see thumbnails in the inspector, reach a picture already in her world, and lose nothing when a
drive is pulled or she navigates away.

**Closes AC2, AC3, AC4 and AC9's second half.** Remaining in EP-034: **AC8** (sources, SP-120),
**AC11** (Linux), **AC12**.

### ⚠️ The number again: 7 Issues, 0 found by tests

SP-118 raised thirteen. SP-119 raised seven. ⚠️ **In both sprints, not one came from a suite.** The suites
were green throughout and were not wrong — they assert what the core does. **What no test covered was
whether a writer could reach it, and what happened when the world moved under her.**

### ⚠️ Six data-loss routes into ONE surface

| Route | Issue |
| ----- | ----- |
| Stale image path after eject | I-0162 |
| Sheet destroyed on eject (regression from I-0162's fix) | I-0165 |
| Every reload clobbered drafts (live since SP-117) | I-0165b |
| Cold open with world away → raw error code | I-0166 |
| ✕ / back / forward / related-list discarded edits | I-0167 |
| Scene Inspector double-click bypassed the guard | I-0168 |

⚠️ **Claude's failure was consistent: fix the trigger in front of it, treat the class as closed.** Four
times on the outage chain, twice on unsaved edits.

> ✅ **What finally worked both times was moving the DECISION to one owner** — the sheet owns the
> navigation veto; the render path owns the outage check — **rather than adding another check at another
> call site.**

### ⚠️ Two design-phase lessons the user named

**1. Core-before-surface is the correct sequence, not a defect.** ⚠️ **Claude had been escalating it as
`capability_without_surface`** ("the fourth and largest instance"). ScriviCore is the capability layer and
the app is one surface over it; **the capability must exist before a surface can be built on it.**
⚠️ **The real risk is the design phase** — *"not just implement the lowest common denominator UI, which
you, as an LLM, tend toward."*

✅ **S11 was the answer, and it worked**: enumerate every field the core exposes and decide each
explicitly. It caught `filename`. ⚠️ **And its limit was found the same sprint** — it enumerated FIELDS,
not OPERATIONS, which is how I-0164 slipped through.

**2. An unlinked asset is a library entry, not debris.** ⚠️ **User ruling:** an orphaned image *"may remain
there and I can link to it with other world objects as needed."* Asset deletion is unbuilt scope.

### ⚠️ Three times a comment asserted what the code did not do

I-0151 (`openObject` "resolves an empty worldID"), I-0165b (`load()` "only overwrites unedited fields"),
I-0166 ("nothing was ever loaded — there is genuinely nothing to show"). ⚠️ **All three were false, all
three cost defects.** **A comment is not an enforcement mechanism.**

### ⚠️ Process friction worth fixing

- ⚠️ **`SCRIVI_BUILD_TESTS` flips to OFF whenever the Xcode build reconfigures the shared `build/` dir** —
  hit **five times**, and a naive `ctest` re-runs a stale binary and reports green.
- ⚠️ **Claude twice handed over code it had not compiled** because Scrivi was running, describing it as
  ready. **Say "this does not build yet."**
- ⚠️ **A pbxproj ID collision** (`C020_BF` already used by `libScriviCore.a`) produced a build failure with
  a correct-looking entry count. **Check identifiers are free, not just well-formed.**

### What went right

- ✅ **S11 written before the UI**, and it changed the design.
- ✅ **The user's unification question exposed three duplications where one was predicted.**
- ✅ **An interop test caught what 565 C++ tests could not** — the save-path index gap.
- ✅ **T-0449 reused `TagChip`/`FlowLayout` rather than rewriting them**, applying SP-118's lesson before
  the duplicate shipped rather than after.
- ✅ **I-0150's guard held all sprint** — both real projects byte-identical across every test run.

### Carried out of SP-119

| Item | Owner |
| ---- | ----- |
| **AC8** — source creation + footnote text (R6) | **SP-120** |
| **AC11** — `[Linux]` parity | **SP-121** |
| **AC12** — full suite verification + Epic close prep | **SP-122** |
| ⚠️ **Asset deletion** — no surface; ✅ **ruled acceptable**, an orphan is a library entry | Unscheduled |
| ⚠️ **Thumbnail generation** — `thumbnailAssetID` is read but never written | Unscheduled |
| ⚠️ **S11 must enumerate OPERATIONS as well as fields** | **SP-120** |
| **I-0147** — the 60 s post-crash lock window; ✅ Accepted | **Network-worlds design** |

---

*Last Updated: 2026-08-24 (**SP-119 ✅ CLOSED, user-approved.** Seven Tasks + seven Issues Verified and
archived in the same step. ⚠️ **Seven Issues, none found by any suite; six were data-loss routes into one
surface.** ✅ **AC2, AC3, AC4 and AC9 close.** Suites: `ctest` **567/567**, interop **122/122**, app
**BUILD SUCCEEDED**. Active Sprints 1 → 0; EP-034 stays 🟡 Active, **5 of 8 sprints closed**.)*

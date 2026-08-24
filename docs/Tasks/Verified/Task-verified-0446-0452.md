# Verified Tasks: T-0446 – T-0452

**Sprint SP-119** · EP-034 `[Cross]` Object Detail & Media · ✅ **All seven Verified 2026-08-24
(user-approved).**

**Closes AC2, AC3, AC4** and **AC9's second half.**

---

## T-0446 — carry the image in the object index `[ScriviCore]`

`ObjectIndexEntry` gains `imageAssetID` / `imageThumbnailAssetID`, so a card list knows an object has an
image without opening it — D8's thumbnails would otherwise cost one file read per row in a 280pt pane.

✅ **User-ruled: IDs in the index, resolution at LIST time.** Assets live at `assets/<category>/<filename>`
— keyed by filename — so a path is only obtainable by reading sidecars. ⚠️ **Persisting one would store a
VOLUME-DEPENDENT value** that goes stale when a drive mounts elsewhere, and would make rebuilding the
object index depend on the assets directory being readable. `scrivi_list_objects` scans assets **once per
call** and joins.

### ⚠️ The unification the user asked for exposed THREE duplications, not one

| Duplication | Copies | What it would have cost |
| ----------- | ------ | ----------------------- |
| Directory scan | 2 | `rebuild()` carried a hand-inlined copy of `scanDir` **which already existed as the shared helper** — a field added to one and not the other presents as *"images work in my project but not from a world"* |
| Index serializer | 3 (`write`, `writeWorldIndex`, `upsertWorld`) | Image survives a project rebuild, vanishes on a world upsert |
| Entry construction in `ObjectStore` | 3 (create, save, promote) | Image attached at creation shows; attached by a later save does not |

⚠️ **The third would have shipped.** `ctest` passed **565/565** with the save path broken, because the C++
tests construct index entries directly. ✅ **An interop test caught it** — `feedback_boundary_tests_not_facade`.

> ✅ **The user's question — "can we unify the two nearly identical rebuild paths?" — was worth more than
> the one path Claude had noticed.**

---

## T-0447 — AC3: the image in the Detail Sheet `[Apple]`

Import, display, replace, remove. A world-scoped object's bytes go **inside the `.scrivworld` package**
(D6), so the image travels with the world.

⚠️ **Ordering, both directions:** replace **imports before detaching** (a failed replace must never leave
her with nothing); remove **detaches before deleting** (the reverse would point the object at bytes that
are gone). Worst case either way is an orphaned file, never a broken reference.

⚠️ **`attachImage` re-reads from disk and patches** — I-0155's lesson applied pre-emptively, since the
Scene Inspector can be editing the same object.

### ✅ S11 — the design-tension check, written BEFORE the UI

⚠️ **The guard against designing to "the easiest and most common"** (user's framing). Every field the core
exposes, with an explicit decision — **not surfacing something is allowed; not having considered it is
not.**

It caught **`filename`** — the field the lowest-common-denominator instinct drops, and the only
human-recognisable link back to the file chosen. Deliberate omissions recorded rather than discovered:
`title` (set to the filename), `thumbnailAssetID` (**read but not written** — nothing generates one yet),
`category` (always `image`), `assetID` (**never shown**), `sidecarPath`.

⚠️ **S11's limit, found by I-0164:** it enumerated **FIELDS, not OPERATIONS.** `listAssets` was marked "not
surfaced" without asking what a writer would use it for — the answer was "attach the picture I already
have," which became T-0451.

---

## T-0448 — AC4 / D8: card thumbnails `[Apple]`

✅ **D8-A:** thumbnail **only when an image exists**, no reserved slot in the 280pt pane.

Paths come from the listing `ObjectCard` **already performs** — zero extra reads, which was T-0446's whole
purpose. ⚠️ **Decoding runs in a DETACHED task, off the main actor**: an image may sit on a sleeping USB
volume, and D8 is explicit that *"a thumbnail that hangs the inspector would be a worse defect than no
thumbnail at all."*

⚠️ **A slow or failed load leaves the row exactly as if there were no image** — never a spinner, never a
broken-image placeholder. ⚠️ **Stale results are dropped when `LazyVStack` recycles a row mid-decode**, or
one object's picture would appear on another's row.

⚠️ **A pending object never reaches the thumbnail path** — it is absent from the listing, so its row is
unchanged by construction rather than by a special case.

✅ **Verified by use**, including with the drive ejected.

---

## T-0449 — AC2: tags `[Apple]`

⚠️ **Owed since SP-117, and its deferral reason turned out to be WRONG.** It was deferred *"for want of a
chip-editor precedent in the app"* — ⚠️ **`TagChip` and `FlowLayout` had been in `WritingToolCards.swift`
since T-0363.** They were promoted from `private` and **reused**, not rewritten.

> ✅ **This is `feedback_look_for_existing_pattern_first` applied BEFORE shipping a duplicate rather than
> after** — the failure that cost SP-118 four defects.

⚠️ **Tags round-trip as `[{"v": "..."}]`, not `["..."]`.** Writing a plain string array would parse back as
an **empty list** — every tag silently dropped on the next read, with the object file looking perfectly
reasonable. ✅ **Verified across an app restart**, which is the check that would have exposed it.

⚠️ **Read-only renders inert pills, not disabled chips** — I-0148's lesson that read-only must be
*enforced*, not styled.

---

## T-0450 — AC9's second half: edits at risk `[Apple]`

✅ **The recover-and-save path already worked** and was proven by the user's own live test before this
sprint planned it: the world returns, fields unlock, Save writes the typing. ⚠️ **What was missing is that
she was never TOLD** her unsaved work was held only in memory.

⚠️ **A SECOND banner, deliberately not a longer first one:** the read-only banner states a fact about the
object; this states a risk to **her work**, and folding them together buries the urgent one in the routine
one. Shown only when `hasChanges` is genuinely true.

✅ **User-ruled: warn, do not persist.** A draft cache surviving quit would be a new durability contract,
and inventing one inside an image sprint is how a persistence model gets set by accident (I-0144's lesson).

---

## T-0451 — I-0164: attach an asset already in the world `[Apple]`

✅ **User-ruled option 1.** "Choose Existing…" lists images already in the package and attaches by writing
the **existing** assetID — no import, no new identity.

⚠️ **The previous asset is deliberately NOT deleted on attach.** Nothing stops two objects referencing one
image, and `removeAsset` would pull the bytes out from under the other. **Import's replace path does
delete, because there it created the replacement itself** — different provenance, different rule.

✅ **S12 verified by use:** attached an older image after importing a newer one, switched between them
freely, **no new files written**.

---

## T-0452 — I-0167 + I-0168: Cancel, and a guarded exit `[Apple]`

**Cancel** beside Save reverts to the saved version; any exit with unsaved edits asks **Save / Discard /
Cancel** and then completes the action attempted.

⚠️ **Cancel re-reads from disk** rather than restoring the in-memory snapshot — "the saved version" must
mean what is actually saved (I-0155).

⚠️ **Six exits, found in two passes.** The first covered the four that originate **inside** the sheet (✕,
back, forward, related-list). ⚠️ **The Scene Inspector's double-click originates OUTSIDE it** — the host
owns the history — and bypassed the guard entirely. ✅ **Fixed by moving the decision to a single owner**
rather than adding a fifth check.

> ⚠️ **The question that finds these: "who else can change what this sheet is showing?"** — not "what other
> buttons are on this sheet?"

---

## Suites at verification

`ctest` **567/567** · interop **122/122** · app **BUILD SUCCEEDED** · both real rig projects
**byte-identical** across every test run (I-0150's guard holding).

---

## ⚠️ What this Task set cost, recorded honestly

**Seven Issues** (I-0162 – I-0168) were raised against these Tasks **after** the suites went green.
⚠️ **Every one was found by the user's live click-through; none by any suite.** ⚠️ **Six are data-loss
routes into a single surface**, reachable by ejecting a drive or navigating away at six different moments.

⚠️ **The recurring failure was Claude's, and it is specific:** fixing the trigger in front of it and
treating the class as closed. It happened four times on the world-outage chain (I-0162 → I-0166) and twice
on unsaved edits (I-0167 → I-0168). **The cure that finally worked both times was moving the decision to
one owner, not adding another check.**

---

*Last Updated: 2026-08-24 (**T-0446 – T-0452 ✅ VERIFIED, user-approved, and ARCHIVED in the same step** at
SP-119 close. Closes **AC2, AC3, AC4** and **AC9's second half**. ⚠️ **T-0446 exposed three duplications
where one was predicted**; ⚠️ **T-0449 disproved SP-117's own deferral reason.**)*

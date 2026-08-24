# Verified Issues: I-0161 – I-0170

⚠️ **I-0162 – I-0168 all came from SP-119's live click-through. None was found by any suite.**

⚠️ **Six of the seven are DATA-LOSS routes into one surface** — the Object Detail Sheet. They are recorded
together because the pattern matters more than any single fix: **a world can go away, or a writer can
navigate away, at six different moments, and each is a different code path.** Each fix was correct and each
covered only the trigger in front of it, until the last two moved the *decision* rather than adding another
check.

> ⚠️ **I-0161 lives in [`Issue-verified-0151-0160.md`](Issue-verified-0151-0160.md)**, filed out of decade
> so SP-118's findings stayed together.

---

## I-0162 — ⚠️ An ejected drive reported the writer's image as DAMAGED, not absent

**Verified 2026-08-24 (user-approved).** SP-119 · `[Apple]` · **Medium**

With a Detail Sheet open, ejecting the world's volume showed *"This image could not be read from disk"* —
a damaged-file message for an outage.

**Two causes.** The sheet never reloaded on a world-availability change, so `imagePath` stayed a snapshot
from before the eject — ⚠️ **`session.worldRevision` already existed for exactly this** (I-0128/I-0129) and
the inspector's cards had watched it all along. And the outage branch was evaluated only at load time, so
a volume vanishing afterwards fell through to the decode-failure branch.

**Fixed:** the sheet watches `worldRevision`, and the section checks the bytes exist **at render time**
before blaming the file.

> ⚠️ **Claude's first two hypotheses were wrong.** The cause was settled only by probing the core directly
> — `loadAllVisible` returns **count=0** for an unavailable world, so the object was not in the listing at
> all, which ruled out everything except a stale snapshot in the view.

---

## I-0163 — ⚠️ An image on disk in a world was INVISIBLE to the app

**Verified 2026-08-24 (user-approved).** SP-119 · `[ScriviCore]` · **High**

Myton's picture was in `Eskandar.scrivworld` and his Detail Sheet showed no image.

⚠️ **The object index is a DERIVED CACHE, and every cache already on disk predated T-0446's
`imageAssetID` field.** `parse()` accepted those files happily and `loadWorldIndex` returns a parseable
cache **verbatim, without rescanning** — so a world whose objects had images reported none, permanently.

⚠️ **T-0446's own tests could not catch this:** they create a project, so their index is always written by
the current build. **The stale-cache case only exists on a real project that predates the field** — which
is what the rig is.

**Fixed** with a `generation` marker beside the schema. A mismatch (absent reads as `0`) makes `parse()`
return nullopt, which every caller already treats as *"rebuild from the object files"*. The files are the
truth (v0.2 §4.2), so the repair costs one scan, cannot lose data, and persists.

> ⚠️ **Bump `kIndexGeneration` when adding an entry field.** Not bumping it is silent and presents exactly
> as this did. ✅ **Confirmed on the rig:** Eskandar's index self-healed to `generation: 2`.

---

## I-0164 — ⚠️ An already-imported asset could not be attached; re-importing orphaned the first

**Verified 2026-08-24 (user-approved).** SP-119 · `[Cross]` · **Medium** · shipped as **T-0451**

T-0447 gave exactly one way to set an image — *Add Image…*, which imports from the filesystem. ⚠️ **A
picture already in the world was unreachable.**

⚠️ **And the workaround is harmful.** `AssetStore` stores bytes at `assets/<category>/<filename>` with the
sidecar at `<filename>.meta.json`, so a same-filename import **overwrites both and mints a new assetID**.
✅ **Proven by test:** one asset on disk (not two), the second ID listed, ⚠️ **the first no longer
resolvable** — any object still pointing at it dangles silently.

⚠️ **S11 did not catch this because it enumerated the FIELDS the core exposes, not the OPERATIONS.**
`listAssets` was recorded as "not surfaced" without asking what a writer would use it *for*.

✅ **RULED (user): option 1 — attach an existing asset.** Options 2 (idempotent import) and 3
(assetID-named files) were explicitly not chosen.

✅ **Verified by use:** attached an older image after importing a newer one and switched between them
freely, with **no new files written**.

> ⚠️ **User ruling on orphans:** *"there is no way yet to remove unwanted images, however, it doesn't rise
> to an issue yet. The orphaned image may remain there and I can link to it with other world objects as
> needed."* ⚠️ **This reframes an unlinked asset: it is not debris, it is a LIBRARY ENTRY** — imported
> once, attachable to any object in the world. **Asset deletion is unbuilt scope, not a defect.**

---

## I-0165 — ⚠️ Ejecting the drive DESTROYED the open Detail Sheet

**Verified 2026-08-24 (user-approved).** SP-119 · `[Apple]` · **High**

The whole pane was replaced by a raw *"ScriviApp:Scrivi Error -1"*, ⚠️ **and unsaved edits were discarded
with it.**

⚠️ **A regression introduced by I-0162's own fix.** Making the sheet re-read on `worldRevision` was
correct, but `load()`'s catch branch sets `detail = nil`, and `openObject` fails for an unavailable world —
so the reload meant to *explain* the outage threw the sheet away instead. **R9 violated outright.**

**Fixed:** a failed re-read keeps the object it already has and clears only `imagePath`; the read-only
banner reports the outage in words.

> ⚠️ **`load()`'s error branch had always been allowed to blank the sheet** because it previously only ran
> on navigation, where blanking is correct. **A fix that changes WHEN a view reloads inherits every failure
> mode of the reload path.**

### I-0165b — ⚠️ and a second, older data-loss path found while fixing it

`load()` overwrote **all three drafts unconditionally**, while a comment asserted the opposite —
*"load() only overwrites a draft field the writer has not edited."* ⚠️ **The claim was false and nothing
enforced it**, so any reload — an Inspector rename (I-0160), a remount (I-0162) — silently discarded her
typing. Live since SP-117.

**Fixed:** a field still equal to what disk last said is adopted; a field she changed is kept. ⚠️ **Scoped
to the SAME object** — navigating elsewhere adopts disk wholesale, or one object's text lands on another.

> ⚠️ **Two comments in this Epic have asserted a property the code did not have** (I-0151's `openObject`
> claim, this one). Both cost real defects. **A comment is not an enforcement mechanism.**

---

## I-0166 — ⚠️ Cold-opening an object with its world away showed a raw error code

**Verified 2026-08-24 (user-approved).** SP-119 · `[Apple]` · **High**

Launch → eject → "Show" on Myton → *"ScriviApp:Scrivi Error -1"*.

⚠️ **R9 violated in the case R9 is most about.** ⚠️ **To a writer, "error -1" on a character she wrote is
indistinguishable from having lost her** (Doc 3: absence is never deletion).

⚠️ **I-0165's fix did not cover this** — it preserved a sheet already displaying an object, and the comment
it left behind (*"nothing was ever loaded — there is genuinely nothing to show"*) was **wrong**:
`ObjectDetailHistory.Entry` carries the object's **name, kind and worldID** all along.

**Fixed** with `isWorldUnavailable` / `unavailableWorldStatus` on `ScriviError`, mirroring the existing
`isWorldPending` idiom, so the sheet renders a named, explained state.

✅ **Swept the other `openObject` call sites** rather than assuming: `SourcesCard` has the same shape but
`source` is the sole project-scoped kind, so its world can never be away; `ObjectCard.rename` reports in
place.

---

## I-0167 — ⚠️ Closing the sheet discarded unsaved edits, with no way to revert

**Verified 2026-08-24 (user-approved).** SP-119 · `[Apple]` · **High** · shipped as **T-0452**

The ✕ called `onClose()` unconditionally and the host reset the history — no prompt, no recovery.
⚠️ **The third route into this Epic's data loss, and the only one a writer triggers with an ordinary
click.**

⚠️ **Back/forward and related-list navigation had the SAME exposure** — they mutate history before
`load()` runs. Fixed in the same pass rather than left for a later report.

✅ **Fixed to the user's design:** a **Cancel** button beside Save reverts to the saved version, and any
exit with unsaved edits asks **Save / Discard / Cancel**, then completes the action attempted.

⚠️ **Cancel RE-READS from disk** rather than restoring the in-memory snapshot — another surface may have
written since (I-0155). ⚠️ **Read-only objects do not prompt**; their drafts are covered by T-0450's
at-risk banner.

> ⚠️ **Explicitly NOT undo**, per the user: *"I do NOT want to get involved with the history here or a real
> undo."* EP-019's sentence-granular history stays out of object editing (D3-C).

---

## I-0168 — ⚠️ The Scene Inspector bypassed T-0452's guard entirely

**Verified 2026-08-24 (user-approved).** SP-119 · `[Apple]` · **High** · shipped as **T-0452**

Editing a field then double-clicking another object in the Scene Inspector discarded the edits.

⚠️ **The guard was in the wrong PLACE.** `ObjectDetailHistory` is owned by the **host**; the inspector asks
the host to navigate, and the host called `visit()` directly — so the sheet, where the guard lived, was
never consulted. ⚠️ **T-0452 swept the four exits that originate INSIDE the sheet and could not see the one
that originates outside it.**

✅ **Fixed by moving the decision, not adding a fifth check:** the host stages an external navigation and
hands it to the sheet, which performs it or prompts first — **one owner for the veto**. When the sheet is
closed the original behaviour is kept.

✅ **Swept every remaining history mutation**: all are inside the guard, on the closed-sheet path, or a
`reset()` after a confirmed close.

> ⚠️ **The question that would have caught it: "who else can change what this sheet is showing?"** — not
> "what other buttons are on this sheet?" The first finds the host; the second cannot.

---

## I-0169 — `[Apple]` The `sources` card had no route to the Detail Sheet

**Severity:** Medium · **Sprint:** SP-120 · ✅ **Verified 2026-08-24** (user-approved, by use)

⚠️ **Found by the user's live pass, and it was her FIRST instinct** — *"I started by trying to show the
data sheet from the sources card in the Scene Inspector. Double click and Right-click 'View Detail' are
not wired up on that sheet yet."*

Every object row in the app opens its sheet on **double-click** and **right-click → "View Detail"**
(AC1 / R7, `ObjectCard.swift:1049-1058`). The `sources` card row was a plain `Button` — single-click
popup, **no `onTapGesture(count: 2)`, no `contextMenu`**. ⚠️ **So the one card in the app whose entire
subject is an object taught a different verb from every other one.**

✅ **Fixed** in `SourcesCard.swift`: double-click and right-click → "View Detail" open the sheet; single
click still opens the citation popup, which is this card's own established behaviour (§3.1.1).

> ⚠️ **The hook ALREADY EXISTED and this card simply never called it.** `CardContext.openObjectDetail`
> (`InspectorCard.swift:149-150`) carries exactly `(objectID, kind, worldID, displayName)`, and
> `ObjectCard` has used it since SP-117. **Nothing new was built.**
>
> ⚠️ **`feedback_look_for_existing_pattern_first` — the same shape as SP-118's four defects**, where each
> violated a rule already written down in this repo before the offending code was typed.

✅ **The hook was confirmed LIVE before the fix was called done**, not assumed: `SceneInspectorView.swift:73`
supplies it to the **Writing** stack, which is where this card lives. ⚠️ An unplumbed hook would have made
the fix silently dead — the exact failure mode of `capability_without_surface`, committed while fixing it.

---

## I-0170 — `[Apple]` A twice-cited source named only ONE citing object

**Severity:** Low · **Sprint:** SP-120 · ✅ **Verified 2026-08-24** (user-approved, by use)

⚠️ **Found by the user's live pass, on real data** — *"The source is actually included in the scene due to
2 objects. The first is Myton at 23, and the second is Tintagael."* ⚠️ **Confirmed on disk**:
`the-lone-golem.scrivi/objects/relationships.jsonl` holds two `cites` edges from one source — to a
**character** and a **location**.

`ObjectSourcesSection.attributionSentence` hardcoded `"Cited by \(objectDisplayName)"`, naming **only the
object whose sheet was open**. Opened from Myton, the popup claimed one citing object where the writer
had two.

✅ **Fixed:** the sentence now walks outward **from the source** and names every citing object — *"Cited by
Myton at 23, and by Tintagael."* ⚠️ **A pending far endpoint is NAMED, not skipped** — `otherDisplayName`
is cached on the edge for exactly that reason, and omitting it would under-report the graph a second way
(Doc 3: absence is never deletion).

> ⚠️ **The SP-120 decision was right and its implementation was incomplete.** T-0456 ruled the aggregate
> card's *"via …"* circular when read on the citing object itself — correct. But the reword **dropped
> information instead of re-centring it**, and ⚠️ **the aggregate card had this right all along**:
> de-duplicating across objects and naming both was an explicit T-0365 design point (*"two rows for one
> source reads as two sources"*).
>
> ⚠️ **THE LESSON: a field-level review could not have caught this.** Every field was present and
> populated; the surface was **quietly under-reporting the writer's own graph**, which reads as correct
> and is not. It took real data with a source cited **twice, across two different kinds** — the case S11's
> operations table pointed at (cite an existing source from a second object) but which only real use
> produced.

---

*Last Updated: 2026-08-24, second pass (**I-0169 + I-0170 ✅ Verified, user-approved, from SP-120's live
pass** — the sources card's missing sheet route, and a twice-cited source naming only one citing object.
⚠️ **This decade file now closes at I-0170; the next Issue is I-0171 and opens a new decade file.**
⚠️ **Both were found by clicking, neither by a suite** — which now holds for **22 consecutive Issues**
across SP-118, SP-119 and SP-120. Prior note follows.)*

*Last Updated: 2026-08-24 (**I-0162 – I-0168 ✅ Verified, user-approved, at SP-119 close.** ⚠️ **All seven
came from the live click-through; none from any suite.** ⚠️ **Six are data-loss routes into one surface**,
found by ejecting a drive or navigating away at six different moments. I-0161 is filed in the previous
decade file with SP-118's findings.)*

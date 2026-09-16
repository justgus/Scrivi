# Scrivi — Asset Presence-Check Ruling (Class C of [I-0197]) v0.1

**Sprint:** [SP-130] · **Task:** T-0508 · **Epic:** [EP-040] · **Date:** 2026-09-16
**Status:** ⚠️ **RULED — awaiting user verification**

---

## ✅ The question this document answers

[I-0197] Class C named five `FileManager.fileExists` calls on asset paths and asked, per site:

> ⚠️ **legitimate UI presence-check, or a core question?**
> ⚠️ **and on an unreachable volume a `fileExists` call is exactly the blocking-stat
> pattern [I-0193] cost 102 s.**

⛔ **It was never answered.** ✅ **"Whatever the code happens to do" is how a rule drifts**, which is
why SP-130 exists even though four of the five sites turn out to be correct.

---

## ⚠️ The ruling separates TWO questions the planning text ran together

⚠️ **SP-130's own record said three sites "already degrade gracefully with the reasoning written
in-line" and treated that as the whole answer.** ⛔ **It is half of it.**

| question | what it asks | ✅ what answers it |
| -------- | ------------ | ----------------- |
| ⚠️ **ARCHITECTURE** | is this backend logic in Swift? | ⚠️ what the check is USED FOR |
| ⚠️ **COST** | does it block the main actor? | ⚠️ WHERE it runs |

⚠️ **Degrading gracefully answers the FIRST question only.** ✅ **A site can say exactly the right
thing to the writer and still freeze the window for 102 s while deciding to say it.**
⚠️ **That conflation is what let the fifth site through the 2026-09-15 re-measurement.**

---

## ✅ R1 — The architectural ruling: all five are LEGITIMATE UI presence-checks

✅ **`fileExists` on a path the core ALREADY RESOLVED is not a bypass of ScriviCore.**

⚠️ **The reasoning, and it is narrow:**

1. ✅ **The core resolved the path.** ⚠️ **Every one of these five calls takes a path that came OUT of
   `scrivi_list_assets` or the object's resolved `imagePath`.** ⛔ **None constructs a package path,
   none knows the layout, none parses a schema.** ⚠️ **That is what separates Class C from Class B
   (`InspectorLayoutStore`, which composes `inspector-layout.json` itself).**
2. ✅ **The question is about NOW, and the core cannot answer it.** ⚠️ **[I-0197] recorded this
   already: `scrivi_list_assets` / `scrivi_import_asset` / `scrivi_remove_asset` all exist and
   ⛔ none answers *"is this resolved path readable RIGHT NOW"* without opening the file.**
   ⚠️ **A resolved path is a snapshot; a removable volume can vanish after it.**
3. ✅ **The consumer is a DRAW DECISION, not a data decision.** ⚠️ **All five choose between a picture
   and a placeholder.** ⛔ **None writes, none repairs, none decides what is true about the project.**

⛔ **R1 does NOT generalise to "Swift may stat inside the package."** ✅ **It licenses exactly one
shape: an existence check, on a core-resolved path, whose only output is which view to draw.**
⚠️ **Anything that stats a path Swift COMPOSED is Class B and is [SP-141]'s.**

---

## ⚠️ R2 — The cost ruling: a presence-check on an asset path MUST NOT run on the main actor

✅ **Rule: these bytes may be on a removable or network volume, so the check is UNBOUNDED in time.**
⚠️ **`fileExists` on a dead SMB mount is [I-0193]'s 102 s freeze**, ⚠️ **and the fact that the code
recovers gracefully afterwards does not help a writer who has already Force Quit.**

✅ **`ObjectCard.swift`'s `ObjectRowThumbnail` ALREADY states this rule and obeys it** — ⚠️ **the
comment at `:975` is the precedent, written under EP-034 trade **D8-A**:**

> ⚠️ `Task.detached` so the decode never runs on the main actor. A 4 MB PNG on a
> sleeping USB drive can take seconds; the inspector must stay live.

⚠️ **R2 is therefore not a new rule.** ✅ **It is the existing rule, now written where the other four
sites can be measured against it** — ⚠️ **which is the whole reason T-0508 is a ruling and not a
sweep** (`feedback_look_for_existing_pattern_first`).

---

## ✅ Site-by-site disposition

| # | site | R1 (architecture) | R2 (cost) | disposition |
| - | ---- | ----------------- | --------- | ----------- |
| 1 | `ObjectCard.swift:980` | ✅ legitimate | ✅ **inside `Task.detached(.utility)`** | ⛔ **NO CHANGE** |
| 2 | `ObjectCard.swift:984` | ✅ legitimate | ✅ **same detached block (`#else` branch)** | ⛔ **NO CHANGE** |
| 3 | `ObjectImageSection.swift:106` | ✅ legitimate | ✅ **bounded — see below** | ⛔ **NO CHANGE** |
| 4 | `ObjectImageSection.swift:137` | ✅ legitimate | ✅ **bounded — see below** | ⛔ **NO CHANGE** |
| 5 | `ExistingAssetPicker.swift:130` | ✅ legitimate | ⛔ **VIOLATES R2** | ⚠️ **FIXED** |

### ✅ Sites 1–2 — `ObjectCard.ObjectRowThumbnail`

⛔ **No change.** ✅ **Both calls sit inside `Task.detached(priority: .utility)` and the surrounding
comment states the rule R2 generalises.** ⚠️ **The stale-result guard (`guard wanted == path`) is
also correct and load-bearing** — ✅ **`LazyVStack` recycles rows, and dropping a stale decode is
what stops one object's picture appearing on another's.**

### ✅ Sites 3–4 — `ObjectImageSection.preview(_:)`

⛔ **No change — and the reason is NOT "it degrades gracefully".** ⚠️ **It is that these two calls
are UNREACHABLE when the volume is away**, so the unbounded case cannot arise:

- ✅ **`preview(path)` is called only from `if let path = resolvedPath`** (`:65`).
- ✅ **`resolvedPath` is non-nil only when the CORE resolved `imagePath`** (`:199–202`).
- ⚠️ **When the world is away the core resolves nothing, `hasUnresolvableImage` is true, and the
  sibling branch draws the outage message** — ⛔ **`preview` is never entered.**

⚠️ **So the check runs against a path the core resolved MOMENTS AGO on a volume it just read.**
✅ **The residual race — the drive pulled between resolve and draw — is exactly what sites 3–4
EXIST to catch**, and it is the case where the stat returns fast (the mount is gone, not hanging).

⚠️ **A second call at `:137` repeats the same stat to decide whether to show the filename.**
✅ **Left as-is deliberately:** ⛔ **hoisting it into a `let` would be a micro-optimisation that
changes nothing measurable**, ⚠️ **and the two checks are semantically independent — one guards the
image, one guards the caption.** ✅ **Recorded here so a later reader does not "fix" it blind.**

### ⛔ Site 5 — `ExistingAssetPicker.thumbnail(_:)` — **THE ONE REAL DEFECT**

⚠️ **This is the site the 2026-09-15 re-measurement cleared and should not have.** ⛔ **Its in-line
comment — *"a failed load is expected, not corruption"* — answers R1, and the re-measurement read it
as answering both.**

⚠️ **What it actually does, on the MAIN ACTOR, inside a `LazyVStack` row builder:**

```swift
if FileManager.default.fileExists(atPath: path),
   let nsImage = NSImage(contentsOfFile: path) {
```

⛔ **Two unbounded operations, not one** — ✅ **the stat AND a full synchronous image decode** —
⚠️ **per visible row**, ⚠️ **on bytes that by construction live in a WORLD PACKAGE, which is the
removable/network case.**

⚠️ **And unlike sites 3–4, there is NO core-resolution gate.** ✅ **`asset.assetPath` comes straight
from `listAssets`, which enumerates the package** — ⚠️ **it is a listing, not a proof of readability,
and the popover can be opened while a world is going away.**

⚠️ **Worse than sites 3–4 by a factor of the row count:** ⚠️ **`ObjectImageSection` stats ONE path;
this stats and DECODES one per asset in the world.**

⛔ **Verdict: violates R2.** ✅ **Fixed by adopting `ObjectRowThumbnail`'s already-proven shape** —
⚠️ **not by inventing a second mechanism.**

---

## ✅ What changed

⚠️ **ONE file: `Scrivi/Views/Detail/ExistingAssetPicker.swift`.**

✅ **`thumbnail(_:)` becomes a small `View` that loads off the main actor**, mirroring
`ObjectCard.ObjectRowThumbnail` — ⚠️ **including its stale-result guard, because this list is a
`LazyVStack` and recycles the same way.**

⚠️ **The placeholder behaviour is UNCHANGED and deliberately so:** ✅ **a failed or slow load still
shows the `photo` icon, never a spinner and never a broken-image glyph.** ⚠️ **The icon is drawn
IMMEDIATELY now rather than after the stat returns** — ✅ **which is the entire user-visible
difference: the popover opens at once instead of waiting on the volume.**

⛔ **The non-macOS branch is untouched** — ✅ **it already drew the icon unconditionally and never
touched the filesystem.**

---

## ⛔ What this ruling does NOT do

- ⛔ **No new core endpoint.** ⚠️ **A `scrivi_asset_is_readable` was considered and REJECTED:** it
  would put an unbounded filesystem stat BEHIND the synchronous C ABI, ⚠️ **which moves the freeze
  rather than removing it**, and ⛔ **every caller is a draw decision that must degrade anyway.**
- ⛔ **No guard.** ✅ **[T-0510] is [SP-143]'s**, deliberately — ⚠️ **a guard written here would have
  to allow-list `InspectorLayoutStore` and then be edited by [SP-141], passing green across the very
  change it polices.**
- ⛔ **[I-0197] is NOT closed.** ✅ **Class C is ruled; Class B ([SP-141]) and the guard ([SP-143])
  remain open.**

---

## ✅ Verification

| DoD item | ✅ status |
| -------- | -------- |
| ⚠️ **Class C ruling WRITTEN DOWN, per site** | ✅ **this document, R1 + R2 + the five rows** |
| ⚠️ **Whatever is ruled MUST NOT block the UI thread** | ✅ **site 5 moved off the main actor; 1–2 already were; 3–4 proven unreachable when unbounded** |
| ⚠️ **`xcodebuild` green: macOS, iOS, visionOS** | ✅ **see [`Sprint-SP-130.md`](Sprints/Sprint-SP-130.md)** |

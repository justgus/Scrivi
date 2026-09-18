# Verified Task: T-0508

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0508** | ⚠️ **Rule the 5 `fileExists` asset sites (Class C of [I-0197])** — ✅ **ruling written; ⛔ one site was a REAL defect and is fixed** | [SP-130] | [EP-040] | **2026-09-18** |

---

## What it was

⚠️ **THE TASK'S OWN PREMISE DID NOT SURVIVE CONTACT, and that is the point worth keeping.** It was
planned as *"2 off-thread, 3 degrade gracefully — so this is a RULING to record, not a defect to fix."*
⛔ **That was wrong on one of the five sites.**

✅ **Writing the ruling is what found it**, by splitting one question into two:

- ✅ **"Degrades gracefully"** answers **ARCHITECTURE** — is this backend logic leaking into Swift?
- ⛔ **It does NOT answer COST** — does it block the main actor?

⚠️ **`ExistingAssetPicker.swift:130` ran `fileExists` AND a full `NSImage` decode SYNCHRONOUSLY, per
row, in a `LazyVStack`, on bytes that live in a world package by construction.** ✅ **Fixed by adopting
`ObjectCard.ObjectRowThumbnail`'s already-proven off-thread shape** (`thumbnail(_:)` → `AssetThumbnail`).

✅ **The ruling:** [`Scrivi_Asset_Presence_Check_Ruling_v0_1.md`](../../Scrivi_Asset_Presence_Check_Ruling_v0_1.md).

---

## How it was verified

⚠️ **The suites could not verify this and none was claimed to.** ✅ **`xcodebuild` green on all three
schemes and 127 interop tests passing were recorded** — ⛔ **but no test opens the picker against an
unreachable volume, and four of the five sites did not change at all.**

✅ **Verified by LIVE passes on the real rig** (`project_test_rig_tintagael_eskandar`):

| Pass | Result |
| ---- | ------ |
| Scale pass | ✅ **passed** (2026-09-16) |
| Adversarial pass | ✅ **passed** (2026-09-16) |
| ⚠️ **The user-visible test — *Choose Existing…* popover** | ✅ **user-confirmed 2026-09-17: opens IMMEDIATELY** |

⚠️ **Before the fix the popover did not appear until every visible thumbnail had been stat'd AND
decoded** — ⛔ **which on a sleeping or disconnected USB volume is [I-0193]'s freeze, once per row.**

---

## The drive-pull test

✅ **COMPLETE 2026-09-18.** ⚠️ **It had been deferred as "informational" — it was not.**

⛔ **It produced two Issues, one Critical**, both archived to
[`../../Issues/Verified/Issue-verified-0221-0230.md`](../../Issues/Verified/Issue-verified-0221-0230.md):

- ⛔ **[I-0221]** — a project on ANY non-APFS volume became unopenable, **repeatedly**, because the app
  regenerated the AppleDouble sidecars that broke it.
- ✅ **[I-0222]** — an unavailable world reported `ScriviError 1` instead of naming the world.

✅ **AND IT CONFIRMED THE PENDING-WORLD ARCHITECTURE WORKS**, which is the more important half:
the manuscript stayed fully usable with the volume gone, edges were held pending rather than pruned,
and everything restored on reattach with **no repair prompt and no data loss**.

⚠️ **Recorded for whoever runs the next platform's equivalent:** ⛔ **every inference drawn from
reading the code during I-0222 was wrong — twice — and only running the endpoints settled it.**
✅ `feedback_live_pass_finds_what_suites_cannot`.

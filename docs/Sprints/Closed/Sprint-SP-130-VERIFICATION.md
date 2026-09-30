---
sprint: SP-130
task: T-0508
status: Procedure — not yet run
---

# SP-130 / T-0508 — Verification Procedure

**Setup · Test · Teardown** for the one behavioural change in this Sprint:
`ExistingAssetPicker`'s thumbnails now load **off the main actor** (ruling **R2**).

⚠️ **THE CHANGE IS INVISIBLE WHEN EVERYTHING IS HEALTHY.** ✅ **On a fast local volume the old code
and the new code look IDENTICAL** — ⚠️ **the stat and decode were always going to finish in a few ms.**
⛔ **So a "does the picker still work" pass verifies NOTHING.** ✅ **The defect only appears when the
bytes are on a volume that is slow or gone**, ⚠️ **which is why T1 (the adversarial case) is the test
that actually matters and T0 is only a control.**

---

## ⚠️ READ FIRST — two things that will bite

### 1. ⚠️ **BACK UP BEFORE ANY UNMOUNT TEST. This is real writing work.**

⚠️ **`the-stairs-of-tintagael.scrivi` is the user's manuscript, not a fixture**, ✅ **and the criterion
under test involves pulling a drive out from under an open app.** ⛔ **The Desktop backup
`…backup-20260729-162050` PREDATES the world binding and is NOT sufficient.**

### 2. ⛔ **THE WORLD DOES NOT CURRENTLY RESOLVE — and you must decide this before starting**

✅ **Measured 2026-09-16:**

| | value |
| - | ----- |
| ⚠️ **`binding.json` `lastKnownAbsolutePath`** | `/Volumes/Scrivi Worlds/Eskandar.scrivworld/` ⚠️ **(SPACE)** |
| ⚠️ **actually mounted** | `/Volumes/Scrivi-Worlds` ⚠️ **(HYPHEN)** |
| ⛔ **`/Volumes/Scrivi Worlds` exists?** | ⛔ **NO** |

⚠️ **The drive appears to have been RENAMED since the binding was written**, ✅ **so Eskandar is
already in the unavailable state** — ⚠️ **and an unavailable world means `listAssets` has nothing to
list, so the picker opens EMPTY and T0/T1 test nothing at all.**

⛔ **DO NOT "fix" this by editing `binding.json` by hand** — ⚠️ **that is a Swift-side write into a
package the core owns, which is the very class of defect [I-0197] exists to close.**

✅ **Setup step S3 resolves this properly, and its result is itself worth recording** — ⚠️ **if the
app RE-BINDS the world cleanly on reattach, that is [EP-031] **AC23** behaving correctly; ⛔ if it
does not, that is a finding that belongs in its own Issue, NOT in this Sprint.**

---

## ✅ SETUP

### S1 — Back up the rig  ⚠️ **NON-OPTIONAL**

```bash
# Adjust the destination if you keep backups elsewhere.
DEST=~/Desktop/SP-130-backup-$(date +%Y%m%d-%H%M%S)
mkdir -p "$DEST"
cp -Rp "/Volumes/Scrivi-Worlds/the-stairs-of-tintagael.scrivi" "$DEST/"
cp -Rp "/Volumes/Scrivi-Worlds/Eskandar.scrivworld"            "$DEST/"
du -sh "$DEST"      # expect ~17 MB total
```

✅ **Confirm the copy is real before continuing** — ⛔ **an empty or partial backup is worse than none,
because it invites the confidence to proceed.**

### S2 — Confirm WHICH BUILD is under test

⚠️ **The rig has run a day-stale binary before** (`feedback_confirm_the_build_under_test`).

```bash
cd /Users/justgus/Xcode-Projects/Scrivi
git log --oneline -1
git status --short          # expect the SP-130 changes present
grep -c "AssetThumbnail" Scrivi/Views/Detail/ExistingAssetPicker.swift   # expect 2
```

⚠️ **Then build and launch THAT source** — ⛔ **do not open a previously-installed Scrivi.app:**

```bash
pgrep -fl "Scrivi.app" || echo "not running"      # must be not running
xcodebuild -scheme ScriviApp -destination 'platform=macOS' build
open ~/Library/Developer/Xcode/DerivedData/Scrivi-*/Build/Products/Debug/Scrivi.app
```

### S3 — Get Eskandar RESOLVING (see "read first" §2)

✅ **Preferred — rename the volume back to what the binding records.** ⚠️ **This changes the DRIVE, not
the package**, ⛔ **and touches no file the core owns:**

```bash
diskutil rename /Volumes/Scrivi-Worlds "Scrivi Worlds"
ls -d "/Volumes/Scrivi Worlds/Eskandar.scrivworld"   # must now exist
```

⚠️ **ALTERNATIVE — if you would rather not rename the drive**, ✅ **use the app's own re-bind flow**
(`Project ▸ Manage Worlds…` ▸ relocate). ⚠️ **Record which route you took** — ✅ **it changes what T1
is actually proving.**

### S4 — Confirm the picker has rows to draw  ⚠️ **or T0/T1 prove nothing**

✅ **Eskandar holds 6 images with sidecars** (measured 2026-09-16):
`Myton.png` · `Myton-at-23.png` · `myton-portrait.png` · `PET2.png` · `Tintagael.png` · `Lantern_Fox.png`

✅ **Three characters already carry an image, so any of them opens a populated picker:**
**Myton** · **Myton at 23** · **Pet**

⚠️ **6 rows is ENOUGH to see the difference but not dramatic.** ✅ **The per-row cost is what this fix
removes, so the effect scales with row count** — ⚠️ **if T1 reads ambiguous, that is why, and §"If the
result is ambiguous" says what to do about it.**

---

## ✅ TEST

### T0 — Control: the healthy path still works  ⛔ **NOT the real test**

1. Open `/Volumes/Scrivi Worlds/the-stairs-of-tintagael.scrivi`.
2. Open any scene; in the Scene Inspector click through to **Myton** (or **Pet**) to open the
   **Object Detail Sheet**.
3. In the **Image** section press **Choose Existing…**.

✅ **PASS:** the popover lists the world's images with thumbnails; picking one attaches it.
⚠️ **This only proves nothing REGRESSED.** ⛔ **It does not test the fix.**

### T1 — ⚠️ **THE REAL TEST: pull the volume WHILE THE PICKER IS OPEN**

⚠️ **This is the case the fix exists for, and the only one where old and new behave differently.**

⛔ **THE ORDER IS LOAD-BEARING, AND THE FIRST VERSION OF THIS PROCEDURE GOT IT WRONG.**
⚠️ **It said "unplug the drive, THEN press *Choose Existing…*" — ⛔ which is UNTESTABLE.**
✅ **Corrected 2026-09-16 from the user's live pass:** ⚠️ **pulling the drive first makes the Detail
Sheet read-only and DISABLES the *Choose Existing…* button**, ⛔ **so the picker never opens and the
code under test is never reached.** ⚠️ **A "pass" that way proves NOTHING** — ✅ **the test must pull
the volume out from under an ALREADY-OPEN picker.**

1. ✅ **Open Myton (or any object with an image) in the Detail Sheet; confirm the image and buttons
   are live.**
2. ✅ **Press *Choose Existing…* FIRST — the picker must already be showing.**
3. ✅ **NOW physically unplug the USB drive** (or `diskutil unmount …`).
   ⚠️ **Do NOT quit or reopen the app** — ✅ **the point is a resolved path going stale underneath a
   live window, which is exactly the residual race sites 3–4 were ruled safe against.**
4. ✅ **Scroll the picker.**

| | ✅ **EXPECTED (fixed)** | ⛔ **OLD BEHAVIOUR (the defect)** |
| - | --------------------- | ------------------------------- |
| popover | ✅ **opens IMMEDIATELY** | ⛔ **does not appear until every visible row is stat'd AND decoded** |
| rows | ✅ **drawn at once, `photo` icon in each slot** | ⛔ **nothing drawn while blocked** |
| window | ✅ **stays live — menus, scrolling, close all respond** | ⛔ **beachball; Force Quit on a dead mount** |
| thumbnails | ✅ **stay as the `photo` icon** (bytes genuinely gone) | — |

⚠️ **THE PASS CRITERION IS RESPONSIVENESS, NOT PICTURES.** ✅ **No thumbnail can load — the bytes are
not there.** ⛔ **A spinner or a broken-image glyph is a FAIL** (ruling R2 / trade D8-A: both draw the
eye to a non-problem).

⚠️ **While the popover is open, confirm the window is genuinely live** — ✅ **scroll the manuscript,
open a menu.** ⛔ **A popover that appeared but froze the window behind it is still a FAIL.**

### T2 — Reattach: nothing was lost  ✅ **[EP-031] AC23 alongside**

1. ✅ **Replug the drive** (or `diskutil mount`).
2. ✅ **Reopen the picker.**

✅ **PASS:** thumbnails now load and fill in; the object's own image returns; ⛔ **no repair prompt, no
data loss, no writer intervention required.**

⚠️ **If the world does NOT re-resolve after reattach, that is a SEPARATE finding** — ✅ **file it as its
own Issue against EP-031/AC23.** ⛔ **It is not SP-130's, and folding it in here would hide it.**

---

## ✅ TEARDOWN

```bash
# 1. Quit Scrivi cleanly (⌘Q). Confirm it is gone:
pgrep -fl "Scrivi.app" || echo "clean"

# 2. Confirm the manuscript survived — compare against the S1 backup.
diff -rq "$DEST/the-stairs-of-tintagael.scrivi" \
         "/Volumes/Scrivi Worlds/the-stairs-of-tintagael.scrivi"
```

⚠️ **EXPECTED DIFFS ARE NOT AUTOMATICALLY FAILURES** — ✅ **opening a project legitimately writes the
world cache** (`project_open_writes_to_project`, I-0174 closed not-a-defect). ⚠️ **What matters is
that SCENE BODIES are unchanged:**

```bash
diff -rq "$DEST/the-stairs-of-tintagael.scrivi/manuscript" \
         "/Volumes/Scrivi Worlds/the-stairs-of-tintagael.scrivi/manuscript"   # expect NO diffs
```

**3. Volume name** — ⚠️ **if S3 renamed the drive, decide deliberately whether to keep it.**
✅ **KEEPING `Scrivi Worlds` (space) is the RIGHT answer** — ⚠️ **it matches the binding, and renaming
back to the hyphen re-breaks the world.** ⛔ **Only revert if you have a reason.**

**4.** ✅ **Keep the S1 backup until T-0508 is marked Verified.**

---

## ✅ What a PASS licenses, and what it does not

| ✅ **PASS means** | ⛔ **PASS does NOT mean** |
| ---------------- | ----------------------- |
| ✅ **Site 5 no longer blocks the main actor** | ⛔ **[I-0197] is closed** — Classes A/B + the guard remain open |
| ✅ **Ruling R2 holds at the one site that violated it** | ⛔ **the other 4 sites were re-tested** — they did not change |
| ✅ **T-0508 may be marked Verified** ⚠️ **(by the user only)** | ⛔ **SP-130 self-closes** — ✅ **closing needs your approval** |

## ⚠️ If the result is ambiguous

⚠️ **With only 6 rows on a fast USB drive, T0 may show no perceptible difference — that is EXPECTED
and is not evidence either way.** ✅ **T1 is the discriminator**, ⚠️ **because a dead mount turns a
few-ms stat into a multi-second one and the per-row multiplier becomes visible.**

✅ **If T1 is still unclear, the sharper instrument is a SLOW mount rather than an absent one** —
⚠️ **the `cache=none,actimeo=1` network-share setup from [I-0195]** — ✅ **which makes each stat slow
but not infinite, so the blocking version hangs visibly while the fixed version paints immediately.**
⛔ **Do NOT conclude "fixed" from T0 alone.**

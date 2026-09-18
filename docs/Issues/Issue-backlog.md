# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

**Currently: 1** — ⚠️ **[I-0223] OPEN** (filed 2026-09-18, unassigned).

| ID | Title | Severity | Sprint |
| -- | ----- | -------- | ------ |
| **I-0223** | `[ScriviCore]` ⚠️ **A world on a SEPARATE VOLUME cannot be shared between platforms.** ⛔ **Both stored paths encode host-specific layout** — the relative one differs per host once project and world are on different volumes, ⚠️ **and can resolve to a plausible WRONG location rather than failing.** ✅ **`volumeLabel` exists in the schema for exactly this and is never populated.** | **Medium** | Not Assigned |

✅ **[I-0221] and [I-0222] were filed, fixed AND user-verified 2026-09-17/18**, then archived to
[`Verified/Issue-verified-0221-0230.md`](Verified/Issue-verified-0221-0230.md) **in the same step**
(`feedback_archive_on_close`). ⚠️ **Both came from ONE live pass on a FAT32 volume, and neither was
reachable by the test suite as it stood.**

✅ **I-0191 moved to `Issue-active.md` 2026-09-07** — fixed the same day it was filed; ⚠️ **awaiting user verification.**


---

## I-0223: A world on a separate volume cannot be shared between platforms

**Status:** 🔴 Open
**Platform:** `[ScriviCore]` — ⚠️ **the resolution logic is shared, so every platform inherits it**
**Component:** `ScriviCore/src/worlds/WorldStore.cpp:154-156` (bind) and `:280-306` (resolve);
`WorldTypes.hpp:57` (`volumeLabel`)
**Severity:** **Medium** — ⚠️ **but see *Impact*: the affected case is how a writer NORMALLY carries a
shared world between machines**
**Sprint:** Not Assigned
**Date Identified:** 2026-09-18

**Found by:** ⚠️ **the user, from first principles, while reviewing [T-0536]'s cross-platform result** —
✅ **asking whether a relative path could work when the world is on a genuinely different volume.**
⛔ **It cannot.** ✅ **Measured and confirmed the same day.**

**Description:**

A binding stores two ways to find its world (`WorldReference`, `WorldTypes.hpp:55-59`):

| field | what it holds |
| ----- | ------------- |
| `lastKnownPath` | ✅ **RELATIVE to the project root** — tried FIRST (`WorldStore.cpp:280`) |
| `lastKnownAbsolutePath` | the full path — tried second (`:304`) |
| `volumeLabel` | ⛔ **declared, serialized, deserialized — and NEVER POPULATED** |

⚠️ **THE RELATIVE PATH ONLY SURVIVES WHEN PROJECT AND WORLD MOVE TOGETHER.** ✅ **That is the case it
was designed for and it works** — ⚠️ **2026-09-18's two-machine pass confirmed it: one project on a
Linux rig and on macOS, both resolving `../../worlds/Eskandar.scrivworld` from their own roots.**

⛔ **ACROSS A VOLUME BOUNDARY IT DOES NOT.** The relative path must climb out of the project's tree to
the filesystem root and back down another branch, ⚠️ **so it encodes the HOST's directory layout —
exactly what differs between machines.**

**Measured 2026-09-18** for the user's scenario:

| host | project | world | computed `lastKnownPath` |
| ---- | ------- | ----- | ------------------------ |
| macOS | `/Users/justgus/Projects/my-project.scrivi` | `/Volumes/My-JumpDrive/…` | `../../../../Volumes/My-JumpDrive/Eskandar.scrivworld` |
| Linux | `/mnt/Projects/my-project.scrivi` | `/mnt/My-JumpDrive/…` | `../../My-JumpDrive/Eskandar.scrivworld` |

⛔ **Different strings. Neither host can use the other's.**

**Actual Behavior — ⚠️ worse than simply failing:**

⚠️ **When the last writer was Linux, macOS resolves the stored relative path to:**

```
/Users/justgus/My-JumpDrive/Eskandar.scrivworld     ⛔ escapes the project; never existed
```

⛔ **That is not an honest "unavailable" — it is a PLAUSIBLE WRONG LOCATION inside the user's home
directory.** ⚠️ **If anything ever existed there it would bind silently to the wrong world.**
✅ **The `lastKnownAbsolutePath` fallback fails too** — ⚠️ **it encodes a host-specific mount point
(`/Volumes/…` vs `/mnt/…`).**

**Expected Behavior:**

✅ **A world on a removable or separately-mounted volume resolves on any platform that can mount that
volume**, ⚠️ **without the writer re-linking on every machine, every time.**

**Impact:**

- ⚠️ **THIS IS THE NORMAL WAY TO SHARE A WORLD.** ✅ **A jump drive carried between machines is the
  obvious workflow** — ⛔ **and it is precisely the case both stored paths fail.**
- ⚠️ **The user re-links on each machine today** (observed 2026-09-18) — ✅ **which WORKS, because each
  binding then holds that host's own paths** — ⚠️ **but the file is Git-visible shared state, so the
  two machines overwrite each other's reference and the re-link is permanent, not one-time.**
- ⛔ **A relative path escaping the package is a CORRECTNESS risk, not only an inconvenience.**
- ✅ **A `.scrivworld` is explicitly designed to be shared** (`WorldTypes.hpp:5-8`: *"it can be moved,
  copied, or opened by another project with nothing left behind"*) — ⚠️ **this is the gap between that
  intent and what the reference can express.**

**Root Cause Analysis:**

`WorldStore.cpp:154-156` computes the relative path with `fs::relative(packagePath, projectRoot)` —
⚠️ **a purely lexical operation with no notion of a volume boundary.** ✅ **Correct for the
move-together case; ⛔ meaningless across volumes.**

⚠️ **THE SCHEMA ALREADY ANTICIPATED THIS AND NOTHING IMPLEMENTS IT.** `volumeLabel` sits in
`WorldReference` with the comment *"hint only; never authoritative"* — ⛔ **grep confirms it is written
and read by `WorldJson.cpp` and populated by NOTHING.** ✅ **Apple's `WorldVolumeStatus.swift:97` even
documents the consequence:** *"Derived from the PATH rather than from `binding.json`'s `volumeLabel`,
which is **empty on the real rig** — nothing may depend on that field."*

**Is there an easy fix? — ⚠️ THE MECHANISM IS EASY; THE DETECTION IS NOT.**

✅ **The addressing scheme is simple and provably works.** Store the volume label plus the path WITHIN
the volume, and resolve `<mount root>/<label>/<path within>`:

| host | resolves to |
| ---- | ----------- |
| macOS | `/Volumes/My-JumpDrive/Eskandar.scrivworld` |
| Linux | `/mnt/My-JumpDrive/Eskandar.scrivworld` |

✅ **Verified 2026-09-18: the same two stored fields resolve correctly on both hosts, including when
the world sits in a subfolder of the drive.** ✅ **It would be a THIRD candidate in the existing
`resolve()` loop (`WorldStore.cpp:280-306`) — tried FIRST when a label is present** — ⚠️ **so the
existing two candidates stay as fallbacks and nothing regresses.**

⛔ **WHAT IS NOT EASY, and why this is not a one-afternoon change:**

1. ⛔ **THE CORE CANNOT DETECT VOLUMES TODAY.** ⚠️ **`grep` finds no `statfs`, no `getmntent`, no
   mount-table access anywhere in `ScriviCore/src`.** ✅ **Apple's layer can do it**
   (`WorldVolumeStatus.swift` uses `mountedVolumeURLs`) — ⛔ **but that is the PLATFORM layer, and
   Doc 3 §4.4.1 forbids a platform-specific model.** ⚠️ **So either the core gains a
   `FileSystem`-level "what volume is this path on / where is volume `<label>` mounted" capability, or
   the platform supplies it across the ABI. ⛔ THAT IS A DESIGN RULING, not an implementation detail.**
2. ⚠️ **MOUNT ROOTS ARE NOT UNIVERSAL.** ✅ **macOS `/Volumes`; ⚠️ Linux varies —
   `/run/media/<user>/<label>` under udisks2 (MEASURED by T-0477 S3), `/mnt/<whatever>` when
   hand-mounted, ⚠️ and `WorldStore.cpp:382` already records the hand-mounted `/mnt` case as a
   "KNOWN RESIDUAL RISK".** ⛔ **Windows uses drive letters with no label in the path at all.**
3. ⚠️ **LABELS ARE NOT UNIQUE OR STABLE.** ⛔ **Two drives can share a label; a drive can be renamed.**
   ✅ **`volumeLabel`'s own comment already says "never authoritative"** — ⚠️ **so it must stay a HINT
   that is confirmed by reading `world.json`'s `worldID`, exactly as the existing candidates are.**
4. ⚠️ **A volume UUID would be more robust than a label and is LESS portable** — ⛔ **another ruling,
   not a detail.**

✅ **HONEST SUMMARY: a half-day if the core may ask the platform where a labelled volume is mounted;
⛔ a design ruling and a cross-platform `FileSystem` extension if it may not.** ⚠️ **The ruling is the
work, not the code.**

**Proposed Resolution (NOT implemented — needs the ruling above):**

1. ⛔ **RULE FIRST: who resolves a volume label to a mount point** — the core via a new `FileSystem`
   capability, or the platform layer across the ABI?
2. ✅ **Populate `volumeLabel` + a `pathWithinVolume` at bind time** when the world is on a volume
   other than the project's.
3. ✅ **Add it as candidate #1 in `resolve()`**, ⚠️ **keeping both existing candidates as fallbacks.**
4. ⚠️ **Confirm by `worldID` after resolving**, ⛔ **never trust the label alone** (§6.4 is already
   built this way).
5. ⚠️ **Decide what Windows does** before claiming cross-platform.

**Files Affected:**

- `ScriviCore/src/worlds/WorldStore.cpp:154-156` (bind), `:188-189`, `:506-507` (the other two bind sites)
- `ScriviCore/src/worlds/WorldStore.cpp:280-306` (the candidate list)
- `ScriviCore/src/worlds/WorldTypes.hpp:57` (`volumeLabel`, and a new `pathWithinVolume`)
- `ScriviCore/src/schemas/WorldJson.cpp:84,117` (already round-trips `volumeLabel`)
- ⚠️ `ScriviCore/src/platform/LocalFileSystem.cpp` — ⛔ **would need the new capability**

**Related:**

- ⚠️ **[I-0194]** fixed the DISPLAY of these escaping paths (normalizing them so the writer sees where
  the app looked). ⛔ **It did not fix RESOLUTION** — ✅ **this Issue is the other half.**
- ✅ **`WorldStore.cpp:287`** records the measured symptom: *"on the rig that surfaced as a six-`../`
  traversal escaping the project package (measured 2026-09-08 against a dead cifs share)."*
- ⚠️ **Fits [EP-041]** `[Cross]` **The Boundary** — ✅ **same class: project state that does not survive
  crossing between platforms** — ⛔ **but NOT scheduled; that is the user's ruling.**
- ⛔ **OUT OF SCOPE for [T-0536]**, which was `inspector-layout.json` key preservation only.

**Verification:**

⚠️ **NOT VERIFIED — no fix exists.** ⚠️ **Verification needs BOTH machines and one physical volume:**
✅ **bind a world on a jump drive from macOS, move the drive to the Linux rig, open the same project,
and confirm the world resolves WITHOUT re-linking** — ⛔ **then the reverse direction.**

---

*Last Updated: 2026-09-18 — **[I-0221] and [I-0222] user-VERIFIED and archived** to
`Verified/Issue-verified-0221-0230.md`; ⚠️ **the backlog is empty again.** ✅ **The drive-pull test that
produced I-0222 is complete.** Prior note follows.*

*2026-09-17 — **two Issues filed from ONE live pass** on a FAT32 USB volume.
✅ **[I-0221]** filed AND fixed the same day: AppleDouble `._*` sidecars
aborted project open, ⚠️ **and the sandbox's `com.apple.quarantine` stamp REGENERATED them on every
write**, so the failure recurred after any manual clean. ✅ **Fixed at the `listDirectory` chokepoint**
after the audit found ~20 scan callers, not the 3 first identified. ✅ **[I-0222]** filed AND fixed the same day:
pulling the volume reported `ScriviError 1` instead of naming the world. ✅ **`ScriviError` now conforms
to `LocalizedError`** (~15 call sites had been showing Foundation's type-name fallback) ✅ **and the two
`worldPending:`/`worldUnavailable:` spellings merged to ONE derived constant on the user's ruling.**
⚠️ **The cards emptying was ruled ACCEPTED, not a defect** — an away world's objects genuinely are not
available. ⛔ **Its first diagnosis was WRONG and the correction is kept in the record**: measurement
showed neither call in the card's load path fails at all. ✅ **The underlying pending behaviour was
CORRECT throughout: the manuscript stayed usable with the volume gone and everything restored on
reattach.**
⚠️ **Both Issues were invisible to a green suite** — fixtures build on APFS and hand-construct their
`detail` strings. Prior note follows.*

*Last Updated: 2026-09-07 — **I-0191 opened** on the user's report of an unexplained folder in the
repo root. ⚠️ **Three garbage-named, EMPTY app-support trees** (dated 2026-08-17) were deleted; ⚠️ **the
mechanism that creates them was NOT fixed.** ✅ **Root cause found by reading the code and REPRODUCED**:
`bootstrapAppSupport` validates `appSupportRoot` in no way, `AbsolutePath` is a bare `std::string`, and
the C ABI's `S()` turns NULL into `""`. ✅ **Test-suite audit (user-requested) came back CLEAN** — every
fixture cleans up via RAII and roots at an absolute `temp_directory_path()`, so ⚠️ **the suite is not the
source**; its real gap is that it can only see its OWN temp dir (AC4). ⚠️ **The exact call site that made
these three folders is NOT identified — recorded as an open question, not guessed.** Prior note follows.*

*Last Updated: 2026-08-20 Removed  references to I-0118 which is verified and does not belong here.  

2026-08-17, later same day (*\*I-0017 ✅ Verified and archived; I-0018 partly fixed and
RESCOPED\*\* — both on the user's report while reviewing this backlog. I-0017 had been fixed and confirmed long
ago and was never filed. I-0018's original complaint (no selection shown on load) is **fixed**; the remaining
behaviour — **the manuscript not scrolling to that selection** — is different from what was reported, so the
Issue is retitled and rescoped rather than left implying the whole thing is broken. ⚠️ It is flagged to be
scoped **together with I-0131 and I-0132**, which are the same underlying question — \*what does it mean to
"be at" a scene?\* — across load, click, and quit. **Backlog is now 1.** Prior note follows.)\*

\*2026-08-17 (**I-0121 and I-0122 ✅ Verified and archived** to
`Verified/Issue-verified-0121-0130.md` at the SP-106 close — both full entries removed from this file in the
same step, which is the discipline the 2026-08-16 note below says was missed twice. **Backlog is now 2:**
I-0017 and I-0018, both 🔴 Open and unassigned. Prior note follows.)\*

*2026-08-16, later same day (*\*I-0058 and I-0112 archived to `Verified/`; I-0121's code fix
applied.\*\* Both Issues had been Verified — 2026-07-09 and 2026-08-11 — and left sitting in this backlog.
⚠️ **The consistency audit earlier the same day did not catch them because it never opened this file**: it
audited `Issue-active.md`, `Issue-Documentation.md` and the `Verified/` archives, and only *grepped*
`Issue-backlog.md`. **An audit scoped to the files that usually go stale will miss the file nobody looks at**
— which is exactly where a Verified Issue goes to be forgotten. I-0121's `rebalancedKeys` guard is now applied
and proven RED-then-GREEN under UBSan (516/516); its CI and coverage halves remain open. Prior note follows.)\*

\*2026-08-16 (**I-0121 opened — ScriviCore CI has been red on every commit since 2026-07-30.**
`rebalancedKeys(1)` divides by zero (`OrderKey.cpp:179`): the ternary guards `n == 0` — already unreachable —
while the divisor is `n - 1`. ⚠️ **Invisible locally by hardware, not by luck**: arm64 returns 0 for integer
division by zero, x86-64 raises SIGFPE, so the developer Mac and the `macos-latest` runner are green while
`ubuntu-latest` crashes. Introduced by **1c42838**, confirmed by `git log -S`, matching the user's report to
the commit. **Assigned to EP-031, to be scoped at the next sprint's planning** (user ruling) together with
adding `-fsanitize=undefined` to CI, which may change the test configuration. Prior note follows.)\*

\*2026-08-11 (I-0112 opened, then **root cause corrected**. Filed as a suspected live-appearance-switch
staleness bug; the user disproved that within minutes — Dark Mode had been active for hours and the app was
launched minutes before the defect was seen, so no switch was involved. Confirmed cause is static: body-text
attribute dictionaries omit `.foregroundColor` (`:517`, `:296-298`) and `textColor` is never set, so AppKit
renders body runs as literal `NSColor.black` against an adaptive dark background. Manuscript-only, as the sole
AppKit text surface. Sprintless/unassigned. I-0017/I-0018/I-0058 unchanged.)\*

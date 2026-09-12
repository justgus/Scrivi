# Verified Tasks — T-0498 (SP-124) · T-0499–T-0501 (SP-128) — EP-038

⚠️ **New decade file.** The previous file closed at **T-0496**
([`Task-verified-0492-0496.md`](Task-verified-0492-0496.md)).

⚠️ **T-0497 and T-0499 are NOT here** — T-0499 is the next available Task number.

---

## T-0498 — ✅ **`[ScriviCore]` Stop inferring `missing` from directory existence**

**Verified:** 2026-09-10 — **user-approved**
**Codebase:** `[ScriviCore]` — ⚠️ **a CORE fix, not a platform refinement**
**Issue:** [I-0181]

### The defect

⚠️ **`WorldStore::resolve` established `missing` from DIRECTORY EXISTENCE** — *package absent AND parent
exists* — ⚠️ **and an UNMOUNTED VOLUME satisfies both conditions.** ✅ **The package is gone; the
mountpoint directory the operator created is still there.** ⚠️ **So the core reported an INTACT world as
MISSING**, ⚠️ **which invites a writer to relink, recreate, or delete** — ✅ **the [I-0115] class of
defect, where a wrong-but-confident status is what causes the damage.**

⚠️ **This was the THIRD narrowing of the same block:** ⚠️ **T-0419 and T-0420/[I-0136] each tightened it
without questioning what it ASKS.**

### What it delivered

✅ **`FileSystem` gained a device-identity primitive** — `Result<std::uint64_t> deviceID(const
AbsolutePath&)`, POSIX `st_dev`. ✅ **Blast radius was EXACTLY the two implementations the sprint
predicted**, plus one test decorator.

⚠️ **`statvfs` IS RULED OUT and must never be substituted:** ✅ **SP-124's S2 MEASURED it SUCCEEDING on an
unmounted path**, reporting the ROOT filesystem's block counts — ⚠️ **a confident success with a
plausible number, which is worse than a failure because nothing downstream can tell it is wrong.**

⚠️ **Windows returns a FAILURE rather than a stub 0** — ✅ **deliberately**: ⚠️ **a stubbed 0 would make
every path compare EQUAL and silently suppress `missing` everywhere**, ✅ **whereas an error is read as
"I could not tell" and falls back to the honest `unavailable`.**

### ⚠️ **THE POLARITY IS EASY TO GET BACKWARDS — I GOT IT BACKWARDS, AND THE CONTROL TEST CAUGHT IT**

✅ **Recorded because the sprint's own caveat predicted exactly this**, and because the reasoning is the
part worth keeping:

⚠️ **A container with its OWN device has SOMETHING MOUNTED ON IT.** ⚠️ **An absent package there proves
only that the world is not on THIS volume** — ✅ **a different and WEAKER claim than "the world is
gone"** (a different drive, a remount, a stale automount) — ⚠️ **so `missing` is WITHHELD.**
✅ **SAME device ⇒ the container is an ordinary directory on the filesystem we are already reading ⇒
absence there is REAL and `missing` is HONEST.**

### ⚠️ **KNOWN RESIDUAL — recorded, not hidden**

⚠️ **`st_dev` proves "not a mount point NOW". ⚠️ It does NOT prove "a volume went away."**
⚠️ **A pulled drive whose mountpoint SURVIVES is INDISTINGUISHABLE by device identity from an ordinary
directory** — ✅ **both read same-as-parent** — ⚠️ **so that case still reports `missing`.**
⚠️ **NO POLARITY SATISFIES BOTH CASES**; this was confirmed by building the opposite one and watching the
control fail.

✅ **It does NOT arise on the path a real writer uses:** ⚠️ **T-0477 S3 MEASURED udisks2 REMOVING the
mountpoint it created**, ✅ **so `/run/media/<user>/<label>` never reaches this branch — the parent does
not exist and `unavailable` is already returned.**
⚠️ **The HAND-MOUNTED `/mnt` case (fstab entries, server deployments, mounting by hand) REMAINS EXPOSED**
— ✅ **closing it needs evidence from the BINDING, not the filesystem.**

### ✅ Verification

✅ **`ctest` 585/585 on macOS.** ✅ **The new test was VERIFIED FAILING against the unfixed core** —
⚠️ **pre-T-0498 behaviour restored → `missing`; fix restored → `unavailable`** — ✅ **with a CONTROL
proving an ordinary deleted world STILL reports `missing`**, ⚠️ **so T-0498 did not trade one false
status for another.**


---

## T-0499 – T-0501 — ✅ **SP-128: honest waiting** (`[Linux]`)

**Verified:** 2026-09-11 — **user-approved.** **Sprint:** SP-128 · **Epic:** EP-038

| Task | What it delivered |
| ---- | ----------------- |
| **T-0499** | ✅ **`EditorShell::load()` runs OFF the UI thread** via `AsyncCall` — ⚠️ **reusing [I-0193]'s machinery, not a second mechanism.** ⚠️ **`load()` returned `bool` and could no longer; `loadFinished(bool)` replaced it.** ⚠️ **Timeout 10 min, deliberately NOT `kDefaultTimeoutMs` (5 s)** — ✅ **that figure aborts a DEAD share; here it would abort the legitimate slow load the Issue exists to support.** |
| **T-0500** | ✅ **A DETERMINATE progress bar** — `n of m scenes`, revealed after 400 ms. ✅ **The total is a COUNT, not an estimate: `openProject` returns the scene list BEFORE the per-scene reads.** ✅ **New smoke `open_progress_smoke` asserts the total never changes and never goes backwards** — ⚠️ **verified FAILING against a deliberately estimate-style implementation.** |
| **T-0501** | ✅ **LIVE PASS on the rig, build 42** — all three observations made by the user. |

### ⚠️ **THE LIVE PASS FOUND TWO DEFECTS IN THIS SPRINT'S OWN WORK**

⚠️ **And `23/23` smokes were GREEN through BOTH of them.** ✅ **That is the sprint's most useful finding.**

| Defect | ⚠️ Why no suite could see it |
| ------ | --------------------------- |
| **[I-0198]** ⚠️ **`Qt::UniqueConnection` SILENTLY REJECTS lambda connections** — ✅ **Qt printed the warning on every launch** — ⚠️ **so `loadFinished` had NO LISTENER and a recents click only reordered the list** | ⚠️ **The load SUCCEEDED (`onDone ok=1`); nothing was listening.** ⚠️ **Two rounds of reading the code produced two WRONG diagnoses; `qInfo` breadcrumbs found it in ONE run** |
| **[I-0199]** ⚠️ **The progress bar rendered on a HIDDEN PAGE** — the stack switched to the editor only in `loadFinished`, i.e. AFTER the load | ⚠️ **It could NEVER be seen at ANY project size.** ⚠️ **"The widget was shown" is true even when its PAGE is hidden**; ✅ **fix: switch to the editor BEFORE loading** |

### ✅ Verification (measured on the rig, non-root)

✅ **`ctest` 590/590** · ✅ **smokes 23/23** · ✅ **build 42 (2026-09-11 22:35:36 UTC), Qt 6.10.2**, read from the binary ON the rig (`feedback_confirm_the_build_under_test`).

⚠️ **NOT this sprint's, and carried by [EP-039]:** ⚠️ **a ~5 s open with one not-responding dialog on a 71-scene / 50-edge / 2-world project on a share.** ✅ **That is the stateless core's per-item traversal cost, exactly as EP-039's rationale predicts** — ⚠️ **scene count is NOT the only driver.**

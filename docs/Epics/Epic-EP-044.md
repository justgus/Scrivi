---
epic: EP-044
status: Draft
platform: ScriviCore
created: 2026-09-22
---

# EP-044: `[ScriviCore]` ⚠️ **World Resolution** — ✅ **know where a world really is, or say you don't**

**Status:** 🔵 **DRAFT — on the [Epic backlog](Epic-backlog.md). ⛔ Not activated; no Sprint assigned.**
**Codebase:** `[ScriviCore]` — ⚠️ **`WorldStore::resolve` and `FileSystem`.** ⛔ **No app-layer work is
expected on either platform.** ✅ **That is deliberate: every defect here is the CORE answering a
question wrongly, and every platform inherits the answer.**
**Issues:** ⚠️ **[I-0223]** · ⚠️ **[I-0192]** · ⚠️ **[I-0181]'s KNOWN RESIDUAL** (✅ the Issue itself is
Verified — ⛔ its residual was explicitly not fixed)
**Precedent:** ✅ **[T-0498]** (SP-124, EP-038) — ⚠️ **it built the `deviceID` primitive this Epic uses,
and it recorded its own limit.**

---

## ✅ Why this Epic exists

⚠️ **THE CORE ANSWERS "WHERE IS THIS WORLD?" FROM THE FILESYSTEM ALONE, AND THE FILESYSTEM CANNOT
ANSWER IT.** ✅ **Three defects, one shape:**

| Issue | ⚠️ What the core says | ⛔ What is true |
| ----- | -------------------- | -------------- |
| **[I-0192]** | `available` — ✅ **and a full object read SUCCEEDS** | ⚠️ **the volume was PHYSICALLY REMOVED** |
| **[I-0181] residual** | `missing` — ⚠️ **positive proof of deletion** | ⚠️ **a pulled drive whose MOUNTPOINT SURVIVED** |
| **[I-0223]** | `available`, bound — ✅ **and it reports SUCCESS** | ⛔ **it bound the WRONG WORLD, in `$HOME`** |

⚠️ **EVERY ONE OF THESE IS A CONFIDENT WRONG ANSWER, NOT A FAILURE TO ANSWER.** ✅ **That is the
severity argument for the Epic:** ⛔ **"world not found" is honest and recoverable** — ⚠️ **"here is
your world" when it is gone, deleted, or *a different world entirely* is not.**

✅ **THE SHARPEST IS [I-0223].** ⚠️ **`WorldStore.cpp:305-315` tries the RELATIVE path FIRST** —
*"relative path first — it survives moving a project and its worlds together"* — ✅ **which is CORRECT
for the common case.** ⛔ **But a relative path from a project on one volume to a world on another
resolves somewhere inside the user's home directory**, ⚠️ **and if anything plausible sits there the
core binds it and reports success.**

⚠️ **THE ORDERING IS NOT THE BUG.** ✅ **Relative-first protects a real and common case.** ⛔ **The bug
is that a relative candidate is tried when it is MEANINGLESS** — ✅ **and [T-0498]'s `deviceID` already
tells us when that is.**

---

## ⚠️ What [T-0498] already built, and what it could not reach

✅ **[T-0498] (SP-124) added `FileSystem::deviceID`** (POSIX `st_dev`) and made `resolve` require
*package absent* **AND** *container on the same device as its parent* before reporting `missing`.
⚠️ **`statvfs` was RULED OUT on evidence** — ✅ **S2 measured it SUCCEEDING on an unmounted path,
reporting the ROOT filesystem's block counts.**

⛔ **ITS OWN RECORD NAMES THE LIMIT, and this Epic inherits it:**

> ⚠️ *a pulled drive whose mountpoint SURVIVES is INDISTINGUISHABLE from an ordinary directory — both
> read same-as-parent — so that case still reports `missing`.* ✅ *It does not arise on the automounted
> path a real writer uses* (⚠️ **T-0477 S3 MEASURED udisks2 removing the mountpoint it created**),
> ⛔ *but the HAND-MOUNTED `/mnt` case (fstab, server deployments) REMAINS EXPOSED and needs evidence
> from the BINDING, not the filesystem.*

✅ **"EVIDENCE FROM THE BINDING, NOT THE FILESYSTEM" IS THIS EPIC'S THESIS IN SIX WORDS.**

---

## ✅ MEASURED 2026-09-22 — ⚠️ **THE CORRECT CASE WORKS, ON BOTH PLATFORMS**

⚠️ **RECORDED BEFORE ANY WORK STARTS, so that fixing [I-0223] cannot break it.**

✅ **THE USER RAN THE POSITIVE TEST:** `dumas-prose-timelines.scrivi` with its world **adjacent in the
same folder**, copied to the `SCRIVI-OTHE` volume, then opened **on both platforms at DIFFERENT MOUNT
POINTS** — ⚠️ **`/Volumes/SCRIVI-OTHE` on Apple, `/mnt/scrivi-other` on Linux.**
✅ **BOTH FOUND THE WORLD VIA THE RELATIVE PATH.**

✅ **THAT IS THE CASE RELATIVE-FIRST EXISTS FOR, AND IT IS NOW MEASURED RATHER THAN ASSUMED.**
⚠️ **The mechanism is mount-point-independent BY CONSTRUCTION** — `WorldStore.cpp:300-303` builds the
candidate from `projectRoot`, not from any absolute prefix — ✅ **and the test proves it end to end,
across two operating systems and two mount points.**

⛔ **SO THE FIX MUST NOT REVERSE THE ORDERING.** ⚠️ **"Try absolute first" would be the obvious-looking
correction to [I-0223] and it would BREAK THIS** — ✅ **a project and its world moved together is the
COMMON case, and the one a writer on removable media hits every day.**
✅ **AC1 is worded accordingly: skip a relative candidate only when it CANNOT be meaningful (different
device), ⛔ never demote it.**

⚠️ **THIS DOES NOT WEAKEN [I-0223].** ✅ **That defect needs the project and world on DIFFERENT
VOLUMES; ⚠️ this test had them ADJACENT.** ⛔ **The two coexist: one is the behaviour to preserve, the
other the behaviour to fix.**

---

## Acceptance Criteria

- [ ] **AC1** — ⛔ **A relative candidate is NOT TRIED when it cannot be meaningful.** ✅ **When the
      project and the world's recorded location are on DIFFERENT DEVICES, the relative path is
      skipped.** ⚠️ **Closes [I-0223].** ⛔ **The ordering itself is NOT reversed** — ✅ **relative-first
      remains correct for a project and its worlds moved together.**
- [ ] **AC2** — ✅ **A resolved world is VERIFIED AS THE RIGHT WORLD, not merely as a readable package.**
      ⚠️ **`world.json` carries an identity; ⛔ `resolve` currently accepts the first candidate that
      PARSES.** ✅ **A package whose identity does not match the binding is NOT that world.**
      ⚠️ **This is what makes [I-0223]'s wrong-bind impossible rather than unlikely.**
- [ ] **AC3** — ⛔ **A world on a REMOVED volume does not read as `available`, and a read through it
      does not SUCCEED.** ⚠️ **Closes [I-0192]** — ✅ **found on the real rig during a physical yank.**
- [ ] **AC4** — ⚠️ **THE HAND-MOUNTED CASE IS ANSWERED OR EXPLICITLY ACCEPTED, with a measurement.**
      ✅ **[T-0498]'s residual: a surviving mountpoint on `/mnt`.** ⛔ **"Accepted" is a legitimate
      outcome; ⚠️ silence is not** — ✅ **and an acceptance must say what a writer sees when it happens.**
- [ ] **AC5** — ⚠️ **EVERY STATUS IS BACKED BY EVIDENCE THE CORE ACTUALLY HAS.** ✅ **`missing` means
      positive proof of absence; `unavailable` means "cannot see it from here"; ⛔ neither may be
      inferred from a reading of directory existence alone.** ⚠️ **This is the rule [I-0181] established
      and that [I-0192]/[I-0223] show is not yet uniformly applied.**
- [ ] **AC6** — ✅ **Tests go through `scrivi_*`, not the facade** (`feedback_boundary_tests_not_facade`),
      ⚠️ **and each is VERIFIED FAILING against the unfixed core.** ⛔ **[T-0498] caught its own
      polarity error precisely because it had a control test; ✅ that discipline is required here.**
- [ ] **AC7** — ✅ **`ctest` green on macOS AND on LINUX, NON-ROOT with tests ON**
      (`project_linux_container_tests_off`).
- [ ] **AC8** — ⚠️ **A LIVE PASS ON THE REAL RIG WITH A REAL DRIVE.** ⛔ **This Epic cannot close on
      synthetic evidence:** ✅ **every defect in it was found by a physical yank or a first-principles
      question about real hardware**, ⚠️ **and [T-0498]'s residual exists precisely because one case
      could not be reproduced without it.** ⚠️ **Confirm the build first**
      (`feedback_confirm_the_build_under_test`).

---

## ⚠️ Rulings owed BEFORE the first Sprint activates

1. ⚠️ **Q1 — WHAT IDENTIFIES A WORLD?** ✅ **AC2 needs an answer.** ⚠️ **`world.json` holds a worldID;
   ⛔ is a matching ID SUFFICIENT proof, or must the binding also record something the package cannot
   forge by being a copy?** ⚠️ **A duplicated world package on another drive is a REAL scenario — a
   writer's backup — and it has the same ID.**
2. ⚠️ **Q2 — WHAT DOES A WRONG-IDENTITY MATCH REPORT?** ✅ **`unavailable` (we cannot see YOUR world)
   is honest.** ⛔ **But it hides that something IS there** — ⚠️ **and a writer staring at a drive that
   obviously contains a world deserves better than silence.** ✅ **A distinct status may be warranted.**
3. ⚠️ **Q3 — IS `available` ALLOWED TO BE CACHED AT ALL?** ⚠️ **[I-0192] succeeded through a removed
   volume until a scene change forced a re-resolve.** ⛔ **Re-resolving on every access is a cost this
   project has already paid for once** ([EP-042], read amplification) — ✅ **so the ruling is about
   WHEN to invalidate, not whether to cache.**
4. ⚠️ **Q4 — DOES THIS EPIC TOUCH THE APP LAYERS AT ALL?** ✅ **The scope says no.** ⚠️ **But if a new
   status is ruled in Q2, both platforms must render it** — ⛔ **and a status no surface shows is
   `project_capability_without_surface` again**, ✅ **which [EP-041] hit THREE times.**

---

## ⚠️ Known traps — each already paid for

- ⚠️ **THE POLARITY IS EASY TO GET BACKWARDS.** ✅ **[T-0498] got it wrong and its CONTROL TEST caught
  it** — ⚠️ **a container with its OWN device has something mounted on it, so an absent package there
  proves only that the world is not on THIS volume.** ⛔ **Reason it through twice.**
- ⛔ **DO NOT FIX `resolve` FROM A READING OF THE CODE.** ✅ **[I-0181]'s own record says so** —
  ⚠️ **`statvfs` looked correct and was measured succeeding on an unmounted path.** ✅ **MEASURE.**
- ⚠️ **The automounted path hides the hand-mounted one.** ✅ **udisks2 removes the mountpoint it
  created**, ⛔ **so a rig test on a USB stick will NOT exercise AC4.** ⚠️ **That case needs a
  deliberate `/mnt` mount.**
- ⚠️ **`volumeIsRemovable`/`Ejectable` both read FALSE on the real test drive**
  (`project_test_rig_tintagael_eskandar`) — ⛔ **do not build on them.**

---

## ⛔ Explicitly OUT of scope

- ⛔ **World LIFECYCLE** — creation, deletion, sharing, the index. ✅ **That is [EP-033]**, ⚠️ **whose
  first deliverable is a product-boundary decision (in-app view vs. separate application).** ⛔ **This
  Epic must not wait on that**: ✅ **resolution correctness is a defect, not a product question.**
- ⛔ **Any Apple or Linux surface work**, unless Q2 rules a new status into existence.
- ⛔ **[I-0218]** (asset-picker ordering) — ⚠️ **it cites [I-0181] as an ANALOGY only.**
- ⛔ **Read-amplification tuning.** ✅ **[EP-042] closed that; ⚠️ Q3 must not reopen it by accident.**

---

## Scope Notes

⚠️ **THIS EPIC WAS SCOPED FROM A TRACKING GAP.** ✅ **[I-0223] had been referenced in five documents
and present in NONE since 2026-09-18** — ⛔ **recovered at [EP-041]'s close, 2026-09-22.**
⚠️ **[I-0181] was still sitting OPEN in the active file although its Task had been Verified on
2026-09-10 and its Sprint closed on 2026-09-11.** ✅ **Both found by reading, not by any check.**

⚠️ **THE THREE ISSUES WERE FILED SEPARATELY, ACROSS THREE SPRINTS, AND EACH LOOKED SMALL ALONE.**
✅ **Together they say one thing: the core's answer to "where is this world?" is not trustworthy when
volumes are involved** — ⚠️ **and volumes are the normal case for a shared world on external media,
which is the arrangement `project_test_rig_tintagael_eskandar` describes as the real rig.**

---
epic: EP-044
status: Draft
platform: ScriviCore
created: 2026-09-22
---

# EP-044: `[ScriviCore]` ⚠️ **World Resolution** — ✅ **know where a world really is, or say you don't**

**Status:** 🔵 **DRAFT — on the [Epic backlog](Epic-backlog.md). ⛔ Not activated; no Sprint assigned.**
**Codebase:** `[ScriviCore]` — ⚠️ **`WorldStore::resolve`, `FileSystem`, ✅ and (after the 2026-09-25
rewrite) the `scrivi.world.v1` / `scrivi.world-binding.v1` SCHEMAS.** ⚠️ **App-layer work is NOT
expected** — ⛔ **BUT AC2's divergence report and Q2's likely new status must reach a writer, or it is
`project_capability_without_surface` a fourth time.** ✅ **Q4 owes that ruling before the first Sprint.**
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
| **[I-0223]** | `available`, bound — ✅ **and it reports SUCCESS** | ⚠️ **it bound a DIFFERENT VERSION of the right world** |

⚠️ **EVERY ONE OF THESE IS A CONFIDENT WRONG ANSWER, NOT A FAILURE TO ANSWER.** ✅ **That is the
severity argument for the Epic:** ⛔ **"world not found" is honest and recoverable** — ⚠️ **"here is
your world" when it is gone, deleted, or *at a state you never bound* is not.**

⚠️ **[I-0223] IS THE MILDEST OF THE THREE AND WAS REWRITTEN 2026-09-25** (⛔ **its original
wrong-world claim was FALSE — the identity check at `WorldStore.cpp:428` already rejects a different
world**). ✅ **It is now a STALE-VERSION defect and is **Low**; ⚠️ [I-0192] and [I-0181]'s residual are
UNCHANGED and remain this Epic's sharpest cases.**

⛔ **A PREVIOUS VERSION OF THIS SECTION CALLED [I-0223] "THE SHARPEST" AND WAS WRONG.**
⚠️ **It claimed a cross-volume relative path lets the core bind a DIFFERENT world from `$HOME` and
report success.** ✅ **IT CANNOT: `WorldStore.cpp:428-436` checks `worldID` on EVERY candidate,
relative included, and returns `missing` on a mismatch** — ⚠️ ***"A world's name is a label; its
worldID is its identity."*** ✅ **`relinkWorld` enforces the same at `:492-498`.**
⚠️ **THE METHOD FAILURE IS RECORDED IN [I-0223]: a partial code read stopped at the candidate ordering
(`:305-315`) and never reached the check 120 lines below.**

✅ **WHAT [I-0223] ACTUALLY IS, AFTER THE 2026-09-25 REWRITE: `worldID` IS IDENTITY, NOT STATE.**
⚠️ **A COPY or a MIS-MATCHED VERSION of the same world carries the SAME `worldID`, so it binds
SILENTLY** — ✅ **a copy is assumed identical and is FINE (user ruling); ⛔ a divergent version is not,
and the core cannot tell them apart.** ⚠️ **`world.json` already carries `modifiedAt` and
`formatVersion` (`WorldJson.cpp:25-26`), but `WorldBindingRecord` records NEITHER** (`:82-98`) —
⛔ **so the binding has NO RECORD OF WHICH VERSION IT BOUND, and divergence is undetectable BY
CONSTRUCTION, not merely unchecked.**

✅ **THE ORDERING IS CORRECT AND STAYS.** ⚠️ **Relative-first protects a project and its worlds moved
together, which the 2026-09-22 measurement below proves end to end.**

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

⚠️ **RECORDED BEFORE ANY WORK STARTS, so that no fix here can break it.**

✅ **THE USER RAN THE POSITIVE TEST:** `dumas-prose-timelines.scrivi` with its world **adjacent in the
same folder**, copied to the `SCRIVI-OTHE` volume, then opened **on both platforms at DIFFERENT MOUNT
POINTS** — ⚠️ **`/Volumes/SCRIVI-OTHE` on Apple, `/mnt/scrivi-other` on Linux.**
✅ **BOTH FOUND THE WORLD VIA THE RELATIVE PATH.**

✅ **THAT IS THE CASE RELATIVE-FIRST EXISTS FOR, AND IT IS NOW MEASURED RATHER THAN ASSUMED.**
⚠️ **The mechanism is mount-point-independent BY CONSTRUCTION** — `WorldStore.cpp:300-303` builds the
candidate from `projectRoot`, not from any absolute prefix — ✅ **and the test proves it end to end,
across two operating systems and two mount points.**

⛔ **SO NO FIX MAY REVERSE THE ORDERING.** ⚠️ **"Try absolute first" was once the obvious-looking
correction to [I-0223] and it would BREAK THIS** — ✅ **a project and its world moved together is the
COMMON case, and the one a writer on removable media hits every day.**
✅ **AFTER THE 2026-09-25 REWRITE NOTHING IN THIS EPIC TOUCHES THE ORDERING AT ALL** — ⚠️ **[I-0223] is
now a VERSION question, not a PATH question**, ⛔ **so this measurement is no longer at risk from it.**
✅ **It is kept because the ordering is load-bearing and must stay measured.**

---

## Acceptance Criteria

> ⛔ **AC1 AND AC2 WERE REWRITTEN 2026-09-25 (user ruling). ⚠️ THE ORIGINALS ARE VOID AND ARE KEPT AT
> THE FOOT OF THIS SECTION**, ✅ **because why they were void is the useful part.**

- [ ] **AC1** — ✅ **A WORLD PACKAGE CAN BE DISTINGUISHED FROM ANOTHER *VERSION* OF ITSELF.**
      ⚠️ **`worldID` is IDENTITY and answers *"is this the same world?"*; ⛔ nothing answers *"is this
      the same STATE of it?"*.** ✅ **This AC adds that: a monotonic revision counter or a content hash
      written into `world.json`, AND recorded in the binding at bind time.**
      ⛔ **`WorldBindingRecord` currently stores `worldID`, `displayName`, `epochOffsetMs`, `reference`
      and `cachedIndex` — NO version of any kind** (`WorldJson.cpp:82-98`). ⚠️ **Until it does,
      divergence is undetectable BY CONSTRUCTION and AC2 cannot be built.**
      ⚠️ **`modifiedAt` ALREADY EXISTS in `world.json` (`WorldJson.cpp:25`) BUT IS NOT SUFFICIENT ON ITS
      OWN** — ⛔ **a timestamp can go backwards, be preserved by a copy, or be rewritten by sync** —
      ✅ **whether it is a usable INPUT is part of this AC's work, not an assumption it may make.**
- [ ] **AC2** — ✅ **A VERSION MISMATCH IS RECONCILED HONESTLY, NOT RESOLVED SILENTLY.**
      ⚠️ **When the bound version and the found version differ, the core REPORTS the divergence AND ITS
      DIRECTION (older · newer · divergent); ⛔ it NEVER silently picks one.**
      ✅ **This is the standing pair of rules, not a new invention: *"core reports, app decides"*
      ([SP-141] ruling) and §6a.0's *absence is never deletion*.** ⚠️ **Closes [I-0223].**
      ⛔ **A COPY IS NOT A MISMATCH** — ✅ **identical `worldID` AND identical version is the SAME world;
      binding to a copy is CORRECT and must stay silent** (⚠️ **user ruling 2026-09-25:** *"A copy of
      the world is assumed to be identical to the world, which isn't a problem if the project links to
      a copy."*).
- [ ] **AC2b** — ⚠️ **MIGRATION FOR WORLDS THAT PREDATE THE VERSION FIELD.** ✅ **AC1 is a SCHEMA CHANGE
      to `world.json` AND to the binding**, ⛔ **so every world created before it has no version to
      compare.** ⚠️ **What a pre-version world resolves to must be DECIDED, not defaulted** — ✅ **and
      `WorldRecord::kSupportedFormatVersion` (currently `1`) is the existing mechanism for exactly this**
      (⚠️ **its comment: *"RAISE THIS ONLY when this build can actually READ the newer shape."***).
- [ ] **AC3** — ⛔ **A world on a REMOVED volume does not read as `available`, and a read through it
      does not SUCCEED.** ⚠️ **Closes [I-0192]** — ✅ **found on the real rig during a physical yank.**
- [ ] **AC4** — ⚠️ **THE HAND-MOUNTED CASE IS ANSWERED OR EXPLICITLY ACCEPTED, with a measurement.**
      ✅ **[T-0498]'s residual: a surviving mountpoint on `/mnt`.** ⛔ **"Accepted" is a legitimate
      outcome; ⚠️ silence is not** — ✅ **and an acceptance must say what a writer sees when it happens.**
- [ ] **AC5** — ⚠️ **EVERY STATUS IS BACKED BY EVIDENCE THE CORE ACTUALLY HAS.** ✅ **`missing` means
      positive proof of absence; `unavailable` means "cannot see it from here"; ⛔ neither may be
      inferred from a reading of directory existence alone.** ⚠️ **This is the rule [I-0181] established
      and that [I-0192] shows is not yet uniformly applied.** ⚠️ **[I-0223] is a RELATED BUT DISTINCT
      failure: ⛔ not a status asserted without evidence, ✅ but a status the core has NO EVIDENCE TO
      ASSERT FROM until AC1 exists.**
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


### ⛔ THE VOID AC1 AND AC2, kept deliberately

> ⚠️ **ORIGINAL AC1** — *"A relative candidate is NOT TRIED when it cannot be meaningful. When the
> project and the world's recorded location are on DIFFERENT DEVICES, the relative path is skipped."*
> ⛔ **VOID: it addresses a hazard the IDENTITY CHECK ALREADY CLOSES.** ✅ **A relative candidate that
> lands on a different world is rejected on `worldID` at `WorldStore.cpp:428`, wherever it resolves.**
>
> ⚠️ **ORIGINAL AC2** — *"A resolved world is VERIFIED AS THE RIGHT WORLD, not merely as a readable
> package… resolve currently accepts the first candidate that PARSES."*
> ⛔ **VOID: ALREADY IMPLEMENTED, and the premise was false.** ✅ **`resolve` does NOT accept the first
> candidate that parses — `:428-436` compares `worldID` and returns `missing` on a mismatch; the
> comment there states the rule explicitly.**

⚠️ **BOTH CAME FROM ONE PARTIAL CODE READ** (2026-09-22) — ✅ **it stopped at the candidate ordering
(`:305-315`) and never reached the check 120 lines below.** ⛔ **THE LESSON, WHICH THIS EPIC'S OWN
TRAPS SECTION ALREADY STATED: do not specify a fix from a reading of the code.**
---

## ⚠️ Rulings owed BEFORE the first Sprint activates

1. ✅ **Q1 — WHAT IDENTIFIES A WORLD? — PARTLY RULED 2026-09-25.** ⚠️ **This question ANTICIPATED the
   copy case correctly** (*"is a matching ID SUFFICIENT proof, or must the binding also record
   something the package cannot forge by being a copy?"*) — ✅ **and the user has now answered the copy
   half:** ⚠️ ***"A copy of the world is assumed to be identical to the world, which isn't a problem if
   the project links to a copy."*** ⛔ **SO A COPY NEEDS NO DEFENCE — it is the same world at the same
   state, and binding to it is correct.** ⚠️ **WHAT REMAINS OPEN is the VERSION half: WHICH mechanism
   AC1 uses — a monotonic counter, a content hash, or `modifiedAt` qualified by evidence.**
   ⛔ **That choice is a real ruling and must not be defaulted.**
2. ⚠️ **Q2 — WHAT DOES A VERSION MISMATCH REPORT?** (⚠️ **reframed 2026-09-25 — it previously asked
   what a WRONG-IDENTITY match reports, ✅ which `:428` already answers with `missing`.**)
   ⛔ **`unavailable` is WRONG here: the world IS there and IS the right world.** ⚠️ **`available` is
   equally wrong: it is not the state the project bound.** ✅ **A DISTINCT STATUS IS LIKELY REQUIRED**
   — ⚠️ **and AC2 needs it to carry the DIRECTION of divergence (older · newer · divergent).**
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
- ⛔ **AC1 IS A SCHEMA CHANGE, AND THE SCHEMA IS SHARED AND SYNC-CARRIED.** ⚠️ **`world.json`'s own
  comment states the hazard exactly:** ⚠️ ***"Forward compatibility is the one property a shared,
  sync-carried package format cannot retrofit: by the time a newer file exists in the wild, the old
  readers that silently mis-parsed it have already shipped."*** ✅ **So AC1 and AC2b are ONE decision,
  not two, ⛔ and AC1 must not land without AC2b's migration answer.**
- ⚠️ **A VERSION FIELD WRITTEN ON EVERY WORLD WRITE IS A WRITE-AMPLIFICATION RISK.** ✅ **[I-0234] cost
  a per-scene `workspace-state.json` write on bulk load** (`project_bulk_load_write_amplification`) —
  ⛔ **decide WHEN the version advances, and measure it, rather than bumping it on every touch.**

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

# Verified Tasks — T-0498 (SP-124, EP-038)

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

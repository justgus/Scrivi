# Active Sprints

⏸️ **SP-124 is PAUSED** (EP-038, sprint 2 of 2) — ⚠️ **the DRIVE PULL.** ✅ **Paused 2026-08-31 by user
ruling, NOT blocked** — ⚠️ **its S1 baseline is captured and the rig's `cifs` mount is left INTACT**, so
it resumes without redoing setup.

✅ **SP-127 CLOSED 2026-09-02** (user-approved) → [`Closed/Sprint-SP-127.md`](Closed/Sprint-SP-127.md) — **EP-035 AC3** delivered. ⚠️ **Its live pass found FOUR defects, all Verified.**

⚠️ **NO SPRINT IS CURRENTLY ACTIVE.** ⏸️ **SP-124 is PAUSED and resumable** — ✅ **its gate is now satisfied: SP-127 delivered the Worlds surface it was waiting for.**

⚠️ **WHY THE SWAP** (user, 2026-08-31): ✅ ***"the App won't incorrectly represent the mount point until
it can correctly represent the mount point."*** ⚠️ **Linux has NO world surface at all** —
`addWorld`/`relinkWorld`/`getWorldStatus`/`getWorldBinding` are bridged with ⚠️ **ZERO callers**.
⚠️ **So a drive-loss sprint could measure OS signals but could never SHOW them to a writer**, and
⚠️ **I-0181 was re-scoped to a LATENT CORE defect once that was seen.** ✅ **Building the surface first
means SP-124's scenarios get verified against something a writer can actually read.**

✅ **THE SWAP WAS VINDICATED.** ⚠️ **SP-127's live pass found a 🔴 DATA-LOSS defect (I-0183) that only
a real degraded mount could trigger** — ⚠️ **10 of 12 relationships destroyed in the user's real
project** — ⚠️ **and it was found THROUGH the new surface, by a writer using it.** ⚠️ **A drive-loss
sprint run first would have measured the OS correctly and never seen it.**

⚠️ **I-0181 is now worth revisiting**: the Worlds dialog is exactly where a false `missing` would
become visible to a writer, which is the condition the Issue was parked on.

✅ **SP-126 CLOSED 2026-08-30** → [`Closed/Sprint-SP-126.md`](Closed/Sprint-SP-126.md) — the three-tab
Scene Inspector shell, ⚠️ **closing EP-035 AC10** and retiring the "Scene Entities" placeholder.

✅ **SP-125 CLOSED 2026-08-29** → [`Closed/Sprint-SP-125.md`](Closed/Sprint-SP-125.md) — the first
Linux object surface.

✅ **SP-123 CLOSED 2026-08-29** → [`Closed/Sprint-SP-123.md`](Closed/Sprint-SP-123.md) — the rig is
BUILT, GREEN, and runs the app on real hardware.

### ⚠️ What is available to activate next

| Candidate | Epic | State |
| --------- | ---- | ----- |
| **AC3** — world binding + ⚠️ **the RELINK UI that does not exist** | EP-035 | ⚪ Not planned — ⚠️ **a moved world currently cannot be repaired from the app at all** |
| **AC4** — object CRUD | EP-035 | ⚪ Not planned |
| ⚠️ **Session persistence** (I-0176/0177/0178) | ⚠️ **NO EPIC** | ⚠️ **Apple's EP-018 equivalent — wants its own Epic** |
| **I-0180** — `[Apple]` object-card label | ⚠️ **NO HOME** | ⚠️ **EP-034 is closed** |

---
---

## SP-124 — `[Linux]` ⚠️ **Drive-loss ground truth + the Linux refinement**

**Status:** ⏸️ **PAUSED 2026-08-31** — ✅ **GATE NOW SATISFIED: SP-127 delivered the Worlds surface (closed 2026-09-02), so this sprint's scenarios can be verified against something a writer can actually read.** ✅ **S1 baseline captured; the rig's `cifs` mount is INTACT.**
**Epic:** [EP-038](../Epics/Epic-active.md) — `[Linux]` The Real Hardware Rig · **sprint 2 of 2**
**Codebase:** `[Linux]` — ⚠️ **one Qt/C++ deliverable (T-0478); T-0477 ships NO code**
**Date Activated:** 2026-08-31
**Closes:** **AC4, AC5, AC6, AC7**
**Tasks:** **T-0477 – T-0479** (three) · **Next available:** T-0497 · Issues **I-0181**, ⚠️ **I-0182** (found by the SP-127 live pass; see §2c)

⚠️ **GATE SATISFIED:** SP-123 closed 2026-08-29 — the rig is reachable, builds natively, and runs the
app on a real display.

---

## 1. Sprint Goal

⚠️ **Find out what Linux ACTUALLY reports when a volume carrying a world goes away** — then implement
`WorldVolumeStatus` for Linux **against those findings**, and ⚠️ **correct the Porting Outline's §9
from what the rig taught**, so the Windows rig executes a recipe rather than re-deriving one.

⚠️ **The sprint's whole value is the ORDER.** ✅ **Instrument, THEN implement.** A refinement written
from documentation is exactly what this Epic exists to prevent.

---

## 2. ⚠️ SCOPE WIDENED AT ACTIVATION — user ruling 2026-08-31

⚠️ **The plan said "pull a USB drive." The user challenged that framing, correctly, and the scope is
now THREE scenarios rather than one.**

### ⚠️ The user's point, and where it holds

> *"A remote drive is indistinguishable in the Operating system from a USB removeable drive. Both
> require mount/umount protocols. Both will interface to a kernel based `/dev/*` device."*

✅ **TRUE at the mount layer** — and it exposes a real gap in the original plan: ⚠️ **`WorldStatus::offline`
is DEFINED by the network case**, and the sprint as written would have shipped it **untested**. On
Apple, `offline` is what `volumeIsLocal == false` detects. ⚠️ **A USB-only sprint proves `unmounted`
and leaves `offline` a guess.**

### ⚠️ Where it does NOT hold — and why the physical pull SURVIVES

⚠️ **A clean `umount` and a physical yank are different events, and they differ in the one place this
Epic cares about:**

| | Clean `umount` | ⚠️ **Device vanishes under a live mount** |
| - | -------------- | --------------------------------------- |
| Open FDs | closed in order | ⚠️ **`EIO` / `ESTALE`** |
| `/proc/mounts` | entry removed | ⚠️ **may persist as STALE** |
| Writes in flight | flushed | ⚠️ **may be TORN** |
| `umount` itself | succeeds | ⚠️ **may refuse — `EBUSY`** |

⚠️ **A clean unmount cannot produce a stranded FD**, and a stranded FD is the state that cost Apple six
Issues. ✅ **So the physical pull is NOT replaced — it is JOINED.**

### ✅ The three scenarios T-0477 must instrument

| # | Scenario | ⚠️ What it is the ONLY source of |
| - | -------- | -------------------------------- |
| **S1** | **Clean `umount`** of a network share | The orderly baseline — ⚠️ **what "gone" looks like when nothing is wrong** |
| **S2** | ⚠️ **Network share killed AT THE SOURCE** (serving host stops sharing / drops off) | ⚠️ **`offline`'s REAL evidence** — ✅ **and this scenario is the user's contribution; the original plan had no test for it at all** |
| **S3** | ⚠️ **PHYSICAL USB yank** on `oathkeeper` | ⚠️ **Stranded FDs, stale mount entries, torn writes.** ⚠️ **AC4 says "PHYSICAL loss is OBSERVED" and means it** |

⚠️ **S2 is NOT a substitute for S3 and S3 is NOT a substitute for S2.** ⚠️ **Recording one and
inferring the other is the exact failure mode this Epic was created to stop.**

### ⚠️ Two facts about the rig that shape the setup

- ⚠️ **`ScriviWorlds` is an AFP volume.** ⚠️ **`oathkeeper` likely cannot mount it at all** — AFP is
  effectively dead on modern Linux (`afpfs-ng` is unmaintained). ✅ **Do NOT plan around AFP.** Use
  **SMB** (`cifs-utils`) or **NFS** from the Mac or the Windows box; ⚠️ **the protocol is a variable to
  RECORD, not a detail to gloss** — `cifs` and `nfs` fail differently, and `nfs` hard-mounts hang where
  `cifs` returns errors.
- ✅ **The rig SLEEPS, exactly as §1 says.** ⚠️ **`uptime` reading 1d 13h on return is NOT evidence
  against sleep** — ⚠️ **suspend-to-RAM does not stop the kernel clock**, and `uptime` counts
  wall-clock since BOOT regardless of suspend. ⚠️ **An intermediate claim in this session that "it was
  never asleep" was WRONG and has been withdrawn** (user correction, 2026-08-31); ✅ **§1's heuristic
  stands and needs no correction.**
- ⚠️ **The WOL question is OPEN and is the USER's to settle** — `nmcli` reports `wake-on-lan: --`
  (unset) and `ethtool` needs `sudo`. ⚠️ **Magic packets to `255.255.255.255` and `192.168.1.255`
  (ports 9/7/0) drew no response**, ✅ **which is consistent with WOL simply being disarmed.**

---

## 2b. ✅ PRELIMINARY CONTAINER PASS (2026-08-31) — ⚠️ **and what it did NOT do**

⚠️ **While the rig was unreachable, a NARROW question was put to a real Linux kernel** in a privileged
container on a loopback ext4 mount: ✅ **how does Linux report MOUNT STATE through the path API?**

⚠️ **Admissible because that question is about KERNEL SEMANTICS, not hardware.** ⚠️ **It closes NO AC,
and T-0478 remains HARD-GATED on the real session.** → `platforms/linux/tools/T-0477-PRELIMINARY-container.md`

| ✅ Finding | ⚠️ Why it matters |
| --------- | ---------------- |
| ⚠️ **`statvfs` LIES** — ⚠️ **it SUCCEEDS on an unmounted path**, silently reporting the ROOT filesystem (14325 → 15300561 blocks) | ⚠️ **Linux's `volumeIsRemovable`, and WORSE**: Apple's lie was a wrong boolean; this is a CONFIDENT SUCCESS with a plausible number |
| `exists` / `isdir` / `listdir` also lie | ⚠️ **The mountpoint DIRECTORY survives** — empty ≠ absent |
| ✅ **`st_dev` vs the parent's WORKS** — and `/proc/mounts` works | ⚠️ **CAVEAT: a dir that NEVER held a mount also matches.** ⚠️ **`st_dev` proves "not a mount NOW", NOT "a volume went away"** |
| ⚠️ **A held FD survived `umount -l` + `losetup -D` ENTIRELY** — read AND write kept succeeding while the PATH broke instantly | ⚠️ **An open FD and a path can disagree COMPLETELY** — ✅ **so "the writer's editor still works" is NOT evidence the volume is present.** ⚠️ **Do NOT generalise to a yank: that is S3's to measure** |

### ⚠️ **I-0181 — the finding that justifies the whole pass**

⚠️ **An unmounted Linux volume is reported `missing`.** The core's rule is *package absent AND parent
exists* (`WorldStore.cpp:330-348`); ⚠️ **an unmounted Linux volume satisfies BOTH.**

⚠️ **MEASURED ON BOTH PLATFORMS:** macOS **removes** `/Volumes/<name>` (parent absent → honest
`unavailable`); ⚠️ **Linux KEEPS the mountpoint** (parent present → ⚠️ **false `missing`**).

✅ **The same core rule is correct on Apple and WRONG on Linux, and the core cannot see the difference.**
⚠️ **This is the strongest possible argument for the Epic's premise**: ⚠️ **porting Apple's file by
reading it would have shipped this defect.** ⚠️ **`missing` is the one status that invites DESTRUCTIVE
remedies against an intact world** (Doc 2 §7.2.1) — ⚠️ **[[I-0115]]'s class, re-earned via a new trigger.**

⚠️ **NOT FIXED.** ⚠️ **S3 must first establish whether the mountpoint survives a PHYSICAL yank** —
udisks2 may remove it, which changes which branch fires.

---

## 2c. ⚠️ SMB OBSERVATIONS FROM THE SP-127 LIVE PASS (2026-09-01) — ⚠️ **rig findings, NOT S3**

⚠️ **These came out of the SP-127 Worlds-surface live pass, not from SP-124's instrumentation.** ⚠️ **They
are recorded here because SP-124 owns drive-loss ground truth** — ✅ **but they are `cifs` findings and
⚠️ they DO NOT satisfy S3, which is a PHYSICAL yank of a LOCAL device.** ⚠️ **Do not let them stand in
for it.**

**Setup:** the Worlds volume was shared from the Mac over SMB (`scripts/mount-shares-on-rig.sh`) and
pulled ON THE MAC while the rig held it mounted — ⚠️ **a source-side disappearance, which is closest to
S2, not S3.**

### ✅ What the app did — ⚠️ **and a correction**

✅ **The app did NOT hang.** ⚠️ **An intermediate claim in this session that it did was MY MISREADING of
the user's words and is WITHDRAWN** (user correction, 2026-09-01): *"The app did not hang."* ✅ **It
stayed responsive and reported the world unavailable** — ⚠️ **but named the object by its raw
`objectID`, which is now [I-0182].**

### ⚠️ Finding 1 — a `cifs` client can serve a PHANTOM listing of a DIFFERENT share

⚠️ **After the volume was pulled and replugged, `/mnt/scrivi-worlds` presented the directory listing of
the OTHER share (`ScriviLinux`) — byte-identical entries, same sizes, same timestamps** (`.DS_Store`
6148, `.scrivi-fd-probe.tmp` 13 B, and `appsupport/ projects/ worlds/`). ⚠️ **`mountpoint -q` reported
it MOUNTED and `/proc/mounts` named the right share** — ✅ **while the volume's real content
(`Eskandar.scrivworld`) returned `ENOENT` and every phantom entry returned `EBADF`.**

⚠️ **This is WORSE than the `statvfs` lie 2b records.** ⚠️ **`statvfs` returns a plausible number; this
returns a plausible DIRECTORY TREE belonging to a different volume.** ⚠️ **Any check that trusts a
listing — "the world folder is still there" — is defeated outright.**

✅ **Cause: the mount carried `noserverino`**, so the client invents inode numbers instead of using the
server's, and `cache=strict` (the default) kept serving entries after the volume was gone. ✅ **FIXED IN
THE RIG SCRIPT**, not in product code: the Worlds mount now uses **`serverino,cache=none`**
(`scripts/mount-shares-on-rig.sh`, commented with this observation).

⚠️ **This is a RIG DEFECT, not a Scrivi defect** — ⚠️ **but it partly CONTAMINATES the rig as a proxy
for drive loss**: for that window the app was reacting to a FABRICATED listing, not to a missing
volume. ⚠️ **Any SMB-based drive-loss run must confirm the mount options first.**

### ⚠️ Finding 2 — a pulled SMB volume leaves a ZOMBIE mount that blocks remount

⚠️ **`mount` lists the entry; `mountpoint -q` denies it; `fuser` reports `Stale file handle`.** ⚠️ **A
fresh mount over it fails `mount error(16): Device or resource busy`** — ⚠️ **which reads as "something
is using it" and sends you hunting for a process that does not exist.** ✅ **`umount -l` clears it.**
✅ **The rig script now DETECTS this (`is_stale`) and clears it automatically before mounting**, and
`--status` reports **STALE** distinctly from **not mounted**.

⚠️ **`/proc/mounts` persisting is exactly what §2's table predicts for a vanished device** — ✅ **so this
is CONFIRMING evidence for the table**, ⚠️ **but over `cifs`, which is not what AC4 asks for.**

### ✅ What this means for S3

⚠️ **S3 is UNAFFECTED and still owed.** ⚠️ **A local yank has no SMB client cache in the path**, so it
⚠️ **cannot produce a phantom listing from an unrelated share** — ✅ **and the errno set it returns
(`EIO`/`ESTALE` on stranded FDs) is still unmeasured.** ⚠️ **§7 of the rig doc stays EMPTY.**

---

## 3. Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0477** | ⚠️ **DRIVE-LOSS INSTRUMENTATION — FINDINGS ONLY, NO CODE.** ⚠️ **THREE scenarios (S1/S2/S3)**; ⚠️ **the USER pulls, Claude instruments** | **High** | 🟡 **In Progress** — ⚠️ **blocked on the rig being AWAKE** |
| **T-0478** | ⚠️ **`WorldVolumeStatus` for Linux** — `unmounted` / `offline` / `missing`, ⚠️ **written AGAINST T-0477's findings, NEVER from documentation** | **High** | 🔵 **Not started** — ⚠️ **HARD-GATED on T-0477 reporting** |
| **T-0479** | ⚠️ **Correct Porting Outline §9** from what the rig actually taught | **Medium** | 🔵 **Not started** |

⚠️ **T-0478 MUST NOT START BEFORE T-0477 REPORTS.** ⚠️ **The gate is the POINT of the sprint, not
ceremony.** ✅ **If T-0477's findings contradict this plan, the findings win.**

---

## 4. T-0477 — what gets captured, per scenario

⚠️ **Capture BEFORE, DURING and AFTER for every scenario.** ⚠️ **"After" alone cannot show a stale
entry, because a stale entry looks exactly like a live one.**

| Probe | Why |
| ----- | --- |
| `/proc/mounts` + `/proc/self/mountinfo` | ⚠️ **Does the entry vanish, or persist STALE?** |
| `statfs()` / `stat()` on the mount root **and** on the world package | ⚠️ **Which one fails first, and with WHAT errno** |
| ⚠️ **A held-open FD, read AND written across the event** | ⚠️ **`EIO`? `ESTALE`? `ENOTCONN`? Silence?** ✅ **This is the probe a container cannot run** |
| `lsblk` / `/dev/*` presence, `findmnt` | Device-node survival |
| `udevadm monitor` (S3), `dmesg -w` | ⚠️ **Kernel's own account** |
| ⚠️ **What `scrivi_get_world_status` returns** at each phase | ⚠️ **The core's verdict is the INPUT to T-0478** |
| ⚠️ **A write in flight** | ⚠️ **Torn? Silently lost? Error?** — Doc 2's repair path depends on the answer |

⚠️ **Record the SURPRISES loudly.** ⚠️ **Apple's headline finding was that the OBVIOUS API LIED**
(`volumeIsRemovable == false` on a hand-unplugged drive). ⚠️ **Whatever Linux's equivalent lie is,
T-0477's job is to catch it — and a finding of "nothing surprising" must be stated as a RESULT, not
assumed.**

### ⚠️ Ownership

| Phase | Who |
| ----- | --- |
| Wake the rig; provision the drive; ⚠️ **PULL IT** | ⚠️ **THE USER — physical** |
| Serve the network share; ⚠️ **kill it at the source** (S2) | ⚠️ **THE USER** (Mac or Windows box) |
| Everything else — scripts, capture, analysis, the write-up | **Claude, over SSH** |

⚠️ **NEVER drive synthetic input at real writing work** (`feedback_never_drive_synthetic_input_at_real_work`).
✅ **Use a COPY of a world, never the live `ScriviWorlds` content.**

---

## 5. Definition of Done

- [ ] ⚠️ **S1, S2 and S3 each OBSERVED on the real rig** and captured before/during/after
- [ ] ⚠️ **The rig doc's §7 is WRITTEN — from the pull, not from documentation** — including the
      runnable steps to reproduce all three on a fresh rig
- [ ] ⚠️ **Which Linux signal is AUTHORITATIVE for `unmounted` vs `offline` vs `missing` is RECORDED,
      with the signals that LIE named explicitly**
- [ ] `WorldVolumeStatus` exists for Linux and ⚠️ **is verified against the REAL volume**, not a fixture
- [ ] ⚠️ **Every inconclusive branch returns the core's status** — ⚠️ **a wrong `missing` is worse than
      an honest `unavailable`** (Doc 2 §7.2.1; I-0115 was this defect shipped)
- [ ] ⚠️ **Porting Outline §9 CORRECTED** — ⚠️ **a §9 that survives unchanged is evidence it was not tested**
- [ ] `ctest` + Linux smokes GREEN on the rig, ⚠️ **non-root, tests ON**
- [ ] ⚠️ **A LIVE PASS on the rig** — ⚠️ **the writer-facing string is READ, not just returned**
- [ ] ⚠️ **Whether the rig answers WOL is settled and recorded** (NIC vs firmware)

---

## 6. ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **The rig sleeps and ignores WOL** | ⚠️ **Physical trip.** ✅ **Settle NIC-vs-firmware in the same trip so it is the LAST one** |
| ⚠️ **AFP is unmountable on Linux** | ✅ **Use SMB/NFS.** ⚠️ **Record the protocol — they fail DIFFERENTLY** |
| ⚠️ **NFS hard-mount HANGS instead of erroring** | ⚠️ **A hang is a FINDING, not a failed experiment.** ⚠️ **It would freeze the app's UI thread** — ✅ **capture it and consider `soft`/`intr` as a recorded trade** |
| ⚠️ **Findings contradict this plan** | ✅ **The findings win.** ⚠️ **This section is a plan, not a prediction** |
| ⚠️ **T-0478 drifts toward mirroring Apple** | ⚠️ **`/Volumes/<name>` has NO Linux equivalent.** ⚠️ **Mirroring the Apple file is the failure mode; the ENUM is shared, the DETECTION is not** |
| ⚠️ **Real writing work at risk** | ✅ **Work on a COPY.** ⚠️ **Back up first** (`project_test_rig_tintagael_eskandar`) |

---

## 7. ⚠️ Out of scope

| Item | Where |
| ---- | ----- |
| ⚠️ **Any Linux object/UI surface** | **EP-035 / EP-036 / EP-037** |
| ⚠️ **Session persistence** (I-0176/0177/0178) | ⚠️ **Wants its OWN Epic** |
| **Repair-matrix behaviour changes** | ⚠️ **EP-036 AC4** — ✅ **unblocked BY this sprint, not done in it** |
| **The Windows rig** | ⚠️ **A later Epic — EXECUTING §9** |
| **CI automation of the rig** | ⚠️ **Out of scope** |

---
---


*Last Updated: 2026-08-30, second pass (**SP-126 ✅ CLOSED — user-approved**, archived to
[`Closed/Sprint-SP-126.md`](Closed/Sprint-SP-126.md) with its six Tasks verified and archived in the
SAME STEP → `Verified/Task-verified-0485-0490.md`. ✅ **The Audit Check was CLEAN**: 6/6 Tasks Verified,
0 unticked DoD items, nothing stale. ✅ **EP-035 AC10 is CLOSED.** ⚠️ **NO Sprint is now active** — see
the table above for what is available. ⚠️ **The largest unhomed item is session persistence**
(I-0176/0177/0178), Apple's EP-018 equivalent, ⚠️ **which wants its OWN Epic rather than being folded
into an existing one.** Next Task **T-0492**; ⚠️ **next Issue I-0181, opening a new decade.** Prior
note follows.)*

*Last Updated: 2026-08-30 (**SP-126's six Tasks ✅ ALL VERIFIED** — user-approved after a live pass on
the **REAL RIG** at build 8: *"all verified in app. no findings."* ✅ **The three-tab shell is
delivered**: Writing · Worldbuilding · Properties, ⚠️ **and "Scene Entities" — Apple's deleted SP-090
placeholder — is finally retired.** ✅ **571/571 ctest + 19/19 smokes on the rig**, Qt 6.10.2.
⚠️ **THREE defects were found by LOOKING, none by any suite**: the Properties tab unreachable at 200px
(scroll arrows, a gesture-only affordance); outline and todo NEVER SAVED (`QTextEdit` has no
`editingFinished`, and ⚠️ **my own comment claimed a focus-out hook I had not written**); and I-0179,
⚠️ **which took THREE rounds because the first two fixed the symptom** — the real defect was that the
relationship label was REDUNDANT on every row, ✅ **hoisted to the group header by user ruling.**
⚠️ **I-0180 filed against APPLE** — the same wrong label has shipped on macOS since EP-031 unnoticed;
✅ **building the surface a second time is what exposed it.** ⚠️ **SP-126 is NOT CLOSED** — that needs
explicit user approval. Next Task **T-0492**; ⚠️ **next Issue I-0181, opening a new decade.** Prior
note follows.)*

*Last Updated: 2026-08-29, third pass (**SP-123 ✅ CLOSED — user-approved**, archived to
[`Closed/Sprint-SP-123.md`](Closed/Sprint-SP-123.md) with its three Tasks verified and archived in the
SAME STEP → `Verified/Task-verified-0474-0476.md`. ✅ **The Audit Check before the close was CLEAN**:
7/7 DoD, all three Tasks Verified, nothing stale in backlog or unverified. ✅ **EP-038's AC1, AC2 and
AC3 are CLOSED.** ⚠️ **The rig runs the Linux app on REAL HARDWARE** — ⚠️ **and surfaced three Issues
(I-0176 no reopen, I-0177 no geometry, I-0178 single-project) within minutes, none findable by any
suite**; ✅ **user-ruled non-blocking**, and ⚠️ **I-0178 wants its OWN Epic** (Apple's EP-018
equivalent). ✅ **`docs/Scrivi_Linux_Rig_Setup_v0_1.md` written AS RUN** — ⚠️ **§7 (drive dismount)
deliberately EMPTY, owed by SP-124/T-0477.** ⚠️ **ONLY SP-126 is now active.** Next Task **T-0492**;
next Issue **I-0179**. Prior note follows.)*

*Last Updated: 2026-08-29, second pass (**SP-126 🟡 ACTIVATED** — EP-035's second sprint, ⚠️ **running
PARALLEL to SP-123**, which waits on `oathkeeper` being awake. ✅ **SP-126 is blocked on NOTHING** — the
four scene-notes endpoints are already bridged and `inspector-layout.json` needs no endpoint at all.
⚠️ **TWO CORRECTIONS FOUND AT PLANNING:** (1) ⚠️ **the draft's tab order was WRONG** — Apple's display
order is `Writing | Worldbuilding | Properties` (`InspectorTab` declaration order), defaulting to
Writing, ⚠️ **which is DISTINCT from the proving order** Writing → Properties → Worldbuilding;
(2) ✅ **the layout schema ALREADY EXISTS and is already populated** in the user's real projects —
`scrivi.inspector-layout.v1`, ⚠️ **APP-SIDE with no `scrivi_*` endpoint.** ⚠️ **T-0486 carries the
sprint's real risk**: Linux does not model `stackSort` or per-scene stacks, so it must PATCH the
document rather than reconstruct it — ⚠️ **reconstructing would silently delete the writer's
Apple-side card layout, invisibly until she reopened the project on the Mac.** ⚠️ **Accepted risk:
two app-side implementations of one schema; if a THIRD platform needs it, move it into ScriviCore.**
⚠️ **T-0491 (per-stack sort + drag-reorder) filed UNSCHEDULED**, user-ruled out of scope. Next Task
**T-0492**; next Issue **I-0176**. Prior note follows.)*

*Last Updated: 2026-08-29 (**SP-125 ✅ CLOSED — user-approved**, archived to
[`Closed/Sprint-SP-125.md`](Closed/Sprint-SP-125.md) with its five Tasks already verified and archived
2026-08-28. ✅ **The Audit Check before the close was CLEAN**: no Task in two layers, no stale status,
all three of its Issues settled. ⚠️ **AC10 (the three-tab shell) was NOT in SP-125's scope** — it did not
exist when the sprint was planned — ✅ **so the sprint closed on its stated scope rather than being
retroactively widened**; the shell is **SP-126**'s. ✅ **I-0171 VERIFIED and archived** in the same step.
⚠️ **Only SP-123 remains active, and it is BLOCKED ON THE USER.** Next Sprint **SP-126** (⚠️ **planning COMPLETE 2026-08-29**, six Tasks T-0485–T-0490, blocked on
nothing); ⚠️ **T-0491 filed UNSCHEDULED** (per-stack sort + drag-reorder, user-ruled out of SP-126);
next Task **T-0492**; next Issue **I-0176**.)*

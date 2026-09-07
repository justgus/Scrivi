# T-0477 — S3 FINDINGS: the physical USB yank

**Scenario:** S3 — ⚠️ **PHYSICAL yank of a LOCAL USB device**
**Rig:** `oathkeeper` (Ubuntu), real hardware, real display
**Observer:** ⚠️ **THE USER, by hand** — 2026-09-07
**Status:** ✅ **S3 OBSERVED.** ⚠️ **AC4's "physical loss is OBSERVED" is satisfied for the udisks2 path.**

---

## 0. ⚠️ How this was gathered — stated plainly

⚠️ **The instrumentation did NOT produce this finding. The user did.**

⚠️ **`volume-loss-probe.sh` was run on the WRONG MACHINE** — on `Flitwick-5` (the MacBook Pro), pointed
at `/run/media/justgus/SCRIVI-OTHE`, ⚠️ **a Linux path that does not exist on macOS.** It therefore
watched nothing and produced noise, and its output was discarded.

⚠️ **THE CAUSE WAS MY INSTRUCTIONS, NOT THE USER'S EXECUTION.** ⚠️ **The runbook never said WHICH
MACHINE each command belonged to**, and §5.1 told the user to build a binary (`scrivi_world_probe`)
⚠️ **that did not exist in the repository at the time** — ✅ **and to build it on a box that has no dev
environment.** ⚠️ **`s3-baseline-capture.sh` then redirected its own failure into a FILE
(`03-world-status-BEFORE.txt`) instead of stdout**, so the run appeared to proceed while its most
important probe had already failed.

✅ **The finding below is what the user OBSERVED DIRECTLY** — the app on screen, the mountpoint, the
absence of any error. ⚠️ **It is recorded as such and not dressed up as instrumented output.**

⚠️ **Machine discipline, from here on** (this is the fix, and it belongs in every future step):

| Machine | Role |
| ------- | ---- |
| **`oathkeeper`** | ⚠️ **The rig. Ubuntu.** The drive plugs in HERE; the app runs HERE; every `/run/media`, `/mnt`, `lsblk`, `dmesg` command is HERE |
| **`Flitwick-5`** | ⚠️ **The MacBook Pro.** Xcode, the dev environment, the Mac side of an SMB share. ⚠️ **NO Linux paths, ever** |

---

## 1. ✅ WHAT WAS OBSERVED

**Setup:** the drive's FAT32 partition, GNOME-automounted at
`/run/media/justgus/SCRIVI-OTHE` ⚠️ **(a udisks2-managed path)**, carrying a copy of the world.
Scrivi running with the project open.

**Event:** ⚠️ **the drive was PULLED BY HAND.** No eject, no unmount.

| # | ✅ Observation | ⚠️ Significance |
| - | -------------- | --------------- |
| **1** | ⚠️ **NO error. NO warning. Nothing.** The drive *"just unmounted, which is what USB is supposed to do"* | ⚠️ **The predicted `EIO`/`ESTALE` storm DID NOT HAPPEN** |
| **2** | ⚠️ **The ENTIRE mountpoint directory `/run/media/justgus/SCRIVI-OTHE` was REMOVED** | ✅ **udisks2 cleans up the mountpoint it created** |
| **3** | ⚠️ **Scrivi CONTINUED to report the world AVAILABLE**, and a double-click on a world object ⚠️ **still returned the SUCCESS message** | ⚠️ **A GENUINE READ SUCCEEDED against a removed volume** — see §4. ⚠️ **NOT staleness: the app performs a real `openObject` and the core caches no verdict** |
| **4** | ✅ **On a SCENE CHANGE, the world flipped to `unavailable`** — and **Manage Worlds** agreed | ✅ **The status is correct once RE-RESOLVED** |

---

## 2. ⚠️ FINDING 1 — udisks2 REMOVES the mountpoint, so I-0181 IS NOT REACHED on this path

⚠️ **This overturns the sprint's working assumption for the automounted case.**

✅ **§2b measured that an unmounted Linux volume is reported `missing`** because the core's rule is
*package absent **AND** parent directory exists* — ⚠️ **and Linux was believed to KEEP the mountpoint,
unlike macOS.**

⚠️ **On a PHYSICAL YANK of a udisks2-automounted volume, that is NOT what happens.** ✅ **The mountpoint
directory is REMOVED**, so the parent does **not** exist, so `sawContainerButNoPackage` is ⚠️ **never
set** — ✅ **and the core resolves the honest `unavailable`, exactly as macOS does.**

| Path | Mountpoint owner | ⚠️ On physical yank | ⚠️ I-0181's false `missing` |
| ---- | ---------------- | ------------------- | --------------------------- |
| `/run/media/<user>/<label>` | **udisks2** | ✅ **REMOVED — MEASURED 2026-09-07** | ✅ **NOT REACHED** |
| `/mnt/<name>` | the operator | ⚠️ **SURVIVES** — see §3 | ⚠️ **REACHABLE** |

⚠️ **This does NOT retire I-0181 or T-0498.** ✅ **It narrows where they bite** — ⚠️ **and it moves the
automounted case, which is what a real writer's machine uses, OUT of the danger zone.**

---

## 3. ✅ THE `/mnt` HAND-MOUNTED CASE — ⚠️ **RULED NOT WORTH RUNNING** (user, 2026-09-07)

⚠️ **The `/mnt` half of the experiment was CONSIDERED AND DELIBERATELY NOT RUN.**

> ✅ *"Manually mounting the drive on oathkeeper via sudo mount to a /mnt/scrivi-worlds will work and
> pulling the drive will work, silently, and will not remove the mountpoint. I don't think there is any
> purpose to performing that side of the experiment."* — the user

✅ **THE RULING IS CORRECT, AND THE REASONING IS WHY IT IS NOT A GAP:** ⚠️ **a hand-created `/mnt`
directory is an ORDINARY DIRECTORY that the operator owns.** ⚠️ **Nothing in the kernel or in udisks2
has any mandate to delete it** — ✅ **udisks2 removes `/run/media/...` precisely BECAUSE it created it.**
⚠️ **The mountpoint surviving is not a behaviour to be discovered; it is what "I made this directory"
means.**

⚠️ **This is recorded as a REASONED RULING, not as an untested branch** — ✅ **and §2's rule that "a
finding of nothing surprising must be stated as a RESULT" is satisfied by stating it here.**

⚠️ **T-0498 therefore keeps its justification:** ⚠️ **the `/mnt` path is REAL and REACHABLE** (any
`fstab` mount, any server-style deployment, any operator who mounts by hand), ✅ **and on that path the
parent DOES survive and the false `missing` DOES fire.** ⚠️ **The fix is still owed** — ✅ **its trigger
is simply narrower than the sprint assumed.**

---

## 4. ⚠️ FINDING 2 — **A REMOVED VOLUME STILL READ SUCCESSFULLY**

⚠️ **THIS IS THE MOST IMPORTANT RESULT OF S3, AND THE SPRINT WAS NOT LOOKING FOR IT.**

⚠️ **After the drive was gone — with the mountpoint already deleted — Scrivi still reported the world
AVAILABLE, and a double-click on a world object STILL RETURNED SUCCESS.** ⚠️ **Only a SCENE CHANGE
flipped it to `unavailable`.**

### ⚠️ **FIRST DIAGNOSIS WITHDRAWN — the code disproved it (2026-09-07)**

⚠️ **This was first written up as an APP-LAYER defect: a cached status with no invalidation, and a
placeholder dialog reusing a name it already held.** ⚠️ **Both are FALSE**, and the truth is worse:

| ⚠️ Claimed | ✅ What the code actually does |
| ---------- | ----------------------------- |
| The dialog reuses a cached name | ⚠️ **`EditorShell::onOpenObjectRequested` calls `bridge_->openObject(...)`, checks `lastCallFailed()`, and PARSES THE NAME FROM THE RETURNED `objectJson`.** Its own comment: *"the object is genuinely read here"* |
| The app caches world status | ⚠️ **It does not.** ✅ **The read is guarded end-to-end**: `ObjectStore::open` → `findByID` → `kindDirFor`, which calls `WorldStore::resolve()` and ⚠️ **refuses unless `available`** |
| `resolve` served a stale verdict | ⚠️ **It caches NO verdict.** ✅ **`available` is set only after READING AND PARSING `world.json`** (`WorldStore.cpp:337-342`) |

✅ **So there is no staleness anywhere in the chain.** ⚠️ **THE FILESYSTEM ITSELF ANSWERED SUCCESSFULLY
FOR A VOLUME THAT WAS PHYSICALLY GONE**, and every layer above correctly trusted a correct answer.

### ⚠️ The likely mechanism — ⚠️ **and 2b already measured its stronger form**

⚠️ **Page cache / unreaped dentries.** ✅ **SP-124 §2b recorded a held FD surviving `umount -l` +
`losetup -D` ENTIRELY** — reading AND writing fine while the PATH broke instantly — and drew the rule
⚠️ ***"the writer's editor still works" is NOT evidence the volume is present.*** ⚠️ **This is the same
rule, one call up.**

### ⚠️ **This is I-0181's SIBLING, not its opposite**

| | ⚠️ The wrong question | ⚠️ The wrong conclusion |
| - | -------------------- | ----------------------- |
| **I-0181** | *"does the directory exist?"* | ⚠️ **Infers ABSENCE it cannot prove** → false `missing` |
| ⚠️ **I-0192** | *"did the read succeed?"* | ⚠️ **Infers PRESENCE it cannot prove** → false `available` |

✅ **Both are `resolve` reasoning from a filesystem answer that does not mean what it appears to mean.**
⚠️ **T-0498's `st_dev` primitive is PLAUSIBLY the fix for both directions** — ⚠️ **but that must be
MEASURED, not assumed.**

### ✅ **HOW TO SETTLE IT — the probe run**

⚠️ **NOT YET SETTLED. Do not fold this into T-0498 until measured** — ⚠️ **fixing `resolve` from a
reading of the code is precisely what I-0181's history warns against.**

✅ **`scrivi_world_probe` now exists and builds on the rig** (`ScriviCore/tools/scrivi_world_probe.cpp`,
⚠️ **Qt-free**). ⚠️ **It did not exist when S3 ran — which is why S3 could not answer this.**

| Phase | Command (🐧 `oathkeeper`) | ⚠️ What it settles |
| ----- | ------------------------ | ------------------ |
| **BEFORE** | `scrivi_world_probe <project>` | Healthy baseline envelope |
| ⚠️ **IMMEDIATELY AFTER the yank** | ⚠️ **same command, REPEATED every ~2 s** | ⚠️ **Does `resolve` still say `available` — and for HOW LONG?** |
| **AFTER a scene change** | same command | Confirms the flip from the ABI, not the screen |

⚠️ **The decisive row is the second, and it must be SAMPLED, not snapshotted** — ✅ **same reason
`volume-loss-probe.sh` streams.**

| ⚠️ If the probe shows… | ✅ Then |
| ---------------------- | ------- |
| `available` ⚠️ **DECAYING** on its own | ✅ **The core is CORRECT** — page cache was answering; the only defect is that nothing re-asks promptly. ⚠️ **NOT T-0498's** |
| `available` ⚠️ **PERSISTING** | ⚠️ **`resolve` asserts presence it cannot prove** — ✅ **T-0498 fixes BOTH directions**; I-0192 folds into it |

### ✅ **PARTIAL MEASUREMENT TAKEN 2026-09-07 — over SSH, drive already gone, no console needed**

⚠️ **The decay/persist question was NOT fully answered** (that needs sampling ACROSS the yank, and the
drive was already out). ✅ **But the STEADY-STATE half was measured directly with `scrivi_world_probe`,
and it CLEARS the core:**

```
### scrivi_get_world_status — world_character_01a000fb-…
{ "ok": true, "result": {
    "lastKnownPackagePath": "/run/media/justgus/SCRIVI-OTHE/Eskandar.scrivworld",
    "packagePath": "",
    "status": "unavailable" } }
```

| ✅ Measured | ⚠️ What it establishes |
| ----------- | ---------------------- |
| ⚠️ **`status` is `unavailable`** with the drive gone | ✅ **NOT `missing`** — ⚠️ **I-0181's false `missing` is confirmed NOT to fire on the udisks2 path**, now from the ABI rather than from reasoning about §2 |
| ✅ **`packagePath` EMPTY, `lastKnownPackagePath` PRESERVED** | ✅ **Exactly the honest split the design intends** — the path is remembered without being claimed |
| ✅ **`binding.json` stores NO status key** (`cachedIndex`, `displayName`, `epochOffsetMs`, `reference`, `schema`, `worldID`) | ⚠️ **There is NO persisted verdict to go stale** — ✅ **corroborates the code reading** |
| ✅ **Stable across 3 repeated calls** | ⚠️ **Not a transient** |

⚠️ **WHAT THIS DOES NOT SETTLE:** ⚠️ **whether `available` PERSISTS or DECAYS in the seconds immediately
after a yank.** ⚠️ **That is still the deciding measurement**, and it still needs the sampled run above.
✅ **But the steady state is correct**, so ⚠️ **whatever I-0192 turns out to be, it is a TRANSIENT
window, not a stuck verdict** — which materially lowers its severity.

### ⚠️ Still unmeasured — do not guess

- ⚠️ **What the refresh trigger actually is.** *"Change scenes"* is what the user did; ⚠️ **whether it is
  scene selection, any navigator action, or a periodic refresh is UNKNOWN.**
- ⚠️ **Whether an FD was stranded.** ⚠️ **The probe never ran on the rig.**

## 5. ⚠️ What S3 did NOT produce — stated as a RESULT

⚠️ **§4's table anticipated stranded FDs returning `EIO`/`ESTALE`, torn writes, and stale
`/proc/mounts` entries.** ✅ **On the udisks2 path, a clean physical yank produced NONE of them** —
⚠️ **at least none visible without the probe.**

| Anticipated | ⚠️ Observed |
| ----------- | ----------- |
| `EIO` / `ESTALE` on held FDs | ⚠️ **No error surfaced at all.** ⚠️ **UNMEASURED at fd level** |
| ⚠️ **STALE `/proc/mounts` entry** | ⚠️ **Mountpoint REMOVED entirely** — ✅ **no stale entry to find** |
| Torn writes | ⚠️ **Not exercised** — no write was in flight |
| ⚠️ **A kernel complaint** (`dmesg`) | ⚠️ **UNMEASURED** — ⚠️ **the kernel may well have logged the removal even though nothing surfaced to the user** |

⚠️ **The SMB findings in SP-124 §2c — the phantom listing and the zombie mount — have NO counterpart
here.** ✅ **That is consistent with §2c's own caveat**: those were `cifs` client-cache artefacts, and
⚠️ **a local yank has no SMB client in the path.**

---

## 6. ✅ What this hands T-0478

| Question | ✅ Answer from S3 |
| -------- | ----------------- |
| Is the mountpoint reliable evidence? | ⚠️ **NO — it depends ENTIRELY on who mounted it.** ⚠️ **udisks2 removes it; a hand-mount does not.** ⚠️ **`WorldVolumeStatus` MUST NOT assume either** |
| Does Linux need a stale-mount defence like Apple's? | ⚠️ **NOT on the automounted path** — ✅ **there is no stale entry.** ⚠️ **Still owed for `/mnt` and for `cifs` (§2c's zombie mount)** |
| Is `unavailable` reachable honestly today? | ✅ **YES** — ⚠️ **the core resolved it correctly once asked** |
| ⚠️ **Can `resolve` trust a successful READ?** | ⚠️ **UNKNOWN — §4 is NOT SETTLED.** ⚠️ **A read succeeded on a removed volume**; ⚠️ **whether that decays (page cache) or persists (`resolve` asserting presence it cannot prove) decides whether this is T-0498's.** ✅ **Settle it with the `scrivi_world_probe` run in §4** |

---

## 7. ⚠️ Instrumentation debt — ⚠️ **carried, not hidden**

⚠️ **These remain unmeasured and are NOT claimed:**

- [ ] ⚠️ **Held-FD behaviour across a physical yank** (`EIO`? `ESTALE`? ⚠️ **false success, as `fsync`
      gave on macOS in the harness dry run?**)
- [ ] ⚠️ **`dmesg` / `udevadm`'s account** of the removal
- [ ] ⚠️ **Torn-write visibility** after a mid-write yank
- [ ] ⚠️ **`scrivi_get_world_status` envelopes** captured before/after ⚠️ **as TEXT** — ⚠️ **the user
      read the app's SCREEN, which is better evidence of writer impact but worse evidence of what the
      ABI returned**

✅ **`scrivi_world_probe` NOW EXISTS** (`ScriviCore/tools/scrivi_world_probe.cpp`, Qt-free, built by
`-DSCRIVI_BUILD_TOOLS=ON`, default) — ⚠️ **it did not when the runbook told the user to run it.**
⚠️ **It requires a dev environment, which `oathkeeper` does NOT have** — ✅ **so either the rig gets one,
or the binary is built on `Flitwick-5` and COPIED, or this debt stays open.**

⚠️ **NONE of this blocks T-0478.** ✅ **The two questions T-0478 actually needed answered — does the
mountpoint survive, and is the core's verdict honest — are ANSWERED.**

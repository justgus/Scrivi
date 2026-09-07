# T-0477 — S2 FINDINGS: the share killed AT THE SOURCE

**Scenario:** S2 — ⚠️ **an SMB share killed at the SERVING host**, while the rig held it mounted
**Rig:** 🐧 `oathkeeper` (Ubuntu) · **Server:** 🍎 `Flitwick-5` (macOS)
**Run:** 2026-09-07 — ⚠️ **TWO PASSES**, `cache=strict` then `cache=none`
**Status:** ✅ **S2 OBSERVED.** ⚠️ **`offline` was NOT.** — see §2.

---

## 0. Setup — ⚠️ **and the confound the user caught**

⚠️ **The served share (`ScriviLinux`) carries `projects/`, `worlds/` AND `appsupport/`.** ⚠️ **Killing it
would have taken the project and the appSupportRoot away TOO**, and the run would have measured
*"everything vanished at once"* rather than *"a world went offline."*

✅ **Caught by the USER before the run** — *"Will unmounting it also cause the base app to miss the
Scrivi data as well?"* ⚠️ **It is exactly the right question and it was not in the plan.**

✅ **Verified isolation before proceeding:**

| Component | Location | ⚠️ Survives the kill? |
| --------- | -------- | --------------------- |
| **Project** | 🐧 `~/ScriviProjects/the-stairs-of-tintagael.scrivi` — **local disk** | ✅ **Yes** |
| **appSupportRoot** | 🐧 `~/.local/share/Scrivi` — ✅ **local XDG default, `XDG_DATA_HOME` unset** | ✅ **Yes** |
| ⚠️ **World** | `/mnt/scrivi-net/worlds/Eskandar.scrivworld` — **the share** | ⚠️ **NO — the variable under test** |

⚠️ **The share ALSO holds same-named project copies** (`/mnt/scrivi-net/projects/the-stairs-of-tintagael.scrivi`).
⚠️ **Opening the wrong one would have silently invalidated the run.**

✅ **No removable drive was involved** — the world is a copy on the Mac's local disk, ⚠️ **deliberately
keeping real writing work OUT of the blast radius of a test that kills a share mid-use.**

**Baseline (both passes):** `status: "available"`, `packagePath` resolving through the share.
⚠️ **The run does not start until the world reads `available`** — a run starting from `unavailable`
proves nothing.

---

## 1. ⚠️ **FINDING 1 — THE ~10-SECOND BLOCK. This is the headline.**

⚠️ **Every filesystem call against the dead share BLOCKS for ~10 s before returning `EHOSTDOWN`
(errno 112).** ⚠️ **It is NOT a caching artifact — it is IDENTICAL under `cache=strict` and
`cache=none`.**

**Measured by sample spacing.** ⚠️ **Both probe loops `sleep 2`**, so any gap above ~3 s is the call
itself blocking:

| Pass | Gaps while the share was down |
| ---- | ----------------------------- |
| `cache=strict` | ⚠️ **12, 12, 13, 12, 12, 13, 12 s** |
| `cache=none`   | ⚠️ **12, 12, 13, 12 s — UNCHANGED** |

⚠️ **`soft` WAS SET IN BOTH PASSES.** ✅ **`soft` prevents an unkillable hang; it does NOT prevent a
ten-second stall.** ⚠️ **The distinction matters: the sprint's §6 risk table predicted hangs for
NFS HARD mounts and treated `cifs` as the safe case. ⚠️ IT IS NOT.**

⚠️ **The cost COMPOUNDS.** ⚠️ **`scrivi_world_probe` makes several core calls per run**, and its loop
degraded to ⚠️ **43-second intervals** in pass 1 — ⚠️ **four times worse than the single-call layer.**

### ⚠️ What this means for T-0478 — ⚠️ **a DESIGN constraint, not a config note**

⚠️ **THE APP MUST NOT CALL THE CORE SYNCHRONOUSLY ON THE UI THREAD FOR WORLD STATUS.** ⚠️ **A writer
whose share dies would get a ~10 s freeze per call, and a UI that freezes is worse than a status that
is wrong** (§6: *"a hang is a FINDING… it would freeze the app's UI thread"*).

✅ **For contrast, the healthy path is 0.090 s** — measured after recovery. ⚠️ **The degraded path is
~110× slower**, so a timeout is discriminating and cheap.

---

## 2. ⚠️ **FINDING 2 — `offline` NEVER APPEARED. The core reported `unavailable`.**

⚠️ **S2 EXISTS BECAUSE `WorldStatus::offline` IS DEFINED BY THE NETWORK CASE.** ⚠️ **This IS the network
case, and `offline` was never returned — in EITHER pass.**

```
15:38:07   "status": "available"
15:38:09   "status": "unavailable"      ← ~1 s after the kill. NOT "offline".
```

⚠️ **This is a finding about the STATUS MODEL, not a failed run.** ⚠️ **It was flagged as a possible
outcome BEFORE the run, and it happened.**

⚠️ **T-0478 must now decide, and the decision is NOT obvious:**

| Option | ⚠️ Consequence |
| ------ | -------------- |
| ⚠️ **Retire `offline`** | ✅ **Honest** — nothing has ever produced it. ⚠️ **But it is a SHARED enum; Apple may still need it** |
| ⚠️ **Make Linux emit it** | ⚠️ **Requires a signal that distinguishes "server gone" from "path gone"** — ⚠️ **`EHOSTDOWN` is one, and `resolve` does not currently look at errno at all** |
| **Leave it unreachable** | ⚠️ **Ships a documented lie** — ⚠️ **the state EP-038 exists to prevent** |

⚠️ **Do NOT let T-0478 quietly pick option 3 by writing the Linux mapping and never emitting `offline`.**

---

## 3. ⚠️ **FINDING 3 — `cache=none` made the CORE's answer WORSE, not better**

⚠️ **This INVERTS the intuition behind §2c's fix.**

| | `cache=strict` | ⚠️ **`cache=none`** |
| - | -------------- | ------------------- |
| Core verdict after the kill | ✅ **`unavailable` within ~1 s** | ⚠️ **NO STATUS AT ALL** — the probe stopped emitting |
| Loop shell | alive, sampling | ⚠️ **alive** — ⚠️ **but `scrivi_world_probe` blocked in EVERY invocation** |
| Last line written | a complete record | ⚠️ **a TRUNCATED line: `15:54:49 ` with no status** |

⚠️ **Under `cache=strict` the client's cache was the ONLY thing answering fast enough for the core to
return an honest `unavailable` promptly.** ⚠️ **Removing the cache removed that fast path.**

✅ **`cache=none` is still CORRECT for §2c's defect** — it stops the client fabricating a listing of a
different share. ⚠️ **But it is NOT a free improvement**, and ⚠️ **the two settings trade different
failures:**

| | ⚠️ `cache=strict` risks | ⚠️ `cache=none` risks |
| - | ----------------------- | --------------------- |
| | ⚠️ **Phantom/stale listings** (§2c) | ⚠️ **No timely answer at all** |

⚠️ **Neither is safe alone.** ✅ **The app needs its own timeout REGARDLESS of mount options** — which is
Finding 1.

---

## 4. ✅ **FINDING 4 — the phantom listing REPRODUCED, and `mountpoint` LIED**

⚠️ **While the server was gone, under BOTH cache settings:**

```
mountpoint -q /mnt/scrivi-net        → says YES          ⚠️ LIES
ls /mnt/scrivi-net/worlds/           → SUCCEEDS, lists Eskandar.scrivworld, sizes ZEROED
ls .../Eskandar.scrivworld/          → "Host is down (os error 112)"
```

⚠️ **The listing survives one level deep and fails on the next.** ⚠️ **Any check that trusts a directory
listing — *"the world folder is still there"* — is DEFEATED.** ✅ **Confirms §2c on a different trigger:**
⚠️ **§2c's phantom came from a PULLED VOLUME behind the server; this one from the SERVER ITSELF going
away.** ✅ **Same visible failure, different cause** — ⚠️ **so the rule generalises.**

⚠️ **NOTE THE ZEROED SIZES.** ⚠️ **`ls -la` reported `0` for entries that are non-empty directories** —
⚠️ **a detail worth remembering: the listing is not merely stale, it is WRONG in a way a size check
would catch and a name check would not.**

---

## 5. ✅ **FINDING 5 — recovery is AUTOMATIC. No zombie mount.**

⚠️ **§2c saw a ZOMBIE mount after a volume was pulled: `/proc/mounts` kept the entry, `mountpoint -q`
denied it, a remount failed `EBUSY`, and `umount -l` was needed.** ✅ **THAT DID NOT HAPPEN HERE.**

```
15:43:53   "unavailable"
15:44:15   "available"     ← automatic, no intervention, no umount -l
```

✅ **Both passes recovered the same way**, and ✅ **the post-recovery call took 0.090 s.**

⚠️ **The distinction is the SERVER vs the VOLUME:** ⚠️ **§2c pulled the volume BEHIND a live server**
(the mount stayed, its backing store did not); ✅ **S2 removed the SERVER and brought it back**
(the mount reconnected). ⚠️ **Do not generalise either to the other.**

---

## 6. ✅ What S2 hands T-0478

| Question | ✅ Answer |
| -------- | --------- |
| ⚠️ **Is `offline` reachable?** | ⚠️ **NOT ON THIS PATH.** ⚠️ **The network case returns `unavailable`.** ⚠️ **T-0478 must rule on the enum** |
| ⚠️ **Can the core be called on the UI thread?** | ⚠️ **NO.** ⚠️ **~10 s per call while a share is dead, unaffected by mount options** |
| Is `mountpoint` trustworthy? | ⚠️ **NO** — ⚠️ **it reported YES throughout** |
| Is a directory listing trustworthy? | ⚠️ **NO** — ⚠️ **it succeeds one level deep with ZEROED sizes** |
| ⚠️ **Is `errno` a usable signal?** | ✅ **PROMISING, and UNUSED.** ⚠️ **`EHOSTDOWN` (112) is specific to a dead server** — ✅ **exactly what `offline` would need**, ⚠️ **and `resolve` never looks at errno** |
| Does the mount recover on its own? | ✅ **YES** — ⚠️ **for a server that returns.** ⚠️ **NOT proven for a pulled volume (§2c saw the opposite)** |

---

## 7. ⚠️ Data, and what is NOT claimed

✅ **3,110 lines captured**, 🐧 `~/scrivi-probe/S2-strict/` and `~/scrivi-probe/S2-none/`
(`00-baseline.txt`, `01-world-status.txt`, `02-mount-layer.txt` each).

⚠️ **NOT measured, and NOT inferred:**

- ⚠️ **The "network off / black hole" variant.** ⚠️ **Only "File Sharing OFF" (a REFUSED connection) was
  run.** ⚠️ **§4 of the runbook says these are DIFFERENT events and a black hole is where hangs live** —
  ⚠️ **the ~10 s block may be far worse there.**
- ⚠️ **NFS.** ⚠️ **`cifs` only.** ⚠️ **An NFS hard mount is expected to block indefinitely.**
- ⚠️ **A held-open FD across the kill.** ⚠️ **The probes re-open by path each time.**
- ⚠️ **What the APP shows a writer.** ⚠️ **This is an ABI-level run** — ⚠️ **the writer-facing string is
  still owed by the sprint's DoD ("a LIVE PASS… the writer-facing string is READ").**

# T-0477 Runbook — the drive-loss session

**EP-038 / SP-124 · T-0477 · created 2026-08-31**

⚠️ **INSTRUMENTATION ONLY. This session writes NO product code.** Its output is findings, and
⚠️ **T-0478 is HARD-GATED on them.**

⚠️ **Work on a COPY of a world. Never the live `ScriviWorlds` content** —
`feedback_never_drive_synthetic_input_at_real_work`.

---

## ⚠️ 0a. WHICH MACHINE — read this before running ANYTHING

⚠️ **THIS SESSION SPANS TWO COMPUTERS, AND EVERY COMMAND BELONGS TO EXACTLY ONE OF THEM.**

| Tag | Machine | What runs there |
| --- | ------- | --------------- |
| 🐧 **`oathkeeper`** | ⚠️ **The Ubuntu rig** | ⚠️ **The drive plugs in HERE.** The app runs here. Every `lsblk` / `/run/media` / `/mnt` / `dmesg` / `udevadm` / `mount` command. ⚠️ **The probe scripts.** ⚠️ **NO Xcode, NO dev environment** |
| 🍎 **`Flitwick-5`** | ⚠️ **The MacBook Pro** | Xcode and the dev environment; the Mac side of an SMB share (S2); ⚠️ **building binaries that must then be COPIED to the rig.** ⚠️ **NEVER run a Linux path here** |

⚠️ **AN EARLIER VERSION OF THIS RUNBOOK DID NOT SAY.** ⚠️ **On 2026-09-07 that cost a whole S3 attempt:**
`volume-loss-probe.sh` was run on 🍎 **`Flitwick-5`** against `/run/media/justgus/SCRIVI-OTHE` — ⚠️ **a
Linux path that does not exist on macOS** — so it watched nothing, produced noise, and had to be killed
by hand. ⚠️ **§5.1 also told the user to BUILD A BINARY on 🐧 `oathkeeper`, which has no dev
environment**, and ⚠️ **to build one that did not exist in the repo at all.**

✅ **Every command block below is tagged 🐧 or 🍎.** ⚠️ **If a block is untagged, it is a BUG in this
document — fix it before running it.**

---

## 0. Before the trip — what is already known

|                                        |                                                                                                                                                                                                                                                                                 |
| -------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| ⚠️ **WOL appears disarmed**            | Magic packets (broadcast + subnet-directed, ports 9/7/0) drew no response; `nmcli` shows `wake-on-lan: --`. ⚠️ **`ethtool` needs `sudo`, so this is the USER's to settle.** ⚠️ **Do NOT read `uptime` as evidence about sleep** — suspend-to-RAM does not stop the kernel clock |
| ⚠️ **`ScriviWorlds` is AFP**           | ⚠️ **Do not plan to mount it on Linux.** AFP is dead there (`afpfs-ng` unmaintained). ✅ **Use SMB or NFS**                                                                                                                                                                      |
| ✅ **The probe is written and dry-run** | `platforms/linux/tools/volume-loss-probe.sh` — ⚠️ **validated on macOS only; that is NOT a Linux finding**                                                                                                                                                                      |

---

## 0b. ⚠️ What the container pass ALREADY established — and the questions it HANDED to this session

⚠️ **A preliminary container pass ran 2026-08-31** (`T-0477-PRELIMINARY-container.md`). ⚠️ \*\*It answers
KERNEL-SEMANTICS questions only and closes no AC.\*\* ✅ \*\*Its value here is that this session now has
SPECIFIC PREDICTIONS TO FALSIFY rather than open-ended observation.\*\*

⚠️ **Each row below is a question to ANSWER, not a fact to confirm. If the rig disagrees, THE RIG WINS.**

| ⚠️ Prediction from the container                                                     | ⚠️ What THIS session must check                                                                                                                                                |
| ------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `statvfs` **succeeds** on an unmounted path, reporting the ROOT fs                   | ⚠️ **Does it also lie after a PHYSICAL yank**, or does it error there?                                                                                                         |
| The mountpoint dir **survives** a clean `umount`                                     | ⚠️ **THE KEY QUESTION: does `/media/<user>/<label>` survive a YANK?** ⚠️ **udisks2 may remove it** — ⚠️ **and that single fact decides whether I-0181 fires on real hardware** |
| A held FD survived `umount -l` entirely (read+write OK)                              | ⚠️ **A yank should give `EIO`/`ESTALE` — but that is a PREDICTION.** ⚠️ **MEASURE IT**                                                                                         |
| `st_dev` vs parent is the reliable signal                                            | ⚠️ **Does it still hold when the device is GONE rather than cleanly detached?**                                                                                                |
| `fsync` lied on **macOS** (OK on a dead device); never failed in the Linux container | ⚠️ **Watch this cell specifically on the yank**                                                                                                                                |

### ⚠️ I-0181 — filed, NOT fixed

⚠️ **An unmounted volume is reported `missing`** because the core's *package-absent + parent-exists*
rule (`WorldStore.cpp:330-348`) is satisfied by Linux's surviving mountpoint. ⚠️ \*\*macOS is NOT exposed
— it removes `/Volumes/<name>`.\*\*

⚠️ **Whether this fires on the REAL rig depends entirely on the mountpoint question above.**
✅ **Answer that first; the fix is T-0478's.**

---

## 1. First thing, while you are at the machine — settle WOL

⚠️ **Do this FIRST so this is the LAST physical trip**, whatever else happens.

```bash
ip -br link                       # find the wired interface name
sudo ethtool <iface> | grep -i wake
```

- **`Wake-on: g`** → WOL is armed at the NIC; ⚠️ **the block is FIRMWARE** (BIOS/UEFI: "Wake on LAN" /
  "Wake on PCIe", and ⚠️ **ErP/EuP ENABLED disables WOL**).
- **`Wake-on: d`** → ✅ **the NIC is simply disarmed.** Arm it and make it survive reboot:

```bash
sudo ethtool -s <iface> wol g                     # now
# persist it (NetworkManager):
nmcli con show                                    # find the connection name
sudo nmcli con mod "<conn>" 802-3-ethernet.wake-on-lan magic
```

⚠️ **Then TEST it**: suspend the box and have the workstation run `wakeonlan e8:fb:1c:af:82:f0`.
⚠️ **Record the answer either way** — ✅ **"needs a physical trip" is a fact the Windows rig inherits**,
and §9 should say so.

---

## 2. Provision the world copy

```bash
mkdir -p ~/Dev/probe-worlds
cp -a <a .scrivworld package> ~/Dev/probe-worlds/     # ⚠️ A COPY
```

⚠️ **The same copy is used for all three scenarios** so the only variable is the loss mode.

---

## 3. S1 — clean `umount` (the orderly baseline)

⚠️ **Run this FIRST.** ✅ **It establishes what "gone" looks like when NOTHING is wrong** — without it,
S2 and S3's findings have nothing to be surprising against.

**Serving side (Mac):** System Settings ▸ General ▸ Sharing ▸ File Sharing, share a folder holding the
world copy. ⚠️ **Enable SMB** (Options ▸ "Share files and folders using SMB").

**Rig:**
```bash
sudo apt install -y cifs-utils
sudo mkdir -p /mnt/scrivi-net
sudo mount -t cifs //<mac-ip>/<share> /mnt/scrivi-net -o username=<user>,uid=$(id -u),gid=$(id -g)
findmnt -T /mnt/scrivi-net          # confirm BEFORE probing
```

```bash
# terminal 1 — start the probe FIRST
platforms/linux/tools/volume-loss-probe.sh /mnt/scrivi-net S1-clean-umount
# terminal 2 — after ~10s of baseline:
sudo umount /mnt/scrivi-net
# back to terminal 1: wait ~15s, Ctrl-C
```

⚠️ **If `umount` returns `EBUSY`, that is a FINDING** — the probe's own held FD is a legitimate cause,
and it is exactly what an open manuscript would do. ⚠️ \*\*Record it; do NOT reach for `-l` (lazy) to make
it succeed\*\*, which would change the event being measured.

---

## 4. S2 — the share killed AT THE SOURCE ⚠️ **`offline`'s only real evidence**

⚠️ **This is the scenario the original plan did not have**, and ⚠️ **`WorldStatus::offline` is defined by
it.** ⚠️ **Without S2, `offline` ships UNTESTED** — and ⚠️ **EP-038 AC5 requires `WorldVolumeStatus` to
distinguish `unmounted` / `offline` / `missing`.**

✅ **NO DRIVE IS NEEDED.** ✅ **NO CONSOLE ACCESS IS NEEDED.** ⚠️ **Everything below is either on the Mac
or over SSH** — ✅ **which is why S2 is runnable while the USB drive is out and the rig's workstation is
in use by someone else.**

### ✅ 4.0 Already prepared (2026-09-07)

| ✅ Done | Where |
| ------- | ----- |
| ✅ **A copy of `Eskandar.scrivworld` placed in the served share** | 🍎 `~/ScriviLinux/worlds/Eskandar.scrivworld` — ⚠️ **57/57 files, `worldID` VERIFIED to match the binding** (`world_character_01a000fb-…`) |
| ✅ **`ScriviLinux` share already published** | 🍎 `sharing -l` → `/Users/justgus/ScriviLinux` — ⚠️ **on LOCAL disk, so it survives the drive being out** |
| ✅ **`scrivi_world_probe` built on the rig** | 🐧 `~/Dev/Scrivi/build-native/ScriviCore/scrivi_world_probe` |

⚠️ **The `ScriviWorlds` share still points at `/Volumes/Scrivi Worlds` and is DEAD while the drive is
out.** ✅ **S2 does not use it** — the world is served through `ScriviLinux` instead.

### 4.1 🐧 Mount the share on the rig

```bash
# 🍎 Flitwick-5 — drives the rig over SSH
./scripts/mount-shares-on-rig.sh --status     # what is mounted now
./scripts/mount-shares-on-rig.sh              # mount both shares
```

⚠️ **CONFIRM THE MOUNT OPTIONS BEFORE PROBING.** ⚠️ **§2c's phantom-listing finding was caused by
`noserverino` + `cache=strict`** — ✅ **the script now uses `serverino,cache=none`**, ⚠️ **but a run that
does not verify this is measuring the CLIENT CACHE, not the server going away:**

```bash
# 🐧 oathkeeper
findmnt -T /mnt/scrivi-net -o TARGET,SOURCE,FSTYPE,OPTIONS
```

### 4.2 ✅ Point a project at the SERVED world — ⚠️ **and capture the BEFORE**

⚠️ **The world must resolve through the SHARE, not through the rig's local `~/ScriviWorlds` copy** —
⚠️ **otherwise the kill at the source changes nothing and the scenario measures itself.**

```bash
# 🐧 oathkeeper — relink the project's binding to the mounted share
#    (or open the project in the app and use Manage Worlds ▸ Relink)
# Then confirm the core sees it as AVAILABLE:
cd ~/Dev/Scrivi
./build-native/ScriviCore/scrivi_world_probe \
    ~/ScriviProjects/the-stairs-of-tintagael.scrivi
```

⚠️ **DO NOT PROCEED UNTIL `status` READS `available`.** ⚠️ **A run that starts from `unavailable`
proves nothing** — ✅ **the whole point is watching a HEALTHY world go `offline`.**

### 4.3 🐧 Start the probes, then 🍎 kill the share

```bash
# 🐧 oathkeeper — terminal 1: the mount-layer probe
platforms/linux/tools/volume-loss-probe.sh /mnt/scrivi-net S2-killed-at-source

# 🐧 oathkeeper — terminal 2: the CORE's verdict, SAMPLED
cd ~/Dev/Scrivi
while true; do
  printf '%s ' "$(date +%H:%M:%S)"
  ./build-native/ScriviCore/scrivi_world_probe \
      ~/ScriviProjects/the-stairs-of-tintagael.scrivi 2>&1 \
      | grep -m1 '"status"'
  sleep 2
done | tee ~/scrivi-probe/S2-world-status.txt
```

⚠️ **The second loop is NEW and is the half S3 could not run.** ⚠️ **It answers the question S3 left
open — does the core's verdict DECAY or PERSIST?** — ✅ **and it is the direct input to T-0478.**

**Then, 🍎 ON THE SERVING MAC — ⚠️ do NOT unmount on the rig:**

- **Either** System Settings ▸ General ▸ Sharing ▸ **File Sharing OFF**
- **Or** take the Mac off the network (Wi-Fi off / unplug)

⚠️ **Try BOTH if there is time. They are DIFFERENT events**: sharing-off is a **refused connection**,
network-off is a **black hole**, ⚠️ **and a black hole is where hangs live.**

### 4.4 ⚠️ What to watch for

⚠️ **WATCH FOR A HANG.** ⚠️ **A `cifs` mount typically errors; an `nfs` HARD mount BLOCKS FOREVER** —
✅ **and a hang is a FINDING, not a failed experiment**, because it would freeze the app's UI thread.
⚠️ **If a probe stops printing, THAT IS THE RESULT** — note the wall-clock time it stopped and how long
until it recovered, if ever.

| ⚠️ Question | ✅ Why it matters |
| ----------- | ----------------- |
| ⚠️ **Does the core report `offline`, or `unavailable`, or `missing`?** | ⚠️ **`offline` has NEVER been observed.** ⚠️ **If it never appears, that is a finding about the STATUS MODEL, not a failed run** |
| ⚠️ **Does the verdict DECAY or PERSIST?** | ⚠️ **The question S3 left open (I-0192)** — ✅ **over `cifs` this measures the CLIENT cache, which is NOT the local page cache, so record it as such** |
| ⚠️ **Does anything HANG?** | ⚠️ **A frozen UI thread is worse than a wrong status** |
| ⚠️ **Does `/proc/mounts` go stale?** | ✅ **§2c already saw a ZOMBIE mount blocking remount (`EBUSY`, cleared by `umount -l`)** — ⚠️ **confirm or contradict** |

### 4.5 ✅ Afterwards

```bash
# 🍎 Flitwick-5 — turn File Sharing back on, then:
./scripts/mount-shares-on-rig.sh --status
./scripts/mount-shares-on-rig.sh --unmount    # if a zombie mount blocks remount
```

⚠️ **If time allows, repeat S2 over NFS** — the failure modes genuinely differ and T-0478 must know
which it is being handed.

---

## 5. S3 — ✅ **DONE 2026-09-07** — ⚠️ **the PHYSICAL USB yank**

> ✅ **S3 IS COMPLETE. Do NOT re-run these steps to "confirm" it.**
> ⚠️ **Findings:** [`T-0477-FINDINGS-S3.md`](T-0477-FINDINGS-S3.md)

### ✅ The result, in three lines

- ⚠️ **A physical yank of a udisks2-automounted volume is SILENT** — no error, no warning.
- ✅ **The mountpoint `/run/media/<user>/<label>` is REMOVED**, so ⚠️ **I-0181's false `missing` is NOT
  reached on that path.**
- ⚠️ **Scrivi kept reporting the world AVAILABLE — and a double-click still SUCCEEDED — until a scene
  change forced a re-resolve.** ⚠️ **The OS was honest; the APP was stale.** ⚠️ **That is a NEW defect
  class the sprint was not looking for.**

### ⚠️ How it was actually gathered — and the lesson

⚠️ **BY THE USER, BY HAND, WATCHING THE APP.** ⚠️ **The instrumentation contributed NOTHING** — it was
run on the wrong machine because ⚠️ **this document did not say which machine.** ✅ **See §0a, which
exists because of this failure.**

⚠️ **The most valuable finding of the whole scenario — app staleness — came from a human noticing that
a double-click still said SUCCESS.** ⚠️ **No probe in §4's table would have caught it**, because every
one of them questions the OS, and ✅ **the OS was telling the truth the entire time.**

### ✅ The `/mnt` hand-mounted half — ⚠️ **RULED NOT WORTH RUNNING** (user, 2026-09-07)

⚠️ **DELIBERATELY NOT RUN, and this is NOT an untested branch.** ⚠️ **A hand-created `/mnt` directory is
an ordinary directory the operator owns; nothing has any mandate to delete it.** ✅ **udisks2 removes
`/run/media/...` precisely BECAUSE it created it.** ⚠️ **The mountpoint surviving is what "I made this
directory" MEANS — not a behaviour to be discovered.**

⚠️ **T-0498 keeps its justification**: ✅ **the `/mnt` path is real and reachable** (`fstab` mounts,
server deployments, hand-mounts), ⚠️ **and there the parent SURVIVES and the false `missing` DOES fire.**

### ⚠️ Debt carried forward — NOT claimed as measured

⚠️ **Unmeasured**: held-FD errno across the yank · `dmesg`/`udevadm`'s account · torn-write visibility ·
`scrivi_get_world_status` envelopes as TEXT.

✅ **`scrivi_world_probe` NOW EXISTS** (`ScriviCore/tools/scrivi_world_probe.cpp`, Qt-free) — ⚠️ **it did
not when this runbook first told the user to run it.** ⚠️ **It needs a dev environment, which
🐧 `oathkeeper` does not have** — ✅ **build it on 🍎 `Flitwick-5` and COPY it over, or leave the debt open.**

⚠️ **None of this blocks T-0478.**

---

## 6. After each scenario

✅ **The probe prints its output directory.** Collect all three:

```bash
ls ~/scrivi-probe/
tar czf ~/scrivi-probe-findings.tgz ~/scrivi-probe/
```

⚠️ **Claude pulls these over SSH and writes them up into the rig doc's §7** — ⚠️ \*\*which is
DELIBERATELY EMPTY until this session happens.\*\*

---

## 7. ⚠️ What must NOT happen in this session

| ⚠️ Do not                                          | Why                                                                                           |
| -------------------------------------------------- | --------------------------------------------------------------------------------------------- |
| ⚠️ **Write any of `WorldVolumeStatus`**            | ⚠️ **That is T-0478, and the GATE is the sprint's point**                                     |
| ⚠️ **Infer S3 from S2, or S2 from S3**             | ⚠️ **They are different events.** ✅ **A clean unmount cannot strand an FD**                   |
| ⚠️ **Use the live `ScriviWorlds` content**         | ⚠️ **Real writing work.** ✅ **Copies only**                                                   |
| ⚠️ **"Tidy" a surprising result**                  | ⚠️ **The surprise IS the deliverable.** ⚠️ **Apple's headline was that the obvious API lied** |
| ⚠️ **Report "nothing surprising" as a non-result** | ⚠️ **State it as a RESULT** — it is a claim about Linux, and T-0478 depends on it             |

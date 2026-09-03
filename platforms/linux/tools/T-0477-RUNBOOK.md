# T-0477 Runbook — the drive-loss session

**EP-038 / SP-124 · T-0477 · created 2026-08-31**

⚠️ **INSTRUMENTATION ONLY. This session writes NO product code.** Its output is findings, and
⚠️ **T-0478 is HARD-GATED on them.**

⚠️ **Work on a COPY of a world. Never the live `ScriviWorlds` content** —
`feedback_never_drive_synthetic_input_at_real_work`.

---

## 0. Before the trip — what is already known

| | |
| - | - |
| ⚠️ **WOL appears disarmed** | Magic packets (broadcast + subnet-directed, ports 9/7/0) drew no response; `nmcli` shows `wake-on-lan: --`. ⚠️ **`ethtool` needs `sudo`, so this is the USER's to settle.** ⚠️ **Do NOT read `uptime` as evidence about sleep** — suspend-to-RAM does not stop the kernel clock |
| ⚠️ **`ScriviWorlds` is AFP** | ⚠️ **Do not plan to mount it on Linux.** AFP is dead there (`afpfs-ng` unmaintained). ✅ **Use SMB or NFS** |
| ✅ **The probe is written and dry-run** | `platforms/linux/tools/volume-loss-probe.sh` — ⚠️ **validated on macOS only; that is NOT a Linux finding** |

---

## 0b. ⚠️ What the container pass ALREADY established — and the questions it HANDED to this session

⚠️ **A preliminary container pass ran 2026-08-31** (`T-0477-PRELIMINARY-container.md`). ⚠️ **It answers
KERNEL-SEMANTICS questions only and closes no AC.** ✅ **Its value here is that this session now has
SPECIFIC PREDICTIONS TO FALSIFY rather than open-ended observation.**

⚠️ **Each row below is a question to ANSWER, not a fact to confirm. If the rig disagrees, THE RIG WINS.**

| ⚠️ Prediction from the container | ⚠️ What THIS session must check |
| ------------------------------- | ------------------------------ |
| `statvfs` **succeeds** on an unmounted path, reporting the ROOT fs | ⚠️ **Does it also lie after a PHYSICAL yank**, or does it error there? |
| The mountpoint dir **survives** a clean `umount` | ⚠️ **THE KEY QUESTION: does `/media/<user>/<label>` survive a YANK?** ⚠️ **udisks2 may remove it** — ⚠️ **and that single fact decides whether I-0181 fires on real hardware** |
| A held FD survived `umount -l` entirely (read+write OK) | ⚠️ **A yank should give `EIO`/`ESTALE` — but that is a PREDICTION.** ⚠️ **MEASURE IT** |
| `st_dev` vs parent is the reliable signal | ⚠️ **Does it still hold when the device is GONE rather than cleanly detached?** |
| `fsync` lied on **macOS** (OK on a dead device); never failed in the Linux container | ⚠️ **Watch this cell specifically on the yank** |

### ⚠️ I-0181 — filed, NOT fixed

⚠️ **An unmounted volume is reported `missing`** because the core's *package-absent + parent-exists*
rule (`WorldStore.cpp:330-348`) is satisfied by Linux's surviving mountpoint. ⚠️ **macOS is NOT exposed
— it removes `/Volumes/<name>`.**

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
and it is exactly what an open manuscript would do. ⚠️ **Record it; do NOT reach for `-l` (lazy) to make
it succeed**, which would change the event being measured.

---

## 4. S2 — the share killed AT THE SOURCE ⚠️ **`offline`'s only real evidence**

⚠️ **This is the scenario the original plan did not have**, and ⚠️ **`WorldStatus::offline` is defined
by it.** Without S2, `offline` ships untested.

Re-mount as in §3, then **on the SERVING Mac** — ⚠️ **do NOT unmount on the rig**:

- **Either** turn File Sharing OFF, **or** take the Mac off the network (Wi-Fi off / unplug).
- ⚠️ **Try BOTH if there is time. They are different events**: sharing-off is a refused connection,
  network-off is a black hole, ⚠️ **and a black hole is where hangs live.**

```bash
platforms/linux/tools/volume-loss-probe.sh /mnt/scrivi-net S2-killed-at-source
```

⚠️ **WATCH FOR A HANG.** ⚠️ **A `cifs` mount typically errors; an `nfs` HARD mount BLOCKS FOREVER** —
✅ **and a hang is a FINDING, not a failed experiment**, because it would freeze the app's UI thread.
⚠️ **If the probe stops printing, that IS the result** — note the wall-clock time it stopped and how
long until it recovered (if ever).

⚠️ **If time allows, repeat S2 over NFS** — the failure modes genuinely differ and T-0478 must know
which it is being handed.

---

## 5. S3 — ⚠️ **the PHYSICAL USB yank**

⚠️ **The one AC4 names explicitly, and the only source of stranded-FD behaviour.**

```bash
# plug the USB drive in; note where it lands
lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,TRAN
cp -a ~/Dev/probe-worlds/<world>.scrivworld /media/<user>/<label>/
```

```bash
platforms/linux/tools/volume-loss-probe.sh /media/<user>/<label> S3-physical-yank
```

⚠️ **Then PULL THE DRIVE OUT BY HAND.** ⚠️ **Do NOT eject it first** — ✅ **a clean eject is S1, and
it is already covered.** Wait ~15s, Ctrl-C.

### ⚠️ The two questions S3 exists to answer

1. ⚠️ **Does `/proc/mounts` keep a STALE entry?** — ✅ **Apple's `isMounted()` exists precisely because a
   stale `/Volumes/<name>` outlives an unclean unmount and reads as "mounted."**
2. ⚠️ **What do the held FDs return** — `EIO`? `ESTALE`? ⚠️ **Or a false success**, as `fsync` gave on
   macOS in the dry run?

### ⚠️ Optional but valuable — the torn write

⚠️ **Only if the first three went smoothly.** Start a large write to the drive, then yank mid-write:

```bash
dd if=/dev/urandom of=/media/<user>/<label>/torn-test.bin bs=1M count=500 &
# yank at roughly 50%
```
⚠️ **Re-mount afterwards and check what survived** — ✅ **Doc 2's repair path depends on whether
partial writes are visible, and this is the only way to know.**

---

## 6. After each scenario

✅ **The probe prints its output directory.** Collect all three:

```bash
ls ~/scrivi-probe/
tar czf ~/scrivi-probe-findings.tgz ~/scrivi-probe/
```

⚠️ **Claude pulls these over SSH and writes them up into the rig doc's §7** — ⚠️ **which is
DELIBERATELY EMPTY until this session happens.**

---

## 7. ⚠️ What must NOT happen in this session

| ⚠️ Do not | Why |
| --------- | --- |
| ⚠️ **Write any of `WorldVolumeStatus`** | ⚠️ **That is T-0478, and the GATE is the sprint's point** |
| ⚠️ **Infer S3 from S2, or S2 from S3** | ⚠️ **They are different events.** ✅ **A clean unmount cannot strand an FD** |
| ⚠️ **Use the live `ScriviWorlds` content** | ⚠️ **Real writing work.** ✅ **Copies only** |
| ⚠️ **"Tidy" a surprising result** | ⚠️ **The surprise IS the deliverable.** ⚠️ **Apple's headline was that the obvious API lied** |
| ⚠️ **Report "nothing surprising" as a non-result** | ⚠️ **State it as a RESULT** — it is a claim about Linux, and T-0478 depends on it |

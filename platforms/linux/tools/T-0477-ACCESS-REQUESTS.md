# T-0477 — ⚠️ **Tasks requiring privileged or physical access (USER-OWNED)**

**EP-038 / SP-124 · 2026-08-31**

⚠️ **Everything Claude can do without privileged access is DONE.** ⚠️ **The items below are blocked on
access Claude does not have and should not be given.** ✅ **Each is self-contained and independently
runnable.**

---

## A. ⚠️ On the MacBook Pro — share `~/ScriviLinux` over SMB (⚠️ **admin, outward-facing**)

✅ **USER RULING 2026-08-31: use the EXISTING `~/ScriviLinux`, not a new folder.** ⚠️ **Claude's
`~/ScriviProbeShare` was redundant and has been DELETED.** ✅ **Everything under `~/ScriviLinux` is
test material; the canonical backups are elsewhere.**

### ✅ Why this fixture is BETTER than the one Claude built

`~/ScriviLinux` (6.1 MB) holds **14 real test projects** plus an `appsupport/` tree — ⚠️ **and two of
them are ALREADY BOUND to a world on a REMOVABLE VOLUME:**

```
the-stairs-of-tintagael.scrivi/worlds/world_character_01a000fb-…/binding.json
the-lone-golem.scrivi/worlds/world_character_01a000fb-…/binding.json

  worldID  : world_character_01a000fb-539a-7402-802e-0d97eeb1e594   ("Eskandar")
  reference: /Volumes/Scrivi Worlds/Eskandar.scrivworld/
  cached   : 35 objects
```

⚠️ **This is EXACTLY the shape the scenarios need** — ✅ **a real binding to a world on a removable
volume, with a populated `cachedIndex`, so the unavailable-world path is genuinely exercised rather
than simulated.** ⚠️ **Claude's hand-assembled copy had no binding at all and would have tested less.**

### The steps

1. **System Settings ▸ General ▸ Sharing ▸ File Sharing → ON**
2. **Shared Folders → `+` → add `~/ScriviLinux`**
3. **Options… → ⚠️ tick "Share files and folders using SMB"** ⚠️ **and tick the box next to your
   account** under "Windows File Sharing" (⚠️ **REQUIRED — an unticked account cannot authenticate**)
4. ⚠️ **Note this Mac's LAN IP**: `ipconfig getifaddr en0` → currently **192.168.1.196**
   ⚠️ **DHCP; re-check at session time.**

**Verify from the rig** (Claude runs this once sharing is on):
```bash
smbclient -L //192.168.1.196 -U <your-mac-username>
```

⚠️ **TURN FILE SHARING OFF when the sprint closes.** ✅ **It is a test fixture, not a standing service.**

---


### ⚠️ A NOTE ON THE BINDING — ⚠️ **no action, and NOTHING is edited on the rig**

⚠️ **The Eskandar binding records a macOS path** (`/Volumes/Scrivi Worlds/Eskandar.scrivworld/`), and
⚠️ **both candidates resolve there on Linux too**, so ⚠️ **the world will report unavailable for a PATH
reason rather than a VOLUME reason.**

⚠️ **An earlier draft of this file proposed RELINKING on the rig to "fix" that. ✅ THAT IS WITHDRAWN
(user ruling, 2026-08-31).** ⚠️ **Relinking is a FEATURE the app owes** — ✅ **EP-035 AC3's relink UI,
which does not exist yet** — ⚠️ **and hand-editing a binding on the rig would be doing behind the app's
back exactly what the app is supposed to do in front of it.** ⚠️ **It would also make the rig's state
untrustworthy for every later pass.**

✅ **What this means for the scenarios, plainly:**

| | |
| - | - |
| **S1** — clean `umount` | ✅ **UNAFFECTED.** ⚠️ **It measures MOUNT-STATE DETECTION** (`/proc/mounts`, `st_dev`, held FDs) — ✅ **it needs a mounted volume that goes away, not a resolvable world** |
| **S3** — the physical yank | ✅ **UNAFFECTED**, for the same reason |
| **S2** — share killed at source | ⚠️ **Measures the same OS-level signals.** ⚠️ **What it CANNOT do until a relink surface exists is show the app transitioning `available → offline`** |

⚠️ **So T-0477's OS-level findings — which are what T-0478 is written against — are fully obtainable
without touching a single binding.** ⚠️ **The end-to-end app-level demonstration waits on EP-035 AC3**,
and ✅ **that is a sequencing fact to record, not a problem to work around.**

---

## B. ⚠️ On `oathkeeper` — package installation (⚠️ **needs `sudo`**)

⚠️ **Neither `cifs-utils` nor `nfs-common` is installed.** ⚠️ **Both mount helpers are privileged;
there is no unprivileged substitute that answers the same question** — ✅ **`gio`/gvfs would give a
FUSE mount, which fails DIFFERENTLY from a kernel mount and would mislead T-0478.**

```bash
sudo apt update
sudo apt install -y cifs-utils nfs-common
```

⚠️ **`nfs-common` is worth installing even though S1/S2 will start with `cifs`** — ⚠️ **NFS hard-mounts
HANG where `cifs` returns errors**, and ✅ **that difference is itself a finding T-0478 needs.**

### B2. Create the mountpoint and grant the mount

⚠️ **`mount` is privileged and Claude cannot run it.** ✅ **Two options — please pick one:**

**Option 1 — you run the mounts** (⚠️ **simplest; needs you present for each scenario**):
```bash
sudo mkdir -p /mnt/scrivi-net
sudo mount -t cifs //192.168.1.196/ScriviLinux /mnt/scrivi-net \
     -o username=<mac-user>,uid=$(id -u),gid=$(id -g),vers=3.0
```

**Option 2 — ✅ RECOMMENDED: an `fstab` entry with `user`**, which lets a NON-ROOT account mount and
unmount it. ⚠️ **This is what makes S1 runnable by Claude over SSH without a password.**
```bash
sudo mkdir -p /mnt/scrivi-net
echo '//192.168.1.196/ScriviLinux /mnt/scrivi-net cifs noauto,user,username=<mac-user>,password=<pw>,uid=1001,gid=1001,vers=3.0 0 0' | sudo tee -a /etc/fstab
sudo chmod 600 /etc/fstab
```
⚠️ **A password in `/etc/fstab` is why `chmod 600` is not optional.** ✅ **Better: use a credentials
file** — `credentials=/root/.smbcred` with `chmod 600`, ⚠️ **and note that neither belongs in the repo.**

⚠️ **With Option 2, `mount /mnt/scrivi-net` and `umount /mnt/scrivi-net` work as an ordinary user**, so
✅ **S1 becomes fully automatable and Claude can run it unattended.**

---

## C. ⚠️ Settle wake-on-LAN (⚠️ **needs `sudo`; ~2 minutes**)

⚠️ **Still genuinely open.** ⚠️ **`nmcli` reports `wake-on-lan: --` (unset) and magic packets to
`255.255.255.255` / `192.168.1.255` on ports 9/7/0 drew no response.** ✅ **The wired interface is
`eno1`.**

```bash
sudo ethtool eno1 | grep -i wake
```
- **`Wake-on: g`** → armed at the NIC; ⚠️ **the block is FIRMWARE** (BIOS/UEFI "Wake on LAN"/"Wake on
  PCIe"; ⚠️ **ErP/EuP ENABLED disables WOL**)
- **`Wake-on: d`** → ✅ **just disarmed.** Arm it and persist:
```bash
sudo ethtool -s eno1 wol g
sudo nmcli con mod "$(nmcli -t -f NAME,DEVICE con show --active | grep eno1 | cut -d: -f1)" \
     802-3-ethernet.wake-on-lan magic
```
⚠️ **Then TEST:** suspend the rig, and from the Mac run `wakeonlan e8:fb:1c:af:82:f0`.

✅ **Worth doing regardless of the outcome** — ⚠️ **"this rig cannot be woken remotely" is a fact the
Windows rig inherits**, and §9 should state it either way.

---

## D. ⚠️ **THE PHYSICAL DRIVE PULL (S3)** — ⚠️ **irreducibly human**

⚠️ **No remote path substitutes for this. It is the entire reason EP-038 exists.**
✅ **Full steps in `T-0477-RUNBOOK.md` §5.** Needs: a USB drive at the rig, a world copy on it, and a
hand to pull it out ⚠️ **WITHOUT ejecting first** (⚠️ **a clean eject is S1 and is already covered**).

---

## ✅ What Claude runs once A + B land

| Scenario | Claude can run it? |
| -------- | ------------------ |
| **S1** clean `umount` | ✅ **YES, unattended** — ⚠️ **with Option 2's `fstab` entry** |
| **S2** share killed at source | ⚠️ **PARTLY** — Claude runs the probe; ⚠️ **the USER toggles File Sharing off / pulls the Mac's network** |
| **S3** physical yank | ⚠️ **NO — user only** |

⚠️ **T-0478 stays HARD-GATED until S1, S2 and S3 have all reported.**

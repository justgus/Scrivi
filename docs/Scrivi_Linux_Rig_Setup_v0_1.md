# Scrivi — Linux Rig Setup (Oathkeeper), v0.1

**Status:** Living document · **Created:** 2026-08-29 (EP-038 / SP-123, T-0476 DoD)
**Scope:** the **runnable steps** that stand up a native Linux test rig for Scrivi.

⚠️ **This document exists so the NEXT rig EXECUTES these steps rather than re-deriving them**
(Porting Outline §9). ⚠️ **It is written AS RUN, on 2026-08-29, not reconstructed from memory** —
reconstructing setup steps a sprint later is how a "reusable procedure" becomes fiction.

⚠️ **INCOMPLETE BY DESIGN.** §7 (drive dismount) is **not yet written** — it is SP-124's work
(T-0477), and writing it now would be from documentation rather than from a real drive pull, which is
the entire reason EP-038 exists.

> ⚠️ **NO credentials in this file.** Passwords, private keys, hostnames and IPs are infrastructure,
> not documentation. Where a step needs one, it says so and stops.

---

## 0. What this rig is FOR, and what a container cannot do

✅ **A container is faster for build + smoke work and never touches a real display.** Use it for that.

⚠️ **The rig exists for what the container is STRUCTURALLY BLIND TO:**

| | Real USB unplug | Docker bind-mount stop |
| - | --------------- | ---------------------- |
| Open file descriptors | ⚠️ **`EIO` / `ESTALE`** | cleanly gone |
| `/proc/mounts` entry | ⚠️ **vanishes, possibly leaving a stale mount point** | never existed as a volume |
| Kernel events | ⚠️ **udev / uevent traffic** | none |
| Partial writes in flight | ⚠️ **may be torn** | not exercised |

⚠️ **Plus: a real display.** ⚠️ **`Xvfb`/`x11vnc` are the CONTAINER's headless answer and are NOT
installed on the rig.**

---

## 1. Host — as built

| | |
| - | - |
| **Hostname** | `oathkeeper` |
| **OS** | Ubuntu **26.04.1 LTS** |
| **Arch / CPU** | x86_64, **12 cores** |
| **Network** | same /24 as the workstation |
| **Remote desktop** | GNOME **Remote Login** (`gnome-remote-desktop --system`) over **RDP** |
| **Client** | **FreeRDP** (`sdl-freerdp`) on macOS — `brew install freerdp` |

⚠️ **Remote Login, NOT Desktop Sharing.** ✅ **Remote Login spawns its OWN session and leaves whoever
holds `seat0` undisturbed** — that is why it was chosen. ⚠️ **Do not switch to user-level Desktop
Sharing, which mirrors the physical screen.**

⚠️ **The box SLEEPS.** ⚠️ **A failed ping, closed port 22, and an incomplete ARP entry mean ASLEEP,
not broken.** ⚠️ **Do not diagnose it as down.**

---

## 2. Access — the four things that each blocked this once

⚠️ **Every one of these cost a debugging session. They are listed in the order they bit.**

1. ⚠️ **Apple's Windows App CANNOT connect at all.** GNOME Remote Login answers with an RDP *server
   redirection* PDU (`LB_TARGET_CERTIFICATE`); the macOS Windows App does not follow it and the
   handoff dies as `ERRINFO_LOGOFF_BY_USER`. ✅ **FreeRDP follows it.** ⚠️ **The binary is
   `sdl-freerdp`** (SDL/Metal, no XQuartz); there is no `xfreerdp3`.
2. ⚠️ **RDP credentials are SEPARATE from the system login.** Passwordless SSH is irrelevant to them.
   `sudo grdctl --system rdp set-credentials <user> <pw>`; verify with
   `sudo grdctl --system status --show-credentials`. ⚠️ **`(null)` means every client is denied.**
   ✅ **The RDP username is arbitrary and need not be a system account.**
3. ⚠️ **The daemon reads credentials ONLY at startup.** After `set-credentials`,
   `sudo systemctl restart gnome-remote-desktop` — otherwise it keeps logging
   *"Credentials are not set, denying client"*.
4. ⚠️ **Two-stage login is BY DESIGN.** The RDP credentials authenticate to the machine; GDM then
   asks which account's session to start and wants the real system password.

### 2.1 ⚠️ SSH: a DEDICATED key is not offered by default

⚠️ **The rig's key was created with a non-default filename** (e.g. `~/.ssh/id_ed25519_Oathkeeper`).
⚠️ **`ssh` only offers its DEFAULT names** (`id_rsa`, `id_ed25519`, …), so without a config entry it
falls through, the server rejects the defaults, and it **prompts for a password** — ⚠️ **which looks
exactly like key auth having regressed. It had not; it had never been offered.**

✅ **Fix, on the CLIENT:**

```
Host oathkeeper
    HostName oathkeeper
    User <account>
    IdentityFile ~/.ssh/<the dedicated key>
    IdentitiesOnly yes
```

⚠️ **`IdentitiesOnly yes` matters** — otherwise ssh offers the other keys first and those failures
count toward the server's `MaxAuthTries`.

⚠️ **A key's COMMENT is a label, not evidence of origin.** ⚠️ **Compare FINGERPRINTS against
`~/.ssh/*.pub`** — a comment reading another machine's name proves nothing.

### 2.2 ✅ THE CONNECT COMMAND — ⚠️ **the one you will come back for**

✅ **This is the command. Everything it needs lives in the file:**

```bash
sdl-freerdp /args-from:file:$HOME/.scrivi-rdp-creds
```

⚠️ **`/args-from` CANNOT be combined with any other argument** — not even `/clipboard`. FreeRDP
rejects the whole line with *"can not be used in combination with other arguments"*. ✅ **Anything you
want added goes IN THE FILE.**

⚠️ **The file takes ONE ARGUMENT PER LINE.** ⚠️ **A single space-separated line FAILS**, and the error
names only the first option, which reads like `/v:` being malformed rather than the format being
wrong:

```
[ERROR] parse_command_line: Command line parsing failed at 'v' value
        '192.168.1.165 /u:rdp-user /p:Grindelwald /cert:ignore /size:2560x1440 /smart-sizing'
```

✅ **`~/.scrivi-rdp-creds` (mode `0600` — it holds the password):**

```
/v:192.168.1.165
/u:rdp-user
/p:Grindelwald
/cert:ignore
/size:2560x1440
/smart-sizing
/clipboard
```

⚠️ **Corrected 2026-09-08.** ⚠️ **The file had been one-line since it was created (2026-08-27) and had
therefore NEVER worked** — every real connection in shell history used the explicit form below. ⚠️ **So
a `/args-from` failure is not a regression; it had simply never run.**

✅ **Known-good fallback**, if the file form ever misbehaves:

```bash
sdl-freerdp /v:oathkeeper /ipv4 /u:rdp-user /p:Grindelwald /cert:ignore \
  /size:2560x1440 /smart-sizing /clipboard
```

⚠️ **`/v:` in the file is the IP, not the hostname.** ⚠️ **If the rig's address moves, the failure will
look like the rig being DOWN rather than the address being stale** — ✅ **check `ping oathkeeper`
before diagnosing anything else.**

⚠️ **Expect the two-stage login** (§2, item 4 — GDM asks for the real system password after the RDP
credentials) and ⚠️ **do NOT quit with `Cmd+Q`** (§2.3).

---

### 2.3 ⚠️ Right-Shift + D DISCONNECTS the session

⚠️ **`sdl-freerdp`'s shortcut modifier defaults to `KMOD_RSHIFT` ALONE**, with single-letter actions:
⚠️ **`D` disconnect**, `M` minimize, `G` grab, `Return` fullscreen. ⚠️ **In a writing application,
where capitals are constant, this fires by accident.** Observed 2026-08-29:

```
[handleEvent]: <KMOD_RSHIFT>+<SDL_SCANCODE_D> pressed, disconnecting RDP session
```

✅ **It disconnects the CLIENT, not the session** — reconnecting resumes where you were.

✅ **Fix — `~/.config/freerdp/sdl-freerdp.json` on the CLIENT** (⚠️ **these are JSON-config options,
NOT command-line flags**):

```json
{ "SDL_KeyModMask": ["KMOD_RSHIFT", "KMOD_RCTRL"] }
```

⚠️ **When a session dies mid-pass, SUSPECT THIS FIRST and read the client log** — it names the
keypress explicitly.

⚠️ **`Cmd+Q` freezes the client** (SDL's quit path deadlocks against a live redirected session).
✅ **Close the window, or `Ctrl+C` in the launching terminal**; `pkill -f sdl-freerdp` if hung.

---

## 3. Toolchain — as installed 2026-08-29

⚠️ **`sudo` requires a password on this rig**, so ⚠️ **the package install must be run BY A HUMAN at
the machine**, not over SSH.

```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential cmake ninja-build git ca-certificates libssl-dev \
    qt6-base-dev qt6-declarative-dev \
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtquick-templates qml6-module-qtquick-layouts \
    qml6-module-qtquick-window \
    qml6-module-qtqml-workerscript qml6-module-qtqml-models \
    libgl1-mesa-dev
```

⚠️ **`libssl-dev` is NOT optional** — `EncryptedFileSecureStore` needs it, and omitting it is what
failed the first SP-122 container attempt.

⚠️ **DROP `xvfb`, `x11-utils`, `x11vnc`** — those are the container's headless answer. ⚠️ **The rig
has a real display; installing them invites a headless run that does not satisfy the AC.**

### ✅ Versions that actually landed

| | Container (24.04) | **Oathkeeper (26.04.1)** |
| - | ----------------- | ------------------------ |
| CMake | 3.28.3 | **4.2.3** |
| GCC | 13.3.0 | **15.2.0** |
| Qt | 6.4.2 | **6.10.2** |
| OpenSSL | — | **3.5.5** |

⚠️ **The rig is SIX Qt minor versions and TWO GCC generations ahead of the container**, and the CMake
pin is a `6.4` floor (`platforms/linux/CMakeLists.txt:17,20`).

✅ **RULED 2026-08-29: install the distro's Qt and TREAT THE DELTA AS A FINDING.** ⚠️ **The pin is NOT
raised** — that would break the 24.04 container. ✅ **The divergence is a feature: it exercises
version-sensitivity the container can never show.** ⚠️ **Any behavioural difference between rig and
container is an ISSUE TO FILE, never a discrepancy to reconcile away.**

⚠️ **CMake 4 dropped support for `cmake_minimum_required` below 3.5, and nlohmann/json 3.11.3 declares
`VERSION 3.1...3.14`.** ✅ **The RANGE syntax saved it** — CMake read the upper bound as the policy
version. ⚠️ **A future dependency without a range WILL fail here.**

---

## 4. Source, build, test

⚠️ **Working trees live under `~/Dev/`, NEVER in `$HOME`** (user instruction 2026-08-29).

```bash
# Repo at ~/Dev/Scrivi
cd ~/Dev/Scrivi
cmake -S . -B build-native -G Ninja \
      -DSCRIVI_BUILD_LINUX=ON -DSCRIVI_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-native --parallel
ctest --test-dir build-native            # expect 571/571, run NON-ROOT
```

✅ **Linux smokes** (each needs its `.sh` wrapper, not the bare binary):

```bash
for sh in platforms/linux/tests/*.sh; do
  n=$(basename "$sh" .sh)
  bash "$sh" "./build-native/platforms/linux/scrivi_linux_${n}"
done                                     # expect 18/18
```

⚠️ **Run `ctest` NON-ROOT.** ⚠️ **"The image built" has never meant the suite ran**, and root masks
real permission behaviour.

⚠️ **A CMake build tree does NOT survive being moved** — `CMakeCache.txt` bakes absolute paths, so
`ctest` keeps reporting the OLD directory after a `mv`. ✅ **`rm -rf` the build dir and reconfigure**
(~2 minutes on 12 cores). ⚠️ **Never try to patch the cache.**

### ⚠️ Transferring source without git credentials on the rig

✅ **Executables and sources can be shipped over SSH**, which avoids putting git credentials on the
rig. ⚠️ **Include UNTRACKED files** — `git ls-files` alone silently omits work in progress and the
configure then fails on missing sources:

```bash
{ git ls-files; git ls-files --others --exclude-standard; } | sort -u > /tmp/filelist.txt
tar -czf payload.tgz -T /tmp/filelist.txt
```

⚠️ **macOS `tar` recreates AppleDouble `._*` files from extended attributes even after you delete
them.** ✅ **Strip them ON THE RIG after extracting**, then verify by checksum:

```bash
find <dest> \( -name '._*' -o -name '.DS_Store' \) -delete
```

⚠️ **A Mac-side build does NOT satisfy the native-build AC** — the transfer path is for iteration only.

---

## 4a. ⚠️ Confirming WHICH BUILD is on the rig

⚠️ **This has bitten once already.** On 2026-08-30 the rig was found running a **day-old binary that
predated an entire sprint** — a live pass against it would have reported on the wrong code, and there
was no way to tell from inside the app.

✅ **The app now stamps itself**, and the stamp cannot drift because `__DATE__`/`__TIME__` are baked in
by the **preprocessor** at compile time:

```bash
~/Dev/Scrivi/build-native/platforms/linux/scrivi_linux --version
# Scrivi (Linux) — built Aug 30 2026 15:35:32, Qt 6.10.2
```

✅ **Also in the GUI: Help ▸ About Scrivi** — ⚠️ **deliberately NOT gated on a project being open**,
because the moment you most want to ask "is this the right build?" is at the landing screen.

⚠️ **`--version` is handled BEFORE `QApplication` is constructed**, so it works over plain SSH where
there is no `DISPLAY`.

### ✅ The deploy loop — ONE command

```bash
platforms/linux/deploy-to-rig.sh            # push + build + verify
platforms/linux/deploy-to-rig.sh --test     # …and ctest + the 19 smokes
```

It packages the **working tree** (⚠️ **including uncommitted and untracked files** — that is what you
are testing), ships it, builds natively, and ⚠️ **fails loudly if the deployed binary does not carry
the build number it just shipped.**

Host defaults to `oathkeeper` (`SCRIVI_RIG`); path to `~/Dev/Scrivi` (`SCRIVI_RIG_PATH`).

### ⚠️ The build NUMBER — and why it belongs to the PUSH, not the compile

✅ **The script prints the number to expect; the app shows the number it has.** They match or they do
not — ⚠️ **no clock arithmetic, which is what made a bare timestamp hard to use.**

```
==> Deployed:  Scrivi (Linux) — build 4 (2026-08-30 19:59:07 UTC), Qt 6.10.2
    ┌────────────────────────────────────────────────┐
    │  Check the app reports:  BUILD 4               │
    └────────────────────────────────────────────────┘
```

In the app: **Help ▸ About Scrivi** (⚠️ **not gated on a project being open**), or
`scrivi_linux --version` over SSH.

⚠️ **TWO CMake-side attempts to generate this during the BUILD both failed, in misleading ways:**

1. ⚠️ **A custom TARGET touching `main.cpp`** — Ninja computes its dependency graph **before** any
   build step runs, so the touch was only seen on the **next** build. ⚠️ **The stamp trailed reality by
   exactly one build, which looks like it works until you check it carefully.**
2. ⚠️ **A custom COMMAND generating a header** — Ninja saw the output already existed and never re-ran
   the step, so ⚠️ **the number froze at 1.**

✅ **The question is "did the source I just pushed reach the rig?", so the counter belongs to the
PUSH.** The script increments it and ships the header with the source, so ⚠️ **the number changes
exactly when the code does.** A local `cmake --build` with no deploy leaves it unchanged — correct,
because nothing was pushed.

⚠️ **The counter lives at `~/.scrivi-rig-build-number`, outside the repo**, so it never conflicts. The
generated header is committed as a fallback for anyone who never runs the script (the Docker image, a
fresh clone), where it reads `build 0 (local build — not deployed)`.

---

## 5. Running the app

⚠️ **Launch from a terminal INSIDE the RDP session.** ⚠️ **Over SSH there is no `DISPLAY` and it will
not start** — and forcing one from SSH pushes onto someone else's session, which does not satisfy the
AC anyway.

```bash
~/Dev/Scrivi/build-native/platforms/linux/scrivi_linux
```

✅ **A desktop launcher is installed** at `~/.local/share/applications/scrivi.desktop` (and copied to
`~/Desktop/`), using the real app icon from `Scrivi/Assets.xcassets/AppIconMacOS.appiconset/`
installed to `~/.local/share/icons/hicolor/512x512/apps/scrivi.png`.

⚠️ **GNOME requires a Desktop launcher be marked trusted**, or it renders as a text file:

```bash
gio set ~/Desktop/scrivi.desktop metadata::trusted true
```

⚠️ **Running `update-desktop-database` while a session is live can cause a brief icon refresh** —
harmless, but warn the user first.

---

## 6. Test data on the rig

| | |
| - | - |
| **Projects** | `~/ScriviProjects/` |
| **Worlds** | `~/ScriviWorlds/` |

⚠️ **Copies of REAL WRITING WORK.** ⚠️ **Verify a transfer by CHECKSUM, not by file count** — an
AppleDouble infestation matches on neither, but a truncated file matches on count alone.

⚠️ **A project's world binding stores an ABSOLUTE path** (`reference.lastKnownAbsolutePath`). ⚠️ **A
project copied from another machine will therefore open with its world UNAVAILABLE and links held
pending** — ✅ **correctly**, because that path does not exist on the rig.

⚠️ **There is NO relink UI on Linux as of 2026-08-29.** `relinkWorld` is bridged and unreached
(EP-035 **AC3**). ⚠️ **So a moved world cannot currently be repaired from the app** — that is
`capability_without_surface` in its exact form.

⚠️ **Opening a project WRITES to it** (a `generation` bump and a world `cachedIndex` reconcile). ✅ **It
is not a risk and touches no prose** — it is how a shared world propagates between projects — ⚠️ **but
it means "I only opened it to look" is not a read-only operation.**

⚠️ **NEVER drive the app with synthetic input (`xdotool`) while real work is open.** ⚠️ **A `ctrl+q`
that silently fails sends the next typed string INTO THE MANUSCRIPT**, and the idle-save persists it
(I-0175). ✅ **The live pass is a HUMAN pass; that is the point of it.**

---

## 7. ✅ Drive dismount and share loss — **WRITTEN FROM THE RIG (2026-09-07)**

⚠️ **Written from THREE OBSERVED EVENTS on `oathkeeper`, not from documentation.** ⚠️ **Where a
prediction failed, the prediction is recorded alongside the result** — that contrast is the point.

**Findings in full:** `platforms/linux/tools/T-0477-FINDINGS-S3.md` (physical yank) ·
`platforms/linux/tools/T-0477-FINDINGS-S2.md` (share killed at source, ⚠️ **two passes**).

### 7.1 ⚠️ The three events are DIFFERENT. Do not infer one from another.

| | **S1** clean `umount` | **S3** ⚠️ **physical USB yank** | **S2** ⚠️ **server killed** |
| - | --------------------- | ------------------------------- | --------------------------- |
| Mountpoint dir | removed | ⚠️ **REMOVED by udisks2** | ⚠️ **SURVIVES** |
| `mountpoint -q` | honest | honest | ⚠️ **LIES — says YES** |
| Directory listing | gone | gone | ⚠️ **SUCCEEDS, sizes ZEROED** |
| Call latency | normal | normal | ⚠️ **~10 s per call** |
| Core verdict | `unavailable` | ✅ **`unavailable`** | ✅ **`unavailable`** |
| Recovery | remount | replug | ✅ **AUTOMATIC, no `umount -l`** |

### 7.2 ⚠️ What a PHYSICAL YANK actually does (S3)

⚠️ **It is SILENT.** ⚠️ **No error, no warning, no kernel complaint reached the user** — the drive
*"just unmounted, which is what USB is supposed to do."*

✅ **udisks2 REMOVES the mountpoint it created** (`/run/media/<user>/<label>` vanished entirely).
⚠️ **This was NOT predicted** — the working assumption was that Linux KEEPS the mountpoint where macOS
removes it. ✅ **For an automounted volume that assumption is FALSE**, and it matters: the core's
`missing` rule needs *parent exists*, so ⚠️ **a false `missing` is NOT REACHED on this path.**

⚠️ **A HAND-MOUNTED `/mnt/<name>` behaves the OPPOSITE way** — ✅ **the directory is yours and nothing
deletes it** — ⚠️ **so the false `missing` IS reachable there.** ⚠️ **Which path you mount on changes
which defect you can hit.**

⚠️ **A read SUCCEEDED against the removed volume** for a short window (page cache / unreaped dentries),
⚠️ **long enough that the app reported the world available and an object opened successfully.**
✅ **The steady state is correct** — ⚠️ **the transient is not.** ⚠️ **Whether it decays or persists is
STILL UNMEASURED** (needs sampling ACROSS a yank).

### 7.3 ⚠️ What a SERVER GOING AWAY does (S2) — ⚠️ **the ~10-second block**

⚠️ **THE MOST IMPORTANT OPERATIONAL FINDING OF THE WHOLE SPRINT.**

⚠️ **Every filesystem call against a dead SMB share BLOCKS for ~10 s before returning `EHOSTDOWN`
(errno 112).** ⚠️ **Measured identically under `cache=strict` AND `cache=none`, with `soft` set in
both.**

⚠️ **`soft` prevents an unkillable hang. It does NOT prevent a ten-second stall.** ⚠️ **The pre-run
assumption — that hangs were an NFS-hard-mount problem and `cifs` was the safe choice — IS WRONG.**

✅ **Healthy call: 0.090 s. Degraded call: ~10 s. ⚠️ ~110× slower**, so a timeout is cheap to
discriminate. ⚠️ **The cost COMPOUNDS**: a probe making several core calls degraded to ⚠️ **43-second
intervals.**

⚠️ **CONSEQUENCE FOR ANY PLATFORM LAYER: do NOT call the core synchronously on the UI thread for world
status.** ⚠️ **A frozen UI is worse than a wrong status.**

### 7.4 ⚠️ The signals that LIE — ⚠️ **name them, as Apple's did**

| Signal | ⚠️ Verdict |
| ------ | ---------- |
| ⚠️ **`mountpoint -q`** | ⚠️ **LIES on server loss** — reported YES throughout S2 |
| ⚠️ **A directory listing** | ⚠️ **LIES** — succeeded one level deep with **ZEROED sizes**; the level below failed `EHOSTDOWN`. ⚠️ **"The world folder is still there" is DEFEATED** |
| ⚠️ **`statvfs`** | ⚠️ **LIES** — ✅ **measured SUCCEEDING on an unmounted path**, reporting the ROOT filesystem's block counts. ⚠️ **A confident success with a plausible number** |
| ⚠️ **A successful `read`** | ⚠️ **LIES transiently** — ⚠️ **S3's page-cache window**, and ⚠️ **a held FD survived `umount -l` + `losetup -D` ENTIRELY** in the container pass |
| ✅ **`st_dev` vs the parent's** | ✅ **WORKS** — ⚠️ **but proves "not a mount NOW", NOT "a volume went away"**; a directory that never held a mount matches identically |
| ✅ **`errno` — `EHOSTDOWN` (112)** | ✅ **SPECIFIC to a dead server** — ⚠️ **and currently UNUSED by `resolve`.** ⚠️ **This is the strongest unexploited signal for `offline`** |

### 7.5 ⚠️ `WorldStatus::offline` has NEVER been observed

⚠️ **S2 IS the network case that DEFINES `offline`, and the core returned `unavailable`** — in both
passes, within ~1 s. ⚠️ **Not once has any scenario produced `offline`.**

⚠️ **This is a finding about the STATUS MODEL, and it must be RULED on, not quietly omitted.**

### 7.6 ⚠️ Cache options trade one failure for another

| | ⚠️ `cache=strict` | ⚠️ `cache=none` |
| - | ----------------- | --------------- |
| Phantom/stale listing | ⚠️ **YES** (§2c's defect) | ✅ **Suppressed** |
| Core's answer while down | ✅ **honest `unavailable` in ~1 s** | ⚠️ **NO ANSWER AT ALL** — every call blocked |
| ⚠️ **Project OPEN, share HEALTHY** | ✅ **baseline** | ⚠️ **~10× SLOWER** (user-reported 2026-09-08) |

⚠️ **The client cache was the ONLY thing answering fast enough for a prompt honest verdict.**
⚠️ **Neither setting is safe alone** — ✅ **the app needs its own timeout regardless.**

### 7.6.1 ⚠️ **The THIRD cost of `cache=none` — measured 2026-09-08, and NOT predicted**

⚠️ **§7.6 was written expecting `cache=none` to cost the OFFLINE VERDICT. It also costs EVERYDAY
SPEED, on a perfectly healthy share** — ⚠️ **which is the cost a writer actually feels, every single
open.**

⚠️ **Measured on the rig, `Eskandar.scrivworld`, share UP:**

| | ⏱ |
| - | -- |
| Recursive listing (`ls -R`) | ⚠️ **0.47 s** |
| Reading all **57** files | ⚠️ **1.49 s** |
| World resolve (core only) | ✅ **0.05–0.08 s** — ⚠️ **so the core is NOT the cost** |

⚠️ **The user reported project open as ~10× slower and was RIGHT to call it a defect** — ⚠️ **an
earlier reading of mine dismissed it as "mount configuration, not an app defect." ⚠️ THAT WAS WRONG,
and the correction is the useful part:** ✅ **the app blocks the UI for the whole read regardless of
WHY the I/O is slow**, ⚠️ **so a slow mount does not CAUSE the defect — it EXPOSES one.**

⚠️ **The cost is UNBOUNDED and grows on three axes:** ⚠️ **worlds accumulate objects** (57 files is a
TEST world), ⚠️ **a project may bind SEVERAL worlds**, and ⚠️ **project and worlds may BOTH be on slow
network storage.**

✅ **Recorded as [I-0195]** — ⚠️ **a PERFORMANCE defect (the data always loads correctly)** requiring
⚠️ **an async read plus a determinate progress indicator**, ✅ **which is achievable because the FILE
COUNT is known early enough to drive a real percentage rather than a spinner.**

⚠️ **For ordinary rig use, prefer `cache=strict`** — ⚠️ **but understand you are MASKING [I-0195], not
fixing it**, ✅ **and re-earning §2c's phantom listing.**

### 7.7 ✅ Runnable steps — reproducing all three on a fresh rig

⚠️ **Every command is tagged 🐧 `oathkeeper` (the rig) or 🍎 the workstation.** ⚠️ **Not saying which
machine cost an entire S3 attempt on 2026-09-07** — the probe was run on the Mac against a Linux path
and watched nothing.

```bash
# 🍎  Build the Qt-free probe, then deploy source to the rig (it compiles THERE —
#     the two machines are different architectures; never copy binaries).
cmake --build build-tests --target scrivi_world_probe
platforms/linux/deploy-to-rig.sh          # or: git push, then git pull on the rig
```

**S3 — physical yank** (⚠️ needs a real removable device):

```bash
# 🐧  Note where it lands, and WHICH mountpoint owner you are testing:
lsblk -f -o NAME,SIZE,TYPE,FSTYPE,LABEL,MOUNTPOINT,TRAN,RM
#     /run/media/<user>/<label>  = udisks2  → mountpoint REMOVED on yank
#     /mnt/<name>                = yours    → mountpoint SURVIVES
platforms/linux/tools/s3-baseline-capture.sh /run/media/$USER/<label> ~/ScriviProjects/<p>.scrivi
platforms/linux/tools/volume-loss-probe.sh   /run/media/$USER/<label> S3-physical-yank
#     …then PULL THE DRIVE BY HAND. Do NOT eject first — a clean eject is S1.
```

**S2 — share killed at the source** (✅ **no drive needed, no console needed**):

```bash
# 🍎  Serve a COPY of a world from LOCAL disk, and mount WITHOUT the removable share:
cp -a <world>.scrivworld ~/ScriviLinux/worlds/
./scripts/mount-shares-on-rig.sh --no-worlds

# 🐧  ⚠️ VERIFY ISOLATION FIRST — the share also carries projects/ and appsupport/.
#     The project and appSupportRoot MUST be on LOCAL disk, or the kill removes
#     them too and you measure "everything vanished" instead of "a world went away".
ls ~/ScriviProjects/            # project: local
echo "${XDG_DATA_HOME:-<unset>}"  # appSupportRoot: ~/.local/share/Scrivi, local

# 🐧  Point the project's binding at /mnt/scrivi-net/worlds/<world>.scrivworld,
#     then CONFIRM the world reads "available" before touching anything:
~/Dev/Scrivi/build-native/ScriviCore/scrivi_world_probe ~/ScriviProjects/<p>.scrivi

# 🐧  Sample the CORE's verdict every 2 s (this is the input to WorldVolumeStatus):
while true; do printf '%s ' "$(date +%H:%M:%S)"; \
  ~/Dev/Scrivi/build-native/ScriviCore/scrivi_world_probe ~/ScriviProjects/<p>.scrivi \
  2>&1 | grep -m1 '"status"'; sleep 2; done | tee ~/scrivi-probe/S2-world-status.txt

# 🍎  THEN: System Settings ▸ General ▸ Sharing ▸ File Sharing → OFF.  Wait ~60 s.
```

⚠️ **Read the gaps between samples, not just the values.** ⚠️ **The loop sleeps 2 s; anything longer is
the CALL BLOCKING**, and that latency is the finding.

⚠️ **`sudo` on the rig needs a TTY.** ⚠️ **`ssh oathkeeper 'sudo …'` FAILS with *"a terminal is required
for reauthentication"*** — ✅ **use `ssh -t`, or run it in an interactive session.**

### 7.8 ⚠️ Still owed

- ⚠️ **The "network off / black hole" S2 variant** — ⚠️ **only "File Sharing OFF" (a REFUSED connection)
  was run.** ⚠️ **A black hole is where hangs live; the ~10 s block may be far worse.**
- ⚠️ **NFS** — ⚠️ **`cifs` only so far.**
- ⚠️ **A held-open FD across a yank** — ⚠️ **the probes re-open by path each time.**
- ⚠️ **Torn writes** — ⚠️ **no write was in flight.**
- ⚠️ **Whether S3's successful read DECAYS or PERSISTS** — ⚠️ **needs sampling ACROSS a yank.**

---

## 8. Per-platform rig status

| Platform | Rig | Status |
| -------- | --- | ------ |
| **Linux** | `oathkeeper` (Ubuntu 26.04.1) | ✅ **Built, green, app launches** (2026-08-29) |
| **Windows** | — | ⚠️ **Not started.** ✅ **It must EXECUTE §§1–6, not re-derive them** |
| **macOS** | the workstation | ✅ Existing |
| **iPad / iPhone / visionOS** | — | ⚠️ Not started |

---

*v0.2 — 2026-09-07. ✅ **§7 WRITTEN from three observed events on the rig** (S1 clean `umount`, S3
physical yank, S2 server killed — ⚠️ **two cache passes**), closing the deliverable SP-124 / T-0477
owed. ⚠️ **Two pre-run assumptions FELL and are recorded as such**: udisks2 REMOVES an automounted
mountpoint on a yank (so I-0181's false `missing` is not reached there), and ⚠️ **`cifs` with `soft`
BLOCKS ~10 s per call against a dead server** — hangs are not an NFS-only concern. ⚠️ **`WorldStatus::offline`
has never been observed and needs a ruling.** ⚠️ **T-0479 must correct Porting Outline §9 from this
document**, not from memory.*

*v0.1 — 2026-08-29. Written as-run during SP-123.*

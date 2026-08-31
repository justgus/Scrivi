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

### 2.2 ⚠️ Right-Shift + D DISCONNECTS the session

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

## 7. ⚠️ Drive dismount — NOT YET WRITTEN

⚠️ **This section is deliberately empty.** ✅ **It is SP-124 / T-0477's deliverable**, and it must be
written **from an actual drive pull on this rig**, not from documentation.

⚠️ **The precedent that makes this non-negotiable:** on Apple, `volumeIsRemovable` and
`volumeIsEjectable` BOTH read **false** on a drive unplugged by hand — ⚠️ **the documented API lied**,
and the signal that worked (`volumeIsLocal`) was found only by pulling a real drive.
⚠️ **Drive-loss-while-editing cost Apple SIX Issues, every one found by ejecting a drive and NONE by a
suite.**

**What this section must eventually record:**

- ⚠️ What the kernel and `/proc/mounts` actually do on unplug — including whether a **stale mount
  point** is left behind
- ⚠️ What open file descriptors return (`EIO`? `ESTALE`?) and what ScriviCore does with that
- ⚠️ Which Linux signal reliably distinguishes **`unmounted`** from **`offline`** from
  **`unavailable`** — ⚠️ **and which plausible-looking signals LIE**, as Apple's did
- ⚠️ Whether a write in flight is torn, and what the repair path does about it
- ✅ The **runnable steps** to reproduce the whole scenario on a fresh rig

⚠️ **Do NOT populate this from `WorldStore.hpp` or from Apple's implementation.** ⚠️ **The whole
purpose of the sprint boundary between SP-123 and SP-124 is to keep
*instrument-before-implement* honest.**

---

## 8. Per-platform rig status

| Platform | Rig | Status |
| -------- | --- | ------ |
| **Linux** | `oathkeeper` (Ubuntu 26.04.1) | ✅ **Built, green, app launches** (2026-08-29) |
| **Windows** | — | ⚠️ **Not started.** ✅ **It must EXECUTE §§1–6, not re-derive them** |
| **macOS** | the workstation | ✅ Existing |
| **iPad / iPhone / visionOS** | — | ⚠️ Not started |

---

*v0.1 — 2026-08-29. Written as-run during SP-123. ⚠️ **§7 is owed by SP-124 (T-0477)**; ⚠️ **T-0479
must correct Porting Outline §9 from this document**, not from memory.*

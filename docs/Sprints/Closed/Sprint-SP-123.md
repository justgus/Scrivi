# SP-123: `[Linux]` EP-038 — Rig reachable + building natively ✅ CLOSED

**Closed:** 2026-08-29 (user-approved) · **Activated:** 2026-08-25
**Epic:** [EP-038](../../Epics/Epic-active.md) — `[Linux]` The Real Hardware Rig · **sprint 1 of 2**
**Outcome:** ✅ **All three Tasks Verified 2026-08-29.** ⚠️ **7/7 DoD items met.**

---

## ✅ What this sprint delivered

⚠️ **A real Ubuntu machine that we can reach, build on, test on, and SEE the app running on** —
⚠️ **the first time the Linux app has EVER run on real hardware.**

| Task | Owner | Outcome |
| ---- | ----- | ------- |
| **T-0474** | ⚠️ **The USER implemented** (2026-08-27) | ✅ **Verified by CLAUDE** — ⚠️ **the one Task with an INVERTED verification direction** |
| **T-0475** | Claude, over SSH | ✅ **Verified** — toolchain; ⚠️ **the user ran the `sudo` installs** |
| **T-0476** | ⚠️ **The USER** | ✅ **Verified** — ⚠️ **app launched on the REAL display**, opened a project |

✅ **`ctest` 571/571 NON-ROOT · 18/18 Linux smokes · native build OUTSIDE a container.**
✅ **Navigator, manuscript view and timeline all nominal** on the rig.

## ⚠️ The headline finding — the rig is FAR ahead of the container, and it cost NOTHING

| | Container (24.04) | **Oathkeeper (26.04.1)** |
| - | ----------------- | ------------------------ |
| CMake | 3.28.3 | **4.2.3** |
| GCC | 13.3.0 | **15.2.0** |
| Qt | 6.4.2 | **6.10.2** |

⚠️ **Six Qt minor versions and two GCC generations apart** — ✅ **and the app built and passed with
ZERO code changes.** ⚠️ **A clean build under GCC 15's stricter C++23 is a real result, not a
formality.**

⚠️ **The CMake 4 risk was REAL and did not fire:** CMake 4 dropped `cmake_minimum_required` below 3.5
and **nlohmann/json 3.11.3 declares `VERSION 3.1...3.14`** — ✅ **the range syntax saved it.**
⚠️ **A future dependency without a range WILL fail on this rig.**

✅ **RULED: the pin stays at 6.4** (raising it breaks the container); ⚠️ **any rig-vs-container
behavioural difference is an ISSUE TO FILE, never a discrepancy to reconcile away.**

## ⚠️ Three Issues, all found within minutes of the first real launch

| ID | Finding |
| -- | ------- |
| **I-0176** | A project open at Quit does not reopen |
| **I-0177** | A maximized window does not reopen maximized — ⚠️ **and splitter sizes are not persisted either** |
| **I-0178** | ⚠️ **Only ONE project can be open at a time** |

⚠️ **These are ONE gap with three symptoms, not three bugs.** ✅ **Apple solved all three in EP-018**
(R1–R5) via `OpenProjectRegistry` + per-window `ProjectSession` + `restoreOpenProjects()`.
⚠️ **I-0178 is the parent, and a Linux equivalent is a STRUCTURAL rework of `ScriviWindow`/
`EditorShell` that wants its OWN Epic.**

⚠️ **NONE was findable by a suite** — they are about what survives a QUIT, which no test exercises.
✅ **User-ruled non-blocking**: gaps in scope never claimed, not failures of what was built.

## ✅ The deliverable that survives

**`docs/Scrivi_Linux_Rig_Setup_v0_1.md`** — the runnable steps, ⚠️ **written AS RUN, not
reconstructed.** ⚠️ **The Windows rig must EXECUTE it, not re-derive it** (Porting Outline §9).

⚠️ **Its §7 (drive dismount) is DELIBERATELY EMPTY** — that is **SP-124 / T-0477**, and it must be
written from a real drive pull. ⚠️ **The sprint boundary is what keeps *instrument-before-implement*
honest.**

## ⚠️ Things that cost a debugging session each

- ⚠️ **The SSH key is a DEDICATED non-default filename**, so ssh never offered it — ⚠️ **which looked
  exactly like key auth "regressing" when it had never been exercised.** ⚠️ **A key's COMMENT is a
  label, not evidence of origin** — I wrongly concluded the rig's key came from another machine.
- ⚠️ **Right-Shift + D disconnects the RDP session** — `sdl-freerdp`'s modifier defaults to
  `KMOD_RSHIFT` ALONE. ⚠️ **In a writing app, where capitals are constant, this fires by accident.**
- ⚠️ **`git ls-files` silently omitted untracked work** from the transfer, and the configure failed on
  missing sources. ⚠️ **macOS `tar` then regenerated AppleDouble `._*` files after they were deleted.**
- ⚠️ **A CMake build tree does not survive a `mv`.**

---

## SP-123 — `[Linux]` ⚠️ **Rig reachable + building natively**

**Status:** ✅ **CLOSED 2026-08-29 — user-approved**
**Epic:** [EP-038](../Epics/Epic-active.md) — `[Linux]` The Real Hardware Rig · **sprint 1 of 2**
**Codebase:** ⚠️ **environment/infrastructure — NO application code changes expected**
**Date Activated:** 2026-08-25
**Closes:** **AC1, AC2, AC3**
**Tasks:** **T-0474 – T-0476** (three) · **Next available:** T-0492 (T-0485–T-0490 → SP-126, T-0491 unscheduled) · Issue **I-0176**

⚠️ **EP-035 runs in PARALLEL.** This sprint is blocked on physical access; EP-035's is not.

---

## 1. Sprint Goal

**A real Ubuntu machine that we can reach, build on, test on, and SEE the app running on** — so that
⚠️ **SP-124 can pull a drive and find out what actually happens.**

⚠️ **This sprint ships NO application code and NO surface.** ✅ **Its deliverable is a working
environment plus the runnable steps that recreate it** — because ⚠️ **the Windows rig must EXECUTE those
steps, not re-derive them** (Porting Outline §9).

---

## 2. ⚠️ T-0474 — MOSTLY DONE ALREADY (user, 2026-08-29); ⚠️ **the box is OFFLINE**

⚠️ **UPDATE 2026-08-29 — this task is much further along than the sprint plan assumed.** The rig was
stood up on **2026-08-27**, before this sprint's plan was written, and the user confirmed its shape:

| Item | State |
| ---- | ----- |
| **Host** | ✅ **`oathkeeper`** — Ubuntu 26.04.1 LTS, resolves to **192.168.1.165** (same /24 as the Mac) |
| **Remote desktop** | ✅ **RDP** via GNOME Remote Login, client is **FreeRDP** (`sdl-freerdp`) on macOS |
| **File transfer** | ✅ **SSH** — ⚠️ **executables can be shipped to the rig over it**, so a Mac-side cross-build is an option |
| **Reachability** | ✅ **Set up and working.** ⚠️ **The box was ASLEEP when probed 2026-08-29** (no ping, 22/3389 closed, ARP incomplete) — ⚠️ **that is sleep, NOT a fault.** |

✅ **T-0474 is COMPLETE** — ⚠️ **it was implemented BY THE USER on 2026-08-27**, before this sprint's plan
was written. ⚠️ **Do NOT re-probe the rig to "check"** — waking it is the user's call, and a failed ping
against a sleeping machine is not evidence of anything.

⚠️ **The hard-won RDP details are recorded in the `project_oathkeeper_rdp` memory, NOT here** — ⚠️ **a
rig's credentials are not documentation.** ⚠️ **Four separate things each blocked that setup once**
(Apple's Windows App cannot follow GNOME's RDP redirect; RDP credentials are separate from the system
login; the daemon reads them only at startup; two-stage GDM login is by design).

### ⚠️ OWNERSHIP — user ruling 2026-08-29

⚠️ **This sprint's three Tasks do NOT share an owner, and that changes who is blocked on whom.**

| Task | Implemented by | Verified by | Note |
| ---- | -------------- | ----------- | ---- |
| **T-0474** | ✅ **THE USER** (2026-08-27) | ⚠️ **CLAUDE** | ⚠️ **The verification direction is INVERTED here.** The user implemented it, so ⚠️ **Claude verifies it** — the one Task in this project where that is true. ⚠️ **Verification waits until the box is AWAKE.** |
| **T-0475** | ✅ **CLAUDE** — over SSH | **The user** | Toolchain + deps. The normal direction. |
| **T-0476** | ✅ **THE USER** | — | ⚠️ **Needs a REAL display and a physical machine**, which is the whole point of the Epic. Blocked on T-0475. |

⚠️ **So the standing rule *"Claude may never mark a Task Verified"* does NOT apply to T-0474** — it
exists to stop Claude self-certifying its OWN work, and this is the user's work. ✅ **Claude verifying
the user's implementation is the rule working as intended, not an exception to it.**

✅ **What happens next:** ⚠️ **when the user wakes `oathkeeper`**, Claude verifies T-0474 (SSH key auth,
reachability, RDP), then does **T-0475** over SSH. ⚠️ **T-0476 then returns to the user** — it needs a
human at a real display.

⚠️ **NO passwords, private keys, hostnames or IPs go into any tracked file.** ⚠️ **A rig is
infrastructure; its credentials are not documentation.**

---

## 3. Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0474** | ⚠️ **Account + SSH key exchange + reachability** — ⚠️ **USER-IMPLEMENTED 2026-08-27** | **High** | ✅ **VERIFIED 2026-08-29 by CLAUDE** (inverted direction; see §2) |
| **T-0475** | **Toolchain + dependencies** — ⚠️ **CLAUDE, over SSH** (⚠️ **the USER ran the `sudo` installs**) | **High** | ✅ **VERIFIED 2026-08-29** — build + 571/571 + 18/18 smokes GREEN natively |
| **T-0476** | **First NATIVE build** + `ctest` non-root + ⚠️ **the app launches on a REAL display** — ⚠️ **THE USER's**, needs a human at the machine | **High** | ✅ **VERIFIED 2026-08-29** — ⚠️ **launched on the REAL display**; navigator/manuscript/timeline nominal; ⚠️ **3 Issues filed** |

### T-0475 — the dependency list

✅ **Start from the container's KNOWN-GOOD list** (`platforms/linux/docker/Dockerfile`), which already
builds this app:

`build-essential` · `cmake` · `ninja-build` · `git` · `ca-certificates` · `libssl-dev` ·
`qt6-base-dev` · `qt6-declarative-dev` · `qml6-module-qtquick{,-controls,-templates,-layouts,-window}` ·
`qml6-module-qtqml-{workerscript,models}` · `libgl1-mesa-dev`

⚠️ **Qt ≥ 6.4 is REQUIRED** — `platforms/linux/CMakeLists.txt:17` (`Quick`, `Widgets`, `QuickWidgets`).
⚠️ **Verify the distro's Qt actually meets that**; an older Ubuntu will not.

⚠️ **DROP `xvfb`, `x11-utils`, `x11vnc`** — ⚠️ **those are the CONTAINER's headless answer.**

⚠️ **UPDATE 2026-08-29: the desktop session and remote-desktop server are ALREADY DONE** — the user
installed them on 2026-08-27 (Ubuntu 26.04.1 desktop + GNOME Remote Login over RDP). ✅ **T-0475 is
therefore TOOLCHAIN ONLY**, which makes it materially smaller than planned.

### ✅ RESULT (2026-08-29) — the rig is GREEN, and the version gap cost NOTHING

| | Container (24.04) | Oathkeeper (26.04.1) |
| - | ----------------- | -------------------- |
| **CMake** | 3.28.3 | ⚠️ **4.2.3** |
| **GCC** | 13.3.0 | ⚠️ **15.2.0** |
| **Qt** | 6.4.2 | ⚠️ **6.10.2** |
| **`ctest`** | 571/571 | ✅ **571/571** |
| **Linux smokes** | 18/18 | ✅ **18/18** |

⚠️ **Repo path on the rig: `~/Dev/Scrivi`** (user instruction 2026-08-29 — ⚠️ **not `$HOME`**;
`~/Dev` already existed). ⚠️ **A CMake build tree does not survive a `mv`** — reconfigure after moving.

✅ **Native configure, build and test ALL PASS non-root as `justgus`** — ⚠️ **with ZERO code changes**,
across a toolchain two LTS generations newer. ⚠️ **A clean build under GCC 15's stricter C++23 is a
REAL result**, not a formality.

⚠️ **The CMake 4 risk was REAL and did not fire:** CMake 4 dropped support for
`cmake_minimum_required` below 3.5, and **nlohmann/json 3.11.3 declares `VERSION 3.1...3.14`**.
✅ **The range syntax saved it** — CMake read the upper bound as the policy version. ⚠️ **A future
dependency without a range WILL fail here**, so this is worth remembering rather than forgetting.

### ⚠️ FINDING (2026-08-29) — the rig and the container are SIX MINOR VERSIONS APART

⚠️ **MEASURED, not assumed** — which is what this Task existed to do:

| | Qt |
| - | -- |
| **Container** (Ubuntu 24.04) | **6.4.2** |
| **Oathkeeper** (Ubuntu 26.04.1) | ⚠️ **6.10.2** |
| **CMake pin** (`platforms/linux/CMakeLists.txt:17,20`) | `6.4` floor |

✅ **6.10.2 satisfies the floor, so it configures** — ⚠️ **but "green on the rig" and "green in the
container" no longer mean the same thing**, and nothing else in this project records that.

✅ **USER RULING 2026-08-29: install 6.10.2 and TREAT THE DELTA AS A FINDING.** ⚠️ **The pin is NOT
raised** — doing so would break the 24.04 container, which only has 6.4.2. ✅ **The divergence is
arguably a FEATURE: it exercises version-sensitivity the container can never show.**

⚠️ **Any behavioural difference between rig and container is an ISSUE to FILE, never a discrepancy to
reconcile away.** ✅ **Low risk measured:** `platforms/linux/src/` contains only **two** version guards
(`NavigatorTree.cpp:237,270`), both `>= 6.0`, and the APIs used are long-stable Widgets classes.

⚠️ **`libssl-dev` is NOT optional** — `EncryptedFileSecureStore` needs it, and omitting it is what failed
my first SP-122 container attempt.

### T-0476 — what "green" must mean here

⚠️ **T-0476 IS THE USER'S** (ruling 2026-08-29) — it needs a human at a real display.

- ✅ **Native build** — ⚠️ **not in Docker**, from a clean clone.
  ✅ **Alternative available:** ⚠️ **executables can be shipped to the rig over SSH** (user, 2026-08-29),
  so a Mac-side build is possible — ⚠️ **but that does NOT satisfy this AC**, which is *"native build on
  the rig"*. ✅ **Keep the transfer path for iterating quickly; do the native build for the AC.**
- ✅ **`ctest` non-root, tests ON** — ⚠️ **expect 571**, matching SP-121/SP-122's Linux figure.
  ⚠️ **A different number is a FINDING**, not a rounding.
- ✅ ⚠️ **The Qt app actually LAUNCHES on the real display** and opens a project.
  ⚠️ **"It compiled" is not this AC.**

⚠️ **Use a THROWAWAY project copy, never the real rig data** (I-0150).

### ⚠️ Record the steps AS YOU GO, not afterwards

⚠️ **T-0479 (SP-124) must correct Porting Outline §9 from what actually happened.** ⚠️ **Reconstructing
setup steps from memory a sprint later is how a "reusable procedure" becomes fiction.**

---

## 4. Definition of Done

- [x] ✅ **Reachable over SSH with key auth** (T-0474) — ⚠️ **the key is a DEDICATED non-default filename**, which is why it looked like a regression; ✅ **no credential is in any tracked file**
- [x] ✅ **Toolchain installed**; ⚠️ **Qt CONFIRMED at 6.10.2, NOT assumed** — ⚠️ **six minor versions above the pin and the container's 6.4.2**
- [x] ✅ **Native build succeeds OUTSIDE a container** — ⚠️ **zero code changes under GCC 15.2 / CMake 4.2.3**
- [x] ✅ **`ctest` NON-ROOT, tests ON: 571/571** — ⚠️ **compared to 571 and MATCHING**; ✅ **18/18 Linux smokes too**
- [x] ✅ **The app LAUNCHED on a real session over RDP** and opened a project — ⚠️ **user-confirmed**; navigator, manuscript and timeline all nominal
- [x] ✅ **The setup steps are recorded AS RUN**, ready for §9 → **`docs/Scrivi_Linux_Rig_Setup_v0_1.md`** (2026-08-29). ⚠️ **§7 (drive dismount) is deliberately EMPTY** — it is T-0477's, and writing it now would be from documentation
- [x] ✅ **User verification GRANTED 2026-08-29** for T-0474, T-0475 and T-0476.

---

## 5. Explicitly OUT of scope

| Item | Where it goes |
| ---- | ------------- |
| ⚠️ **The drive pull / instrumentation** | **SP-124 (T-0477)** — ⚠️ **the sprint boundary is what keeps instrument-before-implement honest** |
| ⚠️ **`WorldVolumeStatus` for Linux** | **SP-124 (T-0478)** — ⚠️ **writing it now would be from documentation** |
| **Porting Outline §9 corrections** | **SP-124 (T-0479)** |
| ⚠️ **Any Linux object/UI surface** | **EP-035** |
| **CI automation** | ⚠️ **Out of scope for the Epic** |

---

*Last Updated: 2026-08-25 (**SP-123 🟡 ACTIVATED** — EP-038's first of two. ⚠️ **T-0474 is BLOCKED ON THE
USER** (account + SSH need physical access) **and gates the other two.** ⚠️ **The rig uses a REAL X
session — `xvfb`/`x11vnc` are the container's answer, not the rig's.** ⚠️ **Qt ≥ 6.4 must be CONFIRMED,
not assumed.** ⚠️ **`ctest` is expected at 571 — a different number is a finding.** ⚠️ **The drive pull is
deliberately in SP-124**, so instrument-before-implement is enforced by a sprint boundary. Next Task
**T-0480**; Issue **I-0173**.)*

---
---

---

*Closed 2026-08-29 by user approval, with its three Tasks verified in the same step.*

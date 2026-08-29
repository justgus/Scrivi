# Active Sprints

⚠️ **TWO Sprints are active, in parallel:**
**SP-123** (EP-038, the rig — ⚠️ **waiting on `oathkeeper` being AWAKE**) and
**SP-126** (EP-035, the three-tab Scene Inspector shell — ✅ **blocked on nothing**).

✅ **T-0474 is COMPLETE** — ⚠️ **the user built the rig on 2026-08-27**, before SP-123 was planned.
⚠️ **`oathkeeper` is ASLEEP, not broken.** ✅ **T-0475 (Claude, over SSH) can start the moment it wakes.**

✅ **SP-125 CLOSED 2026-08-29** → [`Closed/Sprint-SP-125.md`](Closed/Sprint-SP-125.md).

⚠️ **SP-125 delivered the first Linux object surface**, and ⚠️ **the user then found a gap it could not
have closed**: the panel's single "Scene Entities" tab is Apple's DELETED SP-090 placeholder, not the
real three-tab shell. ✅ **SP-126 now builds that shell** — EP-035 **AC10**.

---

## SP-123 — `[Linux]` ⚠️ **Rig reachable + building natively**

**Status:** 🟡 **ACTIVE**
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
---

## SP-126 — `[Linux]` ⚠️ **The three-tab Scene Inspector shell**

**Status:** 🟡 **ACTIVE** — activated 2026-08-29 by user approval
**Date Activated:** 2026-08-29
**Epic:** [EP-035](../Epics/Epic-active.md) — `[Linux]` Object Foundations · **sprint 2**
**Codebase:** `[Linux]` — Qt **Widgets** (`QTabWidget`, already the host)
**Closes:** **AC10** · **Tasks:** **T-0485 – T-0490** (six) · **Next available:** T-0492 (T-0491 = unscheduled sort/reorder)

⚠️ **Exists because the user found a GAP in EP-035's plan** (Epic §3a): the Epic never specified the
tab shell at all, and ⚠️ **SP-125 wired a PLACEHOLDER Apple had already deleted.**

### 1. Sprint Goal

**Give the Linux Scene Inspector its real three-tab shell — Writing · Worldbuilding · Properties —
and populate it in APPLE'S PROVING ORDER.**

⚠️ **"Scene Entities" is not a tab and never was in the finished Apple app.** It is the name of Apple's
SP-090 placeholder — *"a single segmented tab over a stub 'Scene Entities' body"* — which
`SceneInspectorView.swift:6` records SP-090 as REPLACING. ⚠️ **Linux copied the placeholder, not its
successor**, and SP-125 then wired the copy.

### 2. ⚠️ Two corrections found at PLANNING (2026-08-29)

#### ⚠️ 2.1 The tab ORDER in this sprint's own draft was WRONG

⚠️ **The draft said `Writing | Properties | Worldbuilding`.** ✅ **Apple's actual on-screen order is
`Writing | Worldbuilding | Properties`** — the tab bar renders `InspectorTab.allCases`
(`SceneInspectorView.swift:100`), which follows the enum's DECLARATION order
(`InspectorCard.swift:19-22`). ⚠️ **Caught by reading the enum rather than trusting the draft.**

✅ **`Writing` is the DEFAULT tab** (`InspectorLayoutStore.swift:78`).

⚠️ **The PROVING order and the DISPLAY order are different things.** ✅ **Display: Writing ·
Worldbuilding · Properties.** ✅ **Build/prove: Writing → Properties → Worldbuilding** (cheap surfaces
first, per the user). ⚠️ **Do not conflate them.**

#### ✅ 2.2 The layout schema ALREADY EXISTS and is ALREADY POPULATED

⚠️ **This is not a format Linux gets to invent.** `inspector-layout.json` sits at the project root,
carries `scrivi.inspector-layout.v1`, and ✅ **the user's real projects already contain it** — verified
in `the-lone-golem.scrivi`:

```
schema        : scrivi.inspector-layout.v1
selectedTab   : worldbuilding          ← project-level, NOT per-scene
inspectorHidden, defaultStacks, stackSort, scenes{ …4 scenes… }
```

⚠️ **It is APP-SIDE, not core-owned** — there is **no `scrivi_*` endpoint** for it; Apple reads and
writes the file directly (`InspectorLayoutStore.swift:155`). ⚠️ **`SceneMetaJson.hpp:53` explicitly
calls it "view configuration"** and keeps it out of scene metadata.

### 3. ✅ USER RULING (2026-08-29) — Linux READS AND WRITES it

> ✅ **Linux honours `scrivi.inspector-layout.v1` exactly, preserving unknown keys on write.**

⚠️ **PATCH, NEVER RECONSTRUCT** (Porting Outline §4.3, T-0436/T-0437). ⚠️ **Linux does not model
`stackSort` or per-scene stacks this sprint, so it MUST round-trip them untouched** — reconstructing
the document from a Qt struct would silently delete the writer's Apple-side card layout, and
⚠️ **the loss would be invisible until she opened the project on the Mac again.**

⚠️ **ACCEPTED RISK, recorded deliberately:** ⚠️ **two app-side implementations of one schema, with no
core to arbitrate.** ⚠️ **If a third platform needs it, MOVE IT INTO ScriviCore rather than writing a
third parser** — that is the point at which the duplication stops being affordable.

### 4. Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0485** | **The three-tab shell** — `Writing \| Worldbuilding \| Properties`, ⚠️ **in that DISPLAY order**, defaulting to **Writing** | **High** | 🔵 Planned |
| **T-0486** | ⚠️ **`inspector-layout.json` read + write** — ⚠️ **round-trip `stackSort`, `defaultStacks` and `scenes` UNTOUCHED**; persist `selectedTab` at PROJECT level | **High** | 🔵 Planned |
| **T-0487** | **Writing tab** — tags, outline, todo via `setSceneTags`/`setSceneOutline`/`setSceneTodo` | **High** | 🔵 Planned |
| **T-0488** | **Properties tab** — ⚠️ **read-only DERIVED stats** from `getSceneNotes`; ⚠️ **read-only IN FACT, not merely styled** (I-0148) | **High** | 🔵 Planned |
| **T-0489** | ⚠️ **MOVE SP-125's object list into Worldbuilding** — ⚠️ **no behaviour change; "Scene Entities" RETIRES** | **High** | 🔵 Planned |
| **T-0490** | ⚠️ **LIVE pass** (EP-035 AC9) + `ctest` non-root — ⚠️ **file every Issue found** | **High** | 🔵 Planned |

### 5. ✅ Blocked on nothing

`getSceneNotes` (`ScriviBridge.hpp:429`) · `setSceneOutline`/`setSceneTags`/`setSceneTodo` (453–455) are
**already bridged**. ✅ **Properties needs NO new call** — it is derived from the same `getSceneNotes`
result. ✅ **The layout file needs no endpoint at all** — it is app-side by design.

### 6. Definition of Done

- [ ] **Three tabs exist** in Apple's display order `Writing \| Worldbuilding \| Properties`,
      ⚠️ **defaulting to Writing**, ⚠️ **and "Scene Entities" is GONE**
- [ ] ⚠️ **`selectedTab` persists at PROJECT level and does NOT follow the scene** — ⚠️ **verified by
      REOPEN**, never by trusting the write
- [ ] ⚠️ **A project laid out on APPLE round-trips through Linux with `stackSort`, `defaultStacks` and
      per-scene `scenes` INTACT** — ⚠️ **this is the data-loss test, and it is the one that matters most**
- [ ] **Writing tab round-trips** tags / outline / todo to disk, ⚠️ **verified by REOPEN**
- [ ] **Properties shows derived stats**, ⚠️ **read-only in fact** (I-0148: a disabled Apple
      `TextEditor` was still editable)
- [ ] ⚠️ **The object list behaves EXACTLY as SP-125 left it**, in its new tab
- [ ] ⚠️ **No `ObjectKind` name is hardcoded** — ⚠️ **the standing rule does not lapse because this
      sprint is "just UI"**
- [ ] `ctest` ⚠️ **non-root, tests ON** — expected **571**
- [ ] ⚠️ **A LIVE click-through happened** (AC9), and every Issue found is FILED
- [ ] ⚠️ **User verification.** Claude may mark Tasks `Implemented - Not Verified` and nothing more

### 7. Explicitly OUT of scope

| Item | Where |
| ---- | ----- |
| ⚠️ **Per-stack sort + card drag-reorder** | ⚠️ **T-0491 (filed 2026-08-29, user-ruled OUT)** — ⚠️ **`stackSort` is still ROUND-TRIPPED, just not edited** |
| **Project-wide object browser** | Later EP-035 sprint (AC2's other half) |
| **Object CRUD / world binding UI** | Later EP-035 sprint (AC3/AC4) |
| **The Detail Sheet** | **EP-036** |
| **Relationships, sources** | **EP-037** |
| ⚠️ **Moving the layout schema into ScriviCore** | ⚠️ **Only when a THIRD platform needs it** — see §3 |

---

---

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

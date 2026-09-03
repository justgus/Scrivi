# SP-127: `[Linux]` EP-035 — The Worlds surface (Manage Worlds + RELINK) ✅ CLOSED

**Closed:** 2026-09-02 (user-approved) · **Activated:** 2026-08-31
**Epic:** [EP-035](../../Epics/Epic-active.md) — `[Linux]` Object Foundations · **sprint 3**
**Outcome:** ✅ **All five Tasks Verified 2026-09-02** after a live pass on the REAL RIG (build 29).
**Closes:** **AC3**

---

## ✅ What this sprint delivered

✅ **The Linux Worlds surface** — `Project ▸ Manage Worlds…`, gated on a project being open: bound
worlds listed with name, path and status; **Locate…** to repair a moved world; add-existing and
remove-reference with a confirmation stating the package is **not** deleted.

⚠️ **Why it mattered:** `addWorld`, `relinkWorld`, `getWorldStatus` and `getWorldBinding` had been
bridged across the ABI with **ZERO callers** — ⚠️ **a capability with no surface** — so ⚠️ **a project
whose world had moved could not be repaired from the Linux app at all.**

---

## ⚠️ THE SPRINT'S REAL VALUE WAS THE LIVE PASS

⚠️ **The delivery was routine. The live pass (T-0496) found FOUR defects, and NONE of them was
catchable by the suite as it stood.** ✅ **All four are Verified.**

| Issue | What | ⚠️ Why the suite could not see it |
| ----- | ---- | -------------------------------- |
| ⚠️ **I-0183** | 🔴 **DATA LOSS** — an available-but-UNREADABLE world licensed a prune. **10 of 12 relationships destroyed** in the user's real project | ⚠️ **§6 believed it was MITIGATED** — *"the core verifies `worldID`"*. ⚠️ **Wrong check: a COPY shares its `worldID`.** ⚠️ **And the trigger needed a REAL degraded network mount** |
| **I-0184** | The row's path was **CLIPPED**, losing the package name | ⚠️ **A LAYOUT defect — the string was correct throughout.** Only laying out at a real width shows it |
| **I-0185** | A package must be an **ATOM**; the dialog descended into it | ⚠️ **It still WORKED** (the right path came back), so nothing failed. ⚠️ **It was a papercut on the most common action in the app** |
| **I-0186** | Secondary text at **1.07:1** — invisible in **both** real themes | ⚠️ **Qt's NO-THEME fallback renders it a readable grey.** ⚠️ **Every offscreen check — and a screenshot offered as evidence — showed a path no real user ever saw** |

⚠️ **Two of the four needed the USER TO CORRECT CLAUDE'S DIAGNOSIS** before the real defect came into
view:

- **I-0184** — ⚠️ **blamed on a hardcoded 360 px elide.** ✅ **Measurement showed real paths are
  230–278 px and never reach it**; the **row layout** left the label 202 px for a 211 px path, so Qt
  **clipped** rather than elided.
- **I-0185** — ⚠️ **the first fix made descending RECOVERABLE (resolve the path back up) when the
  requirement was that it be IMPOSSIBLE.** ⚠️ **It passed every test written for it**, because those
  tested pure functions rather than the interaction.

✅ **Three NEW smokes exist because of this pass**, each verified to FAIL against the behaviour it
pins: `worlds_path_smoke`, `theme_contrast_smoke`, `package_folder_smoke`.

---

## ⚠️ Lessons this sprint paid for

1. ⚠️ **A HEADLESS RENDER IS NOT EVIDENCE ABOUT COLOUR.** Offscreen Qt uses a fallback palette that
   no user has. ⚠️ **The theme smoke now forces a real gtk3 theme and SKIPS LOUDLY if it cannot.**
2. ⚠️ **A test written from a wrong diagnosis certifies the wrong thing.** I-0185's first fix was
   green on its own tests while the defect was untouched.
3. ⚠️ **A mitigation written in a risk table is not a mitigation until it is exercised.** I-0183's
   check existed, ran, and was the wrong question.
4. ⚠️ **`palette(mid)` and `palette(link-visited)` are NOT text roles.** ✅ **Derive secondary text
   from `WindowText` on `Window`** — see `ThemeColours.hpp`.
5. ⚠️ **Packages are ATOMS in the app** even though they are directories on disk. ⚠️ **Invisible on
   Apple (Cocoa bundles); every Qt platform re-earns it — Windows next.**

---

## ⚠️ Deferred, deliberately

| Item | Where |
| ---- | ----- |
| ⚠️ **World CREATION** | **T-0497** — ⚠️ **this pass RAISED its priority.** ⚠️ **The user hit a DEAD END**: a project with no world has nothing to *add* or *relink*, and ⚠️ **the step-7 test could not be completed on Linux at all — the world had to be created in the APPLE app** |
| ⚠️ **`worldID`-equality weakness** | ⚠️ **Still real**: a copy shares `worldID` AND `modifiedAt`, so no field distinguishes revisions. ✅ **With I-0183 fixed, a wrongly-bound world can no longer DESTROY anything — only leave links pending** |
| ⚠️ **I-0181** (core reports `missing` for an unmounted volume) | ⚠️ **Unassigned, still LATENT.** ⚠️ **Now that Linux HAS a world surface, this Issue is worth revisiting** — the Worlds dialog is where a false `missing` would become visible |

---

## Sprint record as it stood at close

## SP-127 — `[Linux]` ⚠️ **The Worlds surface** — Manage Worlds + ⚠️ **RELINK**

**Status:** 🟡 **ACTIVE** — activated 2026-08-31
**Epic:** [EP-035](../Epics/Epic-active.md) — `[Linux]` Object Foundations · **Closes: AC3**
**Codebase:** `[Linux]` — QWidgets, mirroring Apple's `WorldsView.swift`
**Tasks:** **T-0492 – T-0496** (five) · **Next available:** T-0497 · Issue **I-0187** · ⚠️ **ADOPTED: I-0185**

⚠️ **SP-124 is PAUSED, not blocked** — ✅ **its S1 baseline is captured and the rig's `cifs` mount is
INTACT.**

---

## 1. Sprint Goal

⚠️ **Give Linux a Worlds surface at all.** ✅ **Today `addWorld`, `relinkWorld`, `getWorldStatus` and
`getWorldBinding` are bridged with ZERO callers** — ⚠️ **`capability_without_surface` in its exact
form**, and the only `listWorlds` consumer (`EditorShell.cpp:1806`) merely recovers a display NAME for
an error string.

⚠️ **RELINK IS THE POINT.** ✅ **EP-035 AC3 already says it should be built FIRST**, and the reason is
concrete: ⚠️ **a project whose world moved currently CANNOT BE REPAIRED FROM THE LINUX APP AT ALL.**
⚠️ **Relink is the only affordance that turns an honest error into a recoverable one.**

## 2. ⚠️ Why this sprint preempted SP-124 — user ruling 2026-08-31

> ✅ ***"The App won't incorrectly represent the mount point until it can correctly represent the
> mount point."***

⚠️ **SP-124 could measure OS-level drive-loss signals but could never SHOW them to a writer**, because
⚠️ **there is no surface on which a world's status appears.** ✅ **Consequence, recorded honestly:**
⚠️ **I-0181 was re-scoped from a `[Linux]` reporting defect to a LATENT `[ScriviCore]` resolution
defect** — ⚠️ **it cannot reach a Linux writer today, because nothing reports `missing` to her.**

✅ **Building the surface FIRST means SP-124's scenarios get verified against something readable.**

## 3. Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0492** | **`WorldsDialog` — the list.** Name, path, status, ⚠️ **`available` vs not**, empty state | **High** | 🔵 Not started |
| **T-0493** | ⚠️ **RELINK ("Locate…")** — ⚠️ **the sprint's REASON.** `QFileDialog` → `relinkWorld` → refresh | **High** | 🔵 Not started |
| **T-0494** | **Project ▸ Manage Worlds…** menu wiring + ⚠️ **`editorOnlyActions_` gating** | **High** | 🔵 Not started |
| **T-0495** | **Add existing world** + ⚠️ **remove reference, with a confirmation that says the package is NOT deleted** | Medium | 🔵 Not started |
| **T-0496** | ⚠️ **Live pass on the REAL RIG** + `ctest` + smoke | **High** | 🟠 **RUN 2026-09-01 — steps 1,3,4,5,6 PASS; step 2 and step 7 FAIL** → ⚠️ **I-0183 (DATA LOSS), I-0184, I-0185.** ⚠️ **NOT complete** |

⚠️ **`createWorld` is DEFERRED**, not forgotten: ⚠️ **Apple pairs it with `WorldBookmarkStore`
(a sandbox-grant concern with NO Linux equivalent)**, and ✅ **AC3 asks for "added, status read,
relinked" — not created.** ⚠️ **Filed as T-0497 unscheduled.**

## 4. ⚠️ What to mirror, and what NOT to

✅ **Mirror the FINISHED Apple surface** (`feedback_mirror_the_finished_surface_not_the_placeholder`) —
`Scrivi/Views/WorldsView.swift` (327 lines):

| ✅ Mirror | ⚠️ Why |
| -------- | ----- |
| Row = icon + name + path + status + actions | Path is ⚠️ **middle-truncated** — the interesting part of a world path is the END |
| ⚠️ **"Locate…" ONLY when status ≠ available** | ⚠️ **"Relinking a world that resolves fine is not a repair, it is a way to bind the WRONG package"** |
| ⚠️ **Status text via the SHARED phrasing** | ✅ **`SceneInspector.cpp:678-684` ALREADY mirrors Apple's `writerDescription` word for word** — ⚠️ **REUSE IT, do not restate it** (`feedback_look_for_existing_pattern_first`) |
| Remove = ⚠️ **reference only**, confirmation names the world | ⚠️ **"Remove" next to a world she just built reads as "delete my world"** |

| ⚠️ Do NOT mirror | ⚠️ Why |
| ---------------- | ----- |
| `WorldBookmarkStore` | ⚠️ **macOS sandbox grants. NO Linux equivalent.** ⚠️ **Porting it would be cargo cult** |
| `NSOpenPanel`/`NSSavePanel` | ✅ **`QFileDialog::getExistingDirectory`** |
| "Remove All World References" | ⚠️ **Apple deliberately omits it too** |

### ⚠️ The refresh rule — I-0130, and it is the trap

> ⚠️ **"Every mutating action must call `onWorldsChanged()`, not just `load()`."**

⚠️ **Apple shipped this defect:** a relink repaired the world while ⚠️ **the project-wide warning kept
insisting it was missing until the writer changed scenes.** ⚠️ **Linux has the SAME split** — the dialog
would own its list while `SceneInspector`'s status line lives elsewhere. ✅ **Refresh BOTH.**

## 5. Definition of Done

✅ **ALL MET — user-verified 2026-09-02 on build 17.** ⚠️ **Four defects were found getting here; see §5b.**

- [x] **Project ▸ Manage Worlds…** opens a real dialog, ⚠️ **gated on a project being open**
- [x] Bound worlds listed with name, path, status; ⚠️ **empty state is explicit**, never a blank box — ⚠️ **took TWO fixes (I-0184 clipping, I-0186 contrast)**
- [x] ⚠️ **"Locate…" appears ONLY for a non-available world** and ⚠️ **repairs a MOVED world end to end**
- [x] ⚠️ **After a relink, the SceneInspector status refreshes WITHOUT changing scenes** (⚠️ **I-0130**)
- [x] Add-existing and remove-reference work; ⚠️ **removal confirmation states the package is NOT deleted** — ⚠️ **picker defect I-0185 remains OPEN**
- [x] ⚠️ **Status phrasing REUSES the existing mirror**, ⚠️ **not a second copy** — ✅ `SceneInspector::writerDescription` delegates to `WorldStatusText`
- [x] `ctest` + smokes GREEN on the rig, ⚠️ **non-root** — **572/572 + 21/21**
- [x] ⚠️ **LIVE PASS on the REAL RIG** — ⚠️ **`--version` checked FIRST** (`feedback_confirm_the_build_under_test`)
- [x] `.cpp`/`.hpp` added → ⚠️ **`platforms/linux/CMakeLists.txt` updated** (⚠️ **NOT pbxproj — Linux is CMake-only**)

✅ **Step 7's UI half is CLOSED** — the user drove the rejection through the dialog by hand on
2026-09-02 (§5b).

✅ **`I-0185` ADOPTED and now VERIFIED** — user ruling 2026-09-02 (*"keep SP-127 active and continue
working on I-0185"*), ✅ **verified the same day: *"It works perfectly."*** ⚠️ **It was NOT a DoD item —
the DoD was met without it** — but the user's clarification widened it from one Worlds affordance to
⚠️ **THREE sites including `Open Project`, the app's most common action**, and ⚠️ **their second
correction invalidated my first fix entirely** (making descent *recoverable* when the requirement was
that it be *impossible*).

✅ **ALL SP-127 WORK IS COMPLETE.** ⚠️ **Sprint close requires the user's approval.**

## 5b. ⚠️ LIVE PASS RESULT (T-0496, 2026-09-01) — ⚠️ **step 7 found DATA LOSS**

⚠️ **Run by the USER on `oathkeeper`, build 8** (`--version` confirmed FIRST, per
`feedback_confirm_the_build_under_test`).

| Step | What | Result |
| ---- | ---- | ------ |
| 1 | Menu gated on a project being open | ✅ **PASS** |
| 2 | Name + **path** + status per row | ✅ **PASS — user-VERIFIED 2026-09-02 on build 17.** ⚠️ **Took TWO fixes**: **I-0184** (the row LAYOUT clipped the tail — not the 360 px constant) and ⚠️ **I-0186** (the path was rendered at **1.07:1** contrast in dark and **1.03:1** in light — ⚠️ **invisible in both real themes**) |
| 3 | "Locate…" ONLY when status ≠ available | ✅ **PASS** |
| 4 | Relink repairs a moved world end to end | ✅ **PASS** — ⚠️ **both `the-lone-golem` AND `the-stairs-of-tintagael`**, persisting through quit/reload |
| 5 | ⚠️ **I-0130** — inspector refreshes after relink WITHOUT a scene change | ✅ **PASS** |
| 6 | Add-existing + remove-reference | ✅ **PASS** — ⚠️ **with a file-dialog defect: I-0185, ADOPTED into this sprint and ✅ VERIFIED at build 29** |
| 7 | ⚠️ **Wrong-package relink is REJECTED** | ✅ **PASS — user-VERIFIED THROUGH THE UI 2026-09-02.** ✅ **Locate… → `Alpha Centauri.scrivworld` REJECTED; → `Eskandar.scrivworld` ACCEPTED and the pending notices CLEARED.** ⚠️ **The original run could never have passed — it used a COPY, which shares its `worldID`** → that run found **I-0183** (🔴 data loss) instead |
| 8 | `ctest` + smokes, non-root | ✅ **PASS — 572/572 + 21/21 smokes on the rig (build 17), non-root.** ⚠️ **Two of those smokes are NEW and exist because this pass found what the suite could not**: `worlds_path_smoke`, `theme_contrast_smoke` |

### 🔴 ⚠️ Step 7 — the sprint's OWN risk mitigation was insufficient

⚠️ **§6 lists this exact risk** — *"Relink accepting the wrong package"* — ✅ **mitigated as "the CORE
verifies `worldID` (`scrivi.h`)".** ⚠️ **The verification EXISTS and RAN. It is the WRONG CHECK:**
⚠️ **a COPY of a world carries the SAME `worldID`**, so `WorldStore::relink`'s guard — whose comment
says it prevents *"a same-named package being substituted"* — ⚠️ **cannot detect the one case it names.**

⚠️ **Consequence: `the-lone-golem` lost 10 of 12 relationships** (`relationships.jsonl` 3620 B → 491 B),
⚠️ **including Myton→Ch1-Sc1 and Petch.** ✅ **The world package was UNDAMAGED** — all 35 objects,
matching objectIDs — ⚠️ **so the PROJECT was damaged by reconciling against a healthy world.**

✅ **Recovery source verified** (same `projectID`, strict superset of what survives):
`~/ScriviLinux/projects/the-lone-golem.scrivi/objects/relationships.jsonl`.
⚠️ **NOT restored by Claude — the write to the user's real writing work was left to the user.**

### ✅ Step 7 RE-RUN 2026-09-01 (build 9) — ⚠️ **and what the first run could never have tested**

⚠️ **The original step 7 used `Eskandar.scrivworld copy` — a COPY.** ⚠️ **A copy carries the SAME
`worldID` (and the same `modifiedAt`), so `WorldStore::relink`'s identity check CANNOT reject it.**
⚠️ **The test as designed could not pass, and what it actually exercised was the reconcile path —
which is how it found DATA LOSS (I-0183) instead.**

✅ **A genuinely different world was needed, and the user created one** (`Alpha Centauri`, bound to
`the-sentinel-of-centauri`) — ⚠️ **which required the APPLE app; see the finding below.**

| Case | Result |
| ---- | ------ |
| Relink `Alpha Centauri` → **the Eskandar package** (wrong world) | ✅ **REJECTED** — `worldIDMismatch`, ⚠️ **naming BOTH worldIDs**, and ✅ **the binding was left UNCHANGED** |
| Relink `Alpha Centauri` → **its own package** (control) | ✅ **SUCCEEDS** — so the guard is not simply refusing everything |

⚠️ **Verified at the CORE through `scrivi_relink_world`, on DISPOSABLE COPIES.** ⚠️ **The dialog's
handling of that rejection is still UNTESTED** — §6 requires the surface *"surface the rejection, never
suppress it"*, ✅ **and the core's message is specific enough to show verbatim.**

### ✅ Step 7 — UI HALF CLOSED 2026-09-02, and it closed I-0183's loop too

⚠️ **The user drove the whole chain by hand on the rig**, which is the only test that
covers surface AND core together:

1. Renamed `Eskandar-2.scrivworld` → `Eskandar.scrivworld` on the volume and reloaded.
2. ✅ **The Scene Inspector said *"Objects held pending"*, NOT *"missing"*.** ⚠️ **This is
   I-0183's fix observed FROM THE WRITER'S SEAT** — the world was unreadable at the bound
   path, and the app HELD the links instead of pruning them. ⚠️ **Before the fix this exact
   state destroyed 10 of 12 relationships.**
3. ✅ **Locate… → `Alpha Centauri.scrivworld` → REJECTED**, and ⚠️ **the dialog SURFACED the
   rejection** — §6 required *"surface the rejection, never suppress it"*.
4. ✅ **Locate… → `Eskandar.scrivworld` → ACCEPTED**, and ✅ **the pending notices CLEARED
   without a scene change** — ⚠️ **which re-confirms the I-0130 refresh rule (step 5) on a
   path nobody had exercised: recovery, not repair.**

✅ **Data verified intact afterwards: `the-lone-golem` 12 live edges, `the-stairs-of-tintagael`
46.** ⚠️ **Nothing was pruned across the rename, the pending state, or the relink.**

⚠️ **Note what this run needed that the first one lacked: a GENUINELY DIFFERENT world.** ✅ **The
user had to CREATE one — in the APPLE app, because Linux cannot (see the finding below).**

### ⚠️ FINDING — ⚠️ **Linux cannot CREATE a world at all, and the writer hits it immediately**

⚠️ **Found by the user 2026-09-01 while trying to set up this very test.** ⚠️ **`Project ▸ Manage
Worlds…` offers NO create affordance**, so a Linux-only writer with a project that has no world
⚠️ **cannot proceed.** ✅ **Apple DOES handle it** — it detects a project with no world and ⚠️ **prompts
the writer to create one**, which is how `Alpha Centauri` was made.

✅ **This is T-0497, already filed and unscheduled** — ⚠️ **but the live pass changes its priority
argument.** ⚠️ **§7 deferred creation as "not asked for by AC3"**, which is true — ⚠️ **but the
deferral assumed adding and relinking were enough to be USABLE.** ⚠️ **They are not: a writer whose
project has no world has NOTHING to add or relink**, and the first Linux-only project would dead-end.
⚠️ **The test itself could not be completed on Linux alone**, which is the sharpest evidence available
that this is a gap rather than a nicety.

### ⚠️ What this says about the DoD

⚠️ **Steps 2 and 7 are DoD checkboxes**, so ⚠️ **T-0496 does NOT pass and SP-127 is NOT done.**
✅ **Steps 1, 3, 4, 5, 6 hold** — ⚠️ **including I-0130, the trap the sprint was most worried about.**
⚠️ **The trap that fired was the one believed already mitigated.**

## 6. ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **A second copy of the status phrasing** | ⚠️ **The mirror EXISTS** at `SceneInspector.cpp:678-684`. ✅ **HOIST it to a shared helper** — ⚠️ **a restated list is a defect on sight** |
| ⚠️ **Relink refresh misses the inspector** (I-0130) | ⚠️ **Apple SHIPPED this.** ✅ **Wire the app-wide refresh in T-0493, not as follow-up** |
| ⚠️ **`.scrivworld` is a DIRECTORY** | ✅ **`getExistingDirectory`.** ⚠️ **A file picker cannot select it** |
| ⚠️ **Relink accepting the wrong package** | ✅ **The CORE verifies `worldID` before accepting** (`scrivi.h`) — ⚠️ **surface the rejection, never suppress it** |
| ⚠️ **Verifying needs an unavailable world** | ✅ **The rig has one**: the `ScriviLinux` projects bind Eskandar at a macOS path ⚠️ **that does not resolve on Linux** — ✅ **a genuine unavailable world, no setup needed** |

## 7. ⚠️ Out of scope

| Item | Where |
| ---- | ----- |
| ⚠️ **World CREATION** | ⚠️ **T-0497, unscheduled** — needs the grant question ruled |
| **Object CRUD** | **EP-035 AC4** |
| ⚠️ **I-0181** (core `missing` on unmounted volumes) | ⚠️ **Unassigned, LATENT** — ⚠️ **NOT this sprint's** |
| **Drive-loss scenarios** | ⏸️ **SP-124, PAUSED** |


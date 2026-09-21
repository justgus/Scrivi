---
sprint: SP-142
epic: EP-041
status: Planning
task: T-0537
planned: 2026-09-21
---

# SP-142 — ⚠️ **Retire Linux's duplicate `InspectorLayoutStore`**

**Status:** 🔵 **PLANNING — ⛔ NOT ACTIVATED.** ✅ **UNBLOCKED 2026-09-21: [SP-141] is IMPLEMENTED.**

✅ **Q1 WAS ANSWERED BEFORE [SP-141] IMPLEMENTED, exactly as this plan asked** — ⚠️ **ruled
2026-09-21, "core reports, app decides":** `status` = `ok` | `absent` | `unreadable`, ⛔ **the core
never invents defaults and never overwrites a corrupt file on read**, ✅ **a PUT over one succeeds.**
⚠️ **So S1's gate is already discharged and the endpoints below are real:**
`scrivi_get_inspector_layout(projectRootPath)` and
`scrivi_put_inspector_layout(projectRootPath, documentJson)`.
✅ **The smoke test's last two assertions (missing → defaults, corrupt → not overwritten) map onto
`absent` and `unreadable` and need no rewrite** — ⚠️ **only a repoint.**

⚠️ **THIS FILE SUPERSEDES AN EARLIER PLAN OF THE SAME NAME (2026-09-21).** ⛔ **The earlier version was
filed under `epic: EP-040`** — ⚠️ **stale: [SP-142] moved to [EP-041] when the Epic was split out on
2026-09-18.** ✅ **Three things it said are KEPT and carried below, because they were right and the
rewrite would otherwise have lost them:** ✅ **the drift is ALREADY PROVEN by [I-0215] (§Why);**
✅ **the TOMBSTONE requirement (AC5);** and ✅ **the rule that [I-0197] Class B closes HERE, not in
[SP-141] (§Goal).** ⚠️ **It also named the Apple store as 310 lines; ✅ measured 2026-09-21 it is
**380** (`InspectorLayoutStore.swift`) against Linux's **145 + 82**.**
**Epic:** 🟡 [EP-041] `[Cross]` **The Boundary** → [`../Epics/Epic-EP-041.md`](../Epics/Epic-EP-041.md)
**Task:** **T-0537** — ⚠️ **ID reserved, not yet issued into `Task-active.md`** (issued at activation).
**Serves:** **[EP-041] AC3** · contributes to **AC5** ([I-0197] Class B)
**Codebase:** `[Linux]` — ⚠️ **`platforms/linux/` only.** ⛔ **No ScriviCore change: [SP-141] builds the
endpoints; ✅ this Sprint only CONSUMES them.**

---

## ✅ THE BLOCKER IS GONE — ⚠️ **this section is now HISTORY, kept deliberately**

✅ **[SP-141] WAS IMPLEMENTED AND T-0507 USER-VERIFIED LATER THE SAME DAY (2026-09-21).**
⚠️ **This section is kept because it is the reason Q1 got asked in time** — ✅ **planning a BLOCKED
sprint is what surfaced the absence-semantics question, and asking it before T-0507 implemented cost
nothing.** ⛔ **The three findings below were TRUE WHEN WRITTEN and are all now superseded.**

⚠️ **AS WRITTEN (morning, 2026-09-21): [SP-141] IS ACTIVATED BUT NOT STARTED.** ✅ **Verified three ways:**

1. ✅ **`Task-active.md` recorded T-0507 as 🔵 NOT STARTED.** ⚠️ **(That row is GONE — T-0507 was
   Verified 2026-09-21 and archived to
   [`../Tasks/Verified/Task-verified-0507.md`](../Tasks/Verified/Task-verified-0507.md).)**
2. ✅ **`grep inspector_layout ScriviCore/include/scrivi/scrivi.h` returns NOTHING** — ⛔ **the endpoints
   this Sprint consumes DO NOT EXIST.**
3. ✅ **`Scrivi/App/InspectorLayoutStore.swift` still does its own file I/O** —
   `URL(fileURLWithPath:)` at **:175**, `Data(contentsOf:)` at **:192**, `write(to:.atomic)` +
   `replaceItemAt` at **:368–370.** ⚠️ **That IS the [I-0197] bypass T-0507 exists to remove.**

⚠️ **SO THIS PLAN IS PLANNING, NOT READINESS.** ⛔ **SP-142 cannot activate until T-0507 lands.**
✅ **Planning it now is still worth doing** — ⚠️ **it surfaces the one question T-0507 must answer FOR
this Sprint (Q1 below), and asking it while T-0507 is unstarted is free; asking it afterwards costs a
second endpoint revision.**

---

## ✅ Goal

⚠️ **Linux stops being the SECOND app-side owner of `scrivi.inspector-layout.v1`.** ✅ **The Qt store's
file I/O is deleted and its two behaviours are served by the [SP-141] core endpoints** — ✅ **so one
schema has exactly one owner, which is the whole of [EP-041].**

⚠️ **THE DRIFT IS NOT HYPOTHETICAL — IT HAS ALREADY HAPPENED.** ⛔ **[I-0215]: Linux PRESERVED unknown
keys and Apple DROPPED them.** ✅ **Same file, same schema, two behaviours, no shared owner** — ⚠️ **and
Linux was the one that got it RIGHT.** ⛔ **Retiring Apple's store alone would have left the file with
two writers still.**

✅ **[I-0197] CLASS B CLOSES HERE, NOT IN [SP-141].** ⚠️ **Stated explicitly because the Apple half
lands first and will look like the finish.**

---

## ⚠️ What is actually being retired — measured 2026-09-21, not estimated

✅ **The surface is SMALL and fully enumerated.** ⚠️ **That is the good news and it should be stated
plainly: this is a ~145-line class with THREE call sites.**

| Site | What it does | ⚠️ Disposition |
| ---- | ------------ | ------------- |
| `src/InspectorLayoutStore.cpp` (145 lines) + `.hpp` (81) | reads/writes `inspector-layout.json`, patch-not-reconstruct, atomic temp+rename | ⚠️ **File I/O DELETED.** ⛔ **See Q1 — the CLASS may survive as a thin caller** |
| `src/SceneInspector.cpp:315` | `layout_.load(projectRootPath)` | ✅ **→ core get** |
| `src/SceneInspector.cpp:318` | `tabIndex(layout_.selectedTab())` | ✅ **reads the fetched document** |
| `src/SceneInspector.cpp:130` | `layout_.setSelectedTab(tabID(index))` | ✅ **→ core put (patched document)** |
| `src/SceneInspector.hpp:257` | `InspectorLayoutStore layout_;` member | ⚠️ **stays or becomes a bridge call — Q1** |
| `tests/inspector_layout_smoke.cpp` | ⚠️ **THE DATA-LOSS TEST** | ⛔ **DO NOT DELETE — see below** |
| `CMakeLists.txt` ×4 blocks (lines 64, 675, 734, 803) | source listed in four targets | ✅ **mechanical** |

⚠️ **ONLY TWO BEHAVIOURS ARE IN USE: `selectedTab()` and `setSelectedTab()`.** ✅ **Everything else in
that 145-line class exists to protect keys Linux does not model** — ⚠️ **which is precisely the job
moving into the core.**

---

## Acceptance Criteria

- [ ] **AC1** — ⛔ **`platforms/linux/` contains ZERO direct reads or writes of `inspector-layout.json`.**
      ✅ **Provable by grep**, ⚠️ **and the grep is the AC, not a description of it:**
      `grep -rn "inspector-layout" platforms/linux/src/` returns **only** comments or nothing.
- [ ] **AC2** — ✅ **Tab selection still persists across an app restart on Linux**, ⚠️ **via the core.**
      ⛔ **Behaviour is UNCHANGED from the writer's point of view — this is a refactor, not a feature.**
- [ ] **AC3** — ⚠️ **THE LOSSLESS RULE STILL HOLDS, PROVEN AGAINST A REAL APPLE DOCUMENT.**
      ✅ **An Apple-written layout carrying `stackSort`, `defaultStacks` and per-scene `scenes`
      round-trips through a Linux tab change with EVERY key intact.** ⛔ **This is the AC that must not
      be weakened; see "the test" below.**
- [ ] **AC4** — ✅ **`InspectorLayoutStore.cpp`'s file I/O is GONE** (⛔ **no `QFile`, no temp+rename**),
      ✅ **and the four CMake blocks agree with whatever remains.**
- [ ] **AC5** — ⚠️ **A TOMBSTONE COMMENT is left where the duplicate was**, ✅ **in the form [SP-129]
      used at `EditorShell.cpp:2571`** (*"`readImportedTimelineFile` WAS HERE and is DELETED… ⛔ do not
      reintroduce a direct read"*). ⚠️ **It must name the Sprint/Task, say what replaced it, and forbid
      reintroduction** — ✅ **the whole point is that the next porter finds the rule at the site.**
- [ ] **AC6** — ✅ **Docker build clean; `ctest` green ON LINUX, run NON-ROOT with tests ON.**
      ⚠️ **`project_linux_container_tests_off`: the Dockerfile builds `SCRIVI_BUILD_TESTS=OFF`, so
      "the container is green" does NOT mean `ctest` ran.** ⛔ **Run it in the second image.**
- [ ] **AC7** — ⚠️ **A LIVE PASS ON THE REAL RIG:** open a project, switch inspector tabs, quit, relaunch,
      ✅ **confirm the tab came back** — ⚠️ **then open the SAME project on the Mac and confirm the card
      layout is intact.** ⚠️ **Confirm the build first** (`scrivi_linux --version`,
      `feedback_confirm_the_build_under_test`).

---

## ⚠️ THE TEST IS THE MOST IMPORTANT THING IN THIS SPRINT

⛔ **`tests/inspector_layout_smoke.cpp` MUST NOT BE DELETED WITH THE CLASS IT TESTS.**

✅ **Its own header calls it "THE SPRINT'S DATA-LOSS TEST"** and its fixture is ⚠️ **a REAL Apple-written
layout taken from `the-lone-golem.scrivi`** (35 world objects, 4 scenes with per-scene stacks). ✅ **It
asserts five things, and every one of them is still true after the retirement** — ⚠️ **they are
assertions about the SCHEMA's behaviour, not about the Qt class:**

- ✅ an Apple document round-trips with **every key intact** after a `selectedTab` change;
- ✅ `selectedTab` actually persists;
- ⚠️ an **unknown tab degrades** to `writing` rather than failing the load;
- ✅ a **missing file** yields Apple's ruled defaults rather than an error;
- ⛔ a **corrupt file is NOT overwritten** — ⚠️ **the damaged layout is left for a human.**

⚠️ **REPOINT IT AT THE CORE PATH; DO NOT REWRITE ITS EXPECTATIONS.** ✅ **A test that keeps its
assertions and changes only its subject is exactly the independent witness this Epic has twice paid for
lacking ([I-0214], [I-0215]).**

⚠️ **AND THE LAST TWO ASSERTIONS ARE A REAL QUESTION FOR [SP-141], NOT A FORMALITY.** ⛔ **"Missing file
→ defaults" and "corrupt file → do not overwrite" are BEHAVIOURS, and under the opaque get/put ruling
the CORE now owns them.** ⚠️ **If the core returns an error for a missing document instead of an empty
one, ✅ AC2's defaults move into Qt and this test must say so.** ⚠️ **That is Q1.**

---

## ⛔ Questions this Sprint owes — ⚠️ **Q1 is for [SP-141], and asking it late is expensive**

1. ⚠️ **Q1 — WHAT DOES THE CORE RETURN FOR A MISSING OR CORRUPT LAYOUT DOCUMENT?**
   ⛔ **The [SP-141] ruling settled the SHAPE (one opaque get/put) but NOT the ABSENCE SEMANTICS.**
   ✅ **Three cases, and the Linux store currently answers all three itself:** missing → **defaults**;
   corrupt → ⚠️ **leave it alone, do not overwrite**; unknown tab → **degrade to `writing`**.
   ⚠️ **The third is unambiguously the app's (the core does not know what a tab is).** ⛔ **The first two
   are NOT obvious** — ✅ **the ruling says the core owns "atomicity, durability and REPAIR",** ⚠️ **and
   "do not overwrite a corrupt document" is a repair decision.**
   ✅ **RAISE THIS WITH [SP-141] BEFORE IT IMPLEMENTS**, ⚠️ **not after: `project_envelope_empty_vs_failed`
   records that an empty envelope is already ambiguous between "empty" and "failed", and a missing
   document is exactly that ambiguity again.**

2. ⚠️ **Q2 — Does the `InspectorLayoutStore` CLASS survive as a thin caller, or is it deleted outright?**
   ✅ **Keeping it** means `SceneInspector` is untouched (3 call sites keep their shape) and the Qt-side
   tab-string mapping keeps a home. ⛔ **Deleting it** means `SceneInspector` calls `ScriviBridge`
   directly, matching how every other core call is made on Linux.
   ⚠️ **RECOMMENDATION: KEEP IT, GUT IT.** ✅ **`feedback_look_for_existing_pattern_first` —** ⚠️ **a
   class that holds the decoded document and maps tab strings is doing app-side work the core explicitly
   does NOT do under the opaque ruling.** ⛔ **What must die is its FILE I/O, not its existence.**
   ⚠️ **This also keeps [SP-143]'s guard honest: the guard polices `QFile`/`inspector-layout` in
   `platforms/linux/`, not a class name.**

3. ⚠️ **Q3 — Is the bridge wrapper added in [SP-141] or here?** ✅ **The endpoints are ScriviCore's
   ([SP-141]); ⚠️ the `ScriviBridge` Q_INVOKABLE wrappers are `platforms/linux/`.** ⛔ **If [SP-141]
   adds them for symmetry with the Swift side, this Sprint shrinks to consumption only.**
   ✅ **Assume HERE unless [SP-141] says otherwise** — ⚠️ **and the Sprint's estimate below assumes it.**

---

## Plan of work

| Step | Work | ⚠️ Note |
| ---- | ---- | ------ |
| **S1** | ⚠️ **Confirm the [SP-141] endpoints exist and answer Q1** | ⛔ **Gate. Do not start on a promise** |
| **S2** | ✅ Add the two `ScriviBridge` Q_INVOKABLE wrappers (get/put) | ⚠️ **Follow the existing alphabetical `Q_INVOKABLE QVariantMap` pattern** (`ScriviBridge.hpp:508-519`) |
| **S3** | ✅ Gut `InspectorLayoutStore`: file I/O out, bridge calls in | ⚠️ **Keep patch-not-reconstruct** — ⛔ **the document still round-trips whole** |
| **S3b** | ⚠️ **Leave the TOMBSTONE comment at the deletion site** | ⚠️ **AC5** — ✅ **[SP-129]'s form, `EditorShell.cpp:2571`** |
| **S4** | ✅ Repoint `inspector_layout_smoke` at the new path | ⛔ **Assertions UNCHANGED** — ⚠️ **AC3** |
| **S5** | ✅ CMake: reconcile the four blocks | ✅ Mechanical |
| **S6** | ✅ Docker build + `ctest` NON-ROOT, tests ON | ⚠️ **AC6** |
| **S7** | ⚠️ **Live pass on the rig, incl. the Mac cross-check** | ⚠️ **AC7 — the only thing that proves AC3 for a writer** |

---

## ⚠️ Risks

- ⛔ **THE BLOCKER.** ⚠️ **[SP-141] is unstarted, and SP-142 + SP-143 both queue behind it** — ✅ **the
  Epic's own record already flags this: *"a delay there stalls three quarters of the Epic."***
- ⚠️ **Q1 answered late = a second endpoint revision.** ✅ **Cheap to ask now, and it is the reason this
  plan was worth writing before the blocker cleared.**
- ⚠️ **Deleting the smoke test with the class.** ⛔ **It is named after the class it tests, which makes it
  look like collateral.** ✅ **It is the Epic's best evidence and its fixture is real Apple data.**
- ⚠️ **AC3 declared green on the round trip alone.** ⛔ **A Linux-only round trip proves the bytes
  survive Linux; ✅ only the MAC CROSS-CHECK proves the writer's layout survived.**
  ⚠️ **`feedback_live_pass_finds_what_suites_cannot`.**
- ⚠️ **Four CMake blocks, not one.** ✅ **Easy to fix three and miss the fourth** (lines 64, 675, 734, 803).

---

## ⛔ Out of scope

- ⛔ **The regression guard** — ✅ **[SP-143] / T-0510.** ⚠️ **Deliberately separate: a guard written by
  the same change it polices has no independent witness**, ✅ **and its allow-list must contain
  `InspectorLayoutStore` BEFORE this Sprint and not after** — ⚠️ **so writing it here would make it pass
  green across the very change it exists to detect.**
- ⛔ **Apple-side work** — ✅ **[SP-141] / T-0507.**
- ⛔ **Any other [I-0197] bypass site.** ⚠️ **Class B is this file only.**
- ⛔ **Nested-key identity inside card entries** — ⚠️ **[SP-140]'s AC1 deferred it to [SP-141].**

---

## ⚠️ Estimate

✅ **ONE short sprint — S2–S5 are ~a day of mechanical work over a 145-line class with three call sites.**
⚠️ **S1 and S7 are the real cost**: ⛔ **S1 is a dependency that does not exist yet**, and ⚠️ **S7 needs
the rig AND a Mac cross-check.**
⚠️ **STATED PLAINLY: the tracking overhead here is large relative to the code** — ✅ **the restructure
record already conceded this** (*"more tracking overhead for ~450 lines of production code"*), ⚠️ **and
the defensible alternative of folding [SP-143] in was considered and rejected for the witness reason
above.**

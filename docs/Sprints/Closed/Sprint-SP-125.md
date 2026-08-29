# SP-125: `[Linux]` EP-035 — The Scene Inspector, wired to real objects ✅ CLOSED

**Closed:** 2026-08-29 (user-approved) · **Activated:** 2026-08-25
**Epic:** [EP-035](../../Epics/Epic-active.md) — `[Linux]` Object Foundations · **sprint 1 of 3–4**
**Outcome:** ✅ **All five Tasks Verified 2026-08-28** after a live click-through against real work.

---

## ✅ What this sprint delivered

⚠️ **The FIRST object surface the Linux app has ever had.** `SceneInspector` had been a **67-line stub**
since EP-024/SP-078 — *"wired to NO project data… a muted 'No entities yet.' empty state, and a DISABLED
'Add Entity' button."*

- ✅ `ObjectKindScope` — kind scope **DERIVED** from `scrivi_list_object_kinds`, ⚠️ **grep-proven**: no
  `ObjectKind` name anywhere in `platforms/linux/`. ⚠️ **A restatement would have been OCCURRENCE NINE.**
- ✅ The object list, grouped by kind in the **core's** kind order, read via the scene's EDGES.
- ✅ **Two open affordances**, verified separately; ✅ **`worldID` proven threaded both ways.**
- ✅ **Four distinct explained states** — including one the plan did not anticipate.
- ✅ `ctest` **571/571 non-root, tests ON**; all **18** Linux smokes pass.

## ⚠️ Closing caveat — recorded so the archive does not mislead

⚠️ **The DoD line reads *"the Scene Entities tab lists real objects"*, and that was met — but
"Scene Entities" was the WRONG TAB TO BE WIRING.** ✅ **Found by the user 2026-08-29** (EP-035 §3a):
it is the name of Apple's **SP-090 placeholder**, which Apple itself deleted when it built the real
three-tab shell (Writing | Worldbuilding | Properties). ⚠️ **Linux had copied the placeholder rather
than its successor**, and this sprint wired the copy.

✅ **The WORK is sound and creates no rework** — the object list is Worldbuilding-tab content and moves
into its proper tab intact. ⚠️ **Only the inherited NAME was wrong.** **SP-126** builds the shell and
retires "Scene Entities" (EP-035 **AC10**).

⚠️ **AC10 did not exist when SP-125 was planned**, and the tab shell was neither in its scope
(*"part of AC1 and AC2 — scene-scoped only"*) nor in its out-of-scope list. ✅ **It is therefore NEW
work for SP-126, not unfinished work here** — which is why this sprint closed rather than being widened.

## Issues raised

| ID | Outcome |
| -- | ------- |
| **I-0173** | ✅ **Verified** — ⚠️ **found by the LIVE PASS**; all 571 ctests + 23 smoke checks were green with it present |
| **I-0174** | ⚠️ **CLOSED, not a defect** — ⚠️ **my misdiagnosis, corrected by the user**: a shared world propagating a second project's characters |
| **I-0175** | ✅ **Verified** — ⚠️ **my own process defect**: a synthetic-input driver typed into the user's manuscript; repaired byte-for-byte |
| **I-0171** | ✅ **Verified 2026-08-29** (owned by SP-122) — ⚠️ **it RECURRED here and blocked the `ctest` run outright** before being fixed |

---

## SP-125 — `[Linux]` ⚠️ **The Scene Inspector, wired to real objects**

**Status:** ✅ **CLOSED 2026-08-29 — user-approved**
**Epic:** [EP-038 / EP-035](../Epics/Epic-active.md) — **EP-035** `[Linux]` Object Foundations · **sprint 1**
**Codebase:** `[Linux]` — ⚠️ **Qt WIDGETS, not QML** (see §2)
**Date Activated:** 2026-08-25
**Closes:** ⚠️ **Part of AC1 and AC2** — ⚠️ **scene-scoped only; the project-wide browser is NOT this sprint**
**Tasks:** **T-0480 – T-0484** (five) — ✅ **all Verified 2026-08-28** · **Next available:** T-0490 (T-0485–T-0489 → SP-126) · Issue **I-0176**

⚠️ **Runs in PARALLEL with SP-123.** ✅ **This sprint is blocked on nothing** — every endpoint it needs was
bridged by SP-121.

---

## 1. Sprint Goal

**Replace `SceneInspector`'s stub with real project data** — the scene's objects, grouped by kind, that a
writer can double-click or right-click to open.

⚠️ **This is the FIRST object surface Linux has ever had.** ⚠️ **`SceneInspector.cpp` has been a 67-line
stub since EP-024/SP-078** — *"wired to NO project data… a muted 'No entities yet.' empty state, and a
DISABLED 'Add Entity' button."*

### ✅ USER RULING (2026-08-25) — Scene Inspector FIRST

> ✅ **Wire the existing `SceneInspector` tab to real data.** ⚠️ **The project-wide object browser is a
> LATER sprint** — this one uses a host that already exists and is already docked.

✅ **`EditorShell.cpp:113` already constructs and docks it.** ⚠️ **No new window, dock, or navigation work
is in scope** — that is what makes this slice honest.

---

## 2. ⚠️ Qt WIDGETS, not QML — a correction to the Epic's framing

⚠️ **EP-035 describes the Linux surface as "Qt/QML." That is MISLEADING and would have cost a sprint.**

✅ **Measured 2026-08-25:** `platforms/linux/qml/` holds **exactly two files** (`Landing.qml`,
`NewProjectDialog.qml`). ⚠️ **Everything else — the manuscript editor, navigator tree, timeline panel,
and `SceneInspector` itself — is QWidgets.** `SceneInspector` is a `QWidget` hosting a `QTabWidget`.

⚠️ **Building this tab in QML would mean embedding QML into a widget hierarchy** — the exact problem
`project_ep022_editor_shell` records, where the app had to flip to **Widgets-hosts-QML** because
QPlainTextEdit could not embed in QML on Qt 6.4.

✅ **This sprint is QWidgets. The Epic text is corrected in the same step.**

---

## 3. Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0480** | ⚠️ **Kind scope DERIVED from `listObjectKinds`** — ⚠️ **a Qt restatement would be occurrence NINE** | **High** | ✅ **Verified 2026-08-28** |
| **T-0481** | **Scene Entities tab lists this scene's objects, grouped by kind** — replaces the stub | **High** | ✅ **Verified 2026-08-28** |
| **T-0482** | ⚠️ **Double-click AND context-menu** both open an object — ⚠️ **NO gesture-only path** | **High** | ✅ **Verified 2026-08-28** |
| **T-0483** | ⚠️ **Empty / unavailable / pending states are DISABLED AND EXPLAINED**, never silently empty | **High** | ✅ **Verified 2026-08-28** |
| **T-0484** | ⚠️ **LIVE VNC pass** (EP-035 AC9) + `ctest` non-root — ⚠️ **and file every Issue found** | **High** | ✅ **Verified 2026-08-28** — ⚠️ **I-0173 filed, fixed, and verified** |

### T-0480 — ⚠️ the one that has been got wrong EIGHT times

⚠️ **`scrivi_list_object_kinds` (D5, SP-116) exists precisely so no platform restates which kinds are
world-scoped.** ✅ **It is already bridged** — `ScriviBridge::listObjectKinds()` at `ScriviBridge.cpp:979`.

⚠️ **A hardcoded kind list in C++ or QML would be OCCURRENCE NINE**, and ⚠️ **occurrence FIVE was in
Swift** — a new platform layer is exactly where this recurs. ⚠️ **Qt is no more immune than SwiftUI was.**

⚠️ **Never write the kind names into a Qt list, a switch, or a QML model.** ✅ **Ask the endpoint.**

### T-0482 — ⚠️ both affordances, and both verified

⚠️ **`project_linux_vnc_input_constraints`: the VNC path carries no Shift-combos and no trackpad
gestures.** ✅ **Every action needs a button or menu path.**

⚠️ **EP-038's rig does NOT retire this** — remote passes may still drop modifiers.

⚠️ **Verify double-click and context-menu SEPARATELY** (`feedback_verify_each_half_separately`) —
⚠️ **on Apple, AC7's two halves needed four Issues before the second one worked.**

### T-0483 — ⚠️ absence is never deletion

⚠️ **A scene with no objects, a world that is unavailable, and an object that is pending are THREE
DIFFERENT STATES** and must not collapse into one blank panel.

⚠️ **`ScriviBridge::getWorldStatus` already returns the core's status** (`ScriviBridge.cpp:889`), so
✅ **an unavailable world can be explained TODAY** — ⚠️ **without waiting for EP-038's `unmounted`
refinement**, which improves the wording, not the existence of the state.

### T-0484 — ⚠️ the live pass is REQUIRED, per EP-035 AC9

⚠️ **User-ruled: every surface-shipping sprint ends with a human click-through.** ⚠️ **This sprint ships a
surface, so it owes one.**

⚠️ **It runs over VNC unless SP-123 lands first** — ✅ **if the Ubuntu rig is ready, run it THERE instead**
and note which was used.

---

## 4. Definition of Done

- [x] ✅ **Kind scope is DERIVED from `listObjectKinds`** — ⚠️ **grep proves it**: no `ObjectKind` name
      appears anywhere in `platforms/linux/src/` or `qml/`. (`ObjectKindScope`, the Qt mirror of Apple's
      `ObjectKindScope.swift`.) ⚠️ **The unknown-kind fallback is world-scoped ON PURPOSE** — it names no
      kinds, so it is a safety default, not a restatement.
- [x] ✅ **The Scene Entities tab lists real objects, grouped by kind** — ⚠️ **the stub is GONE.** Seen
      live: 11 kind groups with counts, in the CORE's kind order.
- [x] ✅ **Double-click AND context-menu both open an object**, ⚠️ **verified SEPARATELY** — double-click
      opened a WORLD-scoped `character`, the context menu a PROJECT-scoped `source`. ✅ **That pairing also
      proves `worldID` is threaded in BOTH directions** (SP-104's failure mode). ✅ **Return/Enter opens
      too**, so the affordance is reachable with no pointer at all.
- [x] ✅ **Empty, unavailable and pending are DISTINCT and EXPLAINED** — ⚠️ **plus a FOURTH state the
      plan did not anticipate** (see §6).
- [x] ✅ `ctest` **571/571, NON-ROOT (`builder`), tests ON** — ⚠️ **exactly the expected figure.**
      ⚠️ **Required fixing I-0171 first: it RECURRED and blocked the run outright.** ✅ **All 18 Linux
      smokes also pass**, including the new 23-check `scene_inspector_smoke`.
- [x] ✅ **A LIVE click-through happened** over VNC (SP-123's rig is still blocked on the user), and
      ⚠️ **I-0173 was FILED and fixed in the same sprint.**
- [x] ✅ **EP-035's "Qt/QML" framing is CORRECTED to Widgets** in the Epic text.
- [x] ✅ **User verification — GRANTED 2026-08-28** after a live click-through against real writing work.
      ⚠️ **Sprint CLOSURE still needs explicit user approval** (Claude may not close a Sprint).

---

## 6. ⚠️ FINDINGS — things the plan did not know

### ⚠️ 6.1 An EMPTY list and a FAILED call were INDISTINGUISHABLE at the bridge

⚠️ **This is the finding that shaped the sprint, and it very nearly shipped as a defect.**

`ScriviBridge::parseEnvelope` returns an empty `QVariantMap` when a call fails. ⚠️ **But an OK envelope
also decodes to an empty map when its result carries no keys** — and that is exactly what EVERY list
endpoint emits for an EMPTY list, because `JsonDoc::appendToArray` creates the array key only when there
is a first element to push (`Json.cpp:129-135`).

⚠️ **So `listEdgesFor()` returning `{}` means EITHER "this scene has no objects" OR "the call failed",
and the call site could not tell which.** ⚠️ **T-0483 is precisely the requirement that those two must
never collapse** — so the panel could not have been built correctly without separating them.

✅ **Fix:** `ScriviBridge::lastCallFailed()`, set on every failure path (`parseEnvelope`'s two, plus all
**80** not-bootstrapped guards, which never reach `parseEnvelope` at all and would otherwise report the
PREVIOUS call). ⚠️ **The smoke asserts the flag is per-call and not sticky** — a good call after a bad one
must clear it, or every state after the first error reads as broken.

⚠️ **Existing callers tolerate the ambiguity by degrading silently** (the timeline drops to scenes-only on
a failed historical-events read). ✅ **That is defensible for a strip of dots and NOT defensible for a
panel whose job is to distinguish absence from loss** — but it means other surfaces carry this same
blind spot, and a future sprint that needs the distinction must ask for the flag rather than assume `{}`.

### ⚠️ 6.2 A FOURTH state — the object index unreadable

The plan named three states (empty / unavailable / pending). ⚠️ **There is a fourth**, and it is the
dangerous one: the edges read but the object INDEX did not.

⚠️ **Handled naively, the confirmation filter drops every unconfirmed edge and the panel reports "no
objects are linked to this scene" — a FALSE claim about the writer's data, not a degraded one.** ✅ **The
filter is therefore applied only when the index was actually read**; otherwise the rows are drawn from
the graph alone and the panel says so.

### ⚠️ 6.3 A nonexistent project path is NOT an error

⚠️ **Worth recording because it is the obvious thing to reach for when testing failure, and it is wrong.**
`RelationshipStore::replay` treats a missing edge log as *"no log = empty graph"* and SUCCEEDS
(`RelationshipStore.cpp:72-74`) — deliberately, since a project that has never related anything has no log
to read. ⚠️ **My first smoke assertion used a bad path to prove failure detection and FAILED, correctly.**
✅ **`listObjects` with an unrecognised kind is a real error by construction** — the core refuses it rather
than returning an empty listing, precisely so a typo never reads as *"you have no characters."*

### ✅ 6.4 The label projection is real, and visible

The live pass rendered `character 1 — doc…`. ⚠️ **The elided word is "documented by"** — the `cites`
type's **INVERSE** label, correctly projected because the scene is the `to` endpoint. ✅ **Confirmation
that the panel READS the resolved label rather than recomputing direction**, which is EP-037's known trap.
⚠️ **It is also I-0173: correct, and unreadable.**

### ⚠️ 6.5 I-0171 recurred and blocked the sprint

⚠️ **`ctest` could not run at all until it was fixed.** Both `.dockerignore` files matched only `build/`
while five build directories exist. ✅ **Now matched as a family** (`build-*/`, `**/build-*/`).
⚠️ **The two-file relationship remains an unruled duplicate** — Docker applies `<dockerfile>.dockerignore`
only for that Dockerfile, so a third one silently falls back to the weaker root file.

---

## 5. Explicitly OUT of scope

| Item | Where it goes |
| ---- | ------------- |
| ⚠️ **The project-wide object browser** | ⚠️ **A LATER EP-035 sprint** — user-ruled |
| **Object create / edit / delete** | ⚠️ **Later EP-035 sprint (AC4)** |
| **The Detail Sheet itself** | **EP-036** |
| **Relationships, sources** | **EP-037** |
| ⚠️ **`WorldVolumeStatus` / `unmounted`** | **EP-038 (T-0478)** — ⚠️ **do NOT write it here from documentation** |
| **Card thumbnails (AC5)** | ⚠️ **Later — needs the image work in EP-036** |

---

*Last Updated: 2026-08-28, second pass (**SP-125's five Tasks ✅ VERIFIED — user-approved after a LIVE
CLICK-THROUGH against real writing work.** ✅ **The user confirmed the "opened successfully" dialog** on
`the-lone-golem.scrivi` / `the-stairs-of-tintagael.scrivi`, two projects sharing the **Eskandar** world.
✅ **All five Tasks archived in the same step** → `Verified/Task-verified-0480-0484.md`
(`feedback_archive_on_close`). ✅ **I-0173 + I-0175 Verified and archived; I-0174 CLOSED as NOT A
DEFECT** — ⚠️ **my diagnosis was wrong and the user corrected it: opening a project is not a risk, and
the cache reconcile is a SHARED WORLD propagating a second project's characters.** ⚠️ **(Superseded:
SP-125 was CLOSED 2026-08-29 with user approval.)** Next Task **T-0485**; next Issue **I-0176**.
Prior note follows.)*

*Last Updated: 2026-08-28 (**SP-125's five Tasks 🟠 IMPLEMENTED — Not Verified.** ✅ **The Linux app has
its FIRST object surface**: `SceneInspector`'s 67-line EP-024 stub is replaced by the scene's real
objects, grouped by kind in the CORE's kind order, with counts. ✅ **`ctest` 571/571 NON-ROOT, tests ON** —
⚠️ **which required fixing I-0171 first: it RECURRED and blocked the run outright.** ✅ **All 18 Linux
smokes pass**, including the new 23-check `scene_inspector_smoke`. ✅ **The LIVE VNC pass RAN** and
⚠️ **found I-0173 — a defect all 571 ctests and 23 smoke checks were green against.** ⚠️ **T-0482's two
halves were verified SEPARATELY**: double-click opened a WORLD-scoped `character`, the context menu a
PROJECT-scoped `source`, proving `worldID` threads both ways. ⚠️ **§6 records FIVE findings** — the
biggest being that an EMPTY list and a FAILED call were indistinguishable at the bridge, which made
T-0483 unimplementable until `lastCallFailed()` existed. ✅ **Three rules were added to the Porting
Outline** (§4.4a, §4.4b, §4.6a) from that experience, closing AC6's obligation for this sprint. ⚠️ **NOT
verified — Claude may go no further than `Implemented - Not Verified`.** Prior note follows.)*

*Last Updated: 2026-08-25 (**SP-125 🟡 ACTIVATED** — EP-035's first, ⚠️ **running PARALLEL to SP-123**.
⚠️ **FINDING AT PLANNING: the Linux app is Qt WIDGETS, not QML** — only 2 QML files exist, and
`SceneInspector` is a `QWidget`+`QTabWidget`; ⚠️ **the Epic's "Qt/QML" wording is corrected by T-0484's
DoD.** ✅ **User-ruled: Scene Inspector FIRST**, replacing the 67-line EP-024 stub; the project-wide
browser is a later sprint. ✅ **Every endpoint needed was already bridged by SP-121** — this sprint is
blocked on nothing. ⚠️ **T-0480 is occurrence-NINE territory: DERIVE kind scope, never restate it.**
Next Task **T-0485**; Issue **I-0173**.)*

---

*Closed 2026-08-29 by user approval. Tasks archived 2026-08-28 to `Task-verified-0480-0484.md`.*

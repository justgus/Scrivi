# Scrivi Platform Porting Outline — v0.1

**Task:** T-0465 (EP-034 SP-121) · **Date:** 2026-08-24
**Status:** 🟢 First edition — ⚠️ **written BEFORE its first full use, and therefore unproven.**

> *"We should also mark all the steps that that will take and create a porting outline, because we're
> going to have to do it all again for iPad, iPhone, Windows and eventually visionOS."* — user, 2026-08-24

⚠️ **This document is the reusable procedure. Linux is its first run, not its subject.** ⚠️ **EP-035 must
CORRECT this file from experience** — an outline that survives its first use unedited was almost certainly
not followed.

---

## 0. What a "port" is in Scrivi, and what it is not

ScriviCore holds **all** behaviour. A platform layer is **a binding plus surfaces** and nothing else.

| A port IS | A port is NOT |
| --------- | ------------- |
| Binding the C ABI to the platform's language | Reimplementing logic in Swift/Qt/Kotlin |
| Building surfaces a writer can reach | Re-deciding rulings already made |
| Honouring platform idiom (input, layout, navigation) | Inventing a second source of truth |

⚠️ **The single most expensive mistake available is re-deciding a settled ruling by accident**, because it
does not look like a decision at the time — it looks like writing code. §4 is the list.

---

## 1. ⚠️ The layer order, and WHY it is that order

**Bind → model → read → write → navigate.** Skipping ahead is what EP-034 paid for repeatedly.

| # | Layer | ⚠️ What goes wrong if you skip it |
| - | ----- | -------------------------------- |
| **1** | **ABI binding** — every endpoint, envelope parsing, memory contract | The UI grows its own ad-hoc calls; the binding never becomes a layer. **SP-121 is this step for Linux.** |
| **2** | **Typed model** — decode for display, **patch for save** | ⚠️ **A surface built before its model RECONSTRUCTS objects and silently drops fields** — `image`, `attributes`, anything a newer core added. EP-034's T-0436/T-0437 exist because the app had no typed model and edited one field. |
| **3** | **Read-only surfaces** — lists, cards, detail views | Editing before reading means debugging both at once. |
| **4** | **Editing** — with the guards (see §4) | ⚠️ Data loss. SP-119 found **six** data-loss routes into ONE surface. |
| **5** | **Navigation + history** | Push-navigation needed **four** Issues on Apple *after* the sheet worked. |

⚠️ **Step 1 has no writer-visible output, and that is normal.** It is staged work with a scheduled reader,
**provided the reader is scheduled** — see §6.

---

## 2. The endpoint checklist

⚠️ **Regenerate this; do not trust a copy.** The canonical count comes from the header itself:

```bash
grep -oE "^const char\* scrivi_[a-z_]+" ScriviCore/include/scrivi/scrivi.h | sort -u | wc -l
```

**As of 2026-08-24: 100 endpoints.** Coverage, from `docs/Scrivi_ABI_Binding_Gap_Audit_v0_1.md`:

| Platform | Reaches | Notes |
| -------- | ------- | ----- |
| **Apple** (`ScriviEngine`) | **96 / 100** | ⚠️ 4 unreached — incl. `upsert_relation_type`, which is **unowned** |
| **Linux** (`ScriviBridge`) | **81 / 100** | 34 before SP-121, **+47**; ⚠️ 19 history/buffers deferred to EP-019 |
| iPadOS / iOS | via `ScriviEngine` | Shares the Apple binding; ⚠️ **the SURFACES are the port, not the binding** |
| Windows | 0 | Qt — ⚠️ **should reuse `ScriviBridge` wholesale**, not fork it |
| visionOS | via `ScriviEngine` | As iOS |

⚠️ **A binding is not "done" at 100.** Endpoints with no reader are recorded as such, never counted as
reach (§6).

---

## 3. The per-platform steps

1. **Generate the gap audit** (the T-0460 pattern) — mechanical, before any estimate.
   ⚠️ **SP-121's own planning estimate was wrong by 9%** (43 vs the real 47) because it grepped prose as
   well as declarations. **The audit is what makes the sprint's scope a fact.**
2. **Bind every in-scope endpoint** with: identity threaded **by the binding**, envelope parsing in one
   place, and the memory contract structural (RAII / ARC), not per-call.
3. **Test through the binding** — ⚠️ **never the C ABI** (§4.6).
4. **Build the typed model** before any editing surface.
5. **Build surfaces in dependency order** — list → detail → edit → relate → navigate.
6. **Run a live pass** with a real project. ⚠️ **This is the only step that has ever found the real
   defects** (§5).

---

## 4. ⚠️ The platform-independent rules — re-honour, never re-decide

**Every one of these has already been paid for. A new platform re-earns the defect if it re-decides them.**

### 4.1 ⚠️ DERIVE kind scope. Never restate it.

Call `scrivi_list_object_kinds` and read `isWorldScoped`. ⚠️ **Do not write a list of kind names anywhere,
in any language.**

> ⚠️ **This is the project's most-repeated defect — EIGHT occurrences**, and ⚠️ **occurrence five was in
> SWIFT, not C++.** It is not a backend concern; **a new platform layer is exactly where it recurs.**
> A restated list **rots without being edited**: two occurrences broke when a kind's *scope* changed while
> the list's own text still read correctly.

### 4.2 READ edge labels. Never recompute them.

`listEdgesFor` returns the label already resolved **for the queried endpoint**. Recomputing direction means
a local copy of a rule the core owns.

### 4.3 ⚠️ PATCH objects. Never reconstruct them.

Mutate the JSON the core returned; write it back. ⚠️ **Reconstructing from a typed struct drops every
field the platform does not model** — and the loss is invisible until a writer notices her portrait gone.

### 4.4 Absence is never deletion.

An object whose world is away is **pending**: shown, named, explained, **never rewritten and never
pruned**. A read-only state must be **enforced**, not merely styled (I-0148 — a disabled `TextEditor` on
Apple was still editable).

### 4.4a ⚠️ First check the BINDING can tell absence from failure. Usually it cannot.

**Added 2026-08-28 from SP-125 (EP-035 AC6) — the first port to actually implement 4.4.**

⚠️ **Rule 4.4 is unimplementable until this is true, and on a fresh binding it usually is not.**

Scrivi's envelope is `{"ok":true,"result":{…}}`, and a binding naturally maps a failure to an empty
result. ⚠️ **But an OK envelope ALSO decodes to empty when its result carries no keys — which is exactly
what every list endpoint emits for an EMPTY LIST**, because `JsonDoc::appendToArray` creates the array key
only when there is a first element to push (`Json.cpp:129-135`).

⚠️ **So "no objects" and "the call failed" arrive as the same value.** A surface built on that cannot
honour 4.4 no matter how carefully it is written: it will report one of the two states as the other, and
**the direction it gets wrong is the dangerous one** — an unreadable project reads as an empty project,
which is a false claim about the writer's data.

✅ **Before building any surface that must distinguish them, give the binding an explicit per-call failure
signal** (Linux: `ScriviBridge::lastCallFailed()`). Two things it must get right:

- ⚠️ **Set it on EVERY failure path, not just the parse path.** The Linux bridge had **80** early-return
  guards that never reach the envelope parser; without them the flag reports the *previous* call.
- ⚠️ **It must be per-call, not sticky.** A good call after a bad one must clear it, or every state after
  the first error reads as broken. **Assert this in the smoke** — it is one line and it is the half that
  rots silently.

⚠️ **Existing surfaces will already be tolerating the ambiguity by degrading quietly** (Linux's timeline
drops to scenes-only on a failed read). ✅ **That is defensible for decoration and NOT defensible for
anything whose job is to distinguish absence from loss** — but it means the blind spot is already spread
across the platform, and a later sprint must ASK for the flag rather than assume an empty result.

### 4.4b ⚠️ There is a FOURTH state, and the plan will name three.

**Added 2026-08-28 from SP-125.**

Plans reliably name *empty*, *unavailable*, and *pending*. ⚠️ **The fourth is: the primary read
SUCCEEDED and the confirming read FAILED.**

SP-125's panel reads a scene's edges, then reads the object index to confirm each far endpoint.
⚠️ **When the index fails, the confirmation filter drops every unconfirmed row and the panel reports "no
objects are linked to this scene"** — again turning a read failure into a false claim about the data.

✅ **Apply a confirmation filter ONLY when the confirming read actually succeeded.** Otherwise draw what
the primary read gave you and say the list is unconfirmed. ⚠️ **A partial answer, labelled, beats a
confident wrong one.**

### 4.5 Disabled **and** explained.

A greyed control that explains nothing is the defect, not the fix.

### 4.6 ⚠️ Test through the binding, not the ABI.

> ⚠️ **A facade test cannot see a boundary gap. That is exactly how I-0113 shipped green** — the C ABI was
> missing a kind and the facade tests passed regardless.

### 4.6a ⚠️ A bad path is NOT how you prove failure detection.

**Added 2026-08-28 from SP-125 — my first attempt at this assertion failed, correctly.**

The obvious way to test that a binding detects failure is to pass a nonexistent project path.
⚠️ **In Scrivi that SUCCEEDS.** `RelationshipStore::replay` treats a missing edge log as *"no log = empty
graph"* (`RelationshipStore.cpp:72-74`) — deliberately, because a project that has never related anything
has no log to read. Several stores behave the same way.

✅ **Use an input the core refuses by construction** — e.g. `list_objects` with an unrecognised kind,
which errors rather than returning an empty listing precisely so a typo never reads as *"you have no
characters."* ⚠️ **Pick the trigger from what the core actually rejects, not from what looks obviously
broken.**

### 4.7 ⚠️ No gesture-only affordance.

Every action needs a button or menu path. ⚠️ **VNC carries no Shift-combos and no trackpad gestures**, so
a gesture-only control is **unreachable** on Linux — and unreachable is not "degraded".
⚠️ **Touch platforms invert this**: hover and right-click do not exist on iPhone/iPad, so an
affordance that is *only* a context menu is equally unreachable there.

### 4.8 Unlinked objects are normal.

A created-then-unlinked object is a **library entry, not debris**. ⚠️ **Never roll back a create to tidy
up a failed link** — that deletes the writer's typing for a failure she did not cause.

---

## 5. ⚠️ What the evidence actually shows about verification

**Across SP-118, SP-119 and SP-120: 22 consecutive Issues. NOT ONE came from a test suite.**

⚠️ The suites were green throughout and were **not wrong** — they assert what the code does. ⚠️ **What no
suite covered was whether a writer could REACH the capability, and whether what she read was TRUE.**

Two SP-120 examples, both from clicking:

- **I-0169** — the sources card had no route to the Detail Sheet, using a hook that already existed.
  ⚠️ **It was the writer's FIRST instinct.**
- **I-0170** — a twice-cited source named only one citing object. ⚠️ **Every field present and populated,
  and the surface still under-reported the graph.** No field-level review could have caught it.

⚠️ **Budget for a live pass on every platform.** ⚠️ **It is HARDER on Linux (VNC + a real project) and
harder still on mobile** — and it is the step that finds the defects.

---

## 6. ⚠️ Staged work vs. stranded work

A binding shipped before its surfaces looks exactly like `capability_without_surface`, the defect this
project has paid for eight times. **The distinction is a scheduled reader:**

| Staged (fine) | Stranded (the defect) |
| ------------- | --------------------- |
| A named Epic owns the surfacing | "Someone will use it later" |
| The Epic is **open and tracked** | An intention in a comment |
| Every endpoint exercised by tests | Merged unproven |
| Endpoints with **no** reader are named as such | Counted as reach |

✅ **SP-121 qualifies**: EP-035 was opened in the backlog carrying AC11 **before** the endpoints shipped.

⚠️ **Do not skip the last row.** SP-121 bridged three endpoints with **no known consumer** —
`list_pending_edges` (⚠️ zero call sites on Apple too), `extract_searchable_text` (⚠️ a known drift site,
I-0118), and `resolve_timeline_project_times` (⚠️ possibly internal-only). **Naming them is what keeps
"81/100 bridged" from being read as "81 capabilities a writer can reach."**

---

## 7. Next ports, in the order they are expected

| Order | Platform | First step | ⚠️ Note |
| ----- | -------- | ---------- | ------- |
| **1** | **Linux surfaces** | **EP-035** — open, carries AC11 | SP-121 finished its binding |
| 2 | iPadOS | Gap audit of **surfaces**, not the binding | ⚠️ Binding is shared with macOS; **§4.7 inverts** |
| 3 | iOS | As iPad, with a small-screen navigation trade | ⚠️ The Detail Sheet's D1 ruling needs re-examination, **not re-decision** |
| 4 | Windows | ⚠️ **Reuse `ScriviBridge`** | Forking it doubles §4's surface area |
| 5 | visionOS | Last, per Q-c | Spatial idiom is genuinely new; the rules are not |

---

## 8. ⚠️ This document's own failure mode

⚠️ **An outline nobody edits is an outline nobody followed.** ✅ **EP-035's AC2 requires correcting it from
experience** — that requirement exists because this file was written by the sprint that had not yet done
the work it describes.

⚠️ **§2's counts drift the moment an endpoint is added.** **Regenerate; do not trust.**

## 9. ⚠️ The rig is a BUILD-ONCE PREREQUISITE, and a container is not one

⚠️ **Added 2026-08-25 by user ruling, BEFORE EP-038 runs.** ⚠️ **This section is a PLAN, not a
retrospective** — it must be corrected by the first Epic that executes it.

### ⚠️ The rule

> ⚠️ **Every platform needs REAL HARDWARE before its live pass means anything.** ✅ **A container or
> simulator validates the BUILD and the SUITE. It cannot validate anything the HARDWARE does.**

### ⚠️ What a container is structurally blind to

| Failure mode | Container | Real hardware |
| ------------ | --------- | ------------- |
| **Removable drive unplugged mid-write** | ⚠️ **cannot reproduce** — a bind-mount stop is clean | ⚠️ `EIO`/`ESTALE`, vanished mount entry, possible stale mount point |
| Volume identity / mount semantics | synthetic | ⚠️ **what the OS actually reports** |
| Display, input, gesture availability | ⚠️ **VNC drops Shift-combos and trackpad gestures** | native |
| Power / sleep / device removal events | absent | real |

### ✅ The evidence this rule is built on — Apple's, and it is unambiguous

`Scrivi/App/WorldVolumeStatus.swift:15-23`:

> ⚠️ **`volumeIsRemovable` : false** on a drive unplugged by hand.
> ⚠️ **`volumeIsEjectable` : false** — `diskutil` reports *"Removable Media: Fixed"*.

⚠️ **The documented API for "is this removable" returned FALSE for a removable drive.** The signal that
worked (`volumeIsLocal`) was found ⚠️ **by pulling a real drive.** ⚠️ **No amount of reading would have
produced it**, and a container would have reported success either way.

⚠️ **Drive-loss-while-editing cost Apple SIX Issues** (I-0162, I-0165, I-0165b, I-0166, I-0167, I-0168).
⚠️ **All six were found by ejecting a drive. None by any suite.**

### The per-platform rig checklist

1. **Reachability** — ⚠️ **account, key-based SSH (or platform equivalent), documented path.**
   ⚠️ **No passwords in tracked files.**
2. **Native toolchain** — the app **builds and runs natively**, ⚠️ **not only in a container.**
3. **Suite** — `ctest` runs, ⚠️ **NON-ROOT, tests ON** (`project_linux_container_tests_off`).
4. ⚠️ **Removable media** — a real drive carrying a world copy, ⚠️ **that a human can physically remove.**
5. ⚠️ **Instrumentation FIRST** — record what the OS reports on removal ⚠️ **BEFORE writing the platform
   refinement.**
6. **Real project data** — a genuine manuscript, not a fixture. ⚠️ **Back it up; it is real work.**

### ⚠️ The ordering rule that matters most

> ⚠️ **INSTRUMENT, THEN IMPLEMENT.** ⚠️ **Never write the volume-status refinement from documentation.**

✅ **The platform layer — not the core — decides `unmounted` vs `offline` vs `missing`**
(`WorldStore.hpp:28-35` calls it a *"platform-layer refinement"*). ⚠️ **Each platform gets this wrong in
its own way**, and only its own hardware will say how.

### Per-platform rig status

| Platform | Rig | State |
| -------- | --- | ----- |
| **macOS** | ✅ **Exists** — the real Tintagael/Eskandar rig on USB | ✅ **Proven** — found 22 Issues |
| **Linux** | ⚠️ **BEING BUILT** — native Ubuntu box | ⚠️ **EP-038.** ⚠️ **Docker was the rig and is blind to drive loss** |
| **Windows** | ⚠️ **Box exists on the network; NO rig** | ⚠️ **Later Epic — should EXECUTE this section, not re-derive it.** ⚠️ **The Windows app does not exist yet** |
| **iPad / iPhone / visionOS** | ⚠️ **None** | ⚠️ **Unsolved: removable media is not the failure mode there** — ⚠️ **do NOT assume this checklist transfers** |

⚠️ **The mobile row is deliberately unanswered.** ⚠️ **Copying the desktop rig recipe to iOS would be
exactly the re-decision §4 forbids** — ✅ **the failure modes differ and the checklist must be re-derived
there, not inherited.**

---

---

*T-0465, EP-034 SP-121. First edition — unproven until EP-035 runs against it.*

*⚠️ **§9 added 2026-08-25** by user ruling, from the finding that ⚠️ **Docker cannot validate drive loss**.
⚠️ **§9 is a PLAN and is itself unproven** — **EP-038** is its first execution and owes it corrections.*

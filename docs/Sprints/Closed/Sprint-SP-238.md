# Sprint SP-238 — `[Linux]` [EP-059] C5: custom relationship types on Linux, as the Mac's

**Status:** ✅ **CLOSED 2026-11-23** (user, 2026-11-23: *"close SP-238"*). Was 🟡 **ACTIVE** 2026-11-23 (user, 2026-11-23: *"Q1-Q3 as recommended, implement SP-238"*); ✅ Q1–Q3 as recommended. Built 2026-11-23: AC1–AC2 met; AC3 green in Docker; ✅ AC4 live pass (user, 2026-11-23: *"All steps pass on the Linux VM."*).
✅ **All ACs met**; CI green on `c748b08`; awaiting close approval. Created 2026-11-22 (user, 2026-11-22: *"plan C5-C6"*).
**Carries:** [T-0472] (C5 on Linux; filed 2026-08-25).
**Epic:** [EP-059] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md) — part **C**: C5.
**Size:** S — the endpoint is bridged (`ScriviBridge::upsertRelationType`); one settings section and one form, in the shape [SP-237] sets.

---

## Findings (2026-11-23, from what [SP-237] built)

1. **The core is done.** [SP-237] fixed the endpoint (unique codes, seeded codes and unknown kinds refused, a type in use keeps
   its shape) and `scrivi_list_relation_types` now reports `builtIn` and `inUse`. Linux needs **no core change**.
2. **The bridge is there.** `ScriviBridge::listRelationTypes` returns the list as-is, so `builtIn`/`inUse` already come through.
   `ScriviBridge::upsertRelationType(root, json)` takes the record as JSON; Apple's engine builds that JSON from typed fields.
   `ScriviBridge::listObjectKinds` gives the kinds for From / To.
3. **Project Settings** (`ProjectSettingsDialog.cpp`) is a column of group boxes: Project, Authors (`AuthorsBox`), Writing
   Surface, Typography, Undo History, with a single **Close** button. Every box saves at once, so a type saved from the
   form stays saved, as on the Mac.
4. **Relate…** (`ObjectDetailDialog::chooseRelation`) is a dialog, not a popover. It shows a type menu when two or more
   types fit the object, and just a label when one does.
5. **Apple's lock** on a built-in type is an SF Symbol. Qt has no standard lock icon, and the VM's icon theme may lack one.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Where it lives.** | **As the Mac:** a **Relationship Types** box in Project Settings, after Authors, with each type's labels and ends, **Edit…** on the writer's own types and **New Relationship Type…**, plus **New Type…** beside the type menu in Relate…. || ✅ As recommended (2026-11-23) |
| **Q2** | **The form.** The Mac shows it in a sheet, and in Relate…'s popover. | **One modal dialog**, "New Relationship Type" / "Edit Relationship Type", with the Mac's fields, footers and greying (Reads from this end, Same both ways, Reads from the other end, From / To with Between). From Relate… it opens **over** the picker with From set to the object's kind; on Save the picker shows the new type chosen. || ✅ As recommended (2026-11-23) |
| **Q3** | **The built-in mark** (finding 5). | **A grey "Built in" label** where Edit… would be, with the Mac's tooltip ("Built in: every project has it, and it can't be changed."). It reads the same on every icon theme. || ✅ As recommended (2026-11-23) |

## Plan

- `RelationTypesBox` (new, beside `AuthorsBox`), added to `ProjectSettingsDialog` after Authors.
- `RelationTypeDialog` (new): the form. It builds the record with `QJsonObject` and calls `upsertRelationType`; an empty code
  asks the core for a new one. Kinds come from `listObjectKinds` (never restated).
- `ObjectDetailDialog::chooseRelation`: **New Type…**; after Save the picker is rebuilt with the new type chosen.
- Smokes: `settings_smoke` (the box lists the four built-ins as Built in; a type is made, edited and relabelled) and
  `object_detail_smoke` §8 (New Type… from Relate…, then used, read from both ends). Both through a real project window.
- Live pass on the VM on an existing project, `/mnt/scrivi-net/projects/the-river-of-time.inkwyn` (or one you prefer), with
  "test …" types, since delete is [T-0701].

## What was built (2026-11-23)

- `platforms/linux/src/RelationTypes.cpp/.hpp` (new; in every CMake target that has `ObjectDetailDialog`):
  - `RelationTypeDialog`, the form: "New Relationship Type" / "Edit Relationship Type". Reads from this end, Same both ways,
    Reads from the other end, From / To (Between when Same both ways), the Mac's footers. The kinds come from
    `ObjectKindScope::kinds` (the core's list). A type in use has its kinds and Same both ways greyed, with the Mac's reason.
    Save is off until the labels are typed; it saves off the GUI thread and shows the core's refusal in red.
  - `RelationTypesBox`, Project Settings ▸ Relationship Types: each type's labels and ends; a grey "Built in" (with the Mac's
    tooltip) on the four built-ins (Q3), Edit… on the writer's own; New Relationship Type…; the Mac's footer.
- `ProjectSettingsDialog.cpp`: the box after Authors (Q1).
- `ObjectDetailDialog::chooseRelation(selectCode)`: **New Type…** beside the type menu. The form opens over the picker with From
  set to the object's kind; on Save the picker reopens with the new type chosen (Q2).

## Tests

- `settings_smoke` §8: the four built-ins listed, Built in, no Edit…; New Relationship Type… (Save off until both labels; From
  offers Any kind, Scene and the core's kinds, never world; Same both ways hides the other end); saved under the code the core
  made (`mentor-of`); Edit… changes To to Location, the same type; `objects/relation-types.json` holds it. ✅ A mutant that gives
  built-ins Edit… and never hides To fails it (in a copy of the repo).
- `object_detail_smoke` §8b: Relate… ▸ New Type… opens the form over the picker with From = Character; Save brings the picker back
  with "mentor of" chosen; a relationship made with it at once, read "mentored by" from the other end; the list then reports it in
  use, and its form greys the kinds and Same both ways with the reason, the labels still editable.
- Results (Docker): ctest **886/886**; smokes **49/49**. The CI check scripts clean. No core or Swift change.

## Live pass (AC4) — the Linux VM

On `/mnt/scrivi-net/projects/the-river-of-time.inkwyn` (world Eskandar), with Myton and Veyra. The types made here stay in the
project: there is no delete yet ([T-0701]).

1. On the VM: `cd ~/Dev/Scrivi && git pull`, build (`cmake --build build-native -j3`), launch Inkwyn, open `the-river-of-time`.
2. Project ▸ Project Settings… (Ctrl+Alt+,). Below Authors: **Relationship Types**, four rows each marked "Built in" (hover: "Built
   in: every project has it, and it can't be changed."), New Relationship Type…, and the footer about "mentor of".
3. New Relationship Type…: a "New Relationship Type" window, Save greyed. Type `test mentor of`, `test mentored by`, From and To
   Character. Save: the list gains "test mentor of · test mentored by", "Character → Character", with Edit….
4. Its Edit…: "Edit Relationship Type". To = Location, Save: "Character → Location". Edit… again, To = Character, Save. Close.
5. Inspector ▸ Worldbuilding ▸ the Characters card, double-click Myton. Relate…: "Relate to…" with the type menu and **New Type…**. Choose
   "test mentor of", click Veyra: Related gains "test mentor of" ▸ Veyra.
6. Double-click Veyra: Veyra's Related has "test mentored by" ▸ Myton. Back.
7. Relate… ▸ New Type…: the form opens over the picker, From = Character. Type `test rival of`, tick Same both ways: the other end
   and To go, From reads Between. Save: the picker is back with "test rival of" chosen. Esc.
8. Project Settings…, Edit… on "test mentor of": Same both ways, From and To greyed; below them "Relationships use this type, so
   its kinds and Same both ways can't change. Its labels can." First label → `test teacher of`, Save, Close. Myton's Related reads
   "test teacher of".
9. Right-click Veyra under "test teacher of" ▸ Remove Relationship: the group goes.

## Acceptance criteria

- [x] AC1 — Linux's Project Settings lists and edits relationship types as the Mac's.
- [x] AC2 — Relate… offers New Type…; the new type is selected and usable at once.
- [x] AC3 — A smoke through a real project window; ctest and every smoke green in Docker; CI green. ✅ 886/886, 49/49; CI green on `c748b08`.
- [x] AC4 — Live pass on the Linux VM, on an existing project there. ✅ `the-river-of-time` (user, 2026-11-23: *"All steps pass on the Linux VM."*)

## Progress log

- **2026-11-23** — ✅ **CLOSED** (user, 2026-11-23: *"close SP-238"*). T-0472 archived.
- **2026-11-23** — ✅ **AC4 met**: every live-pass step passes on the VM (user, 2026-11-23: *"All steps pass on the Linux VM."*). T-0472 ✅ Verified. CI green on `c748b08` (Linux App CI; the only workflow the push touched). Awaiting close approval.
- **2026-11-23** — Built: AC1, AC2 met; AC3 green in Docker; T-0472 → 🟠. ⏳ AC4 live pass, CI.
- **2026-11-23** — 🟡 **ACTIVE** (user, 2026-11-23: *"Q1-Q3 as recommended, implement SP-238"*); Q1–Q3 as recommended; T-0472 → active.
- **2026-11-23** — Planned (user, 2026-11-23: *"plan SP-238"*): findings 1–5, Q1–Q3 to rule.
- **2026-11-23** — ✅ Unblocked: [SP-237] closed (user, 2026-11-23: *"Close SP-237."*).
- **2026-11-22** — 🔵 **Created in Planning** via `next-id.py` (user, 2026-11-22: *"plan C5-C6"*); carries [T-0472] (existing). ⛔ Blocked by [SP-237].

*Last Updated: 2026-11-23 (Closed.)*

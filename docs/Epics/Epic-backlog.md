# Epic Backlog

Epics listed here are fully defined but have no active sprint. Epics that were partially delivered and returned here retain their verified AC state. Work resumes when a sprint is assigned.

---

## Backlog Epics

_(EP-019 `[Apple]` Undo/Redo was un-deferred back to Active 2026-07-24 — now in `Epic-active.md`.)_

## [Linux] App — Epic Family (EP-024–EP-026)

The Ubuntu Qt/QML port of the macOS app, split one Epic per capability (mirroring how the Apple app was
built Epic-by-Epic). **EP-020 `[Linux]` (Foundation), EP-021 `[Linux]` (Project Lifecycle & Landing),
EP-022 `[Linux]` (Writing Surface & Scene Navigator), EP-023 `[Linux]` (Manuscript Structure Editing), and
EP-024 `[Linux]` (Scene Inspector Panel) are all ✅ closed** (→ `Epics/Closed/`); **EP-025 `[Linux]`
(Timeline Panel) is now 🟡 Active** (→ `Epics/Epic-active.md`). The Epic below is 🔵 Draft, promoted to Active
when EP-025 lands. Each is verified in Docker+VNC (developer) then on real Ubuntu (alpha tester). Each targets
near-parity with the named Apple Epic. Any new `scrivi_*` endpoint a screen needs is a Task with a
`[ScriviCore]` note, not a separate Epic.

### EP-026: [Linux] Undo/Redo, Menus, Settings & Parity Verification

**Status:** 🔵 Draft
**Goal:** Wire the EP-019 undo/redo history C ABI into the Linux UI (capture + apply + fork popover +
stale-branch purge), the app menu bar, project settings, and run the near-parity verification pass;
prepare the Linux app for the alpha tester on real Ubuntu. Mirrors Apple EP-012 + EP-019.
**Rough scope:** Linux `HistoryCapture` equivalent over `scrivi_history_*`, undo/redo apply, fork
popover, settings sheet (capacity + stale-branch purge), menu bar, full parity checklist, hand-off to
tester. **Depends on:** EP-022 (and benefits from EP-023–EP-025).

---

## EP-032: [Cross] Inline Object References in the Manuscript

**Status:** 🔵 **Proposed** (opened 2026-08-12 from an EP-031 design ruling)
**Codebase:** `[Cross]` — ScriviCore (fragment/scene model, a new relation type) + every UI that renders or
edits manuscript text (Apple, Linux) + export.
**Goal:** Let an **object** be referenced *inside* scene text and rendered there — the capability behind
**footnotes** and **pull quotes** for `source` objects, and the general mechanism for any object reference.
**Origin:** Doc 1 §3.4.1 (`Scrivi_Worldbuilding_Object_Model_v0_2.md`) + Doc 2 §3.1.1.

**Why this is an Epic and not a task.** Attaching a citation to an object is one relation type (EP-031 does
it). Putting a citation *into the manuscript* is a different thing entirely — Scrivi has no way to render an
object inside scene text today:

- scene bodies are plain Markdown (`SceneReader` / `SceneWriter`) with no reference syntax;
- the structured-fragment model (`scrivi.fragment.v1`, EP-029) carries scenes and chapters, **not** object
  references — so cut/copy/paste across a reference is undefined;
- nothing in the editor, the renderer, or export resolves an embedded reference, keeps it current when the
  referenced object changes, or handles the object being **deleted** or its world going **unavailable**
  (EP-031's pending/dangling distinction reaches into manuscript text at that point);
- undo/redo (EP-019) must treat a reference as a unit, not as the characters that spell it.

**Rough scope:** a reference syntax + schema in scene bodies; fragment-model support so references survive
structured cut/copy/paste; resolution + live rendering in the Apple and Linux editors; footnote and
pull-quote presentations for `source`; export behaviour; repair-matrix rows for a reference whose target is
deleted or pending; a `source`→`scene` relation type (**additive** — Doc 1 §3.4.1 confirms no schema change
or edge migration is implied).

**Depends on:** EP-031 (the object model and graph), EP-029 (the fragment model it must extend).
**Explicitly deferred from:** EP-031 / T-0365, which delivers sources as objects **only**.

---

## EP-033: [Cross] World Lifecycle Management — in-app view vs. dedicated application

**Status:** 🔵 **Proposed** (opened 2026-08-14 from the I-0118 design ruling)
**Codebase:** `[Cross]` — undetermined by design; **the product-boundary decision is the Epic's first
deliverable**, and it decides whether this is a Scrivi view or a separate application.
**Goal:** Answer where a **world** is managed across its whole life — created, deleted, shared between
projects, repaired, and its search index maintained — and then build it.
**Origin:** I-0118 Q1 (2026-08-14). The ruling was *"entries persist; never reference-counted, never
deleted unless expressly instructed."* **No such instruction exists anywhere in the product**, and the
user deferred inventing one pending this larger decision.

**Why this is an Epic and not an affordance.** "Add a Remove-from-Spotlight button" was the small version
of the question. The real one is **who owns a world**, and the current answer is nobody:

- a world is **shared between projects** and outlives every one of them, so a per-project panel is
  structurally the wrong home for its lifecycle — every binding project would offer the same destructive
  action against shared state, with no coordination;
- **deleting a world** has no home at all today: `scrivi_remove_world_reference` unbinds *this project's*
  reference and deliberately never touches the package (`scrivi.h:264`), which is correct — but it means
  nothing in Scrivi can delete a world, by design;
- under the I-0118 ruling a world's **search entries are write-only** — donated, never removed, with no
  affordance to clear them even when the package is gone from disk;
- **a world with no project bound to it is unreachable.** Scrivi opens *projects*; a world is only ever
  seen through one. A writer with a world and no project has no way in.

**The fork to rule first:**

| Option | Consequence |
| --- | --- |
| **A — a view inside Scrivi** | Cheapest; reuses the Worlds panel and identity/bookmark plumbing. But the per-project framing stays wrong for shared state, and it cannot manage a world with no project. |
| **B — a dedicated world-management application** | Matches how worlds actually live: one owner, across projects, independent of any manuscript. Costs a second app (packaging, sandbox grants, identity, its own release), and Scrivi still needs *some* in-app surface for binding. |

**Rough scope (either way):** world delete (package + index entries, with the "positively established"
care of I-0115); explicit index-entry removal; a worlds registry or browse path that does not require a
project; sharing/binding across projects; repair for a world whose package moved or died.

**Depends on:** EP-031 (the world partition, bindings, and the world package format).
**Explicitly deferred from:** I-0118, which ships world **indexing** and leaves **removal** to this Epic.

---

*Last Updated: 2026-08-14 (**EP-033 `[Cross]` opened 🔵 Proposed** — world lifecycle management, deferred
out of I-0118 by user ruling. The I-0118 Q1 ruling ("entries persist; never deleted unless expressly
instructed") has **no instruction to give** — nothing in Scrivi deletes a world or its index entries, and
a world with no project bound is unreachable entirely. **The Epic's first deliverable is the product
fork:** a view inside Scrivi vs. a dedicated world-management application. Backlog Epics 2 → 3 (EP-026
`[Linux]`, EP-032 `[Cross]`, EP-033 `[Cross]`); next available Epic **EP-034**. Prior note follows.)*

*2026-08-12 (**EP-032 `[Cross]` opened 🔵 Proposed** — inline object references in the
manuscript, deferred out of EP-031 by user ruling: sources attach to objects now, source-in-manuscript
(footnotes / pull quotes) needs a capability Scrivi does not have and is version-crossing. Backlog Epics
1 → 2 (EP-026 `[Linux]`, EP-032 `[Cross]`); next available Epic **EP-033**.)*

*2026-07-22 (**EP-025 [Linux] Timeline Panel promoted from this backlog to 🟡 Active** (full
detail now in `Epic-active.md`); its first sprint SP-079 activated same day. The largest Linux Epic (full
Apple EP-016 parity, ~5 sprints). Remaining [Linux] family: **EP-026** stays 🔵 Draft (promoted when EP-025
lands). EP-019 [Apple] remains 🔴 Deferred. Prior: 2026-07-22 (EP-024 ✅ closed same-day); 2026-07-15 (EP-022
✅ closed; EP-023 promoted to Active).)*

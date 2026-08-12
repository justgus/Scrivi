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

*Last Updated: 2026-08-12 (**EP-032 `[Cross]` opened 🔵 Proposed** — inline object references in the
manuscript, deferred out of EP-031 by user ruling: sources attach to objects now, source-in-manuscript
(footnotes / pull quotes) needs a capability Scrivi does not have and is version-crossing. Backlog Epics
1 → 2 (EP-026 `[Linux]`, EP-032 `[Cross]`); next available Epic **EP-033**. Prior note follows.)*

*2026-07-22 (**EP-025 [Linux] Timeline Panel promoted from this backlog to 🟡 Active** (full
detail now in `Epic-active.md`); its first sprint SP-079 activated same day. The largest Linux Epic (full
Apple EP-016 parity, ~5 sprints). Remaining [Linux] family: **EP-026** stays 🔵 Draft (promoted when EP-025
lands). EP-019 [Apple] remains 🔴 Deferred. Prior: 2026-07-22 (EP-024 ✅ closed same-day); 2026-07-15 (EP-022
✅ closed; EP-023 promoted to Active).)*

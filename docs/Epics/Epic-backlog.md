# Epic Backlog

Epics listed here are fully defined but have no active sprint. Epics that were partially delivered and returned here retain their verified AC state. Work resumes when a sprint is assigned.

---

## Backlog Epics

_(EP-019 `[Apple]` Undo/Redo was un-deferred back to Active 2026-07-24 — now in `Epic-active.md`.)_

## [Linux] App — Epic Family (EP-024–EP-026)

The Ubuntu Qt/QML port of the macOS app, split one Epic per capability (mirroring how the Apple app was
built Epic-by-Epic). **EP-020 (Foundation), EP-021 (Project Lifecycle & Landing), EP-022 (Writing Surface &
Scene Navigator), EP-023 (Manuscript Structure Editing), EP-024 (Scene Inspector Panel) and EP-025 (Timeline
Panel) are all ✅ closed** (→ `Epics/Closed/`) — **EP-025 closed 2026-07-24**, the last of them.

**EP-026 below is therefore the only `[Linux]` Epic left, and it is still 🔵 Draft** — it was never promoted
when EP-025 landed, because `[Apple]`/`[Cross]` work (EP-027–EP-031) took priority. Promotion is a scheduling
decision that has not yet been made.

Each is verified in Docker+VNC (developer) then on real Ubuntu (alpha tester). Each targets near-parity with
the named Apple Epic. Any new `scrivi_*` endpoint a screen needs is a Task with a `[ScriviCore]` note, not a
separate Epic.

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

## EP-034: [Cross] Object Detail & Media

**Status:** 🔵 **Proposed** (opened 2026-08-18 from a user finding during SP-102)
**Codebase:** `[Cross]` — `[Apple]` first, `[Linux]` for parity. ⚠️ **Probably no ScriviCore change:**
`WorldObjectFields` already carries `subtitle`, `notes` and `image` (`assetID`/`thumbnailAssetID`), and
`scrivi_import_asset` / `scrivi_list_assets` already ship. **This is a missing surface, not a missing
capability.**
**Goal:** Let a writer **see and edit what an object actually is** — description, notes, and imagery —
and attribute it, rather than only naming it.

### Why this is an Epic

**The finding (user, 2026-08-18):** trying to use the new `sources` card, *"I can show the sources card,
but I can't create any sources. None of the card interfaces allow it."* True, and the cause is broader
than sources:

> ⚠️ **Object cards edit exactly ONE field — `displayName`.** There is a single `TextField("Name")` in
> `ObjectCard.swift:647`, and rename is the only mutation the app performs on an object.
> **`subtitle`, `notes` and `image` shipped in SP-095 (T-0371, ✅ Verified 2026-08-12) and the app reads
> and writes none of them.**

This is the same defect shape EP-031 has produced repeatedly — *capability shipped in the core, surface
never built* (the SP-099 R4 audit, I-0117, and `listPendingEdges` with zero call sites). It is the
largest remaining instance.

**The writer's case, in his words:** *"A Chronicle must have an actual chronicle, that is, a story that
it chronicles. Characters need more than just a name. A description and maybe even a picture, drawing,
sketch. A Location needs a description to set the mood and maybe an image to set the right mood. A map
is an image."*

**And the attribution consequence, which is what ties sources to this Epic:** *"If we allow images into
the App for worldbuilding, then we must also be able to cite the correct attribution of those images.
Also, a chronicle may be sourced from another project, or another author's text. That text must be able
to be attributed via a source."*

### ⚠️ Why source creation belongs here and not on the `sources` card

Adding a source *from* the aggregate `sources` card was considered and **rejected by the user**:
*"it would muddy the fact that the source must be associated with an object in the world."*

A citation documents an **object** — a map's image attribution belongs to the map, a chronicle's
provenance belongs to the chronicle. The `sources` card is scene-scoped and aggregate, so creating from
there inverts the relationship the model is built on (Doc 1 §3.4). **Creation belongs where the thing
being documented lives: the object's own detail surface.**

### ⚠️ EP-032 does not cover this

Checked at opening. EP-032 is **reference syntax inside manuscript text** — footnotes and pull quotes,
depending on the fragment model. Object detail has no home in any existing Epic; this work was
genuinely unplanned.

### Rough scope

- An **object detail surface** — view and edit `displayName`, `subtitle`, `notes`; reachable from an
  object card and from the object picker.
- **Long-form text** for `notes`, sized for a chronicle's actual text rather than a one-line field.
- **Image display and import** — `image.assetID` / `thumbnailAssetID` through the existing
  `scrivi_import_asset`; thumbnails on cards, full image in detail. **A `map` is an image**, so this is
  what makes that kind meaningful at all.
- **Source creation + `cites` attachment from the documented object**, closing T-0365's write half and
  making the `sources` card reachable.
- **Attribution for imported images** — the case that forces sources and media into one Epic.
- `[Linux]` parity.

**Depends on:** EP-031 (object model, graph, world partition — `subtitle`/`notes`/`image` and the
`source` kind all land there).
**Unblocks:** **T-0365**, whose `sources` card ships in SP-102 read-only and **cannot show content until
this Epic provides a way to create a source.**

---

*Last Updated: 2026-08-18 (**EP-034 `[Cross]` opened 🔵 Proposed** — object detail & media, from a user
finding while testing T-0365's `sources` card: sources cannot be created anywhere in the app, and the
cause is that **object cards edit `displayName` only** while `subtitle`/`notes`/`image` have shipped in
ScriviCore since SP-095 and are read by nothing. ⚠️ **Source creation was deliberately NOT put on the
`sources` card** — user ruling: a citation documents an object, so creating it from a scene-scoped
aggregate card inverts the model. **EP-032 was checked and does not cover this** (it is manuscript
reference syntax). Backlog Epics 3 → 4 (EP-026 `[Linux]`, EP-032, EP-033, EP-034); next available Epic
**EP-035**. Prior note follows.)*

*2026-08-14 (**EP-033 `[Cross]` opened 🔵 Proposed** — world lifecycle management, deferred
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

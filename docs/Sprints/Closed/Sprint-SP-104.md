# SP-104 — `[Cross]` Post-ruling scope fallout: world reachability & the restated-kind class

**Status:** ✅ **CLOSED 2026-08-15 (user-approved).** All four Issues ✅ Resolved - Verified.
**Epic:** EP-031 `[ScriviCore]` Worldbuilding Object Model & Relationship Graph
**Codebases:** `[ScriviCore]` + `[Apple]`
**Opened:** 2026-08-14 — unplanned, during live verification of the SP-103 scope change.

---

## Why this sprint exists

It was not planned. The user, exercising the app after T-0409, reported: **"Currently I cannot create
objects in the app."** Everything here was found by troubleshooting that one sentence.

The ruling in SP-103 moved every worldbuilding kind into the world. That was a small change at the core —
one predicate, as planned. What it exposed was that **the surrounding system still described the old
partition in four separate places**, and that a world outside the project package was never actually
reachable on Apple at all. None of it was visible while worlds held only artifacts.

> ⚠️ **The blocker was not where the sprint report predicted.** SP-103's progress note said the remaining
> work was 11 test realignments and T-0410's worldless-project prompt. The actual blocker was a **stored
> `isWorldScoped` flag in Swift** that no ScriviCore test could ever see.

---

## Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| I-0114 | ⚠️ **Class issue** — a kind list restated rather than derived (4th + 5th occurrence) | **Critical** | ✅ **Verified (2026-08-14)** |
| I-0115 | A present-but-unreadable world reported as `missing` | High | ✅ **Verified (2026-08-14)** |
| I-0116 | World packages had no persisted sandbox grant | High | ✅ **Verified (2026-08-14)** |
| I-0117 | Manage Worlds could not remove or relocate a world | Medium | ✅ **Verified (2026-08-14)** |
| I-0118 | ⚠️ World contents are not Spotlight-indexed — a regression in reach | Medium | ➡️ **Ruled here, implemented in SP-105** (✅ Verified 2026-08-14) |

## Assigned Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0411 | Finish SP-103's test realignment (inherited: 11 failures) | ✅ **Complete (2026-08-14)** — ctest **513 / 513** |

Every Issue began as a defect found in use; T-0411 was inherited from SP-103.

---

## The finding that matters most

**I-0114 is the fourth and fifth occurrence of one defect shape in a single Epic.** The prior three were
I-0113 (the ABI kind gap), SP-098's `source` table, and SP-103's `kScannedKinds`. Each was fixed
individually; the class was never addressed.

**The standing rule was insufficient, and its insufficiency is now demonstrated rather than argued.**
The rule says *grep for other dispatch lists before adding a kind*. Both new occurrences broke without
anyone adding a kind and without either list being edited — they went stale when a kind's **scope**
changed underneath text that still read correctly.

**The structural cause was access, not discipline.** The canonical list `kAllStorableKinds` sat in
`ObjectIndex.cpp`'s anonymous namespace, unreachable from any other translation unit. `WorldStore` could
not have used it even if the author had looked. A rule asking people to find a list they cannot reference
was never going to hold.

**Standing rule, superseding the earlier two:**

> **Any list that partitions or enumerates `ObjectKind` must be DERIVED from `kAllStorableKinds` and
> `objectKindIsWorldScoped()`, never restated — in any language.** A restatement is a defect on sight,
> even when currently correct. This applies to Swift and QML as much as C++; occurrence 5 was the first
> in Swift, where the C++-side habit had no reach.

---

## Verification notes

**Probed through `scrivi_*`, not the facade** — per the standing rule that a facade test cannot see a
boundary gap (the lesson of I-0113):

```
create WITH worldID:    ok → /Volumes/Scrivi Worlds/Eskandar.scrivworld/characters/probe-ada.json
create WITHOUT worldID: {"ok":false,"detail":"worldRequired"}
```

This is what proved the core was healthy and the blocker was app-side — before any app code was changed.

**I-0115's regression test was verified to reproduce the defect**, not merely to pass: it FAILS against
the pre-fix logic and passes after. A test that only ever passed would not have shown the behavior was
real.

⚠️ **Two of these were latent data-integrity bugs, not cosmetic.** I-0115 told a writer an intact world was
*missing* — the exact status Doc 3 §4.6 reserves for positive proof, precisely because it invites
destructive remedies. I-0116 made that world unreadable on every relaunch. Together they would have
presented a healthy world on an external volume as gone.

---

## T-0411 — the inherited 11 failures, now 0

Cleared 11 → 0. **None was a defect in the SP-103 change itself**, which the original triage predicted
correctly. But two were **not** simple realignments, and both would have gone green while testing nothing:

- **`extractSearchableText` — the failure was hiding a user-facing regression, not a stale assertion.**
  The tests demanded that characters/locations/items be searchable; they no longer are (**I-0118**).
  Realigning them to "expect zero" without noticing *why* would have quietly ratified the loss. The
  assertions now state the honest current behavior, carry a `source` control proving the extractor still
  works, and are written so that **fixing I-0118 fails them loudly**.
- **`a cross-partition edge resolves like a same-partition one (AC10)`** — its endpoints were `character`
  ↔ `artifact`, which since T-0409 are **both world-scoped**. The test would have passed while exercising
  no partition boundary at all — the same vacuous-pass trap SP-103 caught in the AC2 scan counter.
  Rewritten onto `source` ↔ `artifact`, the only genuinely cross-partition pair left.

Two more were self-contradictory rather than merely stale: the demotion test's own comment said scope is
no longer cleared while its last assertion still required `worldID` to be empty; and the load-time-repair
test removed the *project* index to force a rebuild of an object that now indexes in the *world*.

A `kindDir()` / `worldIndexPath()` helper was added to the fixtures so tests stop hardcoding
`projectDir/"objects"/<subdir>` — the same derive-don't-restate rule as I-0114, applied to test code.

## Results

- **ctest 513 / 513 — all green** (was 500/511 at sprint start).
- **macOS app BUILD SUCCEEDED**; ScriviCore builds clean.
- **Three regression tests added**, each asserting a *derived* set rather than a literal list.
- **User-confirmed:** characters now create from the card and land on disk in the world package.

## Not done

- **I-0118 — ✅ RULED here; IMPLEMENTED AND VERIFIED in SP-105 (2026-08-14).** World contents are still not Spotlight-indexed,
  which since T-0409 means no worldbuilding object is findable. The four design questions are now answered
  (see the ruling block in `Issue-active.md`): **world-bound domains, never reference-counted, never
  auto-deleted**; **world-scoped deep links**; **stale entries stay** when a world is offline; **index the
  whole package**. ⚠️ The "never auto-delete" ruling **removed** the refcounting/unbind-hook subsystem the
  question seemed to require — the lifecycle problem does not exist if nothing deletes on a side effect.
  **One sub-question remains open:** what the "express instruction" to delete a world's entries actually
  is — no such affordance exists in the UI today, so as specified the entries cannot be removed at all.
- **T-0410**, the worldless-project prompt. The core half is in place (`detail == "worldRequired"` is the
  discriminator the app reads); the `[Apple]` half is unbuilt. **Now partially mitigated** rather than
  fixed: a writer can reach Worlds → Create, and I-0117 gives the panel honest scope text.
- **Linux** — not built or run this session. `WorldsView` is Apple-only; the `[ScriviCore]` fixes
  (I-0114b, I-0115, and the derived kind lists) are cross-platform and carry no Qt-side work, but have
  not been exercised there.

*Last Updated: 2026-08-14.*

---

## Retrospective (closed 2026-08-15, user-approved)

**Unplanned, and it existed because one sentence from live use — "I cannot create objects in the app" —
turned out to sit on four separate defects.** Two were latent data-integrity bugs (an intact world
reported as `missing`; a world unreadable on every relaunch) that together would have presented a healthy
world on an external volume as gone.

⚠️ **The blocker was not where the prior sprint's report predicted.** SP-103 said the remaining work was
11 test realignments plus a prompt. The actual blocker was a **stored `isWorldScoped` flag in Swift** that
no ScriviCore test could ever see — the 5th occurrence of this Epic's restated-kind defect, and the first
outside C++.

**The durable output is the rule, not the four fixes:** *any list that partitions `ObjectKind` must be
derived, never restated, in any language* — now in `CLAUDE.md` and Doc 1, with `kAllStorableKinds` promoted
out of an anonymous namespace so the rule is followable. The prior rule failed four times because it asked
people to reuse a list they could not reach.

**At close:** ctest **516/516**, macOS interop **86/86** (both re-run after SP-105), app **BUILD SUCCEEDED**.

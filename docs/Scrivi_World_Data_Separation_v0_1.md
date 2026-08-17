# Scrivi — World Data Separation Design (v0.1)

**Epic:** TBD (precedes EP-B; see `Scrivi_Worldbuilding_Object_Model_v0_2.md` §10)
**Sprint:** TBD (design)
**Status:** ✅ **APPROVED 2026-08-05 (Human).** This is **Doc 3 of 3**; all three are now approved.

| Doc | File | Status |
| --- | --- | --- |
| **1** | `Scrivi_Worldbuilding_Object_Model_v0_2.md` | ✅ **Approved 2026-08-05** |
| **2** | `Scrivi_Scene_Inspector_Card_Framework_v0_1.md` | ✅ **Approved 2026-08-05** |
| **3** | `Scrivi_World_Data_Separation_v0_1.md` *(this doc)* | ✅ **Approved 2026-08-05** |

Trades W1–W6 ruled 2026-08-04 (§4); design body (§§6–9) drafted and ruled 2026-08-05. Implementation follows this
doc; per CLAUDE.md any deviation must be surfaced and reconciled before it is built.
**Date:** 2026-08-04
**Author:** Claude (planning), for user review.
**Depends on:** `Scrivi_Worldbuilding_Object_Model_v0_2.md` (Doc 1) §7 — the `worlds/` partition boundary and the
project/world timeline split are **ruled there** and are inputs here, not open questions.

---

## 1. Why this document exists

Doc 1 (§7) establishes the **boundary**: a `.scrivi` package has a project partition (`objects/`) and a world
partition (`worlds/<worldID>/`), a project may span multiple worlds, and the object index spans both so edges
resolve uniformly across them.

What Doc 1 deliberately does **not** settle is **ownership**. Two subsystems are specified as owning data the
ruling places in world scope:

- `objects/historical-events/` — Timeline v0.3 **FR-051**
- `objects/imported-timelines/` — Timeline v0.3 **FR-070**

That is a cross-epic ownership change (EP-016 / EP-025). Folding it into Doc 1 would have put a
timeline-ownership renegotiation inside a document about the object graph, and would have left EP-B inheriting an
unreconciled contradiction. It gets its own doc and its own reconciliation.

> **Note (2026-08-05):** because **Scrivi has not shipped**, this is a change to *specified* locations, not to
> data on disk. Timeline v0.3's FR-051/FR-070 paths are corrected to world scope; no user data exists in the old
> locations, so there is nothing to migrate (§7).

---

## 2. The ruling this document implements

> **Project timeline** — the manuscript's own story-time. Authored by writing scenes, sourced from scene
> sidecars. Owned by the Timeline Panel. **Project-scoped.**
> **Historical timelines** — derived from an external history. That history belongs to the **world**.
> **World-scoped.**

Provenance is the cut: the project timeline is *authored*; historical timelines are *derived*. This also places
`chronicle` (world history as prose) and historical events (world history as dated points) on the same side of
the line.

---

## 3. Scope

**In scope:**

1. **Historical events live in world scope** (`worlds/<id>/historical-events/`), including how the Timeline Panel
   reads them from there.
2. **Historical timelines live in world scope**, each with its own epoch, offsets **world-relative** (Doc 1
   §7.0). *(Earlier drafts specified migrating and re-basing existing project-scoped imported timelines; with
   Scrivi unshipped there is no such data — see §7.)*
3. **Optional scene → world association** (W3). A writer *may* associate a scene with a world; she may equally
   leave it unassociated, which is a valid permanent state — not an incomplete one. When she chooses to
   associate, she picks from the worlds **already in the project**. Associating with a world outside the project
   is a **different action in a different part of the UI** (adding a world to the project — W6), never folded
   into this picker.
4. **Package Structure v0.1 §11 revision** for the `worlds/` partition.
5. **World initialization** — what happens when a writer creates or adds a world (§7).
6. **Cross-project world sharing** — Doc 1's deferred T5-B: an external world store, and what
   `scrivi_import_world` (stubbed in Doc 1 §6) actually does.

**Out of scope:** the object model itself (Doc 1), inspector card UI (Doc 2).

---

## 4. Trade studies — **all ruled 2026-08-04**

| # | Question | Ruling |
| --- | --- | --- |
| **W1** | Where historical events live | ✅ **A** — `worlds/<id>/historical-events/`. Historical events are world data and live there natively. |
| **W2** | Epoch ownership | ✅ **Three-layer chain — see Doc 1 §7.0.** Neither original option. Each **historical timeline** owns its epoch (timelines in one world need not share a reference point); each **world** owns its epoch; each **project** owns the translation (`binding.json`). Timeline offsets are **world-relative**, so rebinding a world changes one number. |
| **W3** | Scene ↔ world association | ✅ **Optional, writer-chosen, project-scoped picker.** *(Trade re-posed — the original options assumed the inspector would present/derive world content. It does not; see §4.1.)* |
| **W4** | World timelines in the Timeline Panel | ✅ **D (new option) — manual only.** World timelines are **never** auto-included; the writer brings a world timeline into the viewer at her discretion. None of A/B/C applied: all three assumed automatic inclusion. |
| **W5** | When world setup runs | ✅ **C** — lazily, on first world creation. A project with no worlds never pays a cost. |
| **W6** | Cross-project sharing | ✅ **A** — a separate package per world (`.scrivworld`). |

### 4.1 W3 — why the trade was re-posed

The original W3 options (sidecar field · derived from `location` edges · project default + override) all rested on
an assumption the user corrected: that the Scene Inspector would **present** world content it judged relevant.

> **Ruled:** everything appearing in the Scene Inspector with regard to worlds is **manually surfaced by the
> writer**. Nothing is presented, proposed, or auto-populated.

That makes "which world scopes this scene" the wrong question. The rulings are:

1. **Scene → world association is optional.** A writer may associate a scene with a world; she may equally never
   do so. **Unassociated is a valid permanent state**, not an incomplete one.
2. **The world picker lists every world in the project.** Not a filtered subset.
3. **The object picker lists objects from *all* worlds in the project — deliberately unfiltered by the scene's
   world association.** This is the load-bearing part, and it inverts the obvious design:

> **Why the object picker must not filter by world:** the writer may not have decided which world the scene
> belongs to yet. *Adding the objects she needs is how that decision gets made.* Filtering the picker by a
> scene→world association would gate the picker on a decision the picker itself exists to help her reach.

4. **Objects from worlds not in the project are out of scope for this picker.** Bringing in an outside world is a
   different action in a different part of the UI (W6), never folded into the scene picker.

**Model consequence:** W3 needs almost no new state — an optional `worldID` on the scene sidecar, written only
when the writer explicitly associates. No default, no inheritance, no derivation. Note that "which worlds does
this scene touch" is *already derivable* from the Doc 1 relationship graph (scene edges → objects → `worldID`),
so the explicit association is a **writer's declaration**, not a computed fact — the two may legitimately differ,
and the declaration wins where it exists.

### 4.2 W4 — manual inclusion only

None of the sketched options applied; all three (row-per-world · merged with tinting · filter/selector) presumed
world timelines would be automatically included and then arranged or filtered. **They are not included at all
until the writer brings one in.** The Timeline Panel shows the project timeline; a world timeline appears only by
explicit writer action, consistent with the manual-surfacing principle in §4.1.

This aligns with existing behavior: Timeline v0.3 already requires an explicit "Import Timeline" action (FR-066)
and per-row visibility toggles (FR-065). W4 extends that stance to world timelines rather than inventing a new
one.

### 4.3 W5 — world setup runs lazily, on first world creation

World setup runs when a project **first creates or adds a world**, not on every open. A project that never uses
worlds does nothing and pays nothing.

> **Tension to resolve when drafting (surfaced, not settled):** W5=C sits somewhat against the EP-027
> repair-on-open precedent, where self-consistency is established *before* validation on every open. The
> reconciliation is that world setup is **triggered by an explicit user action** (creating/adding a world)
> rather than by opening a project, so it is a one-time transition rather than an invariant to maintain. The
> repair-on-open machinery still applies **after** a project has worlds — a world-bearing project must still open
> self-consistent. Drafting should state this boundary precisely so the two patterns don't collide.

### 4.4 W6 — a separate package per world (`.scrivworld`)

A world is its **own package**, not a folder inside a project. This is what makes a world genuinely shareable
across a series of projects rather than copied into each.

```text
Midgard.scrivworld/
  world.json                    ← the world's own epoch (Doc 1 §7.0 layer 2)
  artifacts/  rules/  chronicles/
  historical-events/            ← W1
  historical-timelines/         ← each with its own epoch, world-relative offset
  assets/
```

The project references worlds it uses and stores **only** the project-local binding:

```text
MyNovel.scrivi/
  worlds/
    <worldID>/binding.json      ← epochOffsetMs + reference to the world package
```

**W2's ruling already drew this line** (Doc 1 §7.0): `world.json` and its timelines are shareable; `binding.json`
is project-local and regenerated per project. W6=A simply gives the shareable side its own package boundary.

> **To be designed when drafting:** a referenced world package is **external to the project**, so everything
> Scrivi already knows about external change applies to it — a world may be missing, moved, read-only, or edited
> by another project between opens. This is squarely `Scrivi_External_Change_Repair_Matrix_v0_2.md` territory,
> and drafting must **extend that matrix** with world-package conditions rather than inventing a parallel
> mechanism. Unavailability is already ruled (§4.6, pending edges).

### 4.4.1 World references must be platform-neutral (ruled 2026-08-04)

**The referencing mechanism shall not be platform-specific.** Scrivi targets macOS, iOS, iPadOS, visionOS,
Windows, Android, and Linux; a reference scheme built on one platform's file-access model would fragment
ScriviCore, whose whole premise (Architecture v0.3) is one shared C++ backend with platform layers doing UI only.

This **rules out** making security-scoped bookmarks (Apple) or equivalents the *model*. Constraints for drafting:

- The stored reference is **portable data** — a world identity plus a relative/logical locator that means the
  same thing on every platform, resolvable by ScriviCore with no platform API.
- **Platform-specific access tokens are an implementation detail of the platform layer, not the reference.** If
  macOS sandboxing requires a bookmark to *open* a path, that bookmark is a per-device access artifact stored
  outside the portable reference (the platform layer's business) — it is never the identity of the world.
- A project moved between platforms (or synced) must resolve its world references without rewriting them.
- Resolution failure is **not** an error state — it is §4.6 pending.

> **Settled in §6.4:** `worldID` is the identity; a last-known relative path and absolute path are *hints*
> verified against it. **There is no search and no known-locations registry** — the writer organizes worlds as she
> likes, picks them with a standard file-open dialog, and relinks at her convenience if one moves.

### 4.4.2 World write concurrency — lock/write/unlock (ruled 2026-08-04)

**Reads are simultaneous; writes are atomic.** Merely *having* a world open — in any number of projects — affects
nothing. Only writes serialize.

- **Any number of projects may read a world concurrently.** Opening a world is not an exclusive operation and
  never blocks another project.
- **Writes to a world are atomic**, guarded by a **lock → write → unlock** mechanism so two projects cannot
  interleave writes to the same world.
- The lock is held for **the duration of a write, not the duration of an open session.** A project that holds a
  world open indefinitely must not thereby block every other project from ever writing.

> **To settle when drafting:** the lock must be **platform-neutral** (§4.4.1) — most likely a lock file inside
> the `.scrivworld` package rather than an OS advisory lock, whose semantics differ across the target platforms
> and behave poorly on network volumes. Required behaviors: **stale-lock recovery** (a crashed writer must not
> lock a world permanently — likely a PID/host/timestamp record with a timeout), what a writer does when the lock
> is held (brief wait then report, never hang), and whether ScriviCore's existing `AtomicWrite` composes with the
> lock or is subsumed by it. Note this is *cross-process* mutual exclusion, which `AtomicWrite` alone does not
> provide — it makes a single write atomic, not a read-modify-write sequence across processes.

---

## 4.5 The governing principle — **the writer never waits for infrastructure** (ruled 2026-08-04)

Several rulings in this doc look like separate accommodations. They are one principle, and it overrides the
convenience of the implementation in every case:

> **The writer must never be blocked, delayed, or interrupted from writing by the state of world infrastructure.**
> Missing worlds and unresolved references are conditions to be *surfaced and deferred*,
> never conditions that gate opening a project or editing prose.

This principle produced W5=C (lazy world setup — §4.3), the pending-edge model (§4.6), and the non-blocking
warning surfaces below. Where it conflicts with an established Scrivi pattern — notably EP-027's
repair-on-open — **this principle wins**, and the deviation is deliberate rather than an oversight.

> **Explicit reconciliation with EP-027:** repair-on-open established that a project must be self-consistent
> *before* validation. That precedent stands for **project-internal** data, which is always present and can be
> repaired in bounded time. It does **not** extend to **external world packages**, whose availability is outside
> the project's control and whose repair could block indefinitely on something the writer may not care about
> right now. The boundary is: *project-internal state repairs on open; world-external state goes pending.*

## 4.6 Pending edges — absence is never deletion (ruled 2026-08-04)

When a referenced world is **unavailable** — see the three states below — edges touching that world's objects are
held **pending**. They are neither resolved nor pruned.

**Three unavailability states (ruled 2026-08-05).** The pending behavior is **identical** in all three; only the
*reported status* differs, so the writer can tell what to actually do about it:

| Status | Condition | Writer's remedy |
| --- | --- | --- |
| **offline** | World package is on a **remote/network volume unreachable in the current network configuration**. | Reconnect network/VPN |
| **unmounted** | World is on a **volume that is not mounted** (e.g. removable media). | Mount the drive |
| **missing** | Containing folder **is available and mounted**, the reference was **not** a link elsewhere, and the package **is absent**. | Relink or restore |

Status detection is a **best-effort diagnostic** in the platform layer feeding a platform-neutral enum (§4.4.1);
where it cannot be determined it falls back to a generic **"unavailable"** — **never a guess**. A wrong "missing"
is materially worse than an honest "unavailable", because only "missing" invites the writer toward destructive
remedies (clearing references, restoring from backup) when the world may be perfectly intact on an unreachable
volume.

**Hard rules:**

1. **No edge between a project and a world may be removed or added while that world is offline.** The graph is
   frozen with respect to that world — both directions. Cascade-prune (Doc 1 §5.5) **must not** fire on an
   unresolvable endpoint when the cause is world unavailability.
2. **Pending is a distinct state from broken.** An endpoint that fails to resolve because its *world is absent*
   is pending; an endpoint that fails to resolve while its world *is present* is genuinely dangling and repairs
   per Doc 1 §5.5. **The loader must distinguish these two cases**, because treating the first as the second
   destroys the writer's relationships.
3. **Pending edges survive save.** A project saved while a world is offline must not write away the pending
   edges; they persist verbatim until the world returns.
4. **Restoration is non-destructive.** When the world comes back, pending edges resolve normally with no
   repair pass and no writer intervention.

> **Why this is the highest-severity rule in the doc:** the failure is *silent and unrecoverable*. A writer opens
> her project on a machine where the world package isn't mounted; cascade-prune reads "unresolvable" as "deleted"
> and destroys every relationship into that world. Nothing errors. She may not notice for weeks, and the data is
> gone. Every other hazard in this document is recoverable; this one is not.

**Writer-facing surfaces (non-blocking, all deferred to Doc 2 for design):**

- A **"Worlds" menu** carrying a global **"Remove All World References"** action — the writer's explicit escape
  hatch, never automatic.
- A **warning surface** — in the Scene Inspector and/or a dedicated warning/log view ~~beneath the Timeline
  view~~ **with its own visibility control** (amended 2026-08-17, see below) — reporting that a world is
  unavailable and edges are pending.
- Neither surface blocks writing. They inform; the writer decides if and when to act.

> ⚠️ **Amended 2026-08-17 (user-ruled at SP-102 planning, R1) — the warning view is NOT anchored to the
> Timeline.** This section originally placed it *"beneath the Timeline view"*. In the shipped Apple app the
> Timeline is a **hidden-by-default strip** toggled via View ▸ Show Timeline, so anchoring the warning view to
> it would make **the only project-wide pending report invisible exactly when the writer is not using the
> Timeline** — i.e. most of the time. That defeats the surface's entire purpose, which is to ensure a writer
> is never silently unaware that her world is away.
>
> **Ruled:** the warning view is a strip below the manuscript with its **own View-menu toggle**, independent
> of the Timeline; it **auto-reveals when a bound world becomes unavailable**, is dismissible, and is absent
> when nothing is pending. The *intent* of this section is unchanged — a passive, non-blocking report the
> writer may ignore. Only the anchoring is corrected.
>
> **"Remove All World References" stays in the Worlds menu** and must **never** appear in the warning view:
> placing a destructive global action inside the surface that appears automatically when something looks
> broken is precisely the hazard §4.6 exists to prevent.
- Repair/restore UI (relinking a moved world, reconciling a changed one) is **future work**, explicitly not
  designed here.

---

## 5. Known constraints (inputs, already ruled)

- The **object index spans both partitions** (Doc 1 §4.2) — cross-partition edges must resolve identically to
  same-partition ones. **Resolved in §6.3: the project index _caches_ world entries.**
- **Edge endpoints are bare `{id}`** (Doc 1 §5.2) — moving an object between partitions must **not** require
  rewriting edges, exactly as `item`→`artifact` promotion does not.
- World-scoped kinds are `artifact`, `rule`, `chronicle`, `faction` (Doc 1 §3, §3.3); project-scoped are
  `character`, `location`, `building`, `vehicle`, `item`, `map`, `source`.
- Repair-on-open **before** validation is the established pattern for filesystem-authoritative repair
  (EP-027 / SP-070, which fixed C6 this way) — bounded by §4.5 to project-internal state.

---

## 6. On-disk design

### 6.1 The `.scrivworld` package

```text
Midgard.scrivworld/
  world.json                     ← identity + intrinsic epoch (Doc 1 §7.0 layer 2)
  index.json                     ← this world's object index (scrivi.object-index.v1)
  artifacts/    <slug>.json
  rules/        <slug>.json
  chronicles/   <slug>.json
  factions/     <slug>.json
  historical-events/    <slug>.json      ← W1
  historical-timelines/ <slug>.json      ← W1; each with its own epoch (Doc 1 §7.0 layer 1)
  assets/       images/ …
  .lock                          ← write lock (§6.5); absent when no write is in flight
```

```jsonc
// world.json
{
  "schema": "scrivi.world.v1",
  "worldID": "world_01J…",          // stable identity; the ONLY durable way to name this world
  "displayName": "Midgard",
  "epoch": { "label": "The First Sundering" },
  "createdAt": "…", "modifiedAt": "…",
  "formatVersion": 1
}
```

A world package is **self-contained**: it carries its own object index and assets, so it can be moved, copied, or
opened by another project with nothing left behind.

### 6.2 The project side — bindings only

```text
MyNovel.scrivi/
  worlds/
    world_01J…/
      binding.json               ← this project's epoch translation + reference
```

```jsonc
// binding.json
{
  "schema": "scrivi.world-binding.v1",
  "worldID": "world_01J…",
  "displayName": "Midgard",          // cached for display while the world is unavailable
  "epochOffsetMs": -94608000000,     // world epoch → project timeline (Doc 1 §7.0 layer 3)
  "reference": {                     // platform-neutral (§4.4.1)
    "lastKnownPath": "../Worlds/Midgard.scrivworld",   // RELATIVE where possible
    "lastKnownAbsolutePath": "/Volumes/Shared/Worlds/Midgard.scrivworld",
    "volumeLabel": "Shared"          // hint only; never authoritative
  },
  "cachedIndex": [                   // §6.3 — enables named pending entries
    { "objectID": "artifact_01J…", "kind": "artifact", "displayName": "Sword of Dawn" }
  ]
}
```

**The `worldID` is the identity; every path is a hint.** Paths are the least portable data available
(§4.4.1), so resolution (§6.4) treats them as candidates to be *verified* against `world.json`'s `worldID`, never
as proof.

### 6.3 The object index caches world entries (resolves the Doc 1 §4.2 tension)

Doc 1 §4.2 assumed one package. With external worlds, the project index must cover objects the project neither
owns nor can guarantee are present. **Ruling: the project caches world index entries** in `binding.json`
(`cachedIndex`) rather than composing live per-world indices at open.

| | Cache (**chosen**) | Compose at open |
| --- | --- | --- |
| World available | resolves; refreshed from the world's `index.json` | resolves |
| World **unavailable** | **still names objects** — "⟨Midgard: Sword of Dawn⟩" | entries vanish; only opaque IDs remain |
| Staleness | possible; refreshed on every successful open | none |

Caching wins for one reason that outweighs staleness: **it is what makes §4.6's pending presentation possible.**
Without cached names, a pending card (Doc 2 §7.2) can only show UUIDs, and a writer asked whether to clear
references cannot see what she would be discarding. Staleness is self-correcting — the cache refreshes whenever
the world is reachable — whereas an unnamed pending edge is a decision made blind.

**The cache is never authoritative.** When the world is available, the world's own `index.json` wins and the
cache is overwritten. The cache is only consulted while the world is unavailable, and **only for display**.

### 6.4 Resolution — platform-neutral, identity-verified

**No search, no known-locations registry** (ruled 2026-08-05). **Organization of worlds on the filesystem is
left entirely to the writer**; the project simply keeps the path.

- **Adding an existing world:** the writer selects the `.scrivworld` package with a **standard file-open dialog**.
- **Creating a world:** the writer is asked where to put it on the filesystem — again a standard dialog.
- **The project records that path.** If the world later moves, it is flagged **missing** (§7.2.1) and the writer
  is offered the chance to **relocate it at her convenience** (`scrivi_relink_world`, §8) — never on a blocking
  prompt, per §4.5.

Resolution is therefore short; the first candidate whose `world.json` carries the **matching `worldID`** wins:

1. `reference.lastKnownPath` resolved **relative to the project package** — this is what survives moving a
   project and its worlds together, so it is tried first.
2. `reference.lastKnownAbsolutePath`.

If a candidate exists but its `worldID` **differs**, it is **not** the world — resolution stops and the world is
missing. This is what prevents a same-named package from being silently substituted: a world's name is a label,
its `worldID` is its identity.

On success the binding's paths are **refreshed**, so a world that moved with its project is found directly next
time. On failure the world is **unavailable** (§4.6) with a status per §7.2.1 — never an error, never a block,
and always relinkable later.

> **Why no search.** A filesystem scan would guess at the writer's organization, and §4.5 forbids making her wait
> while the app hunts. An explicit relink is faster, predictable, and honest about what the app knows: it has one
> recorded path, and when that path stops working it says so and asks.

> **Platform layers may supply access tokens, not identity** (§4.4.1). Where an OS requires a token to open a
> path (macOS/iOS security-scoped bookmarks, Android SAF), the platform layer stores it **per device, outside the
> binding**, keyed by `worldID`. ScriviCore asks the platform layer to open a path; it never stores or interprets
> the token. A project synced between platforms carries only the portable reference.

### 6.5 Write locking — lock → write → unlock

Reads need no lock and never block (§4.4.2). Writes take an exclusive lock on the world package:

```jsonc
// Midgard.scrivworld/.lock   — created atomically; absent means unlocked
{
  "schema": "scrivi.world-lock.v1",
  "lockID": "lock_01J…",
  "holder": { "host": "studio-imac", "pid": 4711, "projectID": "project_01J…" },
  "acquiredAt": "2026-08-05T14:03:11Z",
  "heartbeatAt": "2026-08-05T14:03:41Z"
}
```

> ⚠️ **AMENDED 2026-08-12 (SP-098 planning audit) — `holder.host` and `holder.pid` are placeholders.**
> As implemented (SP-097 T-0383) the lock record writes `host: "local"` and `pid: 0`. **Neither is obtainable
> from ScriviCore**: the hostname and process ID are platform facts, and `CoreServices` exposes no provider for
> them (Architecture v0.3 keeps the core free of platform APIs). The fields are **diagnostic only** — nothing
> in the protocol reads them — so the lock's correctness is unaffected: exclusivity comes from
> `createFileExclusive`, and staleness from `heartbeatAt`, both of which work as specified.
>
> **The consequence is honest reporting, not safety.** A writer told "locked by another writer" cannot be told
> *which* machine or process holds it. Populating them properly needs a `CoreServices` host/PID provider
> supplied by each platform layer — worth doing when the warning surface (Doc 2 §7.2) is built and has
> somewhere to display it, **not** before. Until then the fields stay honest placeholders rather than guesses.

- **A lock file, not an OS advisory lock** — advisory-lock semantics differ across Scrivi's seven targets and
  behave poorly on network volumes, which is exactly where a shared world will live. A file is portable.
- **Acquire** = atomic create-if-absent (`AtomicWrite`'s exclusive-create path). Losing the race means waiting.
- **Held per write, never per session** (ruled) — acquire, write, release. Opening a world takes no lock.
- **Heartbeat** refreshed during long writes so a slow write is not mistaken for a dead one.
- **Stale-lock recovery:** a lock whose `heartbeatAt` is older than **60 s** is presumed dead and may be broken.
  A crashed writer therefore blocks others for at most a minute, never permanently.
- **Contention:** retry briefly (a few hundred ms), then **report and move on** — never hang. Per §4.5, a
  contended world write must not stall the writer; it surfaces in the warning view (§4.6) as a retryable state.

> **`AtomicWrite` does not subsume this.** `AtomicWrite` makes a *single file write* atomic within one process.
> The lock provides *cross-process* mutual exclusion across a **read-modify-write sequence** (read index → write
> object → update index). Related, not interchangeable; conflating them yields a lock that appears to work and
> silently doesn't.

---

## 7. World initialization (W5 = C — lazy, on first world creation)

> **Scope correction (ruled 2026-08-05): this is not a data migration.** **Scrivi has not shipped**, so there are
> no user projects in the field carrying `objects/rules/`, `objects/historical-events/`, or
> `objects/imported-timelines/` in the legacy layout. The world-scoped locations in Doc 1 §3 and §6.1 are simply
> **where these kinds live** — not a destination that pre-existing data must be moved to.
>
> Consequently: **no migration code, no legacy-layout detection, no re-basing pass, and no backward-compatibility
> path is written.** Any developer fixture still using the old layout is regenerated, not migrated. This removes
> what §7.1 previously called the highest-risk step in the document.

What remains is **initialization**: creating or adding a world, which runs only when the writer does so. A
project that never uses worlds does nothing here — §4.5.

**Create a world** (one atomic operation; failure leaves the project exactly as it was):

1. Ask the writer where to put the package (§6.4), then create it with `world.json`, a new `worldID`, and the
   writer-supplied epoch label.
2. Write `binding.json` with `epochOffsetMs = 0` and an empty `cachedIndex`.

**Add an existing world:** the writer selects the package (§6.4); the project writes `binding.json` with
`epochOffsetMs = 0` and caches the world's index (§6.3). Nothing in the world package is modified — adding a
world is a **read-only** act toward that world, so it takes no lock.

World-scoped objects are **created in world scope from the start**. An object *moving* between scopes remains
possible but is a deliberate writer action — `item` → `artifact` promotion (Doc 1 §3.1) — not an automatic pass,
and it preserves `objectID` so **no edge is ever rewritten** (Doc 1 §5.2).

### 7.1 Epoch offsets are world-relative from the start

Doc 1 §7.0's chain applies to world timelines as **authored**, with no conversion step:

```text
project_time(event) = event.offsetMs
                    + timeline.epochOffsetMs        // → world time  (world-relative, always)
                    + binding.epochOffsetMs         // → project story-time
```

A new binding starts at `epochOffsetMs = 0`, so a freshly added world sits on the project epoch until the writer
chooses otherwise. Changing it later shifts every timeline in that world coherently — one number, as designed
(Doc 1 §7.0).

> **What this replaces.** Earlier drafts specified a re-basing pass converting project-relative offsets
> (Timeline v0.3 §6.7) to world-relative ones, and flagged it as the highest-risk step in the document — an error
> there would have shifted every historical event silently. With no shipped data to convert, that pass and its
> risk are **deleted outright**. The Timeline Panel's *imported*-timeline format is unaffected; it remains what
> §6.7 describes for timelines imported into a project rather than held by a world.

### 7.2 `rule` location

`rule` objects live at `worlds/<worldID>/rules/` (ruled 2026-08-04) — there is no project-scoped `rule` and no
promotion path. **Package Structure v0.1 §11 must be corrected** to drop `objects/rules/`; this is a
documentation fix to an unshipped layout, not a migration.

---

## 8. C ABI (additive)

```c
/* Worlds */
const char* scrivi_create_world(const char* projectRootPath,
                                const char* displayName, const char* epochLabel,
                                const char* destinationPath);   /* writer-chosen location (§6.4) */
const char* scrivi_add_world(const char* projectRootPath, const char* worldPackagePath);
const char* scrivi_list_worlds(const char* projectRootPath);    /* + availability status */
const char* scrivi_remove_world_reference(const char* projectRootPath, const char* worldID);
const char* scrivi_relink_world(const char* projectRootPath,
                                const char* worldID, const char* newPath);

/* Availability (§7.2.1 states: available | offline | unmounted | missing | unavailable) */
const char* scrivi_get_world_status(const char* projectRootPath, const char* worldID);

/* Epoch chain (Doc 1 §7.0) — already specified there */
const char* scrivi_set_world_epoch_offset(const char* projectRootPath,
                                          const char* worldID, int64_t epochOffsetMs);
const char* scrivi_set_timeline_epoch_offset(const char* projectRootPath, const char* worldID,
                                             const char* timelineID, int64_t epochOffsetMs);

/* Pending edges (§4.6) */
const char* scrivi_list_pending_edges(const char* projectRootPath);
```

> **Implementation status after SP-098 (2026-08-12).** All of the above now ship, including
> **`scrivi_list_pending_edges`** (T-0380) — each row carries the pending endpoint's **cached displayName**
> (AC-A7), its `worldID`, and its status. `scrivi_list_edges_for` also flags a pending far endpoint
> (`otherPending` + `otherWorldStatus`), and the graph is **frozen** toward an unavailable world in both
> directions: create/delete edge refuse with `detail == "worldPending:<status>"`. `scrivi_get_world_binding` was added beyond this list (Doc 1 §6 declares it) and also ships.
> Signatures match as written. `scrivi_import_world` was never implemented — see the note below.
>
> One naming note: this section's `worldPackagePath` parameter ships as `packagePath`; the meaning is
> identical.

`scrivi_import_world` (stubbed in Doc 1 §6) is **superseded** by `scrivi_add_world`, which references rather than
copies — worlds are shared, not duplicated (W6=A).

---

## 9. Acceptance criteria

**Packaging & references**

- **AC-P1** A world is a self-contained `.scrivworld` package; moving it and relinking loses nothing.
- **AC-P2** A binding resolves by **`worldID`**, not by path or name. A same-named package with a different
  `worldID` is **rejected**, and resolution continues to the next candidate.
- **AC-P3** Resolution order is §6.4; a moved world found via a later candidate has its binding **refreshed** so
  the next open resolves directly.
- **AC-P4** No platform-specific data appears in `binding.json`. A project synced across platforms resolves its
  worlds without rewriting the binding.

**Availability & pending** *(the highest-severity group)*

- **AC-A1** With a world unavailable, the project opens **without blocking**, and **no edge into that world is
  pruned** — not on open, not on save, not on rebuild.
- **AC-A2** Saving a project while a world is unavailable **preserves every pending edge** verbatim.
- **AC-A3** Reattaching the world restores all pending edges with **no repair pass and no writer action**.
- **AC-A4** Adding/removing an edge into an unavailable world is **refused**, not silently dropped.
- **AC-A5** The loader **distinguishes dangling from pending**: world present + endpoint missing → prune; world
  absent → pending. Both branches tested explicitly.
- **AC-A6** Status reports **offline / unmounted / missing** where determinable, else generic **unavailable** —
  never a guess (§7.2.1).
- **AC-A7** Pending entries display **names** from `cachedIndex`, not bare IDs (§6.3).

**Initialization** *(no migration ACs — nothing shipped to migrate, §7)*

- **AC-I1** A project with no worlds does no world work on open and pays no cost.
- **AC-I2** Creating a world is **atomic**: an induced failure leaves the project exactly as before, with no
  half-written package and no orphaned binding.
- **AC-I3** Adding an existing world **modifies nothing inside that world package** and takes **no lock**.
- **AC-I4** A new binding starts at `epochOffsetMs = 0`; setting it later shifts every timeline in that world
  coherently, and **no timeline's own offset is rewritten** (§7.1).
- **AC-I5** `objectID` is preserved across `item` → `artifact` promotion and **zero edges are rewritten**
  (Doc 1 §3.1, §5.2).
- **AC-I6** No `objects/rules/` directory is created by any code path (§7.2).

**Locking**

- **AC-L1** Concurrent **reads** from two projects never block.
- **AC-L2** Concurrent **writes** serialize; no interleaved write corrupts the world.
- **AC-L3** A **stale lock** (heartbeat > 60 s) is broken automatically; a killed writer blocks others for at
  most that long.
- **AC-L4** A world held **open** (not writing) by one project never blocks another project's write.
- **AC-L5** Lock contention **reports rather than hangs** (§4.5).

---

## 6. Open questions

**Answered by the 2026-08-04 rulings:**

- ~~Does a project *require* at least one world?~~ **No.** W5=C (lazy setup on first world creation) only makes
  sense if world scope is entirely optional; a project may never have a world at all.
- ~~Should a world be creatable empty?~~ **Yes** — W5=C triggers setup on *world creation*, which presumes
  creating a world is itself an action, not merely a byproduct of promoting content into one.
- ~~Do `rule` objects stay in `objects/rules/` or migrate to world scope?~~ **Migrate — `worlds/<worldID>/rules/`,
  unconditionally.** Rules govern an **environment**, not a manuscript: rules of magic, rules of sci-fi physics.
  A writer defining a rule does so *in the context of a world*. There is no project-scoped `rule`, and no
  promotion path is needed — unlike `item`→`artifact`, this kind does not exist in both scopes. This
  **overrides the legacy `objects/rules/` layout** in Package Structure v0.1 §11 and Doc 1 §3's table, both of
  which need updating.
- ~~What happens when a referenced world is unavailable at open?~~ **Neither blocking nor pruning** — edges go
  **pending** (§4.6) and the writer keeps writing.
- ~~How is a world package referenced — path, bookmark, or identity+search?~~ **Platform-neutral, whatever the
  shape** (§4.4.1). Platform access tokens (e.g. Apple security-scoped bookmarks) are a platform-layer
  implementation detail, never the reference itself. Exact search rules remain to be drafted.
- ~~May two projects hold the same world open simultaneously?~~ **Yes — reads are simultaneous and unrestricted.**
  Only **writes** serialize, via platform-neutral **lock → write → unlock**, held per-write and not per-session
  (§4.4.2).

**Drafted 2026-08-05 (were "open for drafting"):**

- ~~World-reference search rules~~ → **§6.4**, identity-verified resolution order.
- ~~Lock mechanics~~ → **§6.5**, lock file + 60 s heartbeat + stale recovery; `AtomicWrite` explicitly does not
  subsume it.
- ~~Index caching vs. composition~~ → **§6.3**, cache (it is what makes named pending entries possible).

**Ruled 2026-08-05:**

- ~~Where does the known-world-locations list live?~~ **There is no such list** (§6.4). The writer organizes
  worlds on her filesystem however she likes, selects them with a standard file-open dialog, and relinks at her
  convenience if one moves. No search, no registry, no scanning.
- ~~Migration path for `objects/rules/` and the legacy timeline locations?~~ **No migration exists.** Scrivi has
  not shipped, so world-scoped locations are simply *where these kinds live* (§7). No migration code, no
  legacy-layout detection, no re-basing pass, no backward-compatibility path.

**Still open — documentation follow-ups for the implementing sprint, not rulings:**

1. **External Change Repair Matrix extension.** `Scrivi_External_Change_Repair_Matrix_v0_2.md` must gain
   world-package conditions (unavailable at open, moved, `worldID` mismatch, stale lock, world edited by another
   project between opens). Left to that doc rather than duplicated here.
2. **Package Structure v0.1 §11 correction** — add the `worlds/` partition, drop `objects/rules/` (§7.2).
3. **Timeline v0.3 FR-051 / FR-070 correction** — historical events and historical timelines are specified at
   project paths and must be corrected to world scope (§1).
4. **Stale-lock timeout value** — 60 s is a reasoned default, not a measured one; worth revisiting if real
   network-volume writes prove slower.

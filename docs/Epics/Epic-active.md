# Active Epics

## EP-031: [ScriviCore] Worldbuilding Object Model & Relationship Graph

**Codebase:** `[ScriviCore]` primarily (C++ model, index, graph, C ABI) with `[Apple]` object cards on top.
**Status:** 🔵 **Planned 2026-08-05** — blocked on EP-030 (needs the card framework) but its ScriviCore half can
start independently.
**Goal:** Implement the approved object model — new kinds, the object index, the canonical relationship graph,
world packages — then the worldbuilding-object cards on top of EP-030's framework.
**Design:** `docs/Scrivi_Worldbuilding_Object_Model_v0_2.md` ✅ **Approved 2026-08-05** (T1–T6 ruled) +
`docs/Scrivi_World_Data_Separation_v0_1.md` ✅ **Approved 2026-08-05** (W1–W6 ruled).
**Date Created:** 2026-08-05
**Target Close Date:** TBD (6 sprints)

### Acceptance Criteria

- [ ] AC1 — New kinds (`building`, `vehicle`, `artifact`, `map`, `chronicle`, `faction`, `world`, `source`)
      round-trip; legacy 5-kind files load unchanged; `timeline` kind retired. (Doc 1 AC1)
- [ ] AC2 — `objects/index.json` is built on open, updated atomically, and **rebuilt from a scan** when
      missing/stale/corrupt; `findByID` resolves via the index. (Doc 1 AC2–AC3)
- [ ] AC3 — **One canonical edge** per relationship, created from either endpoint, with the inverse as a
      read-time label projection. Duplicate rejection tested for **asymmetric and symmetric** (faction↔faction)
      types. (Doc 1 AC4–AC5)
- [ ] AC4 — Cascade-prune on delete; **orphans survive** and are findable; `objectID` preserved across
      `item`→`artifact` promotion with **zero edges rewritten**. (Doc 1 AC6–AC8)
- [ ] AC5 — `relationships.jsonl` compacts at **30% or 1,000 tombstones**, whichever first; torn final line
      detected and truncated. (Doc 1 AC9)
- [ ] AC6 — The **epoch chain** resolves (event → timeline → world → project); rebinding a world changes exactly
      one number; two timelines in one world relate without any project. (Doc 1 AC13–AC15)
- [ ] AC7 — ⚠️ **Absence is never deletion:** an unavailable world holds edges **pending** — never pruned, never
      modified, surviving save, restored on reattach. Status reports offline/unmounted/missing where
      determinable, else generic unavailable. (Doc 1 AC16–AC17, Doc 3 AC-A1–A7)
- [ ] AC8 — World packages: `worldID`-verified resolution, no search/registry, platform-neutral bindings,
      lock→write→unlock with stale-lock recovery. (Doc 3 AC-P1–P4, AC-L1–L5)
- [ ] AC9 — Worldbuilding-object cards on EP-030's framework: unfiltered picker, in-stack creation with no modal,
      "Remove from scene" deletes the edge only. (Doc 2 AC16–AC24)
- [ ] AC10 — No regression: `ctest` + interop suites green; existing projects open unchanged.

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-095 | Object kinds + fields (`subtitle`/`image`/`worldID`) + object index | 🔵 Planning | — |
| SP-096 | Relationship graph: canonical edges, relation types, append-log, compaction | 🔵 Planning | — |
| SP-097 | Integrity: cascade-prune, orphans, promotion, pending-vs-dangling | 🔵 Planning | — |
| SP-098 | World packages: `.scrivworld`, bindings, resolution, locking, epoch chain | 🔵 Planning | — |
| SP-099 | Worldbuilding-object cards (Apple, on EP-030's framework) | 🔵 Planning | — |
| SP-100 | Verification & Epic close | 🔵 Planning | — |

> **⚠️ Renumbered 2026-08-09 (SP-094–SP-099 → SP-095–SP-100).** EP-031's sprints were planned before the
> 2026-08-07 ruling that made **SP-094** the merged *"EP-019 + EP-030 verification & Epic close"* sprint,
> which collided with EP-031's first sprint. `Sprint-active.md:357` already sequenced EP-031 as
> **SP-095–SP-100**; this table and the task assignments below are realigned to match. T-0365's deferral
> target moves with it: **EP-031 SP-095**.

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0370 | `ObjectKind` additions + `objectKindSubdir` + schema table; retire `timeline` | SP-095 | 🔵 Backlog |
| T-0371 | `WorldObjectFields` extensions: `subtitle`, `image`, `worldID` | SP-095 | 🔵 Backlog |
| T-0372 | `objects/index.json` — build, atomic update, scan-rebuild; `findByID` over index | SP-095 | 🔵 Backlog |
| T-0373 | `relation-types.json` + `canonicalDirection` + `symmetric` | SP-096 | 🔵 Backlog |
| T-0374 | `relationships.jsonl` append-log: create/delete/list, tombstones, torn-line recovery | SP-096 | 🔵 Backlog |
| T-0375 | Canonical normalization + duplicate rejection (asymmetric **and** symmetric) | SP-096 | 🔵 Backlog |
| T-0376 | Compaction at 30% / 1,000 tombstones | SP-096 | 🔵 Backlog |
| T-0377 | Cascade-prune on delete + load-time repair | SP-097 | 🔵 Backlog |
| T-0378 | `scrivi_list_objects` / `scrivi_list_orphaned_objects` | SP-097 | 🔵 Backlog |
| T-0379 | `scrivi_promote_object` (item↔artifact), `objectID`-preserving | SP-097 | 🔵 Backlog |
| T-0380 | ⚠️ Pending-vs-dangling loader distinction + frozen graph toward unavailable worlds | SP-097 | 🔵 Backlog |
| T-0381 | `.scrivworld` package + `world.json` + world index | SP-098 | 🔵 Backlog |
| T-0382 | `binding.json` + `worldID`-verified resolution + relink | SP-098 | 🔵 Backlog |
| T-0383 | Lock→write→unlock + heartbeat + stale-lock recovery | SP-098 | 🔵 Backlog |
| T-0384 | Epoch chain: world/timeline/binding offsets + resolve endpoint | SP-098 | 🔵 Backlog |
| T-0385 | Cached world index entries → named pending entries | SP-098 | 🔵 Backlog |
| T-0386 | Object cards (one implementation, per-kind config) on EP-030's framework | SP-099 | 🔵 Backlog |
| T-0387 | Object picker (unfiltered, all worlds) + inline type-ahead + "Create new…" | SP-099 | 🔵 Backlog |
| T-0388 | In-stack create/edit, edit-state visuals, scene-change complete-or-discard | SP-099 | 🔵 Backlog |
| T-0389 | Pending presentation + Worlds menu + warning view under the timeline | SP-099 | 🔵 Backlog |
| T-0390 | External Change Repair Matrix — world-package conditions | SP-100 | 🔵 Backlog |
| T-0391 | EP-031 verification (AC1–AC10) + Epic close prep | SP-100 | 🔵 Backlog |

> **Highest-risk task: T-0380.** The pending-vs-dangling distinction is the one failure in this Epic that is
> *silent and unrecoverable* — a loader that reads "world unavailable" as "endpoint deleted" destroys every
> relationship into that world with no error shown. Both branches need explicit test coverage before any
> cascade-prune code ships.

---

*Last Updated: 2026-08-11 (**EP-019 ✅ CLOSED + EP-030 ✅ CLOSED (both Human-approved) — double Epic close.**
**EP-019** (Custom Undo/Redo History & Multiple Copy Buffers): AC1–AC8 all Verified across 7 sprints.
AC2 was **amended** (auto-save retired as a commit trigger; 45 s idle-session boundary added) and design
**§4.d relaxed** — disk may lead history by at most one open typing session — both explicitly user-approved
as a design change, separately from the close. T-0217 landed the documentation in design §4.a/§4.a.1/§4.d/
§12.2/§12.8/§14/§15, package-structure §16a, and repair-matrix §6.21. **EP-030** (Scene Inspector Card
Framework): AC1–AC7 all Verified across 4 sprints. **AC12 was rescoped to soft failures** (SwiftUI cannot
catch a trapping view body) after the closing pass found `CardBodyBoundary` was a no-op under a comment
claiming it was a framework guarantee; implemented as **T-0399** (SP-101) and verified on **test-fixture
evidence, user-accepted** — no UI path can make a card fail. **T-0368 closed as delivered by T-0399.**
ctest **413/413** · macOS interop **56/56** (from 53) · TEST SUCCEEDED. Sprints archived: SP-092, SP-093,
SP-094, SP-101. Epics archived to `Closed/Epic-EP-019.md` and `Closed/Epic-EP-030.md`.
**Active Epics 3 → 1 (EP-031 only).** Carried past close, deliberately: **T-0400** history log-segment
rotation (🟢 nice-to-have, user ruling); EP-019 has **no large-scale perf coverage** (T-0216 closed OBE) and
`HistoryCapture` is **not in the test target**, so AC2 items 5–8 rest on live verification alone.
Prior note follows.)*

---

*Last Updated: 2026-08-03 (**EP-029 ✅ CLOSED (Human-approved) — cross-boundary structured Cut/Copy/Paste
complete.** Final sprint **SP-089** delivered all four `[Apple]` tasks: T-0354 (Pass A clipboard, Verified
2026-07-29), T-0357 (title-capture + chapter promotion, Verified 2026-07-29), T-0355 (Pass B structured buffers,
Verified 2026-08-03), T-0356 (AC6 reversible structured undo, Verified 2026-08-03). AC1–AC7 all met; the AC6 undo
runs **app-side** (`HistoryService` stays in-memory; new core endpoint `scrivi_fragment_uncut_paste` is the exact
inverse of paste). ScriviCore `ctest` **369/369**, macOS interop **45/45**, app **BUILD SUCCEEDED**. **EP-029
archived to `Closed/Epic-EP-029.md`; SP-089 archived to `Sprints/Closed/Sprint-SP-089.md`.** Active Epics: EP-019
(held pending SP-057). Prior note follows.)*

*2026-07-27 (**SP-085 ✅ closed (Human-approved) — EP-029 design doc APPROVED.** The EP-029
`[Cross]` design sprint delivered + got approval for `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md`: the
`scrivi.fragment.v1` ordered-pieces schema, extract / paste-splice / cut-merge behaviour (composing EP-027
create/split + EP-028 `SceneMerger`/`ChapterMerger`), buffer-schema evolution (extend `scrivi.buffers.v1` in
place), and a one-reversible-event history shape (`structuredCut`/`structuredPaste`, undo = inverse op). Trades
ruled **T1=A · T2=A · T3=A · T4=A**; Open Questions #1–#3 resolved — no cross-window/cross-project structured
paste in v1; **caret-in-heading paste = refuse + flash the screen** (user override of the drafted silent-retarget);
divider-anchored selection normalised to the adjacent scene body. Task T-0350 Verified; docs-only. **EP-029 → SP-086**
(ScriviCore extract-fragment) next, awaiting go-ahead. Epics Active 1 (EP-029) + held/draft (EP-019 held, EP-026
Linux draft). Next available Sprint **SP-086**, Task **T-0351**. Prior note follows.)*

*2026-07-27 (**SP-056 ✅ closed (Human-approved) — AC6 Verified; EP-019 held pending SP-057; new
Epic EP-029 opened.** T-0213 + T-0214 both ✅ Verified live: multiple copy buffers delivered as explicit
⌘1–9/⌃1–9/⌥1–9 (copy/paste/cut) chords + app-global per-project palette + Edit/Scene/Chapter menu items +
bufferID-tagged cut event (backend schema extended); ctest 327 + interop 43 green. AC6 met. EP-019 → **held
pending** its final sprint SP-057 (AC2/AC7/AC8 verify + history panel + close). **EP-029** `[Cross]`
(cross-boundary structured Cut/Copy/Paste) opened to capture the gap surfaced during SP-056 verification —
manuscript-as-monolithic-document copy/cut/paste with structured buffers, cut-that-merges, paste-that-splits;
5 sprints planned (SP-085 design → SP-086 extract → SP-087 paste-splice → SP-088 cut-merge → SP-089 Apple wiring);
design doc to be written & approved first (T-0350). Epics Active 1 (EP-019, held) + Draft 2 (EP-026 Linux, EP-029);
next available Epic EP-030, Sprint SP-085, Task T-0351. Prior note follows.)*

---

*Last Updated: 2026-07-24 (**EP-025 ✅ CLOSED (Human-approved) — the final sprint SP-084 delivered AC6b and
closed the Epic.** SP-084 (co-located dot **clustering** — aggregate dot: larger core + count + segmented arc
ring + selection arc; hover fan-out; zoom-resolve) + a persistence sweep + full EP-025 verify; T-0346–T-0349 all
Verified live over VNC. Four clustering findings surfaced + fixed + re-verified same session (larger-diameter
aggregate-of-aggregates; fan overlay + grey backing; tighter dismiss; the **phantom double-draw** defect — a
fanned aggregate's members drew twice because the skip-set excluded the fanned aggregate; fixed so all members
are skipped from the baseline loops and the ring is the sole draw). Final container build **green (216/216)** +
new `timeline_cluster_smoke` + 14 regression smokes + app-launch PASS; `scrivi.h` untouched; no pbxproj
(Linux-only). **EP-025 archived to `Closed/Epic-EP-025.md`; Active Epics 1→0.** The full six-sprint completion
summary lives in the archive. Prior note follows.)*

*2026-07-24 (**EP-025 SP-082 ✅ closed (Human-approved) + SP-084 planned + activated** — the final sprint of
EP-025. SP-082 delivered **AC5** (historical events + imported timelines + export + File-menu items,
T-0340–T-0345 all Verified; I-0090/I-0091 fixed + re-verified; container green 211/211 + `timeline_events_smoke`
+ 11 smokes PASS; archived `Closed/Sprint-SP-082.md`). SP-084 then delivered **AC6b** clustering (above) and
closed the Epic. Prior EP-025 history — SP-079 [AC1/AC2], SP-080 [AC3], SP-081 [AC4], SP-083 [AC6a] — lives in
`Closed/Epic-EP-025.md`.)*

# Active Sprint

**No active sprint.** **SP-096** ✅ closed 2026-08-12 (Human-approved) — EP-031's second sprint and the Epic's
core deliverable. **SP-097** (integrity: cascade-prune, orphans, promotion, ⚠️ pending-vs-dangling) is staged in
`Sprint-backlog.md`, awaiting planning and activation.

---

## Recently closed (2026-08-12)

| Sprint | Title | Epic | Archive |
| ------ | ----- | ---- | ------- |
| SP-096 | `[ScriviCore]` Relationship graph — canonical edges, append-log, compaction | EP-031 | `Closed/Sprint-SP-096.md` |
| SP-095 | `[ScriviCore]` Object kinds + fields + object index | EP-031 | `Closed/Sprint-SP-095.md` |

**✅ SP-096 CLOSED** (Human-approved) — T-0373 / T-0374 / T-0375 / T-0376 / T-0402 all Verified. One canonical
edge per relationship with the inverse as a read-time label projection; duplicate rejection from either
creation order; append-only `relationships.jsonl` with torn-line recovery; compaction on both triggers.
**T-0402 replaced Doc 1 §5.2's broken endpoint ID-prefix rule** with index-lookup resolution and amended the
design in the same task.

**Suites at close:** ScriviCore `ctest` **455/455 macOS** · **462/462 Linux (GCC 14, zero warnings)** ·
macOS interop **59 passed / 0 failed**. ⚠️ **First `scrivi.h` change since EP-029** — 5 additive endpoints, all
confirmed exported via `nm`.

**EP-031 progress: AC2 and AC5 met; AC3 met but for one clause** (see below). 2 of 6 sprints closed.

---

## Carried forward — deliberately, not forgotten

1. ⚠️ **The faction↔faction symmetric duplicate test — the last clause blocking EP-031 AC3.** AC3 names
   faction↔faction "at war with" as *the* symmetric case and Doc 1 §9 AC4 warns it is "the one that regresses
   silently." `faction` is **world-scoped and uncreatable until SP-098**, so SP-096 covered the same-kind
   symmetric *shape* with `sibling-of` (character↔character, lexical). **Add the faction-specific case in
   SP-098**, once world packages exist; AC3 cannot be ticked before then.

2. ⚠️ **`rule` still lives at `objects/rules/`, contradicting the approved design.** Doc 1 §3 / Doc 3 §7.2 put
   it at `worlds/<worldID>/rules/`. Left working as it ships, marked `TODO(SP-098)` at the definition.
   Relocation is **T-0381's**, together with the **Package Structure v0.1 §11 correction** to drop
   `objects/rules/`.

3. **`objects/timelines/` is a shared directory** — the retired `timeline` kind and the *live* project
   timeline's `timeline.meta.json` occupied it. Only the kind was retired; a test asserts the panel's file
   survives. Anyone later "cleaning up" that directory should read that test first.

4. **T-0365 is unblocked and split** — the `source` object kind → **SP-097**, the aggregate `sources` card →
   **SP-099**. The `cites` relation type it depends on **already shipped in SP-096**. Source→scene
   (footnotes / pull quotes) is deferred to **EP-032**.

5. **The `platforms/linux` Qt Docker image cannot currently build** — the Docker VM's apt fails GPG
   verification against every Ubuntu repo. Linux verification for SP-095/SP-096 used Debian-based `gcc:14`
   with no Qt (both sprints are pure ScriviCore), so this was not a blocker, but the next `[Linux]` sprint
   will hit it directly. Environmental, not a code defect — no Issue opened.

6. **T-0400 — history log-segment rotation.** 🟢 Nice-to-have (user ruling 2026-08-11), carried from EP-019.
   `activeSegment_` is hard-fixed to `log-000001.jsonl`; capacity/eviction bounds the tree, not the log.
   Additive when done. No sprint assigned.

7. **`HistoryCapture` is not in the test target** (EP-019 carry-over), so T-0396's timing logic has no
   automated coverage; EP-019 AC2 items 5–8 rest on live verification alone.

---

## Notes for SP-097

- **`EndpointResolver` is the single place that decides what an endpoint *is*.** T-0380's pending-vs-dangling
  distinction — the Epic's highest-risk task, the one failure that is *silent and unrecoverable* — should
  extend its unresolved case rather than introduce a parallel path.
- **`error.detail` is the machine-readable discriminator pattern** (SP-096 used `"duplicateEdge"`, and the C
  ABI envelope now emits `detail`/`path` when set). SP-097's pending vs. dangling refusals should use it the
  same way, so callers can tell them apart without string-matching messages.

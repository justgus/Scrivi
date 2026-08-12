# Active Sprint

**No active sprint.** **SP-095** ✅ closed 2026-08-12 (Human-approved) — EP-031's first sprint. **SP-096**
(relationship graph: relation types, `relationships.jsonl`, canonical edges, compaction) is staged in
`Sprint-backlog.md`, awaiting planning and activation.

---

## Recently closed (2026-08-12)

| Sprint | Title | Epic | Archive |
| ------ | ----- | ---- | ------- |
| SP-095 | `[ScriviCore]` Object kinds + fields + object index | EP-031 | `Closed/Sprint-SP-095.md` |

**✅ SP-095 CLOSED** (Human-approved) — T-0370 / T-0371 / T-0372 / T-0401 all Verified. `ObjectKind` completed
at 11 kinds with `timeline` retired and world-scoped kinds **declared but gated** until SP-098;
`WorldObjectFields` extended with `subtitle` / `image` / `worldID`; and `objects/index.json` replaced the O(n)
`findByID` directory scan.

**Suites at close:** ScriviCore `ctest` **432/432 macOS** · **439/439 Linux (GCC 14, clean compile)** ·
macOS interop **59 passed / 0 failed** · app **BUILD SUCCEEDED**. `scrivi.h` untouched; no pbxproj change.

**EP-031 AC2 fully met.** AC1 partially — the enum is complete, but the 3 gated world-scoped kinds and the
`rule` relocation belong to SP-098.

---

## Carried forward — deliberately, not forgotten

1. **⚠️ `rule` still lives at `objects/rules/`, contradicting the approved design.** Doc 1 §3 / Doc 3 §7.2 put
   it at `worlds/<worldID>/rules/`. SP-095 left it working as it ships and marked the definition
   `TODO(SP-098)`. **Relocation is T-0381's**, alongside the world package that receives it — together with
   the **Package Structure v0.1 §11 correction** to drop `objects/rules/` (a documentation fix to an unshipped
   layout, not a migration).

2. **`objects/timelines/` is a shared directory** — the retired `timeline` kind and the *live* project
   timeline's `timeline.meta.json` occupied it. Only the kind was retired; `ProjectCreator` and
   `kTimelineMetaPath` are untouched, and a test asserts the panel's file survives. Anyone later "cleaning up"
   that directory should read that test first.

3. **T-0365 (`sources` card + `source` kind) is deferred with no sprint**, blocked on **OQ-1**: every
   worldbuilding object should be able to carry *multiple* sources — a requirement no design doc states and
   no relation type covers. Likely a `cites` / `documented-by` type with `sourceKind: null`. The user is
   reviewing the existing `source` language before it is scheduled.

4. **The `platforms/linux` Qt Docker image cannot currently build** — the Docker VM's apt fails GPG
   verification against every Ubuntu repo ("At least one invalid signature was encountered"). SP-095 verified
   Linux via Debian-based `gcc:14` with no Qt (it is pure ScriviCore), so this was not a blocker here, but the
   next `[Linux]` sprint will hit it directly. Environmental, not a code defect — no Issue opened.

5. **T-0400 — history log-segment rotation.** 🟢 Nice-to-have (user ruling 2026-08-11), carried from EP-019.
   `activeSegment_` is hard-fixed to `log-000001.jsonl`; capacity/eviction bounds the tree, not the log.
   Additive when done. No sprint assigned.

6. **`HistoryCapture` is not in the test target** (EP-019 carry-over), so T-0396's timing logic has no
   automated coverage; EP-019 AC2 items 5–8 rest on live verification alone.

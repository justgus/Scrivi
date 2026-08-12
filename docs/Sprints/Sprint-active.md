# Active Sprint

**No active sprint.** **SP-097** ✅ closed 2026-08-12 (Human-approved) — worlds now exist. **SP-098**
(integrity: cascade-prune, orphans, promotion, ⚠️ pending-vs-dangling) is staged in `Sprint-backlog.md`,
awaiting planning and activation. It is fully unblocked by SP-097.

---

## Recently closed (2026-08-12)

| Sprint | Title | Epic | Archive |
| ------ | ----- | ---- | ------- |
| SP-097 | `[ScriviCore]` World packages — `.scrivworld`, bindings, resolution, locking, epoch chain | EP-031 | `Closed/Sprint-SP-097.md` |
| SP-096 | `[ScriviCore]` Relationship graph — canonical edges, append-log, compaction | EP-031 | `Closed/Sprint-SP-096.md` |
| SP-095 | `[ScriviCore]` Object kinds + fields + object index | EP-031 | `Closed/Sprint-SP-095.md` |

**✅ SP-097 CLOSED** (Human-approved) — T-0381 / T-0382 / T-0383 / T-0384 / T-0385 / **T-0403** / **T-0404**
all Verified. Self-contained `.scrivworld` packages with their own object index; project-side bindings with
platform-neutral references; **identity-verified resolution** (a same-named package with a different `worldID`
is refused, never substituted); lock → write → unlock with 60 s stale recovery; and the three-layer epoch
chain. ⚠️ **SP-097 and SP-098 had their content swapped** at planning — two integrity tasks were verified
unbuildable without worlds; sprint IDs stayed in sequence.

**Suites at close:** ScriviCore `ctest` **477/477 macOS** · **484/484 Linux (GCC 14, zero warnings)** ·
macOS interop **59 passed / 0 failed**. ⚠️ **10 additive `scrivi.h` endpoints**, all confirmed exported.

**EP-031 progress: AC2, AC3, AC5, AC6, AC8 met — 5 of 10.** 3 of 6 sprints closed.

---

## Carried forward — deliberately, not forgotten

1. **`source` is the only thing keeping EP-031 AC1 unticked.** Every kind named in AC1's original text now
   round-trips, including the three that were gated. `source` was added to AC1 by the 2026-08-12 amendment and
   is **T-0365's ScriviCore half → SP-098**. The `cites` relation type it pairs with already shipped in SP-096;
   the aggregate `sources` card is SP-099.

2. ⚠️ **`objects/timelines/` is a shared directory, and the docs got it wrong once already.** The `timeline`
   *kind* is retired, but the directory holds the **live project timeline's** `timeline.meta.json`, written by
   `ProjectCreator` into every new project. Package Structure §11 said "removed" until SP-097 corrected it to a
   warning. A test asserts the file survives. Read that test before touching the directory.

3. **The design docs have been wrong three sprints running** — §5.2's endpoint ID prefixes (SP-096 T-0402),
   §6.5's `AtomicWrite` exclusive-create path (SP-097 T-0403), and §11's "removed" timelines directory. Each
   was caught by checking shipped code at planning, none by reading the doc. **Keep budgeting planning time for
   that check.**

4. **The `platforms/linux` Qt Docker image cannot currently build** — the Docker VM's apt fails GPG
   verification against every Ubuntu repo. Linux verification for SP-095/096/097 used Debian-based `gcc:14`
   with no Qt (all three are pure ScriviCore), so this was not a blocker, but the next `[Linux]` sprint will
   hit it directly. Environmental, not a code defect — no Issue opened.

5. **T-0400 — history log-segment rotation.** 🟢 Nice-to-have (user ruling 2026-08-11), carried from EP-019.
   Additive when done. No sprint assigned.

6. **`HistoryCapture` is not in the test target** (EP-019 carry-over), so T-0396's timing logic has no
   automated coverage; EP-019 AC2 items 5–8 rest on live verification alone.

---

## Notes for SP-098

- ⚠️ **T-0380 (pending-vs-dangling) is the Epic's highest-risk task** — Doc 3 §4.6 calls it the one failure
  that is *silent and unrecoverable*: a prune pass that reads "world unavailable" as "endpoint deleted"
  destroys every relationship into that world with nothing shown. It is now fully buildable and testable.
- **`EndpointResolver` already distinguishes the two cases.** Its cached-index path deliberately leaves
  `found == false` — the object is *named but unverified* while its world is away. **T-0380 should extend
  that**, not introduce a parallel notion of pending.
- **`WorldStatus` is the vocabulary for "why can't I see this?"** Cascade-prune must consult it *before*
  pruning anything.
- **`error.detail` is the discriminator scheme**: `worldUnavailable:<status>`, `worldIDMismatch`,
  `worldLocked`, `duplicateEdge`, `alreadyExists`. Pending/dangling refusals should join it rather than have
  callers string-match messages.
- **Cascade-prune reaches into EP-027's shipped scene/chapter delete paths** (ruled at SP-097 planning:
  both object *and* scene delete). That is a small, well-covered change — but it is shipped code.

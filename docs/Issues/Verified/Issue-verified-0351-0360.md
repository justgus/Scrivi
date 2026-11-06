# Verified Issues: I-0351 – I-0360

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0341–I-0350 are in [`Issue-verified-0341-0350.md`](Issue-verified-0341-0350.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0351** | `[Linux]` **Every repaint of the manuscript walks the whole document: the scene-break rules ask Qt for each separator's geometry, then drop the off-screen ones** — ~95,000 `blockBoundingRect` calls per keystroke on 1,186 scenes. Found in [SP-183] step 0. → [details](#i-0351) | Medium | [SP-183] | ✅ **Verified 2026-11-05** (the user's live pass on scrivi-vm: no lag) |

## I-0351: `[Linux]` Every repaint of the manuscript walks the whole document for the scene-break rules

**Found:** 2026-11-05, [SP-183] step 0 (`indent_probe`, a throwaway in a copy of the repo: the real `ManuscriptEditor` and
`ManuscriptPresenter` over 1,186 scenes, ~17,500 blocks, Docker, Qt 6.4.2 and 6.10.2, Debug).
**What:** `ManuscriptEditor::paintEvent` (`platforms/linux/src/ManuscriptEditor.cpp:1188–1205`) draws each scene-break rule by
calling `blockBoundingGeometry` for **every** separator in the manuscript, and only then skips the off-screen ones
(`:1204`). [SP-204] found that `blockBoundingGeometry` walks from the first visible block to the one asked for, laying out
every block between, so every paint walks the whole document.
**Measured** (a backtrace from inside the walk confirms `ManuscriptEditor::paintEvent` is the caller):

| | Today | Culled by block number first (probe only) |
|---|---|---|
| `blockBoundingRect` calls per keystroke | ~95,000 | ~180 |
| 20 keystrokes | 88 ms | 16 ms |
| Blocks laid out after the first show | 1,243 | 39 |

**Why it matters:** about 3.6 ms per keystroke on synthetic text, more on a real book. [SP-183]'s indent layout runs per laid-out
block, so with this walk in place a keystroke cost 18 ms instead of 1 ms; [SP-184] adds chapter dividers to the same loop.
**Fix (proposed):** the same as [SP-204]'s branch marks: cull each separator by block number against the visible range before
asking for its geometry. Tested by counting layout calls per keystroke, with a mutation that removes the cull.
**Fix (2026-11-05):** `ManuscriptEditor::paintEvent` culls each separator by block number before its geometry
(`platforms/linux/src/ManuscriptEditor.cpp:1207`), through `visibleBlockNumbers()`, now shared with [SP-204]'s branch marks.
**Evidence:** `indent_smoke` §5 (80 scenes): one keystroke asks for a bounded number of block rectangles; the mutation that
removes the cull is caught (3,772). On a dumas-sized manuscript (1,186 scenes), 20 keystrokes 88 → 21 ms (Qt 6.4.2, Docker).
**Status:** ✅ **Verified 2026-11-05** (user, 2026-11-05: *"all three re-test steps passed."* · *"C1, C2, C3, and C6 all passed."*; pass 1: *"But there was no lag"*).

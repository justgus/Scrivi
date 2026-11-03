# Closed Issue (Not Verified) — I-0345

## I-0345: `[ScriviCore]` The undo history's replay still costs ~1 s on dumas's 36 MB log

**Title:** The undo history's replay still costs ~1 s on dumas's 36 MB log
**Status:** ⚪ **CLOSED — Not Verified (2026-11-02, user-directed): won't do**
**Reason for Closure:** the user, 2026-11-02: *"also don't speed up I-0345."* After [I-0341] the history is 1.5 s of a 6.0 s load; both levers depart from an earlier ruling (Debug builds; Q3's 4 KB).
**Platform:** `[ScriviCore]`
**Date Identified:** 2026-11-02, build 1.0.320 (`3cb4846`), the user's load of dumas-prose-timelines-history on the VM over SMB (82 s in all; `scrivi-vm:~/evidence-2026-11-02/scrivi-history-open.log`); measured after [I-0341].
**Measured (the VM, local copy, `perf`):** `scrivi_history_open` 0.96–1.0 s (the Mac: 0.33 s, SP-200). 45% parsing the
JSON (`parseJson`, already `-O2`), ~10% freeing parsed records, ~19% rebuilding texts from diffs (`finalizeLoad`,
`pruneInconsistentNodes`, `-O0`); a few % copying sub-documents (`JsonDoc::getSubDoc`/`arrayItem` return copies).
**The log:** 36 MB after one day's passes (11.7 MB after SP-200's compaction). **77% (27.8 MB) is Replace All records** —
8,352 scene changes, ~3.2 KB each: under Q3's 4 KB threshold each stores its span from first match to last.
**The levers, each measured or estimated:**
1. Compile `history/HistoryStore.cpp` and `HistoryService.cpp` `-O2` in every build, as [I-0332] (a) did for `Json.cpp`:
   **measured 0.98 → 0.79 s** (−20%). ⛔ Departs from *"All builds Debug until release"* (user, 2026-10-17).
2. Lower Q3's split threshold (4 KB → 1 KB): the history-load design measured dumas's Replace All diffs 6.9 → 4.6 MB at
   1 KB; here that is ~27.8 → ~18.5 MB, roughly −25% of the parse (estimated). ⛔ Departs from the ruling *"no 4KB is
   sufficient"* (user, 2026-11-01).
**Recommendation:** neither now — after [I-0341] the history is ~1.3 s of a load that was 82 s; [I-0342]–[I-0344] are the
seconds that matter. Revisit if a writer's real history grows past this.

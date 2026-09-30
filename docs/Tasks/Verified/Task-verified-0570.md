# Verified Task: T-0570

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0570** | ✅ **Project-drive-lost warning** | [SP-151] | — | **2026-09-30** |

✅ **VERIFIED by the user's pass 2026-09-30 — every part:** with the project's drive pulled, the red
*"Project File Not Available"* banner appears over the manuscript; every scene typed into is kept dirty
and counted; ✅ on reconnect the edits are **saved automatically** and the banner clears; ✅ Quit AND
closing the window both warn — **Cancel aborts, Quit Anyway discards**.

## Rulings (user, 2026-09-30)
- Wording *"Project File Not Available"* — ⛔ not "Not Found" ([I-0115]: never assert absence for a drive
  that may only be unplugged).
- Keep typing under a red, non-dismissable banner; warn before quitting. ⚠️ The close-window guard was
  added by Claude (closing loses the same edits) and passed with the rest.

## ⚠️ The banner's detour
Its first form (`.safeAreaBar(edge: .top)`) was PULLED on a wrong diagnosis — the layout break was the
[I-0261] phantom world plus the pre-existing [I-0263]. ✅ Restored as a TOP OVERLAY, with
`ignoresSafeAreaEdges: []` so its red does not run up behind the toolbar.

## Depends on
[I-0259] (a failed save no longer marks the scene clean) and [I-0260] (the writer-facing message).

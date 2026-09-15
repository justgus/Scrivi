# Verified Tasks: T-0502 – T-0506 (SP-129, EP-040)

⚠️ **T-0504 and T-0505 are DEFERRED, not verified** — ✅ **see the bottom of this file.**

| ID | Task | Issue | Priority | Verified |
| -- | ---- | ----- | -------- | -------- |
| **T-0502** | ✅ **Route imported timelines through `scrivi_list_imported_timelines`** | — | **High** | 2026-09-15 |
| **T-0503** | ✅ **Imported-timeline OFFSET editing on Apple** | — | **Medium** | 2026-09-15 |
| **T-0506** | ✅ **Audit the remaining direct-filesystem call sites** | — | **Medium** | 2026-09-15 |

✅ **Verified by USER LIVE PASS 2026-09-15** — ⚠️ **not by a green suite**
(`feedback_live_pass_finds_what_suites_cannot`).

## T-0502 — the bypass, on BOTH platforms

⚠️ **The planned task was "delete the `FileManager`/`JSONDecoder` bypass; the endpoint is already
bound."** ⛔ **THE ENDPOINT COULD NOT FEED THE SURFACE:** ✅ **`listImportedTimelines` projected
`eventCount` but NOT the events**, ⚠️ **so a straight swap would have drawn every imported timeline
EMPTY.** ⚠️ **Linux had the same bypass for the same reason, and its helper's comment called it
"Apple's pattern".**

✅ **USER RULING: extend the CORE projection.** ✅ **It now emits `events[]` with **`projectOffsetMs`
pre-resolved** (`offsetMs + epochOffsetMs`), ⚠️ **because both platforms were computing that sum by
hand, identically.** ✅ **Apple's decoder and Linux's `readImportedTimelineFile` are both DELETED.**

## T-0503 — offset editing

⚠️ **The epoch offset could previously be set only AT IMPORT** — ⚠️ **a writer who misjudged it had
to REMOVE and RE-IMPORT.** ✅ **The imported row label now carries a context menu** (*Adjust Epoch
Offset…*, *Hide This Timeline*, *Remove Imported Timeline*), ✅ **reusing `EpochOffsetDialog` in edit
mode.** ⚠️ **The grey shade is NOT reassigned on an edit** — ✅ **it is how the writer recognises the
row.**

⚠️ **THE OPEN RISK WAS THE HIT TARGET** — ✅ **a `.contextMenu` on a `.position()`-ed `Text` is only
as large as the text.** ✅ **The live pass confirmed it is reachable.**

## T-0506 — the remaining call sites

✅ **ONE remains: `InspectorLayoutStore.swift` reads `inspector-layout.json`.**
✅ **RECORDED EXCEPTION, and CORRECT:** ⚠️ **no `scrivi_*` endpoint covers inspector layout because
it is UI STATE, not project data.** ⛔ **ZERO direct-filesystem reads of PROJECT DATA remain in
`Scrivi/Views` + `Scrivi/App`.**

## ⛔ T-0504 / T-0505 — DEFERRED, with their design questions

⚠️ **NOT verified and NOT built.** ✅ **SP-129's DoD permits deferral explicitly:** *"BUILT from a
ruled design, or DEFERRED with the design question written down — NOT built as placeholders."*

- **T-0504** ⚠️ **Epoch-label editing** (`setTimelineEpochLabel`) — ⚠️ **ambiguous WHOSE label
  (project vs each imported timeline), and what the label DOES.**
- **T-0505** ⚠️ **Object promotion** (`promoteObject`) — ⚠️ **it MOVES an object between packages,
  there is NO `demoteObject`, and the failure modes need ruling BEFORE a button exists.**

✅ **Full design questions: [`Sprint-SP-129.md`](../../Sprints/Sprint-SP-129.md).**

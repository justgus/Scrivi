---
sprint: SP-173
epic: EP-048
status: Active
activated: 2026-10-09
platform: Linux
created: 2026-10-09
---

# SP-173 — `[Linux]` [EP-048] Project load: no frozen UI, built once, measured ([I-0285])

**Status:** 🟡 **ACTIVE 2026-10-09** (user: *"close SP-166 and activate SP-173"*) — created 2026-10-09 (user: *"Schedule I-0285 into a new Sprint"*).
**Epic:** [EP-048] `[Linux]` Manuscript Renderer Parity → [`../Epics/Epic-active.md`](../Epics/Epic-active.md). I-0285 was found in
[SP-166] and is not caused by this Epic; it is scheduled here so this Epic's work does not make it worse (I-0285 AC2).
**Carries:** [I-0285] → [`../Issues/Issue-active.md`](../Issues/Issue-active.md#i-0285).
**Size:** M. ⚠️ Needs the rig: every figure is taken on dumas over the share, where the cost lives (Docker's page cache hides it).

---

## Goal

✅ **Opening dumas on Linux never freezes the UI, the timeline is built once, every second of the load is accounted for and on the
progress bar, and the load is as short as the measurements allow.**

## What is already known (2026-10-09)

- **Measured** (build 62, `SCRIVI_LOAD_LOG`, dumas over the share): core open 55.6 s · 1,186 scene bodies 57.4 s (worker, on the
  progress bar) · navigator rebuild 28.1 s · timeline reload 27.4 s (both main thread, UI frozen) · presenter's deferred first
  highlight 3.4 s · total 172 s.
- **The 28 s "navigator" is the timeline:** `rebuildNavigator()` ends in `reloadTimeline()`, whose `loading_` guard never fires
  because `applyLoadedProject` clears `loading_` first. The timeline is built twice.
- **Apple already has the shape Linux lacks:** Apple fetches every scene's story time in ONE call, `scrivi_list_story_times`
  (EP-039 AC4 / [I-0213]: 1,174 per-scene calls had cost ~950 ms on a chapter create). Linux still calls `getSceneStoryTime`
  per scene, up to twice per scene per build, and has no binding for the batched call (`Scrivi_ABI_Binding_Gap_Audit_v0_1.md`).

## I-0285 ACs this Sprint meets

| AC | Criterion (short) |
| -- | ----------------- |
| **AC1** | The timeline is not rebuilt during the load |
| **AC2** | EP-048 must not make it worse: load phases recorded before and after; the presenter's ~3.4 s is the baseline |
| **AC3** | Every phase measured, then reduced or ruled irreducible (open, bodies, story time, navigator, highlight, layout) |
| **AC4** | The timeline's data is fetched in the load's worker; the main thread only builds widgets |
| **AC5** | Whatever moves into the worker is counted on the determinate progress bar |
| — | Apple's shape checked first |

## Plan

1. **Apple's shape at open.** Read how Apple opens a project end to end: which core calls, on which thread, what is loaded eagerly
   and what lazily (scene bodies, story times, historical events, story structure). Record it here; any difference Linux keeps is
   ruled, not assumed.
2. **Baseline.** The build-62 log is the "before". Add the missing phase boundaries: the core open's own sub-phases, and the
   first paint.
3. **AC1 — built once.** Stop the mid-load `reloadTimeline()`. One-line class of fix; the load log must show one timeline phase.
4. **Bind `scrivi_list_story_times` on Linux** (Apple's shape) and replace every per-scene `getSceneStoryTime` read in
   `reloadTimeline()` with one map lookup. Empty list = no explicit story times, never failure (the ABI's documented trap).
5. **AC4 + AC5 — off the main thread, on the bar.** Fetch the timeline's data (`getTimeline`, the story-time list, historical
   events, story structure) in the existing `AsyncCall` worker beside the scene bodies; hand it to the main thread in the payload;
   count it on the progress bar.
6. **AC3 — the two big phases.** Measure what the core open (56 s) and the per-scene body reads (57 s, ~48 ms each) spend over
   the share (file-system call counts, as in SP-144's probe). Propose reductions with figures; ⛔ any new or changed C ABI
   endpoint is `[Cross]` and needs a ruling before it is built.
7. **AC2 — the presenter's first highlight** (3.4 s, main thread): measure it on the rig and, if it can be split so it does not
   hold the UI, propose how.
8. **Smokes + Docker ctest green;** the load log before/after recorded here.
9. **Live pass on the rig (user):** open dumas; the UI stays responsive throughout; the progress bar covers the whole wait; the
   timeline draws correctly; the load log shows the new figures.

## Questions for ruling (before activation or at the step that raises them)

- **Q1** (Plan 6): if the body reads need a batched read endpoint, is that in this Sprint (`[Cross]`) or a follow-on?
- **Q2**: the measurement log (`SCRIVI_LOAD_LOG`) — keep it as a permanent, env-gated diagnostic, or remove it at close?

⛔ **NOT in this Sprint:** EP-048's S3–S5 scope · the Linux double open (SP-144 AC5, deferred) unless Plan 1 shows it on this path.

## Progress log

- **2026-10-09** — created in 🔵 Planning; I-0285 assigned.
- **2026-10-09** — activated; I-0285 → `Issue-active.md`. Plan 1 next.

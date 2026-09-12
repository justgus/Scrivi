# Verified Tasks — T-0511 – T-0517

**Sprint:** [SP-131](../../Sprints/Sprint-active.md) · **Epic:** [EP-039] `[Cross]` Project Load Performance
**Verified:** 2026-09-12 (user-approved)

✅ **The in-memory project indexes.** ⚠️ **All seven verified together** — ✅ **AC1–AC3 were ruled ONE
DELIVERY** (one traversal feeds all three indexes), ⚠️ **and the rest are the endpoint, the lifecycle
and the tests that make them safe.**

| ID | Task | AC |
| -- | ---- | -- |
| **T-0511** | ✅ `ProjectIndexRegistry` — one `ProjectIndex` per open project, keyed by root, mutex-guarded. ⚠️ **Mirrors `HistoryRegistry` rather than inventing a second session mechanism** | AC5 (scope) |
| **T-0512** | ✅ `scrivi_close_project` — a NEW ENDPOINT (100 → 101), wired on **Apple** (`ProjectSession.close()` + the quit path) and **Linux** (`showLanding()`, which IS Close Project). ⚠️ **Without it the registry leaks one index per project opened** | — |
| **T-0513** | ✅ All three indexes built from ONE `ManuscriptOrderResolver::resolve()` pass. ⚠️ **`ResolvedScene` extended with `storyTime`** so `SceneStoryTimeIndex` costs no second walk | AC1–AC3 |
| **T-0514** | ✅ `findSceneMetaPath` (6 sites) and `openScene` routed through the index via `CoreServices::sceneLocator`. ⚠️ **The hint is validated by USE; a miss or a failed open TRAVERSES** | AC1, AC5a |
| **T-0515** | ✅ `scrivi_list_story_times` — the SPARSE bulk call. ⚠️ **Empty is the COMMON case and is NOT an error**; `count` is always emitted so the caller never infers from absence | AC4 |
| **T-0516** | ✅ The invalidation tests, through the C ABI. ⚠️ **Each VERIFIED FAILING against a deliberately regressed core** | AC5a–AC5e |
| **T-0517** | ✅ The complexity regression test — ⚠️ **pins the COMPLEXITY, not a duration** | AC8 |

### ✅ Measured result — the real 1,153-scene Dumas fixture

| phase | ⚠️ before | ✅ after | change |
| ----- | -------- | ------- | ------ |
| `get_scene_story_time` × 1154 | ⚠️ **251,800 ms** | ✅ **328 ms** | ✅ **767×** |
| `open_scene` × 1154 | ⚠️ **70,552 ms** | ✅ **356 ms** | ✅ **198×** |
| ↳ **last-100 average** | ⚠️ **`110.29 ms`** | ✅ **`0.30 ms`** | ⚠️ **quadratic → FLAT** |
| **the two hot phases** | ⚠️ **322.4 s** | ✅ **0.68 s** | ✅ **~474×** |

✅ **CONFIRMED IN THE USER'S OWN CONSOLE: `WALL CLOCK 1.06 s`** (was `298–321 s`).

### ⚠️ Two defects found by the new tests — both in this sprint's own work

1. ⚠️ **Invalidating only BEFORE the work was wrong and shipped green at first.** ⚠️ **Mutations call
   `findSceneMetaPath`, which goes through the locator and REBUILDS THE INDEX FROM PRE-WRITE DISK
   STATE** — ✅ **so a write landed correctly and the next read reported `count:0`.**
   ✅ **Fixed with `ProjectIndexInvalidation`, an RAII guard that drops on BOTH sides.**
2. ⚠️ **The first AC5b and AC5e tests asserted through `openScene` and COULD NOT FAIL** — ⚠️ **AC5a's
   validate-on-use fallback silently repairs staleness.** ✅ **Both now assert through an
   INDEX-SERVED endpoint.** ⚠️ **`feedback_boundary_tests_not_facade` one layer deeper: the right
   boundary, but an endpoint that self-heals.**

### ⚠️ What this did NOT fix

⚠️ **The app is still UNUSABLE after the load** — ✅ **[I-0200]**: the navigator rebuilds ~1,200 rows per
click, one `NSTextView` holds the whole 1.8 MB manuscript, and Spotlight re-indexes 1,154 scenes on
every resign. ⚠️ **Different defects, same manuscript.** ⚠️ **And the app remains FULLY SYNCHRONOUS
([EP-039] AC6).**

# SP-084: [Linux] Co-located dot clustering + persistence verify + full EP-025 verify & Epic close

**Status:** ✅ Closed (2026-07-24)
**Epic:** EP-025 `[Linux]` Timeline Panel (5th and final sprint; mirrors the clustering half of Apple EP-016
SP-042 — T-0166 + the T-0174 aggregate-dot revision — plus SP-043's verify/close). **This sprint closed EP-025.**
**Goal:** Close out the Linux Timeline. Co-located project-row dots (scenes and/or historical events at the same
pixel position) collapse into a single **aggregate dot** — a slightly-larger core with a **count** and a
**segmented arc ring** (one segment per member, the active member's segment lit); **hovering fans the members
out** into the hexagonal ring so each is individually hoverable/clickable; **zooming in resolves** the cluster.
Then a **persistence sweep** + a **full EP-025 regression pass** + — on Human approval — **EP-025 close**.
Delivers **EP-025 AC6b**.
**Start Date:** 2026-07-24
**End Date:** 2026-07-24
**Capacity:** ~10–12 hours

---

### Assigned Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0346 | `[Linux]` **Clustering pass + aggregate-dot model** — `computeClusters()` groups project-row members (scene + historical dots) whose current-zoom pixel positions are within one dot-diameter (FR-032) into an `Aggregate` (story-ordered members), recomputed per-frame off live geometry so it always matches zoom/pan. **Post-verify fix:** aggregates render larger, so the run-extension threshold **widens to the aggregate reach once a run is a pair** (measured against the previous member) — two aggregates that would visually overlap now merge; the aggregate centres on its middle member. No backend. | ✅ Verified (2026-07-24, VNC) |
| T-0347 | `[Linux]` **Aggregate-dot paint** — `paintAggregate` draws a slightly-larger core (FR-031) + centred member **count** + a **segmented arc ring** (`360°/N` per member, story order, scene = text tone / historical = warm, the active member's segment lit in the selection colour, FR-031/031a); display-only; centred on the baseline, no line shift / panel growth (FR-033). Collapsed members are skipped in the dot loops. | ✅ Verified (2026-07-24, VNC) |
| T-0348 | `[Linux]` **Hover fan-out + zoom-resolve + member interaction** — `updateHoverFan` fans an aggregate's members into the hexagonal ring (`fanOutMemberPos`, FR-035b) on hover; each fanned dot is a normal dot (tooltip, click-to-select/navigate, drag — routed via `fanMemberAt`, reusing SP-079/080/082 paths); zoom-resolve automatic (clusters use screen-x, FR-032). **Post-verify fixes:** the fan is a true **overlay** (painted last, above all dots) on a **grey backing disc** (`fanRadius`); the hover **keep-region = the backing disc** (tight dismiss); and the **phantom double-draw** defect fixed (see findings). | ✅ Verified (2026-07-24, VNC) |
| T-0349 | `[Linux]` **Persistence sweep + full EP-025 verify + `timeline_cluster_smoke` + Epic close** — new headless `timeline_cluster_smoke` (N co-located → one aggregate of N; zoom-in → resolves to singletons; separated → none; a historical event co-located with scenes joins) via public test hooks + CMake + CI; persistence covered by existing smokes + SP-083's zoom/pan; full EP-025 regression green. Closed AC6b + EP-025. | ✅ Verified (2026-07-24, VNC) |

### Assigned Issues

_None._ All defects were caught mid-review of un-accepted tasks and fixed same-session — recorded as sprint
findings (below), not I-numbers (I-numbers are for defects that escape into accepted work, e.g. SP-082's
I-0090/I-0091). Matches how SP-081's `{"bands":[…]}` shape bug and SP-082's metadata-only-list discovery were
handled.

### Key findings & decisions

- **Clustering is pure Linux UI** — a `TimelinePanel` render/hit-test layer over the dots the shell already
  feeds (`dots_` + `histDots_`). No `scrivi_*`, no schema, `scrivi.h` **untouched**. Aggregates computed
  per-frame (O(n log n)) off live geometry, so zoom/pan/resize never desync the clusters (no invalidation
  bookkeeping).
- **Test-premise correction** (cluster smoke): the first zoom-resolve case used 1-hour-apart dots in a 10-year
  window — un-resolvable within the [1,500] zoom clamp — retuned to 2-day-apart dots in a ~1-year window. The
  clustering code was correct; the test's numbers were wrong.
- **Four VNC findings (all fixed + re-verified same session):**
  1. **(T-0346) Aggregates must cluster at a larger diameter** — an aggregate renders larger than a single dot,
     so two aggregates within a dot-diameter overlapped. The run-extension threshold now widens to the aggregate
     reach once a run is a pair (chained against the previous member), so a chain of co-located dots and two
     overlapping aggregates all merge.
  2. **(T-0348) Fan-out must be an overlay above everything, on a grey backing** — painted last, on a filled
     grey disc (`fanRadius`, high-opacity) so it reads over bands + neighbouring dots.
  3. **(T-0348) Fan dismiss too loose** — the hover keep-region is now exactly the grey backing disc, so the fan
     collapses the moment the pointer leaves the visible fan.
  4. **(T-0348) Phantom dots — a real double-draw defect.** A fanned aggregate's first/last members appeared as
     stray dots outside the ring, and selecting a ring dot lit its phantom twin (same dot drawn twice).
     **Root cause:** the skip-set that omits clustered members from the baseline dot loops **excluded the fanned
     aggregate** (`if (a == fannedAggregate_) continue;`), on the wrong assumption those members would "paint
     individually" — but `paintFanOut` already draws them in the ring, so they drew twice (ring + true baseline
     `xForOffset`); at high zoom the aggregate's wide screen span pushed the extreme members past the grey disc.
     **Fix:** ALL aggregate members go in the skip-set, fanned or not — the ring is the only place a fanned
     member is drawn. The clustering model was correct; only the paint bookkeeping was wrong.

### Verification

- ✅ **Container build green** — Qt 6.4: **216/216** targets, 0 warnings; `scrivi_linux` +
  `scrivi_linux_timeline_cluster_smoke` linked.
- ✅ **New `timeline_cluster_smoke` PASS** — coincident dots → one aggregate of N; zoom-resolve; well-separated
  dots → none; a historical event co-located with scenes joins the aggregate.
- ✅ **All 14 regression smokes PASS** — timeline-events, story-structure, timeline-story-time, merge, create,
  reorder, chapter-reorder, editor-map, save, rename, delete, load, lifecycle, persistence.
- ✅ **Headless app-launch OK** — full window tree (incl. the clustering paint paths) constructs offscreen.
- ✅ **Live VNC walkthrough complete (2026-07-24)** — Human verified the aggregate dot, hover fan-out,
  click-a-member-to-navigate, and zoom-resolve on `the-twisted-remains-of-myself`; all four findings re-verified
  (including the phantom-dot fix). **AC6b met.**
- **Persistence sweep** — no new smoke: existing smokes prove backend persistence (`story_structure` = bands +
  assignments; `timeline_story_time` = story-time; `timeline_events` = historical + imported + visibility) and
  SP-083 (T-0338) proved zoom/pan; the live walkthrough confirms full close/reopen restore.

### New files

Linux CMake only (not pbxproj — Linux-only Qt/C++): `tests/timeline_cluster_smoke.{cpp,sh}`. New CI step wired
in `.github/workflows/scrivi-linux-ci.yml`. Clustering code lives in `src/TimelinePanel.{cpp,hpp}`.
`scrivi.h`/ScriviCore untouched.

### Outcome

Delivered **EP-025 AC6b**. **This sprint closed EP-025** — all of AC1–AC6b Verified. Closed with Human approval
2026-07-24 (the Human verified the phantom-dot fix and authorized both the SP-084 and EP-025 closes).

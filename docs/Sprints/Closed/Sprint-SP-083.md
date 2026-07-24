# SP-083: [Linux] Timeline zoom + pan (brought forward — fixes I-0087)

**Status:** ✅ Closed (2026-07-23)
**Epic:** EP-025 `[Linux]` Timeline Panel (brought forward from its planned position to fix I-0087; mirrors
Apple EP-016 SP-043 pan/zoom — `TimelineScrollCaptureView` scroll-wheel + magnify)
**Goal:** Let the writer **zoom and pan** the linear timeline so a far-outlier scene (e.g. a 2-year flashback)
no longer collapses every other dot to one edge (**I-0087**). Keep the **linear time-axis** (Apple parity — NOT
even-spacing). **Zoom = `Ctrl`+mouse-wheel** (the universal X11/Linux zoom idiom) **zoom-about-the-pointer**,
plus an **always-works `+`/`−` control** at the strip's bottom-right (a plain click — guaranteed over VNC and for
non-power-users; zoom about the pointer, or the strip center if the pointer is away). **Pan = click-drag on the
empty area above/below the dots** (the background was reserved for pan in SP-080). A horizontal scrollbar appears
when zoomed in. Delivers **EP-025 AC6a**.
**Start Date:** 2026-07-22
**End Date:** 2026-07-23
**Capacity:** ~5–7 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0333 | `[Linux]` **`TimelinePanel` zoom model** — `zoom_` (≥1) + `panFraction_` threaded through `xForOffset`/`offsetForX` (band geometry follows for free); **`Ctrl`+wheel** (`wheelEvent`) zooms **about the pointer** (`zoomAbout` holds the story-time under the cursor fixed); clamp [1, 500]; zoom 1 = full-fit. Linear axis unchanged. | High | ✅ **Verified (2026-07-23, VNC)** |
| T-0334 | `[Linux]` **`+`/`−` zoom control + horizontal scrollbar** — two `QToolButton`s at the strip's **bottom-right** (`+` left, `−` right) → `zoomInStep`/`zoomOutStep` (zoom about pointer, or center if outside); a `QScrollBar` along the bottom appears when `zoom_ > 1` (`syncScrollBar`, laid out in `resizeEvent`). Plain clicks → **VNC-safe** even when a Mac mouse wheel doesn't reach x11vnc. | High | ✅ **Verified (2026-07-23, VNC)** — "the zoom buttons work" |
| T-0335 | `[Linux]` **Pan by background drag** — a press on the empty area (no dot/border) with `zoom_ > 1` → `DragMode::Pan` (closed-hand cursor), drag adjusts `panFraction_`; dot-drag (SP-080) + border-drag (SP-081) still win on their zones. Closes **I-0087 + AC6a**. | High | ✅ **Verified (2026-07-23, VNC)** |

**Follow-on tasks added mid-sprint (2026-07-23) — full entries in `docs/Tasks/`:**

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0336 | `[Linux]` Time Delta Picker — anchor to ANY scene (resolved once to a manual offset; no schema change) | ✅ Verified (2026-07-23, VNC) |
| T-0337 | `[Linux]` Story bands wrap the main storyline `[0, last-end]`, not the whole strip; zoom with the timeline; flashback stays assignable | ✅ Verified (2026-07-23, VNC) |
| T-0338 | `[Linux]` Persist timeline zoom + pan per project (INI under app-support so it survives the `--rm` container) | ✅ Verified (2026-07-23, VNC) |
| T-0339 | `[Linux]` Truncate long scene titles in the picker's anchor combo (cap width + elide, full title in tooltip) | ✅ Verified (2026-07-23, VNC) |

### Assigned Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-0087 | `[Linux]` Timeline dots crowd to one edge when a scene is a far time-outlier; no zoom to spread them | High | ✅ **Verified (2026-07-23, VNC)** — SP-083 zoom + pan spread the cluster |
| I-0088 | `[Linux]` A scene before Story Open (flashback) can't be placed — the Time Delta Picker floored "N before" to offset 0, and the chain poisoned every later scene; epoch label also fixed to the left edge | High | ✅ **Verified (2026-07-23, VNC)** — picker gains an epoch anchor + drops the zero-floor, panel anchors the "Story Open" tick to `xForOffset(0)` |

### Implementation notes

- **T-0333.** `TimelinePanel` gained `zoom_` (≥1) + `panFraction_` (visible-window left edge as a fraction of
  the full `[minMs_, maxMs_]`). `xForOffset` maps `visibleFrac = (frac − panFraction_) * zoom_`; `offsetForX`
  inverts it. `zoomAbout(factor, anchorX)` keeps the story-time under `anchorX` fixed (solves `panFraction_`
  from the anchor); `clampPan` keeps the window in-bounds. `wheelEvent` zooms on **Ctrl+wheel** (1.2^notches)
  about the pointer; plain wheel passes through. Band geometry rides `xForOffset`, so bands zoom/pan with the
  dots automatically.
- **T-0334.** Two `QToolButton`s (`+` left, `−` right) at the bottom-right (`layoutControls` in `resizeEvent`)
  → `zoomInStep`/`zoomOutStep` (1.25× about the pointer, or the strip center). A horizontal `QScrollBar` shows
  when `zoom_ > 1` (`syncScrollBar`: range 0..1000, page = 1/zoom, value ← panFraction/maxPan); its
  `valueChanged` maps back to `panFraction_`. `−` disabled at zoom 1. The buttons are the VNC-safe path.
- **T-0335.** A left-press on the empty area (no dot, no border) with `zoom_ > 1` starts `DragMode::Pan`
  (closed-hand cursor, `panStartFraction_` latched); the move converts the pixel delta to a full-window fraction
  (÷ usable ÷ zoom_) and offsets `panFraction_` (clamped); release resets the cursor. Dot/border zone checks run
  first in `mousePressEvent`, so SP-080 dot-drag + SP-081 border-drag are untouched.

### Verification

- ✅ **Container build green** — `docker build --no-cache` (Qt 6.4): all 202 targets, `scrivi_linux` linked,
  0 errors.
- ✅ **Regression smokes green** — `timeline_story_time`, `story_structure`, `scene_merge`, `scene_create`,
  `scene_reorder`, `chapter_reorder`, `editor_map` (+ `scene_save`/`scene_rename`) all PASS with
  `QT_QPA_PLATFORM=offscreen`.
- ✅ **Live VNC (2026-07-23)** — user confirmed the `+`/`−` zoom buttons work, zoom + pan spread the crowded
  cluster (**I-0087**), the flashback places correctly before Story Open (**I-0088**), scene-anchor picker
  (T-0336), band span/zoom (T-0337), zoom/pan persistence (T-0338), and title truncation (T-0339).

### Retrospective

**What shipped.** Timeline zoom + pan on the linear axis, with three VNC-safe input paths (Ctrl+wheel, `+`/`−`
buttons, background-drag pan) plus a scrollbar. The sprint met its own goal — **I-0087 (dot crowding) is fixed**
and confirmed live. It also grew four follow-on tasks (T-0336–0339) and a second issue (I-0088) discovered while
verifying against the real `the-twisted-remains-of-myself` project — the flashback/negative-offset class — all
of which were fixed and verified in the same session.

**Process win.** This sprint corrected a standing workflow error: the Linux Qt app **can** be compiled and
smoke-tested locally via the Ubuntu 24.04 + Qt 6.4 Docker image (`docker build --no-cache …`), rather than
claimed "unbuildable here." Captured in memory `project_linux_docker_build`.

**What did NOT close — carries forward.** SP-083 was *brought forward to unblock SP-081 **T-0332*** (scene→band
**drag-up assignment**). Zoom/pan removed the crowding blocker, but T-0332 **still does not work** with the dots
spread — so the drag-up assignment has a **separate defect** beyond crowding. T-0332 stays open under **SP-081**
and a new Issue (**I-0089**) tracks the residual drag-up failure. SP-083 closing does **not** depend on T-0332
(T-0332 is an SP-081 task).

**Out of scope (unchanged):** clustering/aggregate dots + Epic close → SP-084; historical/imported events →
SP-082.

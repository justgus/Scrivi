# Verified Task — T-0656

**Sprint:** — · **Epic:** none (a live-pass finding, [EP-057]) · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-24** (user, 2026-11-24: *"T-0656: verified."*)
**Archived:** 2026-11-24.

---

## T-0656 — `[Linux]` Landing: a warm grey background with a little depth

**Epic:** none (a live-pass finding, EP-057) · **Status:** ✅ **Verified 2026-11-24** (user, 2026-11-24: *"T-0656: verified."*) · **Sprint:** none · **Date Requested:**
2026-11-02 (user: *"A slightly warm grey color should be used for the launch panel. maybe with a slight gradient, as if it had
some depth."*)

**Why:** on GNOME Wayland, Qt's Adwaita frame draws no shadow — it reserves a 10 px shadow margin and paints nothing in it
(checked in Qt 6.10.2's `qwaylandadwaitadecoration.cpp`: `ceShadowsWidth = 10`; `paint()` fills the title bar and draws a
1-line border only). Landing and a project window in the same system colour could not be told apart.

**What:** `platforms/linux/qml/Landing.qml` — a vertical gradient behind the `StackView`, light `#ece7e0` → `#d8d1c7`, dark
`#45403b` → `#2e2a27` (from `Application.styleHints.colorScheme`); the landing `Page` and `NewProjectDialog.qml` draw no
background (`background: null`). Linux only: on the Mac the system's window shadow does this.

**Tested:** builds; smokes 38/38; an offscreen launch logs no QML warning. ⏳ The look is the user's call, on the VM.

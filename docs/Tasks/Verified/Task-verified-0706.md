# Verified Task — T-0706

**Sprint:** [SP-243] · **Epic:** [EP-060] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-24** (user, 2026-11-24: *"Steps 1-11 pass."*); closed (user, 2026-11-24: *"close SP-243, then complete EP-060"*)
**Archived:** 2026-11-24.

---

## ✅ T-0706 — `[Linux]` EP-060 S3: the Inkwyn glyphs and the 8-item toolbar on Linux

**Epic:** [EP-060] · **Status:** ✅ Verified (user, 2026-11-24: *"Steps 1-11 pass."*) · **Sprint:** [SP-243] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-243.md`](../../Sprints/Closed/Sprint-SP-243.md) · **Date Requested:** 2026-11-24 (user, 2026-11-24: *"close SP-242, then plan Sprint 3"*)

**What:** the Mac's 8-item toolbar on Linux (headers, Import/Export, Tools, no Qt arrow, the » glyph), `GlyphIcon` (palette-tinted,
HiDPI, Qt 6.4 and 6.10), every mapped place on its glyph (cards, tabs, detail bar, Navigator, warnings), the window icon — the
glyphs written by the same script as the Apple symbols.

**Built:** `src/GlyphIcon.{hpp,cpp}`, `ScriviWindow.{hpp,cpp}`, `SceneInspector.cpp`, `ObjectDetailDialog.cpp`, `HistorySection.{hpp,cpp}`,
`WorldsDialog.cpp`, `WorldWarningStrip.cpp`, `NavigatorTree.{hpp,cpp}`, `main.cpp`; `resources/glyphs/` (generated), `resources/inkwyn-256.png`;
`CMakeLists.txt` (Qt6::Svg, `scrivi_linux_glyphs`, the auto-link loop, `glyph_smoke`); `tests/glyph_smoke.{cpp,sh}`, `tests/toolbar_smoke.cpp`;
the Dockerfile, CI and rig doc (`qt6-svg-dev`); `scripts/make-glyph-symbols.swift` (Linux output). Details: the Sprint.

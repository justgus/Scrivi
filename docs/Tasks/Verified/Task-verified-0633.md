# Verified Task — T-0633

**Sprint:** [SP-198] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-03** (iPad A1–A4, A13 and iPhone B1–B7 (user, 2026-11-03), iPhone Duo simulator C1–C4 (user, 2026-11-03: *"all pass!"*))
**Archived:** 2026-11-03, on the user's reported pass.

---

## T-0633 — `[Apple]` EP-055 S1: the presentation layer compiles for iOS, the Mac unchanged

**Epic:** [EP-055] (AC2) · **Status:** ✅ **Verified 2026-11-03** (iPad A1–A4, A13 and iPhone B1–B7 (user, 2026-11-03), iPhone Duo simulator C1–C4 (user, 2026-11-03: *"all pass!"*)) (2026-10-30) — ⏳ the live passes ([SP-198] AC5) · **Sprint:** [SP-198] (🟡 Active) → [`Sprint-SP-198.md`](../../Sprints/Closed/Sprint-SP-198.md) · **Date Requested:** 2026-10-29

**What:** `ManuscriptPresenter`, `ManuscriptPresentedMap` and `ManuscriptTypography` (and whatever else [T-0632] rules shared)
built for both platforms, with UIKit's font and colour types on iOS. No behaviour change on the Mac.
**Tests:** the Mac's interop tests (`scripts/run-interop-tests.sh`), `check-textkit2.sh`, `check-manuscript-type-source.sh`; the
iOS target builds.
**Done (2026-10-30):** `ManuscriptPresenter`, `ManuscriptCommands` and `ManuscriptTypography`'s drawing half build on every Apple
platform (`PlatformFont` / `PlatformColor`, `ManuscriptTypography.swift:1–22`). New `ManuscriptEditing.swift`: the shared types
moved out of `ManuscriptTextView.swift` (`SceneDivider`, `SceneBoundaryTable`, `DividerTextAttachment` with a UIKit drawing and
the ONE factory `DividerTextAttachment.make`); **C1** `ManuscriptEditRules` (the edit verdict, Return, ⌫/⌦) and **C2**
`ManuscriptSceneSync` (render, edit and caret sync, autosave). The Mac's `shouldChangeText`, `insertNewline`, `deleteBackward`,
`deleteForward`, `textDidChange`, `textViewDidChangeSelection` and render now call them. `check-manuscript-type-source.sh` also
catches `UIFont` / `PlatformFont`. **Evidence:** Mac interop 324/324 after each move; both guards pass; macOS, iOS and visionOS build
with no warnings. ⚠️ The `ScriviInteropTests` target was raised to macOS 27.1 to match the app (it no longer compiled).

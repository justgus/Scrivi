# Verified Task — T-0637

**Sprint:** [SP-199] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-03** (iPad A5–A12, iPhone B2–B6, iPhone Duo simulator C1–C4 and the toolbar re-pass on Mac, iPad and iPhone (user, 2026-11-03))
**Archived:** 2026-11-03, on the user's reported pass.

---

## T-0637 — `[Apple]` EP-055 S2: the iOS gestures and the Timeline grabber, with routes that need no gesture

**Epic:** [EP-055] (AC6, AC8) · **Status:** ✅ **Verified 2026-11-03** (iPad A5–A12, iPhone B2–B6, iPhone Duo simulator C1–C4 and the toolbar re-pass on Mac, iPad and iPhone (user, 2026-11-03)) (2026-10-31) — ⏳ the simulator live passes · **Sprint:** [SP-199] (🟡 Active) → [`Sprint-SP-199.md`](../../Sprints/Closed/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What:** design §8. Compact: swipe left goes to the next page and swipe right to the previous one (the system back swipe).
Regular: edge swipes show and hide the Navigator and Inspector, and full-width swipes combine them (G1). Both: the Timeline
grabber (G2). G4: the View menu and shortcuts, VoiceOver actions, and the ⋯ menu. ⛔ Nothing may take the manuscript's
scrolling, selection or the system's text gestures.
**Tests:** an iOS test for the gesture → panel-state mapping; live passes.
**Done (2026-10-31):** new `Scrivi/Views/PanelSwipes.swift`. `PanelSwipe.actions` is the rule table: start within 24 pt of an edge,
end in the outer 12 %. `PanelSwipeInstaller` puts one pan recognizer on the window. It recognises alongside everything, cancels no
touches, ignores the Timeline's frame, and acts only on a quick, mostly sideways swipe (|dx| > 2|dy|, > 60 pt, > 300 pt/s).
`TimelineGrabber` sits above the Timeline at every width. G4: the ⋯ menu (at width compact, "Scene Inspector" goes to the page),
VoiceOver actions on the editor (Scene Navigator, Scene Inspector, Show/Hide Timeline), and the iPad Project menu's existing
⇧⌘I / ⇧⌘T. ⚠️ Those are not the Mac's ⌥⌘I / ⌥⌘T: ⌥⌘ collides with the menu iOS synthesizes (`ScriviApp.swift:531`).
Tests: `PanelSwipeTests` (2), iOS 14/14; Mac interop 327/327; `check-layout-convergence.sh` clean.

**iPad live pass 1 (user, 2026-10-31):** the panels and gestures work, but swipes often needed a second try, swiping up on
the grabber also raised the Dock, the Timeline's horizontal scroll stopped working, and ⇧⌘I showed a panel but did not hide it.
**Fixed:**
- Panels start hidden on iOS and their show/hide is kept for the session only (`ProjectSession.swift` restore and didSets).
- Swipes are counted only when they start on `ManuscriptUITextView`, with a 44 pt edge zone and a looser speed test.
- ⚠️ The Timeline exclusion had never been wired in (the edit silently missed).
- Tapping the grabber toggles the Timeline.
- The iOS Project-menu toggles are now Buttons that flip the live value.
iOS 14/14, Mac interop 327/327.

**iPad live pass 2 (user, on a real iPad with a hardware keyboard, 2026-10-31):** panels hidden on open, the grabber tap,
⇧⌘I / ⇧⌘T, and the Mac's panels left unchanged all passed. Swipes were better but still sometimes needed a second try, and the
Timeline still would not drag. ⚠️ **Correction to pass 1:** the swipe recognizer never took the Timeline's scroll. iOS never had a
Timeline pan: the only pan is the Mac's scroll-wheel capture (`TimelineStripView.swift`, `TimelineScrollCaptureView`).
**Fixed:** a drag on the Timeline's empty space pans on iOS, sharing `pan(by:usable:)` with the Mac's wheel. Like the wheel, it
pans only when zoomed in (pinch). Added a temporary `[SCRIVI-SWIPE]` log in `PanelSwipes.swift` so the swipe misses can be
measured. iOS 14/14, macOS build succeeded.

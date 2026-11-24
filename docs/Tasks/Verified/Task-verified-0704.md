# Verified Task — T-0704

**Sprint:** [SP-241] · **Epic:** [EP-060] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-24** (user, 2026-11-24: *"Step 3 passes now"*); closed (user, 2026-11-24: *"close SP-241, then plan Sprint 2"*)
**Archived:** 2026-11-24.

---

## ✅ T-0704 — `[Apple]` EP-060 S1: the glyph pipeline (SVG → custom SF Symbols) and the Inkwyn app icon

**Epic:** [EP-060] · **Status:** ✅ Verified (user, 2026-11-24: *"Step 3 passes now"* — the last open live-pass step; the rest passed
in run 1) · **Sprint:** [SP-241] (✅ Closed) → [`../../Sprints/Closed/Sprint-SP-241.md`](../../Sprints/Closed/Sprint-SP-241.md)
· **Date Requested:** 2026-11-24 (user, 2026-11-24: *"Ok we're activating EP-060 and planning the first sprint."*)

**What:** `scripts/make-glyph-symbols.swift` turns `Resources/Icons` into 46 `inkwyn.*` custom symbols in the asset catalog;
`inkwyn.characters` replaces `person.2` on the Characters card (the spike); `Scrivi/Inkwyn.icon` is the Mac and iOS app icon.

**Built:** `scripts/make-glyph-symbols.swift`; `Scrivi/Assets.xcassets/Glyphs/` (generated); `Scrivi/Views/Glyph.swift`;
`InspectorCardStackView.swift`, `ObjectRelationsSection.swift`, `ObjectCard.swift` (through the helper); `Scrivi/Inkwyn.icon`;
`project.pbxproj` (Glyph.swift, Inkwyn.icon, the app-icon name); `AppIconMacOS`/`AppIconIOS` deleted; `.gitignore`.
Details, the mapping and the live pass: the Sprint.

# Verified Task — T-0636

**Sprint:** [SP-199] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-03** (iPad A5–A12, iPhone B2–B6, iPhone Duo simulator C1–C4 and the toolbar re-pass on Mac, iPad and iPhone (user, 2026-11-03))
**Archived:** 2026-11-03, on the user's reported pass.

---

## T-0636 — `[Apple]` EP-055 S2: panels by size class — the compact stack and the regular split

**Epic:** [EP-055] (AC6, AC8) · **Status:** ✅ **Verified 2026-11-03** (iPad A5–A12, iPhone B2–B6, iPhone Duo simulator C1–C4 and the toolbar re-pass on Mac, iPad and iPhone (user, 2026-11-03)) (2026-10-31) — ⏳ the simulator live passes · **Sprint:** [SP-199] (🟡 Active) → [`Sprint-SP-199.md`](../../Sprints/Closed/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What:** `EditorView`'s iOS container laid out by horizontal size class. **Compact:** one navigation stack, Navigator →
Manuscript → Inspector, the Inspector pushed as a page (G3), opening on the Manuscript. **Regular:** the `NavigationSplitView`
sidebar plus the `.inspector` trailing column, as on the Mac, with two- and three-panel versions shown to the user for a
choice. The `userInterfaceIdiom` branches (`EditorView.swift:481`) go.
**Tests:** the iOS target, the Mac interop tests, live passes on the iPad, iPhone and Duo simulators.
**Done (2026-10-31):** `EditorView.swift` iOS `container`. **Width compact** is a three-column `NavigationSplitView(preferredCompactColumn:)`
(Navigator | Manuscript | Inspector) opening on `.content`; the system collapses it to the ruled stack, with its own back button and back
swipe. Choosing a scene goes to the Manuscript page. **Width regular** is the two-column split plus the `.inspector` column; `columnVisibility`
is seeded from the saved navigator state and recorded back, as on the Mac (I-0255). The iOS `HStack` Inspector and both
`userInterfaceIdiom` gates (the Inspector, the Timeline) are gone. **Measured on the Duo simulator** with a probe: setting
`preferredCompactColumn` shows each of the three pages, and the Duo's window was 466×678 at width compact, with the back control on the
**right**. ⏳ Two- or three-panel at width regular is the user's call, made by showing and hiding panels on the iPad simulator.

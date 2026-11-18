# Verified Task — T-0681

**Sprint:** [SP-221] · **Epic:** [EP-054] · **Platform:** `[Apple]` + `[Linux]`
✅ **USER-VERIFIED 2026-11-18** (user, 2026-11-18: *"Mac, iPad and Linux live passes all passed, The Import... button on the mac looks the same as the other buttons.  close SP-221"*)
**Archived:** 2026-11-18.

---

## ✅ T-0681 — `[Apple]` iPad Landing: one Import… button

**Epic:** [EP-054] (AC8; moved 2026-11-17 (user, 2026-11-17: *"yes, move T-0681 into EP-054 and plan it"*)) · **Status:** ✅ **Verified 2026-11-18** (user) · **Sprint:** [SP-221] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-221.md`](../../Sprints/Closed/Sprint-SP-221.md) · **Date Requested:** 2026-11-15 (user, [SP-218]'s iPad
live pass: the iPad is used through its larger surfaces more than its menus; *"a nice to have"*)

**What:** replace the Landing's **Import Archive…** (`LandingView.swift:38`) with **Import…**, a menu of **Manuscript…**,
**Scrivener Project…** and **Archive…** — Timeline… left out ([SP-221] Q2); on the Mac, iPad and Linux (Q1).
**Where:** `Scrivi/Views/LandingView.swift` (`Menu("Import…")`); `platforms/linux/qml/Landing.qml` (a `Menu` on the button),
`ShellController::importNewManuscript` (`platforms/linux/src/ScriviWindow.hpp`).

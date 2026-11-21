# Verified Task — T-0694

**Sprint:** [SP-232] · **Epic:** [EP-059] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-21** (user, 2026-11-21: *"This passes now."*)
**Archived:** 2026-11-21.

---

## ✅ T-0694 — `[Linux]` EP-059 F1: the project window's toolbar, as the Mac's

**Epic:** [EP-059] (F1) · **Status:** ✅ Verified (user, 2026-11-21: *"This passes now."*) · **Sprint:** [SP-232] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-232.md`](../../Sprints/Closed/Sprint-SP-232.md) · **Date Requested:** 2026-11-21 (user, 2026-11-21: *"I'd like to scan the Epic for any tasks that involve creating a menu bar of buttons across the top like macOS and iOS have.  If missing, I'd like to make sure its added to the Epic as a new Sprint and AC."*)

**What:** a toolbar across the top of every Linux project window with the Mac's items (`ProjectToolbar.swift`), each the menus' own
action. Linux has no `QToolBar` today ([I-0244]). Findings, questions and ACs: the Sprint.

**Built:** `ScriviWindow::buildToolbar` (`ScriviWindow.cpp`): the Mac's items plus the Navigator, each the menus' own action; View ▸
Show Toolbar; the Mac's names for New Scene / New Chapter / Merge … with Previous. Details: the Sprint.

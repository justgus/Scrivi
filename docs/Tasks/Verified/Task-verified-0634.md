# Verified Task — T-0634

**Sprint:** [SP-198] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-03** (iPad A1–A4, A13 and iPhone B1–B7 (user, 2026-11-03), iPhone Duo simulator C1–C4 (user, 2026-11-03: *"all pass!"*))
**Archived:** 2026-11-03, on the user's reported pass.

---

## T-0634 — `[Apple]` EP-055 S1: the `UITextView` manuscript on iPad and iPhone

**Epic:** [EP-055] (AC2, AC3) · **Status:** ✅ **Verified 2026-11-03** (iPad A1–A4, A13 and iPhone B1–B7 (user, 2026-11-03), iPhone Duo simulator C1–C4 (user, 2026-11-03: *"all pass!"*)) (2026-10-30) — ⏳ the live passes ([SP-198] AC5) · **Sprint:** [SP-198] (🟡 Active) → [`Sprint-SP-198.md`](../../Sprints/Closed/Sprint-SP-198.md) · **Date Requested:** 2026-10-29 (user: *"It should work with the keyboard overlay and a hardware keyboard on both the iPad, iPhone, and iPhone Duo"*)

**What:** the iOS branch of `ManuscriptTextView` replaced by a TextKit 2 `UITextView` host on [T-0633]'s presenter: the viewport
scenes load, read and scroll; typing autosaves through the same path as the Mac; selection; dividers and chapter titles.
⛔ Nothing device- or keyboard-specific: the system's default keyboard and layout behaviour ([SP-198] R3).
**Tests:** [T-0635]'s target (a scene typed on the simulator reaches its `.md` file on disk); live passes on the user's iPad
and iPhone.
**Done (2026-10-30):** `ManuscriptUITextView.swift` — the `UIViewRepresentable`, its coordinator (rebuild, restore, navigate, the
viewport scene from scrolling) and `ManuscriptUITextView` (typing escaped, Return, ⌫, copy/paste as on the Mac; a cross-scene
copy/cut does nothing yet; system undo refused). The iOS stub is gone. ⚠️ Measured: UIKit does not ask `shouldChangeTextIn` for
every `insertText`/`deleteBackward`, so the overrides apply the rules themselves; and `UITextView(usingTextLayoutManager:)`
left the subclass's stored properties uninitialised (nil), so it has a designated `init()` (TextKit 2, asserted).
**Evidence:** the iOS suite (T-0635); a render of the real host on the simulator showed titles, headings, emphasis, hidden escapes
and both dividers.

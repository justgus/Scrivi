# Verified Task — T-0672

**Sprint:** [SP-211] · **Epic:** [EP-058] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-11** (user, 2026-11-11: *"All three (four) are fixed."*)
**Archived:** 2026-11-11.

---

## ✅ T-0672 — `[Linux]` EP-058 S3: authorship on Linux

**Epic:** [EP-058] (S3; AC3, AC4, AC7, AC8) · **Status:** ✅ **Verified 2026-11-11** (user) · **Sprint:** [SP-211] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-211.md`](../../Sprints/Closed/Sprint-SP-211.md) · **Date Requested:** 2026-11-11 (user: *"close SP-210 and plan EP-058 S3"*)

**What:** the Mac's authorship ([SP-210]) on Linux, in its shape and wording: the bridge's five calls and the window's
author from `writingAs`; New Project's Author; Settings ▸ Pen Names; Project Settings ▸ Authors with I write as; Worlds ▸
Authors…; the *Who wrote…?* and *Which of these authors are you?* questions on open. Shape per [SP-211] Q1–Q3 (✅ ruled as recommended).

**Built (2026-11-11):** the bridge's five calls, `adoptWritingAs` and `rememberPenName`, and New Project's author through
`createProject` (`ScriviBridge.*`); the window adopts the open's `writingAs` and asks on a first load (`EditorShell::askAuthor`);
`src/AuthorshipDialogs.{hpp,cpp}` (new) — Settings ▸ Pen Names, Project Settings ▸ Authors with I write as, Worlds ▸ Authors…,
the two questions (no Cancel; Esc and the close button do nothing); Edit ▸ Settings… (Ctrl+,), Project Settings… (Ctrl+Alt+,)
(`ScriviWindow.cpp`); the Author field (`NewProjectDialog.qml`). Evidence (scrivi-vm, scratch copy of the working tree):
`authorship_smoke` (new, in CI); ctest 839/839; every Linux smoke 43/43 — three fixtures (`settings`, `indent`, `replace`)
now name their author, as a project credited to "Scrivi" correctly opens *Who wrote…?* over the window they type into.
Boundary and ABI guard checks clean. ⏳ Linux CI after the push.

**Live pass (user, scrivi-vm, 2026-11-11):** steps 1–6 passed; four findings fixed and confirmed (*"All three (four) are fixed."*): Ctrl+, on the landing window, Settings… last in Edit below a rule, Project Settings… last in Project below a rule, and the dock showing Scrivi's icon (the Wayland app id `scrivi`). Linux App CI ✅ on `59b4a82` (the Authorship smoke step included).

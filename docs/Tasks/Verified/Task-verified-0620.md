# Verified Task — T-0620

**Sprint:** [SP-192] · **Epic:** none (the Linux half: [EP-026] S3) · **Platform:** `[Apple]`
**Implemented:** 2026-10-24 · ✅ **USER-VERIFIED 2026-10-24** (user: *"tests passed. go ahead and mark the Task verified and the sprint closed (It was apple only)."* — after the Mac live pass: *"All 6 steps
pass"*, and the re-check of ⇧⌘Z opening at the mark) — SP-192 AC7
**Archived:** 2026-10-24, on the user's instruction; the Sprint closed the same day (user-approved).

---

## T-0620 — `[Apple]` A marker where ⇧⌘Z will ask which branch

**Epic:** ⚠️ none (Linux half: [EP-026] S3) · **Status:** ✅ **Verified 2026-10-24** (user: *"tests passed. go ahead and mark the Task verified and the sprint closed (It was apple only)."*) · **Sprint:** [SP-192] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-192.md`](../../Sprints/Closed/Sprint-SP-192.md) · **Date Requested:** 2026-10-24 (user: *"Start with the
"will ⇧⌘Z ask me here" marker."*)

**What:** when the history pointer sits on a branch point (the core's `redoFork`, [I-0320]), draw a marker in the
manuscript where the branches part, so the writer can see that ⇧⌘Z will ask which branch. The current branch point
only, with one mark per line a branch starts on ([SP-192] Q1).

**Constraints already established:** a branch point is a point in the HISTORY, not a text position. It is anchored at the
scene and caret where the branches part, and that anchor moves with edits. The marker is drawn, never stored: it must not
enter the scene text, caret stops, Find, copy or word count. A margin mark is the recommendation over an inline one.

**✅ Ruled (user, 2026-10-24):**
- **Q1 Where:** the LEFT MARGIN, beside the line where the branches part.
- **Q2 When:** only while the Shift-Cmd-H markers are showing.
- **Q3 Click:** opens the branch chooser, the same as ⇧⌘Z.
- **Q4 Linux:** *"Eventually yes."* The Linux half goes with the Linux branch chooser in [EP-026] S3 (not yet created).

---

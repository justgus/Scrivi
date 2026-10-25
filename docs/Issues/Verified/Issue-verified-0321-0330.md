# Verified Issues: I-0321 – I-0330

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0311–I-0320 are in [`Issue-verified-0311-0320.md`](Issue-verified-0311-0320.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0321** | `[ScriviCore]` ⚠️ **The branch chooser offered a branch with no text: a change that only typed spaces (then undone) became a branch of its own.** Found by the user 2026-10-24 ([SP-188] Mac pass). → [details](#i-0321) | **Medium** | [SP-188] | ✅ **VERIFIED 2026-10-24** |

---

## I-0321

**Title:** The branch chooser offers a branch with no text
**Status:** ✅ **VERIFIED 2026-10-24** (user, Mac: *"I-0321 passes, mark it verified"*)
**Platform:** `[ScriviCore]` (the Mac shows what the core offers; Linux has no chooser)
**Component:** `HistoryService::forkAheadAt` (`ScriviCore/src/history/HistoryService.cpp`)
**Severity:** Medium. A choice with nothing in it gives the writer nothing to decide by.
**Sprint:** [SP-188]
**Date Identified:** 2026-10-24, user, Mac, on `dumas-prose-timelines`

**What happened (user):** *"I type "One", Undo, "Two", Undo, Redo "One", Then I type some more text after one. Undo, Undo,
Redo "One", Redo again. Choicepoint, somehow I have a choicepoint with no actual text in it. Or its all spaces. That is
unhelpful. Historical branches should be merged with the nearest sibling so that the writer is always presented with text
to choose from."*

**Cause (read from the project's own history log, today's session):** the fork after "Three\." had five branches, one of
them `inserted: "  "`: spaces typed after the sentence and then undone. The undo committed them as an event first, and the
next change from that point became its sibling. A branch can also START with whitespace (`"  One"`), and its preview showed
the leading spaces.

**✅ Ruling (user, 2026-10-24):** the writer is always presented with text to choose from.

**Resolution (2026-10-24):** the chooser's branches (`forkAheadAt`, used by `redoFork`) leave out a branch that has no
text anywhere in it. That branch is folded into its siblings: still in the history tree and the History card, just not
offered. Each branch that is offered shows the first text down its primary path, trimmed. If only one branch with text is
left, there is no chooser. Text = an insertion or removal with a non-whitespace character; a structural step always counts.
Core test `[I-0321]`.
- ⚠️ Not handled: if the blank branch is the PRIMARY one and only one other branch has text, ⇧⌘Z (no chooser) still
  follows the primary, and re-types the spaces.
- ⚠️ Not changed: an undo still commits typed spaces as their own event, which is what creates such a branch. Whether
  whitespace alone should ever be an undo step is a separate question.

---

*Last Updated: 2026-10-24 — **I-0321** archived (user-verified, Mac).*

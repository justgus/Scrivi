# Verified Task — T-0643

**Sprint:** [SP-200] · **Epic:** [EP-052] · **Platform:** `[ScriviCore]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0653] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0643 — `[ScriviCore]` History load b1: a diff is a list of hunks

**Epic:** [EP-052] (AC11) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0653] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-200] (🟡 Active) →
[`../../Sprints/Closed/Sprint-SP-200.md`](../../Sprints/Closed/Sprint-SP-200.md) · **Date Requested:** 2026-11-01

**What:** design §3 b1. `Diff` holds hunks; the old one-change shape is read as one hunk; apply, reverse, the match check,
the I-0107 rebase and the tree's change span follow. `computeDiff` finds several hunks as Q3 rules. Replace All's
diffs are 20% of dumas's log.
**Done (2026-11-01):** `Diff` keeps its first change and adds `more`; `computeDiff` (`HistoryService.cpp:168`) splits a
changed middle over 4 KB with a Myers diff (`:124`) — ⚠️ **by word, not by character** (300 replaced short words are ~600
word edits but ~1,800 character edits; a word never splits a UTF-8 character); merged within 8 bytes, capped at 64.
Apply, reverse, the match check and the I-0107 rebase handle every change; on disk one change keeps the v0.1 shape,
several are `{"hunks":[…]}` (`HistoryStore.cpp:41`). Tests `[T-0643]` (ASCII with a length change, UTF-8, 350 matches
merged to 64; the log's shape and an exact undo after a reopen); a mutation (reverse not shifting) is caught — the first
cases kept every length equal and missed it. ⚠️ **Measured on dumas: 6.86 of its 6.91 MB of Replace All diffs are UNDER
4 KB, so the ruled threshold changes nothing there; 1 KB would take them to 4.6 MB.** ✅ Kept at 4 KB, and the by-word diff accepted (user, 2026-11-01: *"no 4KB is sufficient. These departures are accepted."*). ctest 799/799, Mac interop 328/328, iOS 17/17, Linux smokes 38/38 (Docker, Qt 6.4, offscreen, non-root).

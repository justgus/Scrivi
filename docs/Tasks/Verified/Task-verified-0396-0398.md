# T-0396, T-0397, T-0398 — History Capture Granularity & Presentation (EP-019, SP-093)

**Status:** ✅ **All Verified 2026-08-10** (user-confirmed in the live session)
**Epic:** EP-019 — Undo/Redo History & Copy Buffers · ✅ **Closed 2026-08-11**
→ [`Epic-EP-019.md`](../../Epics/Closed/Epic-EP-019.md)
**Sprint:** SP-093 — history capture granularity + presentation · **Record of truth:**
[`Sprint-SP-093.md`](../../Sprints/Closed/Sprint-SP-093.md) (*"Items closed — 11, all Verified"*)

SP-093 was opened from the SP-092 live verify. **None of its items was a regression from SP-092** —
the capture-granularity behaviour predated it.

---

## T-0396 — `[Apple]` Typing-session coalescing

✅ **Verified 2026-08-10** (exit criteria EC2–EC5). App-side only — `HistoryService` untouched.

Autosave **defers** the commit and records nothing mid-session; an entry seals at a real boundary or
on a **45 s** idle timer. Cursor-move, cut/paste, scene switch and sentence terminators are all
**kept** as commit triggers; **backspace does not commit**.

> ⚠️ **§4.d was relaxed — a changed invariant, user-approved 2026-08-10.** "Disk never contains text
> no history node describes" no longer holds strictly: while a typing session is open, **disk may lead
> history by at most one save**. Bounded by `close()` committing pending text and, on a hard crash, by
> I-0104's head-hash check raising an `externalChange` barrier. The rationale is that
> `HistoryService::record` always appends and undo walks one node per step, so recording per-save
> cannot yield one undo step. Approving EP-019's close was explicitly an approval of the relaxed §4.d.

**Coverage gap carried forward:** `HistoryCapture` is not compiled into the test target, so T-0396's
45 s idle boundary has no automated coverage.

## T-0397 — `[Cross]` Whitespace-kind labels in history

✅ **Verified 2026-08-10** (EC11). Replaces the uninformative "(no text)" row.

A new `whitespaceKind` tree field (e.g. `"newline:2"`) maps to a `whitespaceLabel`
(e.g. `"⏎ new paragraph"`). ⚠️ **`preview` was left untouched deliberately**, so the fork popover,
stale-branch detection and purge consumers are unaffected.

## T-0398 — `[Cross]` Distinguish added vs. deleted text in history rows

✅ **Verified 2026-08-10** (EC10; user-confirmed).

`minus.circle` glyph + orange tint + a "Deleted …" label prefix, with `removedLength` carried in the
tree payload. **Shipped once, shared with I-0106** (the wrong-entry-bolded fix) rather than
implemented twice.

---

*Archived 2026-08-18 during the tracking-documentation audit. `Task-backlog.md` had carried T-0396
and T-0397 as 🟠 Implemented — Not Verified after SP-093 closed, though the Sprint archive's own
header records all eleven of its items as Verified.*

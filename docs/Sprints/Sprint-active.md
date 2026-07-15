# Active Sprint

**No sprint is currently active.**

**SP-064** (EP-022 `[Linux]` — fourth of 4, **closes EP-022**) is **drafted (🔵 Planning)** and ready to
activate — see the full draft in `Sprint-backlog.md`. It does the EP-011-equivalent **cursor/focus polish**
and **quit→reopen surface restore** (last active scene + cursor + scroll, via the `restored{anchor,focus,
scroll}` payload `scrivi_open_project` returns and the `selectionAnchor/focus/scroll` args
`scrivi_save_scene` persists — both already present), then the **full EP-022 verification** pass. Delivers
**AC4 + AC5 + AC6 + AC7** and **closes the Epic**. Awaiting go-ahead to move it here as 🟢 Active.

Last closed: **SP-063** — `[Linux]` Scroll-driven scene switching + active-scene tracking — ✅ closed
2026-07-15, all three tasks Verified (T-0243–T-0245); delivered EP-022 `[Linux]` **AC1/AC4 scroll portion**,
user-verified over VNC. Mid-sprint the navigator-click requirement was reversed (caret **moves** to the
clicked scene's start, per user); two throwaway click-scroll bugs found & fixed before commit. Raised
**T-0249** (unscheduled: Page Forward/Backward + jump to manuscript start/end — bindings undecided).
Archived to `Closed/Sprint-SP-063.md`.

**Active Epic:** **EP-022** `[Linux]` — Writing Surface & Scene Navigator (`Epic-active.md`) — AC1/AC2/AC3
delivered + AC4 scroll portion done; **AC4 (cursor/focus) / AC5 / AC6 / AC7 remain → SP-064** (closes the
Epic).

SP-056/SP-057 (EP-019 `[Apple]` — copy buffers, history panel) remain 🔵 Planning but are **parked** behind
the deferred EP-019.

---

*Last Updated: 2026-07-15 (SP-063 closed, all tasks Verified, EP-022 AC1/AC4-scroll delivered. SP-064
drafted in `Sprint-backlog.md` (🔵 Planning) — cursor/focus + quit-reopen restore + EP-022 verify & close,
AC4/AC5/AC6/AC7 — awaiting activation. T-0249 logged unscheduled.)*

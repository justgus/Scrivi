# Verified Task — T-0584

**Sprint:** [SP-163] · **Epic:** [EP-046] (E2-S3) · **Platform:** `[Apple]` + `[Linux]` (Alt-Return storage)
**Implemented:** 2026-10-05 · ✅ **USER-VERIFIED 2026-10-06 by live pass** (*"1 through 6 above all pass."*)
**Archived:** 2026-10-06, with the Sprint close (*"yes, close SP-163."*).

---

## ✅ T-0584 — `[Apple]` Option-Return stores ONE `\n` — rule what it should store — ✅ **VERIFIED 2026-10-06 (user, live pass)**

**Created:** 2026-10-04 (user: *"file the two tasks and create a planning sprint for EP-046"*)
**Epic:** ✅ **[EP-045] follow-up → [EP-046]** · **Sprint:** ✅ RULED in **[SP-159]** (CLOSED) → [`../Sprints/Closed/Sprint-SP-159.md`](../../Sprints/Closed/Sprint-SP-159.md); implementation → EP-046 **E2-S3** = 🔵 **[SP-163]** (Planning) → [`../Sprints/Sprint-SP-163.md`](../../Sprints/Closed/Sprint-SP-163.md).
**Origin:** [SP-156] AC-measure (→ [`../Sprints/Closed/Sprint-SP-156.md`](../../Sprints/Closed/Sprint-SP-156.md)): Option-Return sends
`insertNewlineIgnoringFieldEditor:` and stores a single `\n` (the user confirmed `0x0A` on 2026-10-04).
⚠️ **Why it matters under EP-046:** a single `\n` is a SOFT break — it displays as a line break today (E1 shows
storage) but RENDERS AS A SPACE once inline rendering lands, so the writer's line break would silently vanish.
✅ **Options:** (a) treat as Return (`\n\n`, a paragraph); (b) a deliberate hard break (`\` + `\n`, the backslash hidden
— the same form AC6 gives a typed trailing backslash); (c) leave it. ⚠️ Recommendation at filing: (b) — a modified
Return usually means "line break within the paragraph".
✅ **RULED 2026-10-05 (Q-E2-4): (b) a hard break, `\` + `\n`.** Implementation → EP-046 **E2-S3** (AC9).

🟠 **2026-10-05 — IMPLEMENTED - NOT VERIFIED** ([SP-163]): `insertNewlineIgnoringFieldEditor:` stores `\` + `\n` — ✅ and Linux's Alt-Return the same (shared corpus updated; `escape_smoke` PASS in the canonical Linux image). ⏳ Live pass.
✅ **VERIFIED 2026-10-06 (user):** the full SP-163 live pass, its two fixes and their re-checks — last: *"1 through 6 above all pass."* ✅ Archived 2026-10-06 with the SP-163 close.

# Verified Task — T-0585

**Sprint:** [SP-164] · **Epic:** [EP-046] (E2-S4) · **Platform:** `[Apple]`
**Implemented:** 2026-10-06 · ✅ **USER-VERIFIED 2026-10-06 by live pass** (*"1 through 6 pass."* — after the Replace All and Navigator-filter fixes)
**Archived:** 2026-10-06, with the Sprint close (*"yes, close SP-164 and run the Audit Check."*).

---

## ✅ T-0585 — `[Apple]` Find/Replace across hidden escape backslashes — Verified (user, 2026-10-06)

**Created:** 2026-10-04 (same request) · **Epic:** **[EP-046]** · **Sprint:** ✅ RULED in **[SP-159]** (CLOSED) → [`../Sprints/Closed/Sprint-SP-159.md`](../../Sprints/Closed/Sprint-SP-159.md); implementation → EP-046 **E2-S4** = 🟡 **[SP-164]** (ACTIVE) → [`../../Sprints/Closed/Sprint-SP-164.md`](../../Sprints/Closed/Sprint-SP-164.md).
**Origin:** [SP-155] (EP-045 AC4), recorded there as a follow-up and carried unfiled until now.
⛔ **The defect:** the writer sees `*`; storage holds `\*`. AppKit's Find matches STORAGE, so searching for `*` does
not find it, and a Replace could split an escape pair (leaving an orphaned `\` or a live mark).
⚠️ **Why under EP-046:** once markers hide too (Model B), the gap between what the writer sees and what is stored
widens — `**bold**` shows as **bold** — so Find must match the PRESENTED text, and Replace must write through the
escape layer. ✅ The AC3 SOURCE↔PRESENTED map is the existing seam for this.
✅ **RULED 2026-10-05 (Q-E2-5): match the PRESENTED text; replacements written through the escape layer.** Implementation →
EP-046 **E2-S4** (AC10); first task: whether `NSTextFinder` can be pointed at presented text (design §7).

🟠 **2026-10-06 — IMPLEMENTED - NOT VERIFIED** ([SP-164]) — Find matches the PRESENTED text; Replace writes through the escape layer. ⏳ Live pass.

✅ **2026-10-06 — VERIFIED (user, live pass 2)** — Replace All on dumas: 1,173 edits / 1,171 scenes, apply 83 ms, history 220 ms
(was ~1 minute); the find bar shows its count; one ⌘Z restores all; the Navigator filter matches a period. Record →
[`../../Sprints/Closed/Sprint-SP-164.md`](../../Sprints/Closed/Sprint-SP-164.md).

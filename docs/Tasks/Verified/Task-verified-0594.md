# Verified Task — T-0594

**Sprint:** [SP-165] · **Epic:** [EP-048] (S1) · **Platform:** `[Cross]`
**Implemented:** 2026-10-07 · ✅ **USER-VERIFIED 2026-10-07** (*"md4c treats emoji as punctuation, accepted.  You may verify and close SP-165."*)
**Archived:** 2026-10-07, with the Sprint close.

---

## ✅ T-0594 — `[Cross]` EP-048 S1: the Markdown analyzer in ScriviCore (md4c) + the Apple agreement test — Verified (user, 2026-10-07)

**Created:** 2026-10-07 at [SP-165]'s activation (user: *"We do want to activate EP-048 … get start on EP-048"*) · **Epic:** **[EP-048]** ·
**Sprint:** ✅ **[SP-165]** → [`../../Sprints/Closed/Sprint-SP-165.md`](../../Sprints/Closed/Sprint-SP-165.md)
✅ Carries SP-165's work: EP-048 **L1** (md4c 0.5.2 via pinned FetchContent; `MarkdownAnalyzer` in `ScriviCore/src/markdown/`
reporting what `MarkdownBlocks.Analysis` reports, in UTF-8 bytes; `scrivi_analyze_markdown`, a new endpoint) and **L2** (the
interop agreement test over the AC3 + S5 corpora), plus the per-block cost on 1.85 MB. Q1–Q3 ruled 2026-10-07.

🟠 **2026-10-07 — IMPLEMENTED - NOT VERIFIED** ([SP-165]): ctest macOS 665/665, Linux (Docker, uid 1000) 669/669, interop 208/208; L2 agrees on S5 4,000/4,000, AC3-escaped 1,918/1,918, structural 46/46, AC3-raw 1,912/1,918 (six listed). ⏳ One ruling (symbol class) → [`../Sprints/Sprint-SP-165.md`](../../Sprints/Closed/Sprint-SP-165.md). Found [I-0280] (fixed), [I-0281].

✅ **2026-10-07 — VERIFIED (user):** *"md4c treats emoji as punctuation, accepted.  You may verify and close SP-165."* No live pass applies (nothing a writer sees); verified on the suites and the L2 report. Symbol class ruled accepted (Q4).

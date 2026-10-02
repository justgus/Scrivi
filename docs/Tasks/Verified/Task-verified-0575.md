# Verified Task: T-0575

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0575** | ✅ **The divider that ENDS a chapter is tinted — the accent colour at 60%** | [SP-151] | — | **2026-10-02** |

✅ **VERIFIED by the user 2026-10-02:** *"yes 60% strength is acceptable."*

## What shipped
- ✅ `DividerTextAttachment.endsChapter` (set in `rebuildStorage` at each chapter boundary) →
  `controlAccentColor.withAlphaComponent(0.6)`; scene dividers keep `secondaryLabelColor`.
- ✅ Three passes by user feedback: `labelColor` (*"too subtle"*) → full accent (*"may be too much"*) → accent at 60%.
- ✅ Scene dividers were NOT dimmed: `secondaryLabelColor` (5.89:1) is the measured visibility floor ([I-0252]).
- ⚠️ Applies with chapter titles on too. ⚠️ **Linux N/A** — it has no chapter-titles toggle (pre-existing gap).
- ⚠️ The divider is slated to become a typeset `* * *` mark (Manuscript Rendering trade study §1.5); carry this rule over.

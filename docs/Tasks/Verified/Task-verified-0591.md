# Verified Task — T-0591

**Sprint:** [SP-163] · **Epic:** [EP-046] (E2-S3) · **Platform:** `[Apple]`
**Implemented:** 2026-10-05 · ✅ **USER-VERIFIED 2026-10-06 by live pass** (*"1 through 6 above all pass."*)
**Archived:** 2026-10-06, with the Sprint close (*"yes, close SP-163."*).

---

## ✅ T-0591 — `[Apple]` Balance emphasis across SCENE boundaries — ✅ **VERIFIED 2026-10-06 (user, live pass)**

**Created:** 2026-10-05 (user: *"Lets file the balancing task for the cut copy across scene boundaries."*) · **Epic:** **[EP-046]** · **Sprint:** 🟡 **[SP-163]** (added by the user at activation, 2026-10-05) → [`../Sprints/Sprint-SP-163.md`](../../Sprints/Closed/Sprint-SP-163.md).
**Origin:** [SP-162] retrospective (→ [`../Sprints/Closed/Sprint-SP-162.md`](../../Sprints/Closed/Sprint-SP-162.md)).

**"Balanced"** = every opening emphasis marker has its closing marker in the same paragraph, so the parser renders the
formatting and the markers stay hidden. An unbalanced `**bo` shows its `**` LITERALLY and the bold is lost (Q-E2-7).
E2-S2 keeps every in-scene edit balanced (`ManuscriptPresenter.balancedEdit` via `shouldChangeText`, EP-046 AC12 / Q3).

⛔ **READ 2026-10-05 — a selection that crosses a scene divider takes a separate path that never reaches it:**
1. **Cut / ⌫ / ⌦ across scenes** — `Coordinator.deleteAcrossScenes` (`ManuscriptTextView.swift`, [I-0270]) deletes each
   scene's part with `storage.deleteCharacters`, bypassing `shouldChangeText`: `**bo|ld**` ⟨divider⟩ `|…` leaves an
   unclosed `**bo` in the first scene.
2. **The cross-scene copy for Scrivi's own paste** — `structuredCopyIfCrossBoundary` keeps ScriviCore's
   `fragmentExtract` result: raw source slices per scene, no markers added at the cut points.
3. **What other apps get** from that copy — `MarkdownEscapes.map(frag.plainText).presented` removes escapes but KEEPS
   `**`/`*` and `## ` (⛔ against [SP-162] Q2: markers removed).
4. **Pasting that fragment** — `pasteStructuredFragment` (through ScriviCore) does not merge bold into bold.

✅ **Expected:** the same rules as E2-S2, per scene — each scene's remaining text balanced; each scene slice of the
fragment carries its own markers; other apps get the presented text (`balancedCopy`); a paste merges like an in-scene
paste. ✅ **Seam:** `balancedEdit` / `balancedCopy` work on any storage range inside one scene, so each scene part can
go through them; ⚠️ open: whether the per-scene balancing belongs in ScriviCore's fragment ops (Linux would then get it
— [EP-048] L7) or stays Apple-side, as E2-S2's does. Rule that when scheduled.
⚠️ Each scene part must stay ONE history event per gesture (`recordGroupedEdit`), as [I-0270] made it.

🟠 **2026-10-05 — IMPLEMENTED - NOT VERIFIED** ([SP-163]) — ✅ **RULED Apple-side** (2026-10-05): `deleteAcrossScenes` balances each scene part; the structured copy carries balanced pieces and gives other apps the presented text; the structured paste continues / re-opens the caret's span. ⏳ Live pass.
✅ **VERIFIED 2026-10-06 (user):** the full SP-163 live pass, its two fixes and their re-checks — last: *"1 through 6 above all pass."* ✅ Archived 2026-10-06 with the SP-163 close.

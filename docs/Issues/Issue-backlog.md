# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

**Currently: 1.** Only **I-0018** remains, 🟠 **partly fixed and rescoped** — not assigned to a Sprint.

⚠️ **Verified Issues do not belong in this file.** I-0058 (Verified 2026-07-09) and I-0112 (Verified
2026-08-11) sat here for weeks after verification and were archived to `Verified/` on 2026-08-16. Archive an
Issue in the same step it is verified.

✅ **I-0121 and I-0122 archived 2026-08-17**, in the same step SP-106 closed — to
[`Verified/Issue-verified-0121-0130.md`](Verified/Issue-verified-0121-0130.md). Both were Verified with the
sprint. Their full entries were removed from this file rather than left behind, per the rule above.

✅ **I-0017 archived 2026-08-17** → [`Verified/Issue-verified-0011-0020.md`](Verified/Issue-verified-0011-0020.md).
⚠️ The user reports it was *"fixed ages ago and confirmed fixed"* — **it had simply never been moved out of
this file**, so the backlog kept reporting an open defect that was not open. That is the third instance of
exactly the rot the warning above describes (after I-0058 and I-0112), and the second found by the user
rather than by an audit.

---

## I-0018: Manuscript does not scroll to the restored scene on app load

**Status:** 🟠 **PARTLY FIXED — rescoped 2026-08-17 (user-verified).** The original complaint is **fixed**:
the Navigator *does* now show the selection on load. What remains is a **different behaviour** the original
report did not name — **the manuscript does not scroll to that selection**, so the writer is shown a
highlighted scene in the navigator while the text view sits somewhere else.
**Platform:** macOS
**Component:** `SceneNavigatorView.swift`, `ViewportSceneLoader.swift`, `ManuscriptTextView.swift`
**Severity:** Low
**Sprint:** Not Assigned
**Date Identified:** 2026-06-08 · **Rescoped:** 2026-08-17

**Fixed (original scope):**
On load, no scene was selected/highlighted in the Navigator; it self-corrected only on first scroll. The root
cause was `viewportSceneID` being left nil during `loadAll()`. `loadAll(activeSceneID:)` now sets
`currentIndex`, `cursorSceneID` **and** `viewportSceneID` when the backend supplies a resume scene
(`ViewportSceneLoader.swift:128-134`), so the highlight is present from the first frame.

**Still open (rescoped):**
The manuscript viewport is not scrolled to the restored scene on load. The selection is shown; the text is
not brought to it.

⚠️ **Interacts with [[I-0131]] and [[I-0132]], and should be scoped with them.** All three are the same
question in different guises — *what does it mean to "be at" a scene?* — and the answer has to be consistent
across restore-on-load (this Issue), navigator click (I-0132's focus/caret question), and quit-time resume
(I-0131). Fixing this one in isolation risks a fourth inconsistent behaviour. ⚠️ It also touches the code
I-0131 just changed: `restoreWritingSurface` already scrolls and places the caret, so the likely fix is
ensuring that path runs on load when a resume scene exists, rather than adding a second scroll mechanism.

**Resolution:** TBD — pending the keyboard/focus/caret model ruling that [[I-0132]] also needs.

---

---


---

*Last Updated: 2026-08-17, later same day (**I-0017 ✅ Verified and archived; I-0018 partly fixed and
RESCOPED** — both on the user's report while reviewing this backlog. I-0017 had been fixed and confirmed long
ago and was never filed. I-0018's original complaint (no selection shown on load) is **fixed**; the remaining
behaviour — **the manuscript not scrolling to that selection** — is different from what was reported, so the
Issue is retitled and rescoped rather than left implying the whole thing is broken. ⚠️ It is flagged to be
scoped **together with I-0131 and I-0132**, which are the same underlying question — *what does it mean to
"be at" a scene?* — across load, click, and quit. **Backlog is now 1.** Prior note follows.)*

*2026-08-17 (**I-0121 and I-0122 ✅ Verified and archived** to
`Verified/Issue-verified-0121-0130.md` at the SP-106 close — both full entries removed from this file in the
same step, which is the discipline the 2026-08-16 note below says was missed twice. **Backlog is now 2:**
I-0017 and I-0018, both 🔴 Open and unassigned. Prior note follows.)*

*2026-08-16, later same day (**I-0058 and I-0112 archived to `Verified/`; I-0121's code fix
applied.** Both Issues had been Verified — 2026-07-09 and 2026-08-11 — and left sitting in this backlog.
⚠️ **The consistency audit earlier the same day did not catch them because it never opened this file**: it
audited `Issue-active.md`, `Issue-Documentation.md` and the `Verified/` archives, and only *grepped*
`Issue-backlog.md`. **An audit scoped to the files that usually go stale will miss the file nobody looks at**
— which is exactly where a Verified Issue goes to be forgotten. I-0121's `rebalancedKeys` guard is now applied
and proven RED-then-GREEN under UBSan (516/516); its CI and coverage halves remain open. Prior note follows.)*

*2026-08-16 (**I-0121 opened — ScriviCore CI has been red on every commit since 2026-07-30.**
`rebalancedKeys(1)` divides by zero (`OrderKey.cpp:179`): the ternary guards `n == 0` — already unreachable —
while the divisor is `n - 1`. ⚠️ **Invisible locally by hardware, not by luck**: arm64 returns 0 for integer
division by zero, x86-64 raises SIGFPE, so the developer Mac and the `macos-latest` runner are green while
`ubuntu-latest` crashes. Introduced by **1c42838**, confirmed by `git log -S`, matching the user's report to
the commit. **Assigned to EP-031, to be scoped at the next sprint's planning** (user ruling) together with
adding `-fsanitize=undefined` to CI, which may change the test configuration. Prior note follows.)*

*2026-08-11 (I-0112 opened, then **root cause corrected**. Filed as a suspected live-appearance-switch
staleness bug; the user disproved that within minutes — Dark Mode had been active for hours and the app was
launched minutes before the defect was seen, so no switch was involved. Confirmed cause is static: body-text
attribute dictionaries omit `.foregroundColor` (`:517`, `:296-298`) and `textColor` is never set, so AppKit
renders body runs as literal `NSColor.black` against an adaptive dark background. Manuscript-only, as the sole
AppKit text surface. Sprintless/unassigned. I-0017/I-0018/I-0058 unchanged.)*

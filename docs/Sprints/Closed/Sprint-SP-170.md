---
sprint: SP-170
epic: EP-047
status: Closed
closed: 2026-10-08
activated: 2026-10-08
platform: Apple
created: 2026-10-08
---

# SP-170 — `[Apple]` [EP-047] **S4**: Markup Hints on/off ([T-0589])

**Status:** ✅ **CLOSED 2026-10-08** — activated 2026-10-08 — ✅ **CLOSED 2026-10-08 (user-approved):** *"close SP-170"* All ACs met; T-0589 VERIFIED and archived. (user: *"yes please"* (to *"Shall I activate SP-170 and start?"*)). Created 2026-10-08; rulings P11 taken in planning.
**Tasks:** ✅ [T-0589] → [`../../Tasks/Verified/Task-verified-0589.md`](../../Tasks/Verified/Task-verified-0589.md)
**Epic:** [EP-047] `[Apple]` Manuscript Typography & Preferences → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md). **EP-047's last Sprint.** Previous: [`Sprint-SP-169.md`](Sprint-SP-169.md).
**Carries:** [T-0589] (moved from the backlog at activation) (caret rules ruled 2026-10-05; "where it lives" ruled P1: `project-settings.json`).
**Authority:** EP-047 **AC7**; rulings **P1** (stored in `project-settings.json`), **P11** (⇧⌘H; default ON).
**Size:** S–M. ✅ Needs no rig: the live pass is on the Mac.

---

## ⚠️ What reading the code found (2026-10-08)

- ✅ **The reveal lives in ONE place** — `ManuscriptPresenter.reveal(for:in:)` (and `isHidden(_:in:revealing:)`), as [T-0589]'s seam
  note said. ✅ With hints OFF the revealed sets are empty; toggling re-presents only the lines and spans whose drawing changes —
  ✅ the attributes-only `.editedAttributes` edit the reveal already uses: no rebuild, no history event, no `.md` change.
- ✅ **[T-0589]'s caret rules for hints-OFF ALREADY HOLD.** Since [SP-162] (design §3.6) a stop run's HOME is independent of the
  reveal: heading prefix and opening marker → AFTER (rules 1, 2), closing marker → BEFORE (rule 3). ⛔ So hints-off changes what is
  DRAWN, not where the caret goes — Plan 4 PROVES it with a test rather than assuming it.
- ✅ **⇧⌘H is free** — not in any menu (`keyboardShortcut` survey) nor `ManuscriptNSTextView.keyDown` (no letter chords), and not a
  system chord. ⌥⌘M (Minimize All) and ⌥⌘H (Hide Others) were ruled out.
- ⚠️ **List prefixes are not hints** — they stay visible and dimmed in both states ([SP-163] Q7). Rule 4 of [T-0589] ("a formatted
  section always begins and ends with a visible character") was delivered by EP-046 E2-S3.

## Goal

✅ **The writer turns the Markup Hints on and off — View ▸ Show Markup Hints, ⇧⌘H — per project, and it travels.** Off: the page
looks finished even with the caret beside a heading or a bold word; the caret still lands where typing does what she expects.

## EP-047 ACs this Sprint meets

| AC | Criterion (Epic wording, short) |
| -- | ------------------------------- |
| **AC7** | View menu (+ ⇧⌘H), persisted in `project-settings.json`; OFF follows the ruled caret rules; toggling re-presents only — no rebuild, no history event |

## Plan

1. **Setting:** `markupHints` (bool; ABSENT = ON, P11) in `project-settings.json` via `ProjectPreferences` (written only once chosen —
   the SP-168 rule).
2. **The presenter:** `hintsEnabled`; when false `reveal` reveals nothing and `isHidden` hides every marker and prefix. A change
   re-presents only what was revealed (attributes-only, the reveal's own mechanism).
3. **View ▸ Show Markup Hints** (⇧⌘H) on macOS — a `Toggle` beside Show Scene Inspector / Timeline / Buffers, bound to the focused
   project's preferences; `ManuscriptTextView` takes `markupHints` and applies it in `updateNSView` WITHOUT a rebuild.
4. **Tests** (through the app's real paths): hints OFF — caret at a heading's start, on a bold word's first and last character: every
   marker stays hidden (drawn width ~0); the caret's home per rules 1–3; toggling re-presents without a text change or history event;
   settings default ON and round-trip; mutation-checked.
5. **Live pass** (user, Mac) — steps in the chat reply.

⛔ **NOT in this Sprint:** Linux (EP-048 L10) · a "show ALL markup" mode ([T-0589]: hints only).

## ✅ Results (2026-10-08)

**Built:** `ManuscriptPresenter.hintsEnabled` + `setHints(_:selection:in:)` — `reveal` reveals nothing when off and re-presents
only what changes (attributes only); `isHidden` hides every prefix and marker when off; the PENDING pair stays shown ·
`ProjectPreferences.markupHints` (absent = ON; written only once chosen) · `ManuscriptTextView.markupHints`, applied in
`makeNSView` / `updateNSView` WITHOUT a rebuild · View ▸ **Show Markup Hints** (⇧⌘H) on macOS, disabled with no project.

| Check | Result |
| ----- | ------ |
| `MarkupHintsTests` (5) | ✅ ON reveals; OFF hides and DRAWS hidden beside the caret; T-0589 caret rules 1–3 hold with hints OFF (through the text view's real snapping); a toggle changes no text and posts no `textDidChange`; settings default ON, round-trip, written only once chosen |
| Mutations (5) | ✅ 5/5 killed. ⚠️ The first test draft would NOT have caught M3 (a toggle that does not re-present): it checked the presenter's LOGIC (`isHidden`), not what was drawn without moving the caret — a drawn check was added before mutating |
| Interop (full) | ✅ 243/243 in 29 suites |
| iOS + visionOS builds · type-source guard · stub parity | ✅ · ✅ · ✅ |

## Acceptance Criteria

- [x] AC7: View ▸ Show Markup Hints + ⇧⌘H; persisted (absent = ON); OFF hides every hint; caret rules 1–3 hold (tested); a toggle
      re-presents only — no rebuild, no history event, no `.md` change
- [x] Interop green; type-source guard clean
- [x] Live pass (user, Mac) — ✅ 2026-10-08, steps 1–5: *"1. passes. 2. passes. 3. passes. 4. passes. 5. passes."*

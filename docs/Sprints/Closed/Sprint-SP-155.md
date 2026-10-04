---
sprint: SP-155
epic: EP-045
status: Closed
closed: 2026-10-03
activated: 2026-10-03
platform: Apple
created: 2026-10-03
---

# SP-155 — `[Apple]` [EP-045] S3: The escape layer (AC4)

**Status:** ✅ **CLOSED 2026-10-03 (user-approved):** *"live check passes, close SP-155"*. Q-Linux ruled (option 1).
**Epic:** [EP-045] → [`../Epics/Epic-EP-045.md`](../Epics/Epic-EP-045.md)
**Design:** [`../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md) §5 (AC4), §4.4 (R3 = (c))
**Size:** ⚠️ **MEDIUM** — the first change the writer can SEE (or rather, should not see).

---

## What AC4 is

✅ A `*` the writer types or pastes is stored as `\*` (all 32 marks; R1: paste too), and the backslash is HIDDEN
by storage attributes (R3 = (c)) so the writer sees exactly what she typed. ✅ The file then holds valid CommonMark
in which her punctuation can never turn into formatting. ⛔ Existing text is never rewritten (R2).

---

## ⚠️ Measured at planning (2026-10-03, off-screen TextKit 2 harness against the real `ManuscriptEscapes.swift`)

| Path | Observed | Consequence |
| ---- | -------- | ----------- |
| **Typing** | ✅ every keystroke arrives at `insertText(_:replacementRange:)` — escaping there turned `a*b` into `a\*b` | ✅ one override covers typing (and IME commits, which use the same call) |
| **Paste** | ⛔ **does NOT go through `insertText`** — `x_y` was inserted UNESCAPED | ⚠️ paste (and probably drag-and-drop) needs its own hook — R1 requires it |
| **⌫ after a `*`** | ⛔ removed only the `*`: `a\*b` → `a\b` — an ORPHANED backslash that would become VISIBLE | ⚠️ deletion must treat `\*` as ONE unit |
| **⌦ before the `\`** | ⛔ removed only the backslash: `a\*b` → `a*b` — the `*` becomes LIVE MARKUP | ⚠️ same fix |

---

## Acceptance Criteria

- [x] **AC4a — Hiding styler.** Every hiding backslash (escape or hard line break, per `MarkdownEscapes.map`) gets
  `hiddenKey` + the (c) styling. ✅ Applied in `rebuildStorage`, ✅ after undo/redo's `applySceneChange` (design
  trap #3: that path re-applies only font + colour), ✅ and over the edited paragraph after every edit.
- [x] **AC4b — Typing escapes** in `insertText(_:replacementRange:)`.
- [x] **AC4c — Paste escapes** (R1), through its own hook; ✅ drag-and-drop text checked too.
- [x] **AC4d — An escape pair is deleted as ONE unit** by ⌫, ⌦, word/line deletes and cut — ⛔ never leaving an
  orphaned `\` or a bare live mark.
- [x] **AC4e — Copy/cut put what the writer SEES on the pasteboard** (un-escaped), so an in-app copy→paste is
  not double-escaped (`\\*`) and other apps get her text, not backslashes. ✅ Includes the cross-boundary
  structured copy (`plainText`) and the copy-buffer feed (`bufferService.load`).
- [x] **AC4f — Undo** of an escaped keystroke is one step and leaves no stray backslash (EP-019).
- [x] **AC4-round-trip** — design AC4's test: type → store → parse (`.full`, AC8) → presented equals what was
  typed, over the 32 marks.
- [x] **AC-build** — macOS + iOS + visionOS; interop green via `scripts/run-interop-tests.sh`.
- [x] **AC-live** — the user types and pastes punctuation and sees exactly what she typed.

⛔ **NOT in this Sprint:** AC5–AC7, AC10. ⛔ Find/Replace matching across hidden backslashes (searching for `*`
will not match a stored `\*`) — recorded as a follow-up, not silently changed.

---

## ✅ Q-Linux — RULED 2026-10-03: OPTION 1 (user: *"option 1. Don't worry about Linux yet. We'll get to that later."*)

✅ Ship on Apple; ⚠️ Linux shows the backslashes until [EP-048]. Recorded so it is not mistaken for a defect.

⚠️ **The Linux app opens the SAME scene files and has none of this** ([EP-048] is unscoped). Once AC4 ships, a `*`
typed on the Mac is stored as `\*`, and ⛔ **Linux will DISPLAY the backslash**. ⚠️ The reverse too: a `*` typed on
Linux is stored bare and will render as formatting on the Mac once EP-046 lands.
⚠️ The real project (`the-stairs-of-tintagael`) is edited on BOTH. **Options:** (1) ship on Apple now and accept
visible backslashes on Linux until [EP-048]; (2) hold AC4 until Linux can at least hide escapes; (3) ship, and
add a minimal Linux display-only hide as a small `[Linux]` follow-up.

---

## Progress log

### 🔵 2026-10-03 — Sprint CREATED in Planning

✅ Drafted at the user's request: *"Close SP-154. LEt's proceed to AC4."* ✅ Four input/delete paths measured
first (table above).

### 🟡 2026-10-03 — Q-Linux ruled (option 1); Sprint ACTIVATED

✅ User: *"option 1. Don't worry about Linux yet."* → ship on Apple; Linux shows backslashes until [EP-048].

### 🟢 2026-10-03 — AC4 implemented ([T-0579]); ⚠️ AC4f (undo) + AC-live need the user

✅ **Measured first** (harness): copy/cut/drag → `writeSelection(to:type:)`; paste/drop → `readSelection(from:type:)`;
✅ the text-storage delegate slot is FREE under TextKit 2 and `didProcessEditing` fires for typed AND programmatic
edits. ✅ **Built:** `EscapeHidingStyler` (storage delegate — ONE hook for typing, paste, undo apply, cross-scene
delete, rebuild); `insertText` escapes; `readSelection` escapes foreign text and restores Scrivi's OWN copy verbatim
(⚠️ so R2's intended `##` is not turned literal by an in-app copy→paste); `writeSelection` puts the PRESENTED text on
the pasteboard; `shouldChangeText` widens a split pair to the whole pair; copy-buffer pastes verbatim; the
cross-scene copy and the buffer palette preview show presented text. ✅ **Tests drive the REAL
`ManuscriptNSTextView` + styler** (private pasteboards only): 152/152. ✅ macOS / iOS / visionOS BUILD SUCCEEDED.

### ⛔→✅ 2026-10-03 — live check found a defect: copy and paste bypassed the escape layer — FIXED ([T-0579])

⛔ The user: copy put stored backslashes on the clipboard and a paste from a text editor was stored unescaped.
✅ Cause measured: AppKit passes `NSStringPboardType`, not `.string`, so both overrides fell through. ⛔ The tests
had named `.string` themselves. ✅ Fixed + tests moved onto AppKit's own dispatch; red-then-green (4 → 0); 153/153.
⚠️ AC-live re-check owed.

### ✅ 2026-10-03 — live check PASSES; [T-0579] VERIFIED; Sprint CLOSED (user-approved)

✅ *"live check passes, close SP-155"* — covers AC4f (undo) and AC-live. ✅ [T-0579] archived →
`../../Tasks/Verified/Task-verified-0579.md`. ✅ **EP-045 AC4 MET.** ⚠️ Carried as known, not as defects: Linux shows
the backslashes until [EP-048] (ruled); Find/Replace does not match across a hidden backslash (follow-up).

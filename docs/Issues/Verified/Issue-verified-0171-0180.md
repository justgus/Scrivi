# Verified Issues — I-0171 … I-0180

⚠️ **New decade file.** The previous decade closed at **I-0170**
([`Issue-verified-0161-0170.md`](Issue-verified-0161-0170.md)).

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0172** | `[Apple]` ⚠️ **`ForkPopover` forced a nested AppKit layout pass to measure its content** | Low | ⚠️ **None** — unassigned, EP-019-adjacent | ✅ **Verified 2026-08-25** |

---

## I-0172 — `ForkPopover` forced a nested AppKit layout pass

**Filed:** 2026-08-25 (SP-122, from **T-0469's console log**) · **Verified:** 2026-08-25 (user-approved)

### The defect

`ForkPopover.show(fork:in:onSelect:onCancel:)` sized its popover by calling
`host.view.layoutSubtreeIfNeeded()` and then reading `fittingSize`. ⚠️ **`layoutSubtreeIfNeeded()` forces
a layout pass**, and when `show` is invoked from inside an existing pass AppKit logs:

> *"It's not legal to call -layoutSubtreeIfNeeded on a view which is already being laid out. If you are
> implementing the view's -layout method, you can call -[super layout] instead."*

⚠️ **Logged ONCE per session, with no user-visible symptom.** ✅ **Filed anyway because a forced nested
layout is the class of defect that becomes a real layout bug later** — not because anything was broken.

### The fix — `ForkPopover.swift:70-88`

Measure with **`NSHostingController.sizeThatFits(in:)`**, which asks SwiftUI to size the content directly
and provokes no nested layout pass on the host view.

⚠️ **Measured UNBOUNDED (`.greatestFiniteMagnitude`), deliberately.** A first attempt passed a bounded
`420×320` proposal — ⚠️ **that would have been a silent behaviour change**: content wider than 420
previously reported its full width and was then clamped, but under a bounded proposal it wraps and
reports a **taller** height instead. ✅ **The existing clamp remains the single place bounds are applied**,
so the resulting popover geometry is identical to before.

### ⚠️ How this was verified — and how it was NOT

✅ **Verified 2026-08-25 on user approval.**

⚠️ **The evidence is COMPILATION plus user approval, not exercise of the popover.** Recorded plainly:

- ⚠️ **The fork popover appears only when redoing into a branch point in the undo history**, which no test
  in SP-122 exercised.
- ⚠️ **The warning it removes was logged ONCE in an entire session**, so its absence from a future log
  would be weak evidence either way.
- ✅ **`BUILD SUCCEEDED`** under Xcode 27 (AppleClang 21), no new diagnostics.

⚠️ **The behavioural check that was never run:** hit a redo fork, confirm the popover still sizes
correctly and the warning is gone. ⚠️ **If a sizing regression ever appears in that popover, THIS is the
change to suspect first.**

### Why it carries no Epic

`ForkPopover` is the **undo/redo history fork picker**, so ⚠️ **EP-019 is where this file gets real
attention** — but this Issue is ⚠️ **NOT an EP-019 commitment** and was not scheduled into it.
⚠️ **It is explicitly NOT EP-035**, which is `[Linux]` Qt/QML and would never open this Swift file.

---

*Archived 2026-08-25 on user verification, in the same step it was marked Verified.*

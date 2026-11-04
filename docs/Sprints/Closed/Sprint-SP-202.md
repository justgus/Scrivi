---
sprint: SP-202
epic: EP-050
status: Closed
closed: 2026-11-04
platform: Linux
created: 2026-11-04
---

# SP-202 — `[Linux]` [EP-050] **S3**: AC6 — measure what Orca is given, then rule

**Status:** ✅ **CLOSED 2026-11-04** (user, 2026-11-04: *"this is a pass. We should be able to close SP-202 and SP-203."*) — — created and activated 2026-11-04 (user: *"let's implement AC6 for EP-050"*). ✅ The measurement is
done (below); ✅ Q1–Q3 ruled 2026-11-04 (as recommended): the fix is EP-050 S4 = [SP-203]; the Orca listen is [T-0660] ([EP-057]).
**Carries:** [T-0658] → [`../../Tasks/Verified/Task-verified-0658.md`](../../Tasks/Verified/Task-verified-0658.md) ✅ verified.
**Epic:** [EP-050] → [`../../Epics/Closed/Epic-EP-050.md`](../../Epics/Closed/Epic-EP-050.md) — AC6: *"MEASURE what Orca hears, then rule — a
criterion here or EP-048's"*.
**Size:** S (measurement only; any fix is a later Sprint).

---

## How it was measured (2026-11-04)

Orca reads a Qt widget over AT-SPI, and Qt's AT-SPI bridge answers every text call (`GetText`, `GetTextAtOffset`,
`CaretOffset`, `SetCaretOffset`) from the widget's `QAccessibleTextInterface`. So the probe asks THAT interface directly,
in-process, on the REAL `ManuscriptEditor` + `SceneDocument` + `ManuscriptPresenter` wired as `EditorShell` wires them
(the `find_smoke` harness). Docker `scrivi-linux` (Ubuntu 24.04, Qt 6.4), offscreen, working tree at `90a81d2`. A
throwaway probe built in a COPY of the tree; nothing was added to the repo. (scrivi-vm was off; it was not needed for this.)

Fixture: two chapters, *"Chapter One"* / *"Chapter Two"*; scene 1 =
`Mr\. Smith said **bold** and *soft* here.` · `## The claim stated plainly` · `- a list item`; scene 2 =
`The second scene, 1\. not a list.`

## What Qt gives Orca

| Probe | Result |
|---|---|
| Role / children | `EditableText`; 3 children, none a heading (scroll/viewport parts) |
| `characterCount` | **146 = the stored document's length** |
| `text(0, n)` | `Chapter One⏎⏎Mr\. Smith said **bold** and *soft* here.⏎⏎## The claim stated plainly⏎⏎- a list item⏎⏎Chapter Two⏎⏎The second scene, 1\. not a list.` |
| Line by line (`textAtOffset`, line) | `Mr\. Smith said **bold** and *soft* here.` · `## The claim stated plainly` · `- a list item` · `The second scene, 1\. not a list.` (chapter titles as plain lines) |
| Caret, Scrivi → AT-SPI | Scrivi caret 31 → reported **31: a STORED position** |
| Caret, AT-SPI → Scrivi | `setCursorPosition(30)` (inside the hidden `**`) → Scrivi's caret **28** — ✅ the same place one Left arrow from 31 lands: Scrivi's snap rules (L5) apply |

**Findings:**
1. ⛔ **[I-0277] holds on Linux:** Orca is handed the STORED manuscript — escape backslashes (`Mr\.`, `1\.`), heading `##`,
   emphasis `**` / `*`. The presenter hides them on screen only.
2. ✅ **Text and caret agree today** (both stored), so Orca's cursor does not drift from Scrivi's. ⚠️ A fix that changes the
   text without translating EVERY member (caret, lines, offsets) breaks that — the Apple AC1 lesson ([I-0282]).
3. ✅ A caret set by Orca lands where Scrivi's caret rules put it, never inside hidden markup.
4. ⚠️ **No heading objects:** headings and chapter titles are plain lines of one text object, so Orca's heading navigation
   (Apple's AC4 rotor counterpart) has nothing to move to.
5. ✅ Linux already has the map a fix would use: `ManuscriptFind::PresentedText` (presented ⇄ stored, shared with Find) —
   Apple's AC1 shape (*"one map, shared with Find"*).

⚠️ **Not measured:** what Orca SPEAKS at its default punctuation level (whether it voices `*`, `#`, `\`). That needs Orca
running in a desktop session (Q3).

## Questions — ✅ Q1–Q3 RULED 2026-11-04 (user: *"Q1-Q3 ruled as recommended"*)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Where the fix lives** (AC6: *"a criterion here or EP-048's"*). | ✅ **RULED: here, in EP-050**: it is the same defect as [I-0277], A3 ruled Linux *"its own criterion"* in this Epic, and EP-048 is the display half. A new Sprint, S4: a `QAccessibleTextInterface` for `ManuscriptEditor` translating every member through `PresentedText`, as Apple's AC1–AC3. |
| **Q2** | **What Orca gets.** | ✅ **RULED: Apple's A1, the page at rest** (every Markdown mark hidden, list prefixes kept), and Apple's A2 (heading navigation) measured against Orca in that Sprint before its mechanism is ruled. |
| **Q3** | **An Orca listen before this Sprint closes?** | ✅ **RULED: yes, short** — held as [T-0660] under [EP-057] (the rig pass never holds a Sprint open): on scrivi-vm, Orca on, the arrow keys over the fixture's lines: record what it speaks. Five minutes; it settles whether punctuation is voiced at Orca's default. Alternative: skip it — the AT-SPI text above already shows the defect. |

## Acceptance Criteria

- [x] AC1 — What Qt hands AT-SPI for the manuscript measured on the real editor: text, lines, caret both ways, headings
- [x] AC2 — Q1–Q3 ruled (2026-11-04); AC6 marked in [EP-050] per Q1 → [SP-203]
- [x] AC3 — The Orca listen → [T-0660] ([EP-057]; Rig-Live-Pass-Held.md), after [SP-203]'s fix

## Progress log

- **2026-11-04** — Created and activated; measured (above).
- **2026-11-04** — Q1–Q3 ruled as recommended; S4 = [SP-203] created; the listen carried to [T-0660]. ✅ All ACs met or carried — ready to close on approval.
- **2026-11-04** — ✅ **CLOSED** (user, 2026-11-04: *"this is a pass. We should be able to close SP-202 and SP-203."*). [T-0660] (the Orca pass) passed; Tasks verified and archived.

---
sprint: SP-203
epic: EP-050
status: Active
platform: Linux
created: 2026-11-04
---

# SP-203 — `[Linux]` [EP-050] **S4**: Orca reads the page at rest

**Status:** 🟡 **ACTIVE** — created and activated 2026-11-04 ([SP-202] Q1–Q3, user: *"Q1-Q3 ruled as recommended. Once EP-050
implementation is complete continue on…"*). ✅ Implemented and tested (below); ✅ Q1 ruled 2026-11-04 (as recommended: record the difference, build nothing); the Orca listen is
[T-0660] ([EP-057]).
**Carries:** [T-0659] → [`../Tasks/Task-unverified.md`](../Tasks/Task-unverified.md).
**Epic:** [EP-050] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md) — AC7 (Apple's AC1–AC3 on Linux).
**Builds on:** [SP-202] (the measurement: Orca was given the STORED manuscript).

---

## What was built (2026-11-04)

- **`ManuscriptAccessibility`** (`platforms/linux/src/ManuscriptAccessibility.cpp`) — a `QAccessibleTextInterface` for
  `ManuscriptEditor`, installed by a Qt accessibility factory (`main.cpp`). Every member answers in PAGE positions:
  text, count, character / word / sentence / paragraph / line at, before and after an offset, caret, selection,
  character rectangles, offset at a point, scroll to a range. Lines are Qt's VISUAL lines, mapped in and out (Apple: AppKit's).
- **The one map** — `ManuscriptFind::PresentedText` (Find's, Apple's Q3) gains the WHOLE PAGE: each scene's boundary before it
  (`lead`): chapter titles as they are, a divider as the words **"Scene break"** (Apple Q2). Find never sees a lead.
  Refreshed per query; only the scenes an edit touched are rebuilt.
- **The caret both ways (Apple AC2)** — `ManuscriptEditor::placeCaret`: a caret Orca sets is a PLACEMENT through Scrivi's caret
  rules, never read as an arrow step (proved: from before an escape's `\`, placing on the escaped mark read as → and landed one on).
- **Qt's own events** carry STORED positions; they are re-sent in page positions once the edit has settled.
- ⛔ **No editable-text interface** (Orca reads and moves; typing arrives as keys).

## Tests

| Check | Where | Result |
|---|---|---|
| `accessibility_smoke` — the page at rest (`\.`, `##`, `**`, `*` gone; titles; "Scene break"), every unit of `text()`, lines / words / paragraphs, caret both ways over every body unit, the escape step case, selection, an edit and a same-length edit, cost | Docker (Qt 6.4) + scrivi-vm (Qt 6.10.2), CI | ✅ PASS |
| `accessibility_atspi.sh` — over a REAL AT-SPI bus in its own D-Bus session and Xvfb: GetText, line at offset, Set/GetCaretOffset, `text-caret-moved`, `text-changed:insert` (a key typed into that display) | scrivi-vm, build 1.0.328 (`1be7b83`) | ✅ `atspi-ok` |
| Mutations: the divider's words · `pageIndex`'s bound · `placeCaret`'s placement · `setCursorPosition` without it · `documentRange`'s end · the edit marking · the event translation (AT-SPI) | Docker copy; scrivi-vm copy | ✅ 7 / 7 caught |
| ctest + Linux smokes | scrivi-vm, build 1.0.325 | ✅ 806/806 · 39 smokes |

**Cost (1.77 MB stored, 1,186 scenes → 1.59 M page units):** the FIRST query builds the map — **200 ms on the VM**, 500 ms
in Docker — once, when Orca first reads the manuscript; a keystroke then caret + line + word queries: **26 ms (VM)** / 7 ms
(Docker). ⚠️ The first query holds the window that once (Apple's map is lazy the same way).

## Measured: heading navigation (Q2 of [SP-202], before its mechanism is ruled)

- Orca 50.2's heading navigation (H / Shift+H, and the headings list) finds objects of role **HEADING** through the AT-SPI
  **Collection** interface (`structural_navigator.py`, `ax_utilities_collection.py`), and outside web content it is OFF until
  the writer turns it on (Orca+Z).
- ⛔ **Qt's AT-SPI bridge implements no Collection interface** — measured on scrivi-vm, Qt 6.10.2: the application and the
  editor both answer *no* (`get_collection_iface()`), for Scrivi AND for a bare `QPlainTextEdit`.
- So heading navigation cannot reach Scrivi's headings by ANY interface Scrivi builds; Orca reads a heading as a line.

## Questions — ✅ Q1 RULED 2026-11-04 (user: *"Q1 and Q3 ruled as recommended"*)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Headings on Linux** (Apple's AC4 rotor). | ✅ **RULED: record the difference, build nothing:** Orca's heading navigation needs Qt's bridge to implement Collection, which Scrivi cannot add. A writer moves scene to scene with the Navigator, and Orca reads the line the caret lands on (⚠️ Linux has no Go to Scene command). Alternative: a Scrivi command, Next / Previous Heading, that moves the caret (Orca then reads it) — new keys on both platforms. |

## Acceptance Criteria

- [x] AC1 — Orca is given the page at rest: every text member through the one map (Apple AC1)
- [x] AC2 — The caret both ways, through Scrivi's caret rules (Apple AC2)
- [x] AC3 — Cost measured and recorded (Apple AC3)
- [x] AC4 — Qt's text events in page positions, proved over AT-SPI
- [x] AC5 — Q1 ruled (2026-11-04): Orca reads a heading as a line; heading navigation is not offered on Linux (Qt has no Collection)
- [x] AC6 — The Orca listen → [T-0660] ([EP-057], `Rig-Live-Pass-Held.md`)

## Progress log

- **2026-11-04** — Created and activated; built; Docker + scrivi-vm green (above). ⚠️ Found on the way: **scrivi-vm's clock
  was 35.8 h slow** (chrony slewing, not stepping) — ninja refused to build a copy; stepped with `chronyc makestep`.

---
sprint: SP-221
epic: EP-054
status: Closed
closed: 2026-11-18
platform: Cross
created: 2026-11-17
---

# SP-221 — `[Apple]` + `[Linux]` [EP-054] AC8: the Landing's **Import…**

**Status:** ✅ **CLOSED** 2026-11-18 (user, 2026-11-18: *"Mac, iPad and Linux live passes all passed, The Import... button on the mac looks the same as the other buttons.  close SP-221"*).
**Carries:** [T-0681] → [`../Tasks/Task-backlog.md`](../../Tasks/Task-backlog.md) (moved into EP-054 the same day).
**Epic:** [EP-054] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md) — AC8, the surfaces on each platform.
**Builds on:** [SP-206] (Import Archive… on the Landing), [SP-218] / [SP-219] / [SP-220] (Import ▸ Manuscript… and Scrivener
Project…, each of which makes a new project with none open).
**Size:** S — a button becomes a menu of flows that already exist.

---

## What reading the code found (2026-11-17)

- **The Landing is ONE view on the Mac and the iPad.** `LandingView.swift:38` — `Button("Import Archive…") { env.presentImportArchive() }`,
  with no `#if`; Open Project… and New Project… beside it. So changing it for the iPad changes the Mac's too (→ Q1).
- **Linux has the same button.** `platforms/linux/qml/Landing.qml:292` — `Import Archive` → `shell.importArchive("")`. Its
  Manuscript… and Scrivener Project… flows (`ShellController::importManuscript(into, scrivener)`) are not callable from QML yet:
  `importArchive` is `Q_INVOKABLE`, `importManuscript` is not.
- **Every flow already works with no project open:** `presentImportManuscript(nil, scrivener:)` on Apple, `importManuscript(nullptr, …)`
  on Linux — each makes a new project. Only **Timeline…** needs an open project ([SP-215]), and the Landing has none (→ Q2).
- T-0681 was written before S7: its list lacks Scrivener Project….

## Goal

From the start screen a writer can bring in anything Scrivi imports — a manuscript, a Scrivener project or an archive — from one
**Import…** button, without reaching for the menu bar (the iPad is used through its larger surfaces more than its menus).

## Plan

1. **Apple** (`LandingView.swift`): Import Archive… becomes an **Import…** menu button — Manuscript…, Scrivener Project…, Archive…
   (Q2, Q3) — each calling the flow the File ▸ Import menu calls.
2. **Linux** (`Landing.qml`): the same, per Q1 — a Menu on the button; `importManuscript` reachable from QML.
3. **Checks:** the macOS and iOS builds; interop; Linux build, ctest and smokes (a smoke that the Landing's Import… lists the
   three); CI after the push.
4. **Live passes (user):** from the start screen, each of the three on the iPad, the Mac and Linux.

## Questions

| # | Question | Recommendation | Ruling |
| - | -------- | -------------- | ------ |
| **Q1** | **Where.** T-0681 says the iPad; the Landing is one view on the Mac and the iPad, and Linux has the same button. | **All three Landings.** One view on Apple, so the Mac follows for free; Linux adopts Apple's shape in the same work (the standing rule). | ✅ As recommended (2026-11-18) |
| **Q2** | **Timeline… on the Landing.** It imports into an open project, and the Landing has none. | **Left out.** A greyed item the writer can never use from there only raises the question why; File ▸ Import ▸ Timeline… stays where a project is open. | ✅ As recommended (2026-11-18) |
| **Q3** | **The form.** | **A menu on the button** (SwiftUI `Menu`, a QML `Menu`): one tap shows Manuscript…, Scrivener Project…, Archive…, in File ▸ Import's order. No second dialog to answer. | ✅ As recommended (2026-11-18) |

⛔ **NOT in this Sprint:** Into This Project on the iPad ([T-0680], waiting on [EP-055]); any change to File ▸ Import.

## Acceptance Criteria

- [x] AC1 — The Landing's Import… lists Manuscript…, Scrivener Project…, Archive… (per Q2), each starting the same flow as File ▸ Import, on the iPad and the Mac
- [x] AC2 — The same on Linux (per Q1)
- [x] AC3 — macOS and iOS builds, interop, Linux build, ctest and smokes green; CI green
- [x] AC4 — **Live passes (user)** on the iPad, the Mac and Linux: plan step 4

## Progress log

- **2026-11-17** — 🔵 **Created in Planning** (user, 2026-11-17: *"yes, move T-0681 into EP-054 and plan it"*), issued via
  `next-id.py`; [T-0681] moved into [EP-054] (AC8). Q1–Q3 to rule.
- **2026-11-18** — 🟡 **Activated** (user, 2026-11-18: *"Q1-Q3 as recommended, activate and implement SP-221"*); [T-0681] → active.
- **2026-11-18** — **Built.**
  - AC1: `LandingView.swift` — `Menu("Import…")` with Manuscript…, Scrivener Project…, Archive…; each calls File ▸ Import's flow
    (`presentImportManuscript(nil, scrivener:)`, `presentImportArchive()`).
  - AC2 (code): `Landing.qml` — the button opens a `Menu` below it; `ShellController::importNewManuscript(scrivener)`
    (`Q_INVOKABLE`, `importManuscript(nullptr, …)`).
  - AC3: macOS and iOS builds ✅; interop 350 passed, 0 failed (2 skipped). Linux on scrivi-vm (its clock caught up at
    09:02 EST): build ✅, ctest 881/881 ✅, smokes 43/43 ✅, and the app launched offscreen with no QML warning from `Landing.qml`.
    No smoke loads `Landing.qml` (it needs the QML module and the `shell` context), so the menu itself is the live pass's.
    ⏳ CI after the push.
- **2026-11-18** — ✅ **AC4: the live passes** on the Mac, iPad and Linux all passed; the Mac's Import… looks like its neighbours. CI on 27bc6e4: Apple ✅, Linux App ✅ (ScriviCore CI not triggered: no core change).
- **2026-11-18** — ✅ **CLOSED** (user, 2026-11-18: *"Mac, iPad and Linux live passes all passed, The Import... button on the mac looks the same as the other buttons.  close SP-221"*); [T-0681] → ✅ Verified → `Verified/Task-verified-0681.md`.

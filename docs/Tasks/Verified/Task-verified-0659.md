# Verified Task — T-0659

**Sprint:** [SP-203] · **Epic:** [EP-050] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-04** (the Orca pass on scrivi-vm, [T-0660]: Pass)
**Archived:** 2026-11-04, with [SP-202] and [SP-203]'s close (user, 2026-11-04: *"this is a pass. We should be able to close SP-202 and SP-203."*).

---

## T-0659 — `[Linux]` EP-050 S4: Orca reads the manuscript as the page shows it

**Epic:** [EP-050] (S4, AC7) · **Status:** ✅ **Verified 2026-11-04** (the Orca pass on scrivi-vm, [T-0660]: Pass) — ⏳ the Orca listen [T-0660] ([EP-057]) ·
**Sprint:** [SP-203] → [`../../Sprints/Closed/Sprint-SP-203.md`](../../Sprints/Closed/Sprint-SP-203.md) · **Date Requested:** 2026-11-04 ([SP-202] Q1–Q2, user: *"Q1-Q3 ruled as recommended"*)

**What:** Apple's AC1–AC3 on Linux — Orca is given the page at rest (no escapes, `##`, emphasis markers; chapter titles; a
divider as "Scene break"); every text member through the one map (`PresentedText`, Find's); a caret Orca sets goes through
Scrivi's caret rules; Qt's text events re-sent in page positions.

**Done (2026-11-04):** `platforms/linux/src/ManuscriptAccessibility.cpp` (the interface + event translation, installed in
`main.cpp`); `ManuscriptFind.cpp` (`pageText` / `pageIndex` / `documentPosition`, the leads); `ManuscriptEditor::placeCaret`.
`accessibility_smoke` (CI) and `accessibility_atspi.sh` (over a real AT-SPI bus) pass on scrivi-vm build 1.0.328 and in
Docker; mutations 7/7 caught; ctest 806/806, 39 smokes. Cost: first query 200 ms (VM, 1.77 MB), a keystroke's queries 26 ms.

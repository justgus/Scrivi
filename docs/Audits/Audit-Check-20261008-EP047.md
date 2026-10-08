# Audit Check — 2026-10-08 — before the [EP-047] close

**Scope:** EP-047 `[Apple]` Manuscript Typography & Preferences — Sprints SP-167 · SP-168 · SP-169 · SP-170; Tasks T-0596 · T-0597 ·
T-0598 · T-0589; Issues I-0278 · I-0281 · I-0282. Plus the index-wide statistics check 4 asks for.
**Method:** the seven mechanical checks of [`Audit-Guidelines.md`](Audit-Guidelines.md) § The Audit Check — greps and counts.
⚠️ **Read-only. This Check changed nothing**; its findings are ruled as part of the Epic close. Tree at `bfbd7aa` (clean).

## Results

| # | Check | Result |
| - | ----- | ------ |
| 1 | AC status agreement (`Epic-Documentation.md` ⇄ `Epic-active.md`) | ⚠️ **F-1, F-2** — wording, not state: every Sprint row and both files agree all ACs are met |
| 2 | Evidence exists (archive entries) | ✅ T-0589, T-0596, T-0597, T-0598 → `Tasks/Verified/`; I-0278 → `Verified/Issue-verified-0271-0280.md`; I-0281, I-0282 → `Verified/Issue-verified-0281-0290.md`. ⚠️ **F-3** — I-0282 is not named in EP-047's entry |
| 3 | Sprint status agreement | ✅ SP-165, SP-167–SP-170 CLOSED and SP-166 ACTIVE in `Sprint-active.md`, `Sprint-Documentation.md`, `Epic-active.md` and each record's front matter (one apparent mismatch was EP-047's own status line, "ACTIVE … with [SP-167]" — not a Sprint status) |
| 4 | Counts, re-derived | ✅ Issue batch 28 = 6 (I-0271–73, 76, 78, 79), batch 29 = 2 (I-0281, 82) — stated = derived. ⚠️ **F-4** (pre-existing, index-wide) |
| 5 | Table/entry parity | ✅ the two batch files' table rows ∪ `## I-0` sections equal their stated counts (as check 4) |
| 6 | Orphan files | ✅ every top-level file in `Epics/`, `Sprints/`, `Tasks/`, `Issues/`, `Audits/` is one the guidelines name (`Sprint-SP-166.md` = EP-048's active Sprint; `Epic-EP-044.md` = EP-044's draft record) |
| 7 | ID continuity (SP-165–SP-170, T-0594–T-0598, I-0280–I-0282) | ✅ every ID has a home (closed record, verified archive, or the active file for SP-166 / T-0595 / I-0280). ⚠️ **O-1, O-2** |

## Findings

- **F-1** `Epic-Documentation.md`, EP-047 row: *"AC1–AC9, rulings **P1–P4**"* — the rulings run **P1–P11** (P5, P5a, P6–P11 were taken
  in S2–S4 planning). **Recommend:** correct in the close.
- **F-2** `Epic-active.md`, EP-047 **AC4** (*"all **8** storage sites"*) and the TRAP note above the table (*"at all eight"*) — SP-168
  measured **NINE** (four hard-coded system fonts + five `bodyFont`). **Recommend:** correct the count in the closed record, keeping the
  original wording visible as superseded (an AC is not silently rewritten).
- **F-3** I-0282 (*a typeface or size change lost the writer's place*; found and fixed in SP-168, verified after four live checks) is
  an EP-047 Issue but is **not named** in EP-047's **Issues** line. **Recommend:** add it in the close.
- **F-4** ⚠️ **Pre-existing, not EP-047's:** `Epic-Documentation.md` § statistics states *"**Total Epic IDs issued: 48** (EP-001–EP-048)"*
  — EP-049 was issued 2026-10-04 and has its own index row (49 rows; `next-ids.json` epic = 50). It is also a RESTATED COUNT, which
  ruling **[R-15]** forbids in any layer. **Recommend:** remove the figure (point to the table and `next-ids.json`), per R-15.

## Observations (no action proposed)

- **O-1** `Issue-backlog.md` records I-0281 being FILED but not LEAVING (moved to `Issue-active.md` at SP-169's activation); T-0589's
  move out of `Task-backlog.md` got a dated line. Asymmetric, harmless.
- **O-2** **I-0280** (the fenced-code-block crash, SP-165 / EP-048) is still 🟠 Resolved - Not Verified in `Issue-active.md` — its
  one-minute live check on the Mac is owed. Outside EP-047; carried by EP-048.

✅ **Nothing large or systemic** — no grounds to recommend a full Audit.

# EP-053: `[Cross]` **Range-Based Structural Editing** — cut, copy, paste and undo as one Range, one core call, no reload

**Status:** ✅ **CLOSED 2026-11-04** (user, 2026-11-04: *"approved."*) — ✅ the Goal met; Sprints [SP-186]–[SP-191] closed; ✅ Audit Check run at the close (findings below). — activated 2026-10-17 — created and started the same day (user: *"draft EP-053, start the first sprint and publish
the design doc for my perusal"*). S1 = [SP-186], ✅ closed 2026-10-20. S2a = [SP-187], ✅ closed 2026-10-21. S3 = [SP-188], ✅ closed 2026-10-26. S4 = [SP-189] and S5 = [SP-190], ✅ closed 2026-11-04.
**Design:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_3.md`](../../Scrivi_Structured_CutCopyPaste_Design_v0_3.md) (v0.3, [SP-191]; supersedes v0.2) — ✅ ruled
2026-10-17 (R1–R7, Q1–Q5). Supersedes v0.1 (EP-029) where they differ.
**Codebase:** `[ScriviCore]` first, then `[Apple]` and `[Linux]` together — the same shape and the same threads on both (R5).
**Absorbs:** [I-0297] (a paste's undo takes 40 s over the share — its last part, the reload), [I-0311] (a chapter-opening paste
scrambles the reading order), and the structural half of [SP-185].
**Relates to:** [EP-052] (Background Writes): once EP-052's per-project write queue exists, EP-053's structural calls run on
it; until then each platform runs them off its main thread itself (S2).

**Goal:** ✅ A cut, copy, paste or undo across scenes and chapters touches only the files in its range — a two-scene paste is
**3 reads and 5 writes** (✅ re-ruled 2026-10-20 from R7's 4 + 4: the chapter sidecar's scene list is kept current) — is one
ScriviCore call, never reloads the manuscript, and behaves and performs the same on the Mac and on Linux — ✅ over a network
share on Linux, on a local disk on the Mac (re-ruled 2026-11-04, user: the two-hop loop *"is too restrictive to be useful"*).

**Why (measured 2026-10-17, 1,186 scenes):** a copy read every scene sidecar (2,678 file calls); a paste re-read its chapter
three times (184); both apps re-opened the whole project after every paste (~1,700 + every scene text — the Mac on its main
thread, one core call per scene); a cut no longer removed structure, so paste and cut were not inverses. Design §2–§3.

**Sprints:**

| Sprint | Scope | Status |
| ------ | ----- | ------ |
| **[SP-186]** | **S1** — `[ScriviCore]` the Range (`scrivi.range.v1`) and the range operations: read, paste ([I-0311] fixed), structural cut, each returning the delta; the index patched in place; a two-scene paste = 3 reads + 5 writes (re-ruled 2026-10-20) | ✅ **CLOSED 2026-10-20** (user-approved) → [`../Sprints/Closed/Sprint-SP-186.md`](../../Sprints/Closed/Sprint-SP-186.md) · [T-0614] verified |
| **[SP-187]** | **S2a** — `[Apple]` + `[Linux]` the scene list as the model of record (design §4.2a), no visible change: the range endpoints bound on both, the delta types, Linux's list holding the text, Apple's partial re-render; baseline counts | ✅ **CLOSED 2026-10-21** (user-approved) → [`../Sprints/Closed/Sprint-SP-187.md`](../../Sprints/Closed/Sprint-SP-187.md) · ✅ Q1–Q4 ruled |
| **[SP-191]** | **S2b** — `[Apple]` + `[Linux]` copy builds the Range from memory; paste and its undo (a range cut); split, chapter split, merge and chapter merge as range calls (undoable, sidecars restored, no barriers); the delta applied, no reload, off the main thread on both; old history nodes converted; [I-0314] · ➡️ takes [SP-185] AC4 | ✅ **CLOSED 2026-10-22** (user-approved) → [`../Sprints/Closed/Sprint-SP-191.md`](../../Sprints/Closed/Sprint-SP-191.md) · [T-0619] verified |
| **[SP-188]** | **S3** — `[Apple]` + `[Linux]` structural cut; Backspace / Delete over scenes = Cut minus the clipboard (Q4); cut's undo (a range paste); old `structuredCut` nodes still undo | ✅ **CLOSED 2026-10-26** (user-approved) → [`../Sprints/Closed/Sprint-SP-188.md`](../../Sprints/Closed/Sprint-SP-188.md) · AC11's Mac pass over a share → [SP-190] |
| **[SP-189]** | **S4** — `[Cross]` the background sync verification (disk wins, silently — Q3) | ✅ **CLOSED 2026-11-04** (user-approved) → [`../Sprints/Closed/Sprint-SP-189.md`](../../Sprints/Closed/Sprint-SP-189.md) |
| **[SP-190]** | **S5** — Mac and rig live passes over the share, timed by the diagnostic logs | ✅ **CLOSED 2026-11-04** (user-approved) → [`../Sprints/Closed/Sprint-SP-190.md`](../../Sprints/Closed/Sprint-SP-190.md) |

**Planning rulings (user, 2026-10-20):** ✅ each operation ships with its undo (paste + its undo in S2, cut + its undo in
S3; verification moves to S4, history no longer a Sprint of its own) · ✅ split, merge and chapter create/merge are in scope,
in S2 · ✅ (later the same day) S2 split into S2a [SP-187] and S2b [SP-191]; the list stays an ordered array (Q4) · ✅ new history op names `rangePaste` / `rangeCut` in `scrivi.history.v1`, backward-compatible (old
`structuredPaste` / `structuredCut` nodes keep undoing).

**Tasks:** ✅ [T-0614] (SP-186 — VERIFIED, archived) · ✅ [T-0615] (SP-187 — VERIFIED 2026-10-21, archived) · ✅ [T-0619] (SP-191 — VERIFIED 2026-10-22, archived) · ✅ [T-0616] (SP-188) · ✅ [T-0617] (SP-189) · ✅ [T-0618] (SP-190) — VERIFIED 2026-11-04, archived · ✅ [T-0661] ([EP-057]: the rig pass — VERIFIED 2026-11-04, archived).
**Issues:** ✅ [I-0311] · ✅ [I-0297] · ✅ [I-0314] (all VERIFIED 2026-10-22, archived, with [SP-191]).

### ✅ Audit Check at the close (2026-11-04) — ruled by the user ("approved")

| # | Finding | Ruling |
|---|---|---|
| F1 | The Goal still read "4 reads and 4 writes" (re-ruled 3 + 5 on 2026-10-20) and the Mac "over a network share" (re-ruled to a local disk on 2026-11-04) | ✅ Fixed at the close (above) |
| F2 | `Closed/Sprint-SP-187.md`'s front matter read `status: Active` (its body: closed 2026-10-21) | ✅ Fixed |
| F3 | `Issue-Documentation.md` listed batch 32 (I-0311–I-0320) as 6 Issues; the file holds 7 | ✅ Fixed |

Clean: evidence (T-0614–T-0619, T-0661 archived; I-0297, I-0311, I-0314, I-0346–I-0348 with rows and entries; I-0349 backlogged),
Sprint status (SP-186–SP-191 closed everywhere), batch parity, no orphan files, ID continuity (next EP-058, SP-204, T-0662, I-0350).


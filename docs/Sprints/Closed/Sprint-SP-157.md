---
sprint: SP-157
epic: EP-045
status: Closed
closed: 2026-10-04
activated: 2026-10-04
platform: Apple
created: 2026-10-04
---

# SP-157 — `[Apple]` [EP-045] S5: Block intents as prose (AC7) + the caret-path measurement (AC10)

**Status:** ✅ **CLOSED 2026-10-04 (user-approved):** *"You may close SP-157, archive T-0581 and T-0582."* → [`../../Tasks/Verified/Task-verified-0581-0582.md`](../../Tasks/Verified/Task-verified-0581-0582.md).
**Tasks:** [T-0581] AC7 · [T-0582] AC10 → [`../../Tasks/Verified/Task-verified-0581-0582.md`](../../Tasks/Verified/Task-verified-0581-0582.md)
**Epic:** [EP-045] → [`../Epics/Epic-EP-045.md`](../../Epics/Epic-EP-045.md) — ⚠️ **its LAST two ACs.** Closing this Sprint leaves the Epic ready for its Audit Check and close.
**Design:** [`../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md) §7 (AC7), §4.5 (the 42 tab-led cases), AC10 row
**Authority:** [`../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md`](../../Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md) §4D.4(a) (user: *"option 2"*), §10.2; [I-0206]'s re-open condition → [`../Issues/Closed/Issue-closed-0206.md`](../../Issues/Closed/Issue-closed-0206.md)
**Size:** ⚠️ **SMALL.** AC7 is mostly a decision plus tests; AC10 is a measurement, not a gate.

---

## Why one Sprint, not two

✅ Both ACs are small and independent. ✅ **AC7 adds nothing to the caret path** (see below), so AC10 measures the
FINISHED E1 whether it runs before or after AC7. ⚠️ AC10 needs the user's console run on the real app, so batching
it with AC7's live look costs the user one session instead of two.

---

## ⚠️ Measured at planning (2026-10-04)

### AC7 — there is no renderer in E1 to suppress anything in

- ⛔ **Nothing in the app reads the parser's block intents.** The only parser reference in `Scrivi/` is the
  `interpretedSyntax` constant (`ManuscriptEscapes.swift:26`). Under R3 = (c) the screen shows STORAGE in the body
  font. A tab-indented paragraph ALREADY displays as ordinary prose: no monospace switch, no code-block styling.
- ✅ **The ONE place E1 interprets Markdown for display is the escape map**, and it ALREADY treats unexposed blocks
  as prose. Measured over the AC3 corpus (2,000 typed strings, escaped):

  | Oracle | Agrees with the map |
  | ------ | ------------------- |
  | `.full` as-is | 1,958 / 2,000. ⛔ The **42** that differ are exactly the tab-led cases from design §4.5: inside a code block the parser shows `\*` literally |
  | ✅ **`.full` with leading indentation removed** (prose by definition) | ✅ **2,000 / 2,000** |

  ✅ So `\ta\*b` presents `\ta*b` (escape hidden, as prose), where a code-block reading would show `a\*b`.
- ⚠️ **Block quotes and tables:** typing can't create them (`>` and `|` are escaped when typed), so they occur only in
  existing files (R2). E1 shows their markers as stored (`> q*`), consistent with R3 = (c).
- ⚠️ **The real manuscript has none of these:** `dumas-prose-timelines` (1,185 scene files, 1,853,406 bytes) has 0
  tab-led, 0 four-space-led and 0 `>`-led lines. AC7's hazard comes from typing, pasting, or other projects.

### AC10 — the baseline exists, and the logs to repeat it are already in the code

✅ [I-0206] measured the caret path on `dumas-prose` (1,823,873 characters, one `NSTextView`, TextKit 2) on
2026-09-14: `keyDown` typing **57.8–88.6 ms** (constant); our own work (`[SCRIVI-KEY] total`, `didChangeText`)
**0.5–1.4 ms**; `setSel` **5.4 / 16.8 / 57.1 ms** at offsets 123,465 / 513,559 / 1,816,059 (linear in offset);
`center` ~92 ms flat. ✅ The same logs still exist: `[SCRIVI-EDIT] keyDown` (`ManuscriptTextView.swift:2738`),
`[SCRIVI-KEY]` (`:931`), `[SCRIVI-EDIT] didChangeText` (`:2412`), `[SCRIVI-NAV] setSel` (`:1881`).
⚠️ **What E1 added to that path since:** the escape-hiding styler on every edit (paragraph-scoped, plus one line of
look-behind since [SP-156]), caret snapping in `setSelectedRanges`, the pair-widening in `shouldChangeText`, and
the Return / ⌫-join overrides. ⛔ **None of these has its own timing log.** They run INSIDE `keyDown`, so they are
measured in total but cannot be attributed. → **AC10a**.

---

## ✅ One ruling to take before activation

| # | ⚠️ Question | ⚠️ Recommendation |
| - | ----------- | ----------------- |
| **Q-AC7** | ⛔ **What does AC7 deliver in E1, when there is no renderer?** **(a)** AC7 is MET by the escape map's prose treatment (already true, measured above), **codified** by a corpus test against the prose oracle and a stated rule in code and design. The "draw unexposed blocks as prose" half becomes an explicit acceptance criterion of [EP-046], where a renderer exists. **(b)** Also build the intent-demotion step now (map `codeBlock` / `blockQuote` / `table` → paragraph) for EP-046 to call. | ✅ **RULED (a) 2026-10-04** (user: *"I rule Q-AC7 option a is correct."*) — first adopted as recommended at activation, then ruled explicitly. (b) builds code with no caller until EP-046 ("do nothing speculative"), and its shape depends on EP-046's renderer, which is not designed yet. ⚠️ (a) moves real work to EP-046, so it must be WRITTEN INTO EP-046's ACs, not left as a note. |

---

## Acceptance Criteria

### AC7 — unexposed block intents are prose

- [x] **AC7a — The rule is stated** where it acts: a comment on `MarkdownEscapes.map` (escapes are processed
  regardless of indentation, i.e. code blocks are read as prose), plus design §7 amended with the measurement above.
- [x] **AC7b — Corpus test against the PROSE oracle:** the AC3 corpus checked against `.full` of the source with
  leading indentation removed, 2,000 / 2,000. ✅ Design AC7's own test, as written: *"a 4-leading-space paragraph
  renders as prose"* (`    a\*b` presents `    a*b`, escape hidden), plus a tab-led one.
- [x] **AC7c — Through the real view:** a tab-led and a 4-space-led paragraph TYPED into `ManuscriptNSTextView` keep
  their escapes hidden (the styler agrees with the map).
- [x] **AC7d — Per Q-AC7 (a): [EP-046] gains an AC** — *"unexposed block intents (`codeBlock`, `blockQuote`, `table`)
  are DRAWN as prose"* — written into its backlog entry.
- [x] **AC7-live — The user types a tab-indented and a 4-space-indented paragraph containing `*` and `_`** and sees
  ordinary prose with no backslashes.

### AC10 — the caret path, measured (⛔ a recorded measurement, not a gate)

- [x] **AC10a — Attribution logs for E1's own additions**, in the existing `[SCRIVI-EDIT]` pattern (logged only above
  0.5 ms): the styler's `restyle`, and the `setSelectedRanges` snap. ⚠️ Only logging; no behaviour change.
- [x] **AC10b — Harness numbers (Claude, off-screen, repeatable):** the real `dumas-prose-timelines` text, read-only,
  loaded into a TextKit 2 `NSTextView` with the real styler. ⚠️ **AMENDED at activation:** the interop test host is
  SANDBOXED (`Scrivi.entitlements`: `app-sandbox`, user-selected files only), so it cannot read the fixture. ✅ The
  harness instead compiles the REAL `ManuscriptEscapes.swift` + `EscapeHidingStyler` EXTRACTED from
  `ManuscriptTextView.swift` at build time (measures the current source, never a hand copy). ⚠️ It does not contain
  `ManuscriptNSTextView`'s overrides — those are covered by AC10c's console run. Measured: the full-document restyle (the `rebuildStorage`
  pass), a per-keystroke restyle near the END, Return and ⌫-join near the end, and the caret snap.
- [x] **AC10c — The user's console run on the real app**, repeating [I-0206]'s rows on `dumas-prose-timelines`:
  typing near the start, middle and end; `setSel` at the three [I-0206] offsets (by navigating to a scene there);
  Return; ⌫ at a paragraph start. ⚠️ **On a COPY of the fixture**, so the run's typing never lands in the shared
  test data.
- [x] **AC10d — Recorded** side by side with [I-0206]'s table in EP-045 and design (AC10 row). ✅ Per the existing
  ruling (design: *"[I-0206] stays closed, ⛔ a NEW Issue if it bites"*): a regression is filed as a NEW Issue, not a
  re-opened I-0206 and not a Sprint failure.

- [x] **AC-build — macOS + iOS + visionOS build; interop green** via `scripts/run-interop-tests.sh`.

⛔ **NOT in this Sprint:** any rendering (→ [EP-046]); making `setSel` faster ([I-0206] is closed: only a NEW Issue
re-opens the cost question); Option-Return (unruled follow-up from [SP-156]); Linux (→ [EP-048]).

---

## Progress log

### 🔵 2026-10-04 — Sprint CREATED in Planning

✅ Drafted at the user's request: *"lets create sprint(s) for AC7 and AC10. Your discretion on whether this is one
Sprint or two."* → ONE (reasons above). ✅ ID issued by `next-id.py`. ✅ Measured first: no block-intent consumer in
the app; map vs prose oracle 2,000/2,000 (`scratchpad/sp157/main.swift`, throwaway); `dumas-prose-timelines` sized
and scanned. ⚠️ Task IDs are allocated at activation.

### 🟡 2026-10-04 — Sprint ACTIVATED; Q-AC7 = (a) adopted; [T-0581] + [T-0582] allocated

✅ User: *"Please complete planning for SP-157, activate it, and then implement it."* ⚠️ Q-AC7 was not ruled by name;
(a) — the recommendation — is ADOPTED on that instruction and recorded as such. ✅ AC10b amended: the test host is
sandboxed, so the harness is a build-time extraction of the real styler (above).

### 🟢 2026-10-04 — AC7 implemented ([T-0581]); AC10a + AC10b done ([T-0582]); ⚠️ AC7-live + AC10c need the user

✅ **AC7:** no behaviour change — the rule is stated on `MarkdownEscapes.map`, design §7.1 amended, [EP-046] carries
the drawing AC. ✅ Tests: prose-oracle corpus 2,000/2,000 (exactly 42 differ from plain `.full`), design AC7's own
4-space/tab test, and typing through the real view. ✅ **They pin EXISTING behaviour, so there is no red-before; a
MUTATION (the map reading indented lines as code blocks) fails all three — 50 issues — and was reverted.**
✅ **AC10a:** `[SCRIVI-EDIT] restyle=` and `[SCRIVI-EDIT] caretSnap=` (> 0.5 ms only).
✅ **AC10b — harness on the real text** (`scratchpad/sp157/ac10/`, throwaway; 1,185 scenes, 1,855,198 chars):

| E1 piece | Cost |
| -------- | ---- |
| Styler over the whole manuscript (each rebuild) | **~4 ms** (median of 5; the new `restyle` log fired at 3.6–4.9 ms) |
| Styler per keystroke, at [I-0206]'s offsets 123,465 / 513,559 / 1,816,059 | **0.002 / 0.001 / 0.002 ms** |
| Styler per keystroke, in the paragraph holding a backslash | 0.008 ms |
| A storage edit near the end (delegate restyle + TextKit 2) | 0.018 ms median, 0.073 max |
| Caret snap, all three offsets | < 0.001 ms |

✅ **E1's own additions are noise against [I-0206]'s `57.8–88.6 ms` keystroke.** ⚠️ Limits: no on-screen layout,
dividers only (no chapter headings), `ManuscriptNSTextView`'s overrides not included — AC10c measures the real app.
✅ 165/165; macOS / iOS / visionOS BUILD SUCCEEDED.
⚠️ **AC10c plan change:** rather than a fixture COPY (a duplicate shares the original's `projectID` and its shared
world — the [I-0174] class), the run uses the fixture itself and undoes its edits with ⌘Z. Claude captures the
console with `log stream` (predicate `process == "Scrivi"`, `[SCRIVI-`).

### ✅ 2026-10-04 — Q-AC7 RULED (a); live test performed; AC10c measured and recorded

✅ User: *"I rule Q-AC7 option a is correct. I performed the live test."* + the console. ✅ Claude's own
`log stream` capture of the same run agrees (190 `keyDown` lines; 3 `restyle` lines; 0 `caretSnap` lines).
⚠️ **AC7-live:** performed (a tab-led and space-led paragraph with `*` and `-` were typed); ⚠️ **awaiting the user's
explicit pass** before [T-0581] is marked Verified.

✅ **AC10c — the real app on `dumas-prose-timelines` (1,855,917 chars):**

| Where (scene · offset) | Typing `keyDown` | Return | ⌫ | Arrow keys | `[SCRIVI-KEY]` (our work) |
| ---------------------- | ---------------- | ------ | -- | ---------- | ------------------------- |
| **Start** (scene 0 · ~0) | **20.0–42.6 ms** | 26.8 ms | 40.4 ms | **0.6–1.7 ms** | 2.4–4.3 ms |
| **Quarter** (scene 291 · ~460,000) | **37.1–66.2 ms** | 60.5 ms | 37.1–62.8 ms | 23.0 / 32.4 ms | 2.3–3.5 ms |
| **End** (scenes 1182/1184 · ~1.85 M) | **89.1–121.7 ms** | 108.6 / 118.4 ms | 109.8 ms | **89.1–117.0 ms** | 2.3–2.8 ms |

| Other row | [I-0206] (2026-09-14) | ✅ Now (2026-10-04) |
| --------- | --------------------- | ------------------- |
| `setSel` @ ~460,000–513,559 | 16.8 ms @ 513,559 | 22.9 / 22.4 ms @ 460,440 (one navigation; the other two offsets were not reached by Navigator, so not logged) |
| `center` | ~92 ms flat | ✅ **0.4–1.5 ms** |
| E1: `restyle` per edit | — | ✅ logged only twice above 0.5 ms (**0.6 / 0.7 ms**); otherwise below 0.5 |
| E1: `restyle` whole document (open) | — | ✅ **4.7 ms** (harness: ~4 ms) |
| E1: `caretSnap` | — | ✅ **never above 0.5 ms** (0 log lines in 190 keystrokes) |

✅ **FINDINGS (recorded, ⛔ not a gate — design: *"[I-0206] stays closed, a NEW Issue if it bites"*):**
1. ✅ **E1's own additions are negligible** — the styler ≤0.7 ms per edit, 4.7 ms per whole-document rebuild, the
   snap never measurable; Return and ⌫ cost the same as a typed character at the same place.
2. ⚠️ **The keystroke cost is LINEAR IN DOCUMENT OFFSET — for typing, Return, ⌫ AND arrow keys** (~20–43 ms at the
   start, ~37–66 ms a quarter in, ~89–122 ms at the end; arrows 1 ms → 90 ms). ✅ Our own work stays flat
   (`[SCRIVI-KEY]` 2.3–4.3 ms), so the scaling is AppKit's, like the `setSel` cost [I-0206] measured.
   ⚠️ **This REVISES [I-0206]'s *"typing is CONSTANT"*:** it was constant because it was measured at ONE place
   (its location was not recorded). ⚠️ It is also CONSISTENT with [I-0267]'s unexplained fast and slow runs (arrows
   0.5–5 ms vs 77–85 ms) being a difference of caret POSITION — ⛔ unproven, since those runs' positions were not
   recorded.
3. ⚠️ **Our own per-keystroke work rose:** `[SCRIVI-KEY] total` 2.3–4.3 ms vs [I-0206]'s 0.5–1.3 ms, ✅ all of it in
   `bounds` (`recomputeBoundaries` → `SceneDivider.sceneBoundaries`, two whole-document attribute scans per
   keystroke). ✅ **That shape PREDATES E1** (`d4e279e:1692`, the same two scans); E1 (AC1) changed only the key it
   reads. ⛔ **Cause of the increase NOT attributed.** ~3 ms of a 20–120 ms keystroke.

### ✅ 2026-10-04 — AC7-live PASSED; [T-0581] VERIFIED

✅ User: *"yes no backslashes appeared."* → **EP-045 AC7 MET.** ⚠️ On AC10 the user answered *"kind of"* to whether the end-of-manuscript cost bothers them — ⛔ **I first wrote
that it "bothers the user" and that the user "said it bites"; the user corrected that:** *"I didn't say it bites. I said
I was kind of concerned. You are inflating my concern."* ✅ Recorded as said.

### ✅ 2026-10-04 — Sprint CLOSED (user-approved); T-0581 + T-0582 archived; T-0583 + I-0275 backlogged

✅ User: *"Yes, lets create the Task to cache scene boundaries. Every little bit helps. You may close SP-157, archive
T-0581 and T-0582. Let's backlog the Issue for the real cost as well."* ✅ [T-0583] → Task backlog. ✅ [I-0275] →
Issue backlog, with the user's concern quoted as said. ✅ EP-045 AC1–AC10 met. ⚠️ **AC11 was ADDED afterwards** (2026-10-04, [T-0583] → [SP-158]) at the user's direction.

## Retrospective

**Completed:** ✅ AC7 ([T-0581]): the escape map's prose reading codified and tested; drawing carried to [EP-046].
✅ AC10 ([T-0582]): attribution logs, an off-screen harness on the real text, and the user's run recorded beside
[I-0206]'s table.
**Returned to Backlog:** none. **Filed:** [T-0583], [I-0275].
**What went well:** ✅ measuring first (no block-intent consumer; map vs prose oracle) shrank AC7 to a ruling plus
tests; ✅ capturing the console with `log stream` meant the user only had to type; ✅ the arrow-key rows separated
AppKit's cost from ours without a profiler.
**What to improve:** ⛔ **I overstated the user's concern** ("it bites", "bothers the user") — the user had said
*"kind of concerned"*. ✅ Quote the user; do not upgrade a hedge into a ruling.
**Carry-forward notes:** ⚠️ EP-045 is NOT yet ready to close: **AC11** ([T-0583], [SP-158]). ⚠️ [I-0275]'s first step is a
profile, not a design.


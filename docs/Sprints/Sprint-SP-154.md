---
sprint: SP-154
epic: EP-045
status: Complete
activated: 2026-10-03
platform: Apple
created: 2026-10-03
---

# SP-154 — `[Apple]` [EP-045] S2: The two coordinate spaces (AC3) + parsing mode (AC8)

**Status:** 🟢 **COMPLETE 2026-10-03 — awaiting user approval to close.** (Activated 2026-10-03.)
**Epic:** [EP-045] → [`../Epics/Epic-EP-045.md`](../Epics/Epic-EP-045.md)
**Design:** [`../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md`](../Scrivi_Manuscript_Renderer_E1_Design_v0_1.md) §4 (AC3)
**Size:** ⚠️ **MEDIUM** — a new seam through every caret path, plus one measurement for a ruling.

---

## What AC3 is, in one paragraph

✅ Today the caret's position and the stored character offset are **the same number**, and every caret path
assumes it. ⛔ Once AC4 escapes typed punctuation (`*` is stored as `\*`), they stop being the same: the writer
sees one character where storage holds two. ✅ AC3 introduces the **SOURCE** space (every stored character, the
save path's) and the **PRESENTED** space (what the writer sees and lands on), a per-fragment scanner that maps
between them (design §4.2 — ⛔ **not** a document-wide table, which is [I-0196]'s hang class on a 1.85 MB
manuscript), and routes the caret paths through it. ⚠️ **On today's text the mapping is the identity** — no
escapes exist yet — so this Sprint installs the seam without changing anything the writer sees.

---

## ⚠️ Found at planning (2026-10-03) — a gap the design does not close → ruling **R3**

⛔ **The design never says HOW the escape backslash is hidden in E1.** AC3's scanner assumes `\*` presents as `*`,
but the study measured that **rendering attributes cannot hide a character** (§4A.1), and character hiding is
scoped to **EP-046** (Model B). ⚠️ **So, as designed, after AC4 the writer would SEE `\*` every time she types
`*`.** That blocks AC4, not AC3.

✅ **TextKit 2 is in use** (`textLayoutManager != nil`, guarded by `scripts/check-textkit2.sh`), so there are
display-only routes that leave storage alone — ⛔ **none is measured in this app**:
(a) an `NSTextContentStorage` delegate presenting a substituted paragraph; (b) an `NSTextLayoutFragment`
subclass that skips drawing the backslash; (c) storage attributes that shrink/clear it (⚠️ the undo path
re-applies only `.font` + `.foregroundColor`, design trap #3).

✅ **This Sprint MEASURES, it does not ship:** AC-R3 below produces the evidence for the user's ruling.

---

## Inventory (read 2026-10-03, `ManuscriptTextView.swift`, 2,905 lines)

`setSelectedRange` ×8 · `selectedRange()` ×24 · `scrollRangeToVisible` ×14 · `byteOffset(charOffset:in:)` ×5
(`:2183`) · `charOffsetForByteOffset` (`:522`). ⚠️ **The first task is to classify every one as SOURCE or
PRESENTED** — ⛔ design §4.3: the existing UTF-8 conversion (`:2183`, scene-local, byte-oriented) and AC3's
(fragment-local, character-oriented) **must not be merged**.

---

## Acceptance Criteria

- [x] **AC3a** — ✅ **The scanner** (design §4.2): `presentedToSource` / `sourceToPresented` over ONE fragment,
  using the ruled all-32 escapable set as a single list (write set = read set).
- [x] **AC3b** — ✅ **Corpus test against Apple's parser as the oracle** (`AttributedString(markdown:)`'s
  presented text) — ⛔ not the 10 probes the study ran. ✅ Includes multi-unit UTF-16 (é, 👋), a trailing `\`,
  `\\`, a backslash before a NON-escapable character (stays visible), and every one of the 32.
- [x] **AC3c** — ✅ **Every caret/selection call site classified and routed.** Each PRESENTED site converts
  through the mapping; each SOURCE site is left alone and says so in a comment. ✅ On today's (escape-free) text
  the mapping is the identity, so ⛔ **no writer-visible change** — proven by the existing suites staying green.
- [x] **AC8** — ✅ **ONE parsing mode chosen and stated** in code and in the design (study §4C.2 mixes them).
- [x] **AC-R3** — ✅ **A MEASUREMENT for ruling R3, not a shipped feature:** in an off-screen TextKit 2
  `NSTextView` (the same harness method that found [I-0252]'s cause), test route (a) — and (b) if (a) fails —
  for: is the backslash drawn? does the caret step over it? does a selection/copy include it? does save see the
  stored bytes? ⚠️ Results recorded in the design doc for the user to rule on.
- [x] **AC-build** — macOS + iOS + visionOS build; interop tests green via `scripts/run-interop-tests.sh`.

⛔ **NOT in this Sprint:** AC4 escaping (⛔ blocked on R3), AC5–AC7, AC10.

---

## Progress log

### 🔵 2026-10-03 — Sprint CREATED in Planning

✅ Drafted at the user's request: *"yes, draft the next sprint for AC3."* ✅ **R1 ruled the same day — paste is
escaped like typing.** ⚠️ **R3 raised at planning** (above).

### 🟡 2026-10-03 — Sprint ACTIVATED; R2 ruled

✅ User: *"activate SP-154, R2 is no escape pass."* ✅ **R2: no escape pass** — and existing Markdown is
INTENDED markup (the `dumas` projects' `##` scene headings). ⚠️ **Consequence for AC3b/AC8:** the parser
must treat existing `##` lines as headings, so the parsing mode chosen in AC8 cannot be inline-only for
whole documents. ⛔ R3 still owed.

### 🟢 2026-10-03 — AC3a + AC3b done; AC-R3 MEASURED → ruling R3 needed before AC3c

✅ **AC3a:** `Scrivi/Views/ManuscriptEscapes.swift` (new; registered in all three app targets in
`project.pbxproj`) — `MarkdownEscapes.map` / `escape`, ONE 32-mark list. ⚠️ **Offsets are UTF-16**, not
`Character` as the design sketched (👋 is one Character, two units; the caret is an `NSRange`).
✅ **AC3b:** suite "Markdown escapes: source ↔ presented" — edge cases, all 32, and a **2,000-string** typed
corpus, each checked against **Apple's parser**: ✅ all agree. ⚠️ **The oracle found one real gap, now fixed:** a
backslash before a newline is a CommonMark HARD LINE BREAK and Apple hides it; the scanner showed it. Typing
never produces it, but existing files may (R2). ✅ **142/142.**
✅ **AC-R3:** measured — design §4.4. ⛔ (a) paragraph substitution hides the backslash but BREAKS the caret
(jumped 0 → 8). ✅ (c) storage attributes hide it and keep the caret in source offsets, with ONE invisible extra
stop that a single selection hook can snap past. ⚠️ **AC3c's scope depends on the ruling:** under (c) the 46
call sites need no conversion — only the snap hook.

### 🟢 2026-10-03 — R3 RULED (c); AC3c done; AC8 measured → user confirmation owed — [T-0578]

✅ **R3 = (c)** (user: *"YEs R3 should be (c)."*). ✅ **AC3c, as (c) reshapes it:** the caret and every selection
stay in SOURCE offsets, so the 46 call sites need NO conversion. ✅ The ONE new behaviour lives in the EXISTING
single entry point every caret placement already passes through — `setSelectedRanges` (T-0572's) — calling
`MarkdownEscapes.snapCaret` / `snapSelection`, keyed on `MarkdownEscapes.hiddenKey`. ⚠️ **Inert until AC4's
styler marks a backslash hidden** — no writer-visible change now.
✅ **MEASURED in an off-screen TextKit 2 view compiled against the REAL `ManuscriptEscapes.swift`:** → `0,1,2,4,5,6`
and ← `6,5,4,2,1,0` (the hidden stop skipped both ways); shift-selection never ends inside `\*`; typing after the
mark lands correctly. ⚠️ **A click hit-tests to the unreachable boundary — the first rule snapped it FORWARD (one
character right of the click); refined so only a → step continues past the mark and everything else lands before the
backslash** (re-measured: click → 2). ✅ 145/145 interop; macOS / iOS / visionOS BUILD SUCCEEDED.
⚠️ **AC8:** measured (design §4.5) — **recommend `.full`**; awaiting the user's confirmation.

### 🟢 2026-10-03 — AC8 CONFIRMED; [T-0578] VERIFIED; Sprint COMPLETE

✅ User: *"1. confirm .full for AC8. 2. passes."* ✅ **AC8:** `MarkdownEscapes.interpretedSyntax = .full`, stated with
its reasons in `ManuscriptEscapes.swift` and design §4.5. ✅ **[T-0578] verified** and archived →
`../Tasks/Verified/Task-verified-0578.md`. ✅ **All ACs met. ⚠️ Awaiting approval to CLOSE.**

---
sprint: SP-165
epic: EP-048
status: Closed
closed: 2026-10-07
activated: 2026-10-07
platform: Cross
created: 2026-10-07
---

# SP-165 — `[Cross]` [EP-048] **S1**: the Markdown analyzer in ScriviCore (md4c) + the agreement test

**Status:** ✅ **CLOSED 2026-10-07 (user-approved):** *"md4c treats emoji as punctuation, accepted.  You may verify and close SP-165."* — activated 2026-10-07. ✅ Q1–Q3 ruled; ✅ Q4 (symbol class) ruled = (a) accepted. All ACs met; [T-0594] VERIFIED and archived.
**Tasks:** ✅ [T-0594] → [`../../Tasks/Verified/Task-verified-0594.md`](../../Tasks/Verified/Task-verified-0594.md) · **Issues:** 🟠 [I-0280] (fixed here; ⏳ its live check is owed → stays in `Issue-active.md`) · 🔵 [I-0281] (found here → Issue backlog, linked to [EP-047])
**Epic:** [EP-048] `[Linux]` Manuscript Renderer Parity → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md). **EP-048's first Sprint.**
**Authority:** [`../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md`](../../Scrivi_Manuscript_Renderer_E2_Design_v0_1.md) §2.4, §9 (Q-E2-8 = L-b);
the Apple analyzer this one must agree with: `Scrivi/Views/MarkdownBlocks.swift`.
**Size:** M. ✅ **Needs no rig** — everything here is verifiable on macOS (ctest + interop) and in Docker (ctest on Linux).

⚠️ **The Linux rig is unavailable (2026-10-07).** This Sprint was chosen first because it needs none. Next comes S2, the presenter (L3–L6). Its
code and Docker smokes can be built without the rig, but its live pass cannot. At that point work switches to [EP-047] (user, 2026-10-07).

---

## ⚠️ What reading the code found (2026-10-07)

- ⛔ **Design §9's L-b says *"Linux calls C++ directly"*. It does not.** `platforms/linux/src` includes ONE core header,
  `<scrivi/scrivi.h>`. The Linux app is a C ABI client exactly like Apple. → **Q1**.
- ✅ **The ABI's offset convention already exists:** *"scene-local UTF-8 byte offsets"* (`scrivi.h:673`, the fragment endpoints).
  The analyzer follows it. Each platform converts to its own units (Apple UTF-16 `NSRange`; Qt UTF-16 `QString`).

## ✅ Spike — how md4c reports escaped characters (EP-048's named first measurement, design §13)

md4c 0.5.2, `md_parse` with text callbacks, offsets = callback pointer − input base:

| Input | md4c reports | Meaning |
| ----- | ------------ | ------- |
| `Mr\. Smith\, ok` | `Mr`@0 · `.`@3 · ` Smith`@4 · `,`@11 · ` ok`@12 | ✅ the escaped mark is text AT ITS OWN POSITION; the backslash (2, 10) is in no callback = **uncovered** |
| `2 \* 3 \* 4` | `*`@3, `*`@8, no span | ✅ literal, backslashes uncovered |
| `2 * 3 * 4` | one text run 0–9, no span | ✅ the `2 * 3 * 4` class is correct (the defect Apple's spike caught) |
| `**bold** and *it* x` | STRONG{`bold`@2} · ` and `@8 · EM{`it`@14} | ✅ markers uncovered, content covered with span context |
| `# Heading \#5` | H{`Heading `@2 · `#`@11 · `5`@12} | ✅ prefix uncovered; escaped `#` covered |
| `line one\⏎line two` | `line one`@0 · **BR** (static string, NOT in the buffer) · `line two`@10 | ✅ hard-break backslash uncovered |
| `a &amp; b` | ENTITY `&amp;`@2 len 5 | ⚠️ entities arrive RAW. Covered as a whole, so nothing to hide. Apple decodes them; the analyzer must not hide them either |
| `\\back` | `\`@1 · `back`@2 | ✅ first backslash uncovered, second covered |

✅ **So md4c's text callbacks point into the input, and covered/uncovered works exactly as on Apple** (design §9 L-a
predicted it; now measured). ⛔ Span callbacks carry NO positions, so a span's extent = the covered text between enter/leave.

## Goal

✅ **ScriviCore can tell either platform what a block of manuscript Markdown IS**: its headings, list items, emphasis markers,
bold and italic, and reveal spans, by source position. ✅ A test proves it agrees with Apple's parser, so Linux renders what
Apple renders.

## EP-048 ACs this Sprint meets

| AC | Criterion (Epic wording) |
| -- | ------------------------ |
| **L1** | A source-mapped Markdown analyzer in ScriviCore (md4c): per block → kind, markers, bold, italic, prefixes, spans; first measurement: how md4c reports escaped characters |
| **L2** | Agreement test: the core analyzer and Apple's `AttributedString` parser agree over the AC3 corpus + the S5 corpus (interop test) |

## ✅ Questions — Q1–Q4 RULED 2026-10-07

| # | Question | Ruling |
| - | -------- | ------ |
| **Q1** | How does Linux reach the analyzer (it is a C ABI client, above)? | ✅ **A new C ABI endpoint**, `scrivi_analyze_markdown`, JSON-over-string. It is a NEW endpoint, so the gap audit records it. The Swift L2 test calls the same endpoint |
| **Q2** | Where does md4c come from? | ✅ **FetchContent, md4c 0.5.2, pinned by URL_HASH** (`55d0111d…ef21`), the same way nlohmann/json and Catch2 come in. MIT. It builds on all three platforms |
| **Q4** | The symbol-as-punctuation disagreement (found by L2) | ✅ **(a) ACCEPTED** — recorded, md4c unpatched |
| **Q3** | Sprint split | ✅ **S1 = L1 + L2 (this Sprint) · S2 = presenter L3–L6 (rig) · S3 = commands L7 · S4 = Find/Replace L8** |

## Plan

1. **md4c via FetchContent** into ScriviCore, PRIVATE (it never appears in a public header; the `Json` precedent).
2. **`MarkdownAnalyzer`** (`ScriviCore/src/markdown/`): `analyze(block)` returns the same model as `MarkdownBlocks.Analysis`.
   That is headings (line, prefix, level), list items (line, prefix, ordered, number), markers (range, opens), style runs
   (bold/italic) and spans. ⚠️ **NOT escape backslashes** — on BOTH platforms those come from the rule-based escape map
   (Apple `MarkdownEscapes`, Linux `ManuscriptEscapes`), not the parser; corrected in the work. Offsets are UTF-8 bytes relative to the block. The Apple
   analyzer's rules are ported as RULES, not re-derived: AC7 demotion (code block → indented prose, inline only; table →
   nothing), nested headings not rendered, ATX-only, the empty-list-item case, and the delimiter-run marker rule (a run is a
   marker only where the style differs across it).
3. **`scrivi_analyze_markdown(const char* blockUTF8)`** → JSON envelope; `scrivi.h` + `scrivi_c_api.cpp`. Recorded in →
   `docs/Scrivi_ABI_Binding_Gap_Audit_v0_1.md` (Apple: bound for the test; Linux: bound in S2).
4. **Catch2 tests through `scrivi_*`** (`feedback_boundary_tests_not_facade`): the spike table above as cases, AC7, nested
   headings, lists, the empty item, multi-byte text (offsets in BYTES), mutation-checked.
5. **L2 agreement test** (`Scrivi/Tests/ScriviInteropTests.swift`): for every block of the AC3 corpus and the S5 corpus, call
   `scrivi_analyze_markdown` and `MarkdownBlocks.analyze`, convert the core's UTF-8 offsets to UTF-16, and compare field by
   field. Any disagreement is listed, never averaged away.
6. **Cost** (design §13: *"md4c per-block cost on 1.85 MB"* not measured): analyze every block of dumas through the ABI and
   record the per-block and whole-manuscript times. ⚠️ This is the number that decides whether S2's highlighter can call the ABI
   per `highlightBlock`.
7. **Docker**: ctest on Linux as non-root (`project_linux_container_tests_off`).
8. **Design doc §9 correction**: L-b's route is the C ABI (Q1).

⛔ **NOT in this Sprint:** the Linux presenter (S2) · anything a writer can see. ⛔ So there is **no live pass**; verification is
by the suites and the agreement report.

## ✅ Results (2026-10-07)

**Built:** md4c 0.5.2 (FetchContent, `URL_HASH` pinned, `SOURCE_SUBDIR` names nothing so only `md4c.c` compiles into
`libScriviCore.a`; `enable_language(C)`) · `ScriviCore/src/markdown/MarkdownAnalyzer.{hpp,cpp}` · `scrivi_analyze_markdown`
(`scrivi.h`, `scrivi_c_api.cpp`; per-byte styles cross as RUNS) · `ScriviEngine.analyzeMarkdown(block:)` + visionOS stub
(`check-engine-stub-parity.sh` clean) · `MarkdownAnalyzerCApiTests.cpp` (19 cases) · `MarkdownAnalyzerAgreementTests` (4 tests).

| Check | Result |
| ----- | ------ |
| ctest macOS | ✅ 665/665 |
| ctest Linux (Docker, Ubuntu 24.04, GCC, **uid 1000**) | ✅ 669/669 |
| Interop (full, `run-interop-tests.sh`) | ✅ 208/208 in 24 suites |
| Mutations (analyzer, 11) | ✅ 9 killed; 2 equivalent (M5 static-text guard is redundant with `end <= start`; M8 every md4c-styled stretch has a marker). ⚠️ The first pass let 3 live mutants through — the tests were strengthened (a bullet `*` beside bold, `*it***bold**`, a soft break) |
| Cost (Debug core, through the ABI, JSON included) | ✅ dumas-prose 1,158 scenes / 1.82 MB / 5,771 blocks: **30 ms** whole manuscript; per block median **4.5 µs**, p99 10 µs |

### L2 — agreement (`MarkdownAnalyzerAgreementTests`, plus a CLI harness compiling `MarkdownBlocks.swift` against `libScriviCore.a`)

| Corpus | Agree |
| ------ | ----- |
| S5 — 4,000 arbitrary selections wrapped in `**` / `*` over escaped dumas | ✅ **4,000 / 4,000** |
| AC3 — 2,000 typed strings, ESCAPED (1,918 blocks) | ✅ **1,918 / 1,918** |
| Structural — headings, lists, quotes, code, tables, the spike table (46) | ✅ **46 / 46** |
| AC3 — the same strings RAW (live markup from random punctuation) | ⚠️ **1,912 / 1,918** |

✅ **One rule was added to reach that:** an ESCAPE backslash takes the coverage and style of the mark it escapes — Apple's
source position for `(` in `**\(and**` starts at the backslash; md4c's does not. (S5 had been 31 / 4,000 without it.)

⚠️ **The six RAW disagreements, each reduced to a minimal block** (pinned in the test's `knownDisagreements`, which must still
disagree — a fix on either side flips it):

| Minimal block | Class | Who is right |
| ------------- | ----- | ------------ |
| `👋*{*` · `*<*👋` · `_._👋` | md4c 0.5.2 = CommonMark 0.31: Unicode **symbols** (S*) count as punctuation for flanking (`md_is_unicode_punct__`, "P and S"); Apple's parser: P* only | Spec versions differ. Only a delimiter glued to a symbol (emoji, `©`, `€`) and a letter |
| `*~*` | GFM strikethrough: a lone `~` between stars — Apple no emphasis, md4c emphasis | Punctuation-only edge |
| `**$*` | Apple attributes the leftover `*` of `**` to the inner position | ⛔ Apple position error (md4c right) |
| `" She said⏎*no* twice."` | ⛔ **[I-0281]** Apple shifts every continuation line's columns by the first line's 1–3-space indent | ⛔ **Apple mis-renders real prose** (md4c right) |

✅ **Q4 RULED 2026-10-07 (user): (a) ACCEPTED** — *"md4c treats emoji as punctuation, accepted."* md4c is NOT patched; Linux
may render emphasis where Apple does not when a delimiter is glued to a symbol and a letter. The `*~*` case is accepted
with it (punctuation-only). For the Apple position errors, Linux is CORRECT; the fix is Apple's ([I-0281] → [EP-047]).

### ⛔ Found on the way: [I-0280] — a fenced code block crashed Apple's analyzer (FIXED)

The first L2 run crashed the test host (stack overflow, `analyze` ↔ `demoteIndented` ×70+). AC7 demotes a code block by
stripping indentation and re-analysing, and stripping cannot remove a FENCE, so it recursed forever. ⚠️ Shipping on Apple since
E2-S1. ✅ Fixed on both sides (demote once; a fenced block is drawn as stored). Catch2 regression bites (SIGSEGV when reverted).

### ⚠️ Found on the way (recorded, not fixed)
- The gap audit's table is a 2026-08-24 snapshot: `scrivi.h` declares **106** endpoints; five later ones have no row (note added there).
- `build/` is reconfigured with tests OFF by every Xcode build, so a `ScriviCoreTests` run after an interop run uses a STALE
  binary. Cost me one false result; reconfigure with `-DSCRIVI_BUILD_TESTS=ON` after any Xcode build.

## Acceptance Criteria

- [x] md4c 0.5.2 fetched and pinned; absent from every public header
- [x] `MarkdownAnalyzer` + `scrivi_analyze_markdown`; gap audit records it
- [x] Catch2 cases through the ABI, mutation-checked; ctest green on macOS and in Docker (Linux, non-root)
- [x] L2: the agreement test passes over the AC3 + S5 corpora, or every disagreement is listed and ruled — ✅ passes; every
  disagreement LISTED; ✅ symbol class RULED accepted (Q4)
- [x] Per-block cost on 1.85 MB recorded
- [x] Design §9 corrected (Q1)

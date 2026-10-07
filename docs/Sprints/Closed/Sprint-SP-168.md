---
sprint: SP-168
epic: EP-047
status: Closed
closed: 2026-10-07
activated: 2026-10-07
platform: Apple
created: 2026-10-07
---

# SP-168 — `[Apple]` [EP-047] **S2**: one source for the manuscript's type — and the bundled typefaces

**Status:** ✅ **CLOSED 2026-10-07 (user-approved):** *"close SP-168"* — All ACs met; T-0597 and [I-0282] VERIFIED and archived. (user: *"yes"* (to *"Shall I activate SP-168 and start?"*)). Created 2026-10-07; plan approved.
**Tasks:** ✅ [T-0597] → [`../../Tasks/Verified/Task-verified-0597.md`](../../Tasks/Verified/Task-verified-0597.md) · **Issues:** ✅ [I-0282] (found by the live pass, fixed here; VERIFIED after four live checks) → [`../../Issues/Verified/Issue-verified-0281-0290.md`](../../Issues/Verified/Issue-verified-0281-0290.md)
**Epic:** [EP-047] `[Apple]` Manuscript Typography & Preferences → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md). Previous: [`Sprint-SP-167.md`](Sprint-SP-167.md).
**Authority:** EP-047 rulings **P2** (per project), **P5/P5a** (bundled only; the seven), **P6** (default Literata), **P7** (headings
and chapter titles in the face), **P8** (size per project), **P9** (16 pt · fixed 1.45 × line spacing · chapter titles at H1 in the
text colour). Fonts: [`../../Resources/Fonts/README.md`](../../../Resources/Fonts/README.md) (committed `621bc5c`).
**Size:** M–L. ✅ Needs no rig: the live pass is on the Mac.

---

## ⚠️ What reading the code found (2026-10-07)

- ⛔ **The manuscript's type is set in NINE places: four hard-code a SYSTEM font, five use `ManuscriptPresenter.bodyFont`.**
  `ManuscriptTextView.swift` — hard-coded: `textView.font` (`:55`, which drives typing attributes), the undo path (`:482`),
  `rebuildStorage` body (`:741`) and chapter heading (`:782`, system bold +2, secondary grey); via `bodyFont` (`:2616`, `:2635`, `:2696`, `:2846`, `:2858`). The presenter
  derives headings, bold and italic from `bodyFont` / `headingFont(level:)` (`ManuscriptPresenter.swift:24-34, :175-188`).
- ⚠️ **Bold is `NSFont.monospacedSystemFont(…weight:)`** — a SYSTEM-font call. With a bundled face, weights come from the variable
  font's `wght` axis (or Courier Prime's static Bold), and "bold inside a heading" is the face's heaviest weight (the specimen rule).
- ⚠️ **Italic is `NSFontManager.convert(toHaveTrait: .italicFontMask)`** — it finds the italic of an INSTALLED family. Bundled
  families are registered per process, so whether this resolves the bundled italic file must be MEASURED; if not, the italic is
  chosen by file, as the specimen does.
- ⚠️ **Line spacing is each font's own** today. P9 fixes it at 1.45 × size — a paragraph style in STORAGE (every body site) and in
  the presenter's heading and list paragraphs. ⛔ The presenter's list paragraphs set their own `paragraphStyle` (hanging indent), so
  ONE builder must produce every paragraph style, or a list line loses the line height (and, in S3, the indent).
- ⚠️ **Interop tests use `ManuscriptPresenter.bodyFont` directly** (7 sites in `ScriviInteropTests.swift`) — they move to the one source.
- ✅ A change of `showChapterTitles` already re-presents through `rebuildStorage` (`lastShowChapterTitles`, `:232`), outside history.

## Goal

✅ **A Scrivi manuscript is set in Literata at 16 pt on a 1.45 line by default — and the writer can choose any of the seven bundled
faces and a size, per project, and it travels.** ⛔ Nothing reaches the `.md`.

## EP-047 ACs this Sprint meets

| AC | Criterion (Epic wording, short) |
| -- | ------------------------------- |
| **AC4** | One source for body attributes at all storage sites + the presenter; undo, rebuild, Replace All, paste and Return keep it |
| **AC5** | Project Settings picks a bundled face and a size; body, headings, chapter titles, bold and italic render in it; default Literata; OFL texts ship; changing it re-presents with no history event and no `.md` change |

## Plan

1. **Bundle the fonts** in every Apple app target (the `Resources/Fonts` folder as a resource, licenses included) and **register them
   at launch** with `CTFontManagerRegisterFontURLs(…, .process, …)` — one call for macOS, iOS and visionOS.
2. **One shared list of faces**: a `Resources/Fonts/fonts.json` (family, role, files, display name) read by Apple now and Linux in
   EP-048 L10 — ⛔ never a Swift restatement of the seven (the CLAUDE.md standing rule's principle: derive, don't restate).
3. **`ManuscriptTypography`** (new): the project's face + size → body font, heading fonts (Apple's 22/18/16 ÷ 13 ratios), bold (700),
   bold-in-heading (the face's heaviest), italic, chapter-title font (H1, text colour), and THE paragraph-style builder (1.45 ×
   size; the list hanging indent; S3's first-line indent later). An unknown face in the settings falls back to Literata and keeps
   the stored value.
4. **Every site reads it**: the nine places above, the presenter, the chapter heading; `ManuscriptTextView` takes the typography as
   it takes `showChapterTitles`, and a change re-presents through the existing rebuild — no history event, no `.md` change.
5. **Settings** (`project-settings.json`, through `ProjectPreferences`): `typeface` (family name) and `textSize` (points). Project
   Settings gains **Typography**: a typeface menu (each name drawn in its own face) and a size stepper (10–32 pt).
6. **Tests** (interop): AC4's survival list (undo, rebuild, Replace All, paste, Return keep the face and line height); `.md` bytes
   unchanged by a face change; the settings round-trip; the fallback; mutation-checked.
7. **Measure**: open and keystroke cost on dumas in Literata vs today's monospace ([I-0275] is the baseline — a proportional face
   lays out differently).
8. **Live pass** (user, Mac) — steps in the chat reply.

⛔ **NOT in this Sprint:** the first-line indent (S3) · Markup Hints (S4) · Linux (EP-048 L10) · iOS manuscript surfaces (none exist).

## ✅ Results (2026-10-07)

**Built:** `Resources/Fonts/fonts.json` — THE list of faces (default Literata), read by the app, the specimen and, later, Linux;
`docs/specimens/specimen-fonts.json` removed · `Scrivi/Views/ManuscriptTypography.swift` (new, ✅ in `project.pbxproj`, 3 targets):
`BundledFonts` (manifest, `register()` at launch, all Apple platforms) and `ManuscriptTypography` (the stored value is
cross-platform; fonts, sizes, THE paragraph-style builder and body/chapter-title attributes are macOS) · the presenter holds the
ONE stored `typography`; its static `bodyFont` / `headingFont` are GONE · all nine sites read it (`ManuscriptTextView`: `:55`
font + typing attributes, the undo path, `rebuildStorage`, the chapter title, five `ManuscriptNSTextView` writes) · heading lines get
their own line height in the presenter; list lines use the builder · `ProjectPreferences.typeface` / `textSize` (written only once
CHOSEN — a save of another setting never pins today's default) · Project Settings ▸ **Typography** (face menu in each face; size
stepper 10–32) · the `Fonts` folder bundled in the macOS, iOS and visionOS apps · `scripts/check-manuscript-type-source.sh` (new).

| Check | Result |
| ----- | ------ |
| Interop (full) | ✅ 224/224 in 27 suites (+8 `ManuscriptTypographyTests`, +1 on-demand cost test) |
| ctest macOS | ✅ 672/672 (core unchanged) |
| iOS + visionOS builds | ✅ both succeed; every bundle carries 16 `.ttf`, 7 `OFL.txt`, `fonts.json` |
| Engine stub parity · type-source guard | ✅ clean · ✅ one source |
| Mutations (7) | ✅ 7/7 killed — rebuild font, chapter-title style, text view ignoring the project type, line height, italic file, default pinned on save; ⚠️ the undo-apply path has NO behavioural test (private, needs live history): a mutation there SURVIVED the suite → `check-manuscript-type-source.sh` catches it (proved) |
| Cost (on-demand, 1.8 MB, same harness) | old 13 pt mono: 31.2 ms first layout · 0.9 ms key · 0.3 ms arrow; **Literata 16: 19.9 · 0.8 · 0.2**; Courier Prime: 19.6 · 0.8 · 0.2; Inter: 23.1 · 0.8 · 0.2. ✅ The face costs nothing extra. ⚠️ The harness does NOT reproduce [I-0275]'s 89–122 ms (no dividers, no coordinator work, no on-screen window) — the live pass is the real check |

⚠️ **Found on the way (test honesty):**
- Four interop expectations were written for the monospaced 13 pt font — rewritten from the typography source. One compared bold
  "emphasized" with PLAIN "emphasized" and passed only because bold and regular were the same width in a monospaced face.
- ⛔ `ManuscriptFixture` inserted its text as a bare String into empty storage — NO font at all — so two "storage keeps one plain
  font" checks compared EMPTY sets (`isSubset` passed vacuously). The fixture now writes the app's body attributes.
- Headings sit on 1.25 × their own size (the specimen the user chose from); body on 1.45 × body (P9). A fixed body maximum would
  have clipped a larger heading, so the presenter gives heading lines their own.

## ⏳ Live pass (user, 2026-10-07) — steps 1–7 PASS; one defect → [I-0282]

✅ *"1. passes. 2. passes. 3. passes. 4. passes. … 5. passed. 6. passes. 7. passes."* ⚠️ Step 4 found [I-0282]: a face or size change
lost the writer's place (caret, reading position). ✅ Fixed in this Sprint (`rebuildKeepingReadingPosition`); interop 226/226. ⏳ Re-check.

### ✅ [I-0282] — four live checks to the real cause (recorded because the first three fixes passed every test)

1. **Fix 1** anchored on a character and settled the scroll — ✅ tests; ⛔ live: the view jumped to Chapter 25.
2. **Fix 2** scrolled to the anchor FIRST (the T-0573 pattern) instead of clamping a pixel target — ✅ tests; ⛔ live.
3. **Fix 3**, from the user's log (`caretLineY=624533` past the document's end): measure only what the viewport has DRAWN —
   TextKit 2's `ensureLayout` positions disagree with the drawn viewport on a long document — ✅ tests; ⛔ live (`caretLineY=nan`).
4. **Fix 4 — the cause:** `updateNSView` set `tv.font` BEFORE the place was read; that re-fonts the whole document and invalidates
   layout. ✅ One method (`applyTypography`) does the whole sequence, and the tests call it — with the old order restored they log
   `caretLineY=nan` and fail. ✅ *"works perfectly. Both for font changes and size changes."*

⚠️ **Lesson** (→ memory `feedback_test_through_the_real_dispatch`): test through the app's real sequence, not the routine it calls.

## Acceptance Criteria

- [x] Fonts bundled in every Apple target with their OFL texts; registered at launch; `fonts.json` the one list
- [x] AC4: one source at all nine sites + the presenter; the survival tests pass, mutation-checked (+ the static guard for the undo path)
- [x] AC5: typeface menu + size in Project Settings; default Literata 16 pt on 1.45; chapter titles at H1 in the text colour; a change
      re-presents through the rebuild (no history, no `.md` change); settings travel
- [x] Cost measured and recorded
- [x] ctest, interop green; engine stub parity
- [x] Live pass (user, Mac) — ✅ steps 1–7 pass; ✅ [I-0282] verified on re-check 4

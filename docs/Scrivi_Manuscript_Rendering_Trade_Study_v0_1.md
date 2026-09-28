# Scrivi — The Manuscript as a Rendered Surface: A Trade Study v0.1

**Status:** 🟡 **DRAFT — for ruling.** ⚠️ **NOT an approved design document.**
**Date:** 2026-09-28
**Codebase:** `[Apple]` first — `Scrivi/Views/ManuscriptTextView.swift` (2,422 lines) — ⚠️ **but the
format questions are `[Cross]` and bind Linux permanently.**
**Occasioned by:** ✅ **user request 2026-09-28**, two topics raised together:
1. ⚠️ ***"the Scene divider lines… on the Apple implementation these lines seem to have been lost."***
2. ⚠️ ***"I'd like us to start thinking about moving the ManuscriptView from a text/markdown editor, to a
   typeset wysiwyg editor… In short, I'd like ManuscriptView to become a Markdown Renderer."***

**Companion / partly supersedes:**
[`Scrivi_Apple_App_Shape_Trade_Study_v0_1.md`](Scrivi_Apple_App_Shape_Trade_Study_v0_1.md)
— ⚠️ **that study concluded font/style controls "DO NOT FIT" because `isRichText = false`.**
⛔ **§6 shows that conclusion rests on a misreading of `isRichText` and must be revised.**

⚠️ **THE USER HAS RULED THAT THIS STUDY MUST ANSWER THE [EP-032] QUESTION** (2026-09-28) — ✅ **§8.**
✅ **THE Q7 SPIKE WAS RUN 2026-09-28 AT USER REQUEST — ✅ §4A.** ⚠️ **It answered both halves and
CORRECTED THIS STUDY TWICE:** ⛔ **rendering attributes CANNOT hide a marker (so Model B is dearer than
v0.1 assumed)**, ✅ **and `AttributedString` beat a hand-written scanner on CORRECTNESS, withdrawing
§3A.3's recommendation.**

---

## 0. Sourcing rule (inherited from the App Shape study)

✅ **Every macOS API claim carries a fetched URL** (⚠️ **author's knowledge cutoff is May 2026, so an
unfetched API claim is worthless**).
✅ **Apple doc pages are JS-rendered — fetched via `developer.apple.com/tutorials/data/documentation/<path>.json`**
(`reference_apple_docs_json_route`).
⚠️ **Claims about SCRIVI are marked ✅ read-in-code with `file:line`, or ⛔ not read.**

---

## 1. ⚠️ The divider: it was not lost, and the cause is known

✅ **The user's hypothesis was *"their rendition in dark mode may have reverted to dark and are currently
invisible. I do not know."*** ⚠️ **The code says the divider is still inserted on every rebuild, so the
dark-mode hypothesis is the live one.**

### 1.1 It is still built

✅ **`ManuscriptTextView.swift:614-616`** inserts a divider attachment between **every** pair of adjacent
scenes on each `rebuildStorage`. ⛔ **Nothing removed the feature.**

### 1.2 ⚠️ The causal chain — three steps, all read

| # | Event | ✅ Evidence |
| - | ----- | ---------- |
| 1 | **[I-0112]** (dark mode, 2026-08-11) wrapped the divider stroke in `controlView?.effectiveAppearance.performAsCurrentDrawingAppearance { … }` | ✅ `Issue-verified-0111-0120.md:199-206` |
| 2 | ⚠️ **That fix was labelled PRECAUTIONARY and was never verified** — ✅ ***"the divider was never *reported* as misrendering… If verification shows no difference it is harmless"*** | ✅ same record, `:205-207`. ⚠️ **I-0112 was verified in DARK MODE ONLY and its verification was about BODY TEXT** |
| 3 | ⛔ **[T-0526] (EP-039, TextKit 2) rewrote that exact code and DROPPED the guard** | ✅ `NSTextAttachmentCell` is TextKit 1 + AppKit-only, so it had to go. ⛔ **`grep` for `performAsCurrentDrawingAppearance\|effectiveAppearance` across `Scrivi/Views/` returns NOTHING** |

### 1.3 ⛔ **MEASURED 2026-09-28 — AND THE THEORY IN §1.2 WAS WRONG**

⚠️ **A diagnostic harness reproduced `DividerTextAttachment.image(for:)` VERBATIM and measured PIXELS
under both appearances.** ⛔ **The "baked wrong appearance" theory did not survive it.**

#### ✅ The handler DOES resolve correctly

```
rasterised under DARK  → line pixel rgba(1.000, 1.000, 1.000, 0.047)   ← WHITE
rasterised under LIGHT → line pixel rgba(0.000, 0.000, 0.000, 0.047)   ← BLACK
```

✅ **The app's own comment is CORRECT on this point:** ⚠️ *"an `NSImage` drawn with a handler resolves
against the CURRENT appearance at draw time."* ⛔ **So [T-0526] dropping [I-0112]'s
`performAsCurrentDrawingAppearance` guard did NOT cause this.**

⚠️ **A FIRST VERSION OF THIS TEST SAID THE OPPOSITE, AND IT WAS UNSOUND** — ✅ **recorded because the
mistake is instructive: `NSImage(size:flipped:)` draws LAZILY, so sampling the image later re-runs the
handler under whatever appearance is current AT SAMPLE TIME.** ⛔ **Both images therefore looked
identical, which reads exactly like "baked".** ✅ **The fix is to rasterise INSIDE the creating
appearance (`tiffRepresentation`) and sample outside it.**

#### ⛔ THE REAL CAUSE: `separatorColor` IS INVISIBLE HERE — 1.34:1

⚠️ **Semantic colours are TRANSLUCENT and must be COMPOSITED over the editor background before contrast
means anything.** ✅ **Measured, composited, WCAG relative luminance:**

| Colour | Dark: composited | ⚠️ contrast vs editor bg |
| ------ | ---------------- | ----------------------- |
| ⛔ **`separatorColor` (TODAY)** | `rgba(0.204, 0.204, 0.204)` | ⛔ **`1.34 : 1`** |
| `quaternaryLabelColor` | `rgba(0.204, 0.204, 0.204)` | ⛔ **`1.34 : 1`** (identical) |
| `tertiaryLabelColor` | `rgba(0.336, 0.336, 0.336)` | ⚠️ **`2.26 : 1`** |
| `secondaryLabelColor` | `rgba(0.602, 0.602, 0.602)` | ✅ **`5.89 : 1`** |
| body `textColor` (for scale) | `rgba(1.000, 1.000, 1.000)` | `16.67 : 1` |

⚠️ **In Light the same figures are `1.25 : 1` (separator) against `21.00 : 1` (text).**
⛔ **`separatorColor` is `white @ 9.8% alpha` in Dark — a hairline, by design, for use between CONTROLS
in chrome, not as a content mark inside a writing surface.**

✅ **SO THE DIVIDER IS RENDERING EXACTLY AS WRITTEN. ⛔ IT IS MIS-SPECIFIED, NOT MIS-DRAWN.**
⚠️ **And it is equally faint in LIGHT mode** — ✅ **so this is not a Dark Mode defect at all; ⛔ Dark is
simply where the user noticed it.**

### 1.4 ⚠️ What this changes about the fix

⛔ **Restoring the [I-0112] appearance guard would change NOTHING** — ✅ **measured above.**
⚠️ **The fix is a COLOUR/WEIGHT decision, not an appearance-context one.**
✅ **`secondaryLabelColor` reaches `5.89 : 1` in Dark and `3.95 : 1` in Light** — ⚠️ **visible without
competing with prose.** ⛔ **`tertiaryLabelColor` (`2.26 : 1`) is probably still too faint for a
1 px line.**

⚠️ **THE STUDY'S OWN §1.2 CAUSAL CHAIN IS THEREFORE SUPERSEDED.** ✅ **[I-0112] and [T-0526] are still
accurate history — ⛔ but neither is the cause, and [I-0112] itself said the divider *"was never
reported as misrendering"* and its guard was *"precautionary"*.** ✅ **It was precautionary AND
unnecessary; the real defect was always the colour choice.**

### 1.5 ✅ But the user has already redefined what the divider IS

⚠️ **The user: *"In a typeset version of a manuscript, the scene divider is typically three asterixes
`* * *`… these renders are non typing areas of the manuscript and need to be in place because some
functions require that the cursor be at the beginning or end of a scene or chapter and the writer needs
to be able to tell that she is there visually."***

✅ **THAT IS A FUNCTIONAL REQUIREMENT, NOT A STYLING PREFERENCE.** ⚠️ **A hairline says *"something
changed here."* `* * *` says *"this is a scene break, and the caret is outside it."*** ⛔ **Only the
second satisfies the stated need.**

✅ **RECOMMENDATION (§9 R1): fix the VISIBILITY now as a small defect** (⚠️ **the writer is blocked in
Dark Mode today**), ⛔ **but do NOT design `* * *` as a patch** — ✅ **it is the renderer's first
increment, and three attachment designs invented separately is how they end up inconsistent.**

---

## 2. ⚠️ THE CENTRAL CONSTRAINT — storage characters ARE the file

⚠️ **This is the single most important finding in this study, and every option in §5 is judged against
it.**

✅ **THE MANUSCRIPT'S SAVE PATH IS A LITERAL SUBSTRING OF THE TEXT STORAGE:**

```swift
// ManuscriptTextView.swift:811, 818 — the LIVE TYPING path
let extracted = (tv.string as NSString).substring(with: range)
parent.loader.updateText(extracted, at: segIdx)
```

✅ **Same shape on the undo/redo path (`:376`).** ⚠️ **`range` is `sceneBoundaries[segIdx]`, recomputed
from live storage on every keystroke (`:802` comment: *"they shift with every keystroke"*).**

### 2.1 ⛔ What this forbids

⚠️ **If a character is not in the storage, it is not in the `.md` file.** ⛔ **THEREFORE:**

- ⛔ **Markdown markers CANNOT simply be hidden.** ⚠️ **Deleting `**` from storage to show bold text
  DELETES IT FROM THE MANUSCRIPT ON THE NEXT KEYSTROKE.**
- ⛔ **`AttributedString(markdown:)` CANNOT be used to BUILD THE STORAGE.** ✅ **VERIFIED against Apple:
  *"Markup characters are removed — the Markdown syntax characters themselves don't appear in the final
  string"*** ([`AttributedString`](https://developer.apple.com/documentation/foundation/attributedstring)),
  ⚠️ **and there is NO parsing option to keep them** (✅ **all five members of `MarkdownParsingOptions`
  fetched — §3A.1**).
  ⚠️ **⛔ BUT THIS IS A NARROWER PROHIBITION THAN v0.1's FIRST DRAFT CLAIMED.** ✅ **It forbids
  `AttributedString` as the DOCUMENT; ⛔ it does NOT forbid it as a PARSER over a COPY** — ⚠️ **which is
  the user's proposal and is viable. See §3A.**
- ⚠️ **Any inserted glyph (`* * *`, a bullet, an image) becomes manuscript text** unless it is carried
  as something the substring does not see.

### 2.2 ✅ What already lives OUTSIDE the boundaries, and how

⚠️ **The app already solves this problem once, and the pattern is the precedent to follow**
(`feedback_look_for_existing_pattern_first`):

✅ **Chapter headings and dividers are placed OUTSIDE `sceneBoundaries`** — ✅ `:357` states it:
*"Dividers and scriviHeading runs sit outside [range] and are untouched."* ⚠️ **`recomputeBoundaries`
(`:1555-1596`) derives each scene's range by ENUMERATING `.attachment` and `.scriviHeading` and
SKIPPING them.** ✅ **So they render, and the substring never sees them.**

### 2.3 ⛔ AND THAT PATTERN HAS A LANDMINE IN IT

```swift
storage.enumerateAttribute(.attachment, in: whole, options: []) { value, range, _ in
    if value != nil { dividers.append(range.location) }      // :1556-1558
}
```

⛔ **EVERY attachment is treated as a SCENE DIVIDER.** ⚠️ **There is no type check.**
⛔ **The moment a renderer adds an attachment for ANYTHING ELSE — an image, a `* * *` mark, an [EP-032]
object reference — `sceneBoundaries` silently splits at the wrong places**, ✅ **and `sceneBoundaries`
is what the save path slices with.**

⚠️ **THIS IS A DATA-LOSS PATH, NOT A LAYOUT BUG.** ✅ **Any renderer work must fix this FIRST** —
⚠️ **and it is cheap to fix now (a marker attribute on the divider attachment) and expensive after three
features depend on the current behaviour.**

---

## 3. ✅ The API that makes this possible — TextKit 2 rendering attributes

⚠️ **The problem in §2 looks fatal: markers must stay in storage, but must not be shown as markers.**
✅ **TextKit 2 separates those two concerns, and the app is ALREADY on TextKit 2.**

### 3.1 ✅ `NSTextLayoutManager.setRenderingAttributes(_:for:)`

✅ **FETCHED** —
[`setRenderingAttributes(_:for:)`](https://developer.apple.com/documentation/uikit/nstextlayoutmanager/setrenderingattributes(_:for:)):

> ⚠️ ***"Rendering only — `setRenderingAttributes` does NOT affect the text storage. It only impacts how
> text is displayed… The underlying `NSTextContentManager` remains unchanged."***

✅ **Availability: iOS 15.0+ / visionOS 1.0+ (and the macOS AppKit peer).** ⚠️ **Companions:
`addRenderingAttribute`, `removeRenderingAttribute`, `invalidateRenderingAttributes(for:)`,
`enumerateRenderingAttributes`.**

### 3.2 ⚠️ It is the TextKit 2 successor to temporary attributes

✅ **FETCHED** — TextKit 1's
[`setTemporaryAttributes`](https://developer.apple.com/documentation/appkit/nslayoutmanager/settemporaryattributes(_:forcharacterrange:))
is ***"used only for onscreen drawing and are not persistent in any way"*** — ⚠️ **but it is
`NSLayoutManager` ONLY, i.e. TextKit 1.** ⛔ **The app is on TextKit 2 deliberately ([SP-133]) and
touching `tv.layoutManager` DOWNGRADES it** (✅ `:1480` records exactly that trap).
✅ **So `setRenderingAttributes` is the correct door, and `setTemporaryAttributes` is a trap.**

### 3.3 ✅ `renderingAttributesValidator` — the lazy, viewport-scoped door

✅ **FETCHED** —
[`renderingAttributesValidator`](https://developer.apple.com/documentation/uikit/nstextlayoutmanager/renderingattributesvalidator):

```swift
var renderingAttributesValidator: ((NSTextLayoutManager, NSTextLayoutFragment) -> Void)?
```

> ⚠️ ***"The framework invokes this callback whenever it needs to validate rendering attributes for a
> range during layout"*** — ✅ ***"enables lazy, on-demand attribute computation PER FRAGMENT rather than
> setting all attributes upfront, which is useful for performance when dealing with large documents."***

⚠️ **THIS IS THE ANSWER TO §7's COST OBJECTION.** ✅ **The renderer never parses the whole manuscript: it
is ASKED for attributes, one layout fragment at a time, as fragments come into view.** ⛔ **A 1.85 MB
document is not parsed on load; only what is on screen is.**

### 3.4 ✅ THE CONSEQUENCE — the architecture falls out

✅ **The `.md` bytes stay in storage, byte-for-byte, and the save path (§2) is untouched.**
✅ **Bold/italic/heading appearance is applied as RENDERING attributes over the marker's range.**
✅ **Parsing is lazy and viewport-scoped via the validator.**
⚠️ **The markers remain VISIBLE as characters unless something further is done** — ✅ **which is §4.**

---

## 3A. ⚠️ THE USER'S PROPOSAL, TAKEN SERIOUSLY — "why not?"

⚠️ **The user, 2026-09-28:** *"Writers are not editing markdown. They are typing content… But why not!
If the writer types `**` then Scrivi will know that the following text should be bold until the writer
types `**` again. Same for Italics, strikethru, underline. Should we reserve `#` for the Chapter
heading, but allow `##` and `###` for smaller headings? If the writer types enter, it should be
interpreted as a desire for a new paragraph. I think you need to look more closely at how we would use
AttributedString. We would use it as a rendering mechanism, not necessarily as an editing mechanism."*

✅ **THE FRAMING IS RIGHT AND IT CHANGES THE STUDY'S SHAPE.** ⚠️ **§2 concluded *"`AttributedString`
CANNOT be used"* — ⛔ **that was too strong, and it conflated TWO uses.** ✅ **The distinction the user
draws — RENDERER vs EDITOR — is exactly the one that makes it work.**

### 3A.1 ✅ What `AttributedString` can and cannot be here

| Use | Verdict | ✅ Evidence |
| --- | ------- | ---------- |
| ⛔ **As the STORAGE** (parse `.md` → put the result in `NSTextStorage`) | ⛔ **NO — DATA LOSS** | ✅ **FETCHED: there is NO option to preserve markup.** ⚠️ `MarkdownParsingOptions` has exactly five members — `allowsExtendedAttributes`, `interpretedSyntax`, `failurePolicy`, `languageCode`, `appliesSourcePositionAttributes` — ⛔ **and none of them keeps the syntax characters.** ⚠️ **Storage IS the file (§2), so stripped markers are deleted markers** |
| ✅ **As the PARSER** (parse a COPY, read the structure, throw the string away) | ✅ **YES — and this is the user's proposal** | ✅ **`appliesSourcePositionAttributes: true` maps each run BACK to the original source** |

✅ **SO THE USER IS RIGHT: `AttributedString` IS USABLE — AS A PARSER, NOT AS THE DOCUMENT.**
⚠️ **The corrected claim for §2.1 is narrower: ⛔ do not BUILD STORAGE from it; ✅ do use it to LEARN
what the storage means.**

### 3A.2 ⚠️ The mapping problem — and its real limit

✅ **FETCHED** —
[`markdownSourcePosition`](https://developer.apple.com/documentation/foundation/attributescopes/foundationattributes/markdownsourceposition):
⚠️ **it maps a parsed run back to its place in the ORIGINAL Markdown**, ✅ **and Apple's own example is
exactly our case:**

> ✅ ***"after parsing `"This is *emphasized*."`, the text `emphasized` has a Markdown source position
> that starts at column `10`. This index is the `"e"` character, NOT the `"*"` formatting character."***

⛔ **BUT THE STRUCT IS THINNER THAN IT FIRST APPEARS.** ✅ **FETCHED
[`AttributedString.MarkdownSourcePosition`](https://developer.apple.com/documentation/foundation/attributedstring/markdownsourceposition):
its ENTIRE surface is `startLine`, `startColumn`, `endLine`, `endColumn`** (1-based; columns counted in
UTF-8) ⛔ **— NO byte-offset properties, and NO `range(in:)` method.**
⚠️ **Availability `macOS 13.0+`; ✅ Scrivi deploys to `27.0`, so it is available.**

⚠️ **CONSEQUENCE: line/column → storage offset is OUR arithmetic to write**, ✅ **and the app already
does UTF-8↔character offset conversion for exactly this reason (`byteOffset(charOffset:in:)` `:1933`).**
⛔ **It is not free, and it is a correctness risk** — ⚠️ **the same class as [I-0206]'s offset work.**

### 3A.3 ⚠️ A SIMPLER OPTION THE USER'S FRAMING OPENS UP

⚠️ **If `AttributedString` is only ever a PARSER, then it is one of two possible parsers** — ✅ **and the
alternative deserves a hearing:**

| Parser | ✅ For | ⛔ Against |
| ------ | ----- | --------- |
| **P1 — `AttributedString(markdown:)` + source positions** | ✅ **Apple's own CommonMark parser — correct by construction, free, maintained** | ⛔ **Line/column → offset arithmetic is ours.** ⚠️ **Parses a BLOCK at a time; ⛔ it is not incremental, and a per-fragment validator wants a per-fragment answer** |
| **P2 — a small hand-written scanner for the subset Scrivi renders** | ✅ **Works directly in storage offsets — NO mapping layer at all.** ✅ **Naturally incremental (scan one line/fragment).** ✅ **Renders exactly the subset the user listed and nothing else** | ⛔ **We own its correctness.** ⚠️ **Markdown edge cases (nested/unbalanced/escaped markers) are notoriously fiddly** |

⚠️ **THIS STUDY DOES NOT PICK.** ✅ **But note the user's list — bold, italic, strikethrough, underline,
`#`/`##`/`###`, paragraphs — is SMALL AND CLOSED**, ⚠️ **which is the condition under which P2 is
normally the better engineering choice.** ⛔ **P1's advantage (full CommonMark) is only an advantage if
we intend to render full CommonMark, and the user has not asked for that.**
✅ **A SPIKE SHOULD DECIDE IT** (§10 Q7).

### 3A.4 ⚠️ "Type `**` and it becomes bold" — what that actually requires

✅ **The user's mental model is: type the marker, see the effect.** ⚠️ **Under §3's architecture that is
NATURAL, not special** — ✅ **the validator re-parses the fragment on the next layout pass and the run
renders bold.** ⛔ **No "smart" input handling, no autocorrect-style substitution, no hidden state.**

⚠️ **THE OPEN QUESTION IS ONLY WHAT HAPPENS TO THE `**` ITSELF** — ✅ **§4.**

### 3A.5 ⚠️ `#` for chapter headings — ⛔ this one has a CONFLICT the user should know about

⚠️ **The user asks: *"Should we reserve `#` for the Chapter heading, but allow `##` and `###` for
smaller headings?"***

⛔ **CHAPTER TITLES ARE NOT IN THE SCENE TEXT TODAY, AND THAT IS LOAD-BEARING.** ✅ **READ:**
- ✅ **Chapter headings are INSERTED BY THE VIEW at chapter boundaries** (`:622-627`), ⚠️ **carry the
  `.scriviHeading` attribute, and sit OUTSIDE `sceneBoundaries` (`:357`)** — ⛔ **so they are NEVER
  saved into any scene file.**
- ✅ **The title is read from `loader.allScenes[…].chapterTitle`** — ⚠️ **it comes from the CHAPTER's
  metadata (the core's schema), not from prose.**
- ✅ **They are TOGGLEABLE** (`showChapterTitles`, a `ProjectPreferences` value) — ⚠️ **a writer can
  turn them off; ⛔ text in a file cannot be toggled off.**

⚠️ **SO `#` AS "THE CHAPTER HEADING" WOULD CREATE TWO OWNERS OF ONE FACT** — ✅ **the chapter's metadata
title, and a `#` line in the first scene's prose.** ⛔ **That is the duplication class this project has
filed repeatedly ([I-0215] and its siblings), and here it would be worse: ⚠️ renaming a chapter in the
navigator and renaming it in the prose would disagree, with no rule for which wins.**

✅ **THE CLEAN SPLIT, RECOMMENDED:**
- ⛔ **`#` is NOT a chapter title.** ✅ **Chapter titles stay metadata, rendered by the view.**
  ⚠️ **They can still RENDER as a heading — that is F2 typography (§5) and needs no syntax at all.**
- ✅ **`##`/`###` (and `#` if the writer types it) are ORDINARY IN-SCENE HEADINGS** — ✅ **real Markdown,
  saved in the body, rendered as headings.** ⚠️ **A writer who wants a section break inside a scene gets
  one, and nothing competes for ownership.**

⚠️ **THIS IS A RULING THE USER OWES (§10 Q8)** — ✅ **the study recommends the split above, ⛔ but the
user asked the question and it is theirs to settle.**

### 3A.6 ✅ "Enter means a new paragraph" — ⚠️ mostly free, one wrinkle

✅ **In Markdown, a paragraph break is a BLANK LINE (`\n\n`); a single `\n` is a soft break.**
⚠️ **So "Enter = new paragraph" means either:**
- ✅ **(a) RENDER a single `\n` with paragraph spacing** — ⚠️ **NOT possible via rendering attributes:
  paragraph spacing is `NSParagraphStyle`, which AFFECTS LAYOUT (§4.1).** ⛔ **It would have to be a real
  storage attribute, which the undo path (`:358-369`) currently strips.**
- ✅ **(b) INSERT `\n\n` on Enter** — ✅ **trivial, honest, and the file stays canonical Markdown.**
  ⚠️ **But it changes what the writer's Backspace does, and [EP-019]'s sentence-granular undo sees it.**

⚠️ **(b) IS THE SAFER ANSWER; ⛔ neither is free.** ✅ **Flagged as part of Q2.**

---

## 4. ⚠️ The marker-visibility question — the decision that shapes the feature

⚠️ **Three known models. ✅ This study does NOT pick one; it states the cost of each.**

### 4.1 ⛔ THE HARD CONSTRAINT: rendering attributes CANNOT change layout

⚠️ **This is what decides whether markers can be HIDDEN rather than merely styled.**

✅ **TextKit 1's `setTemporaryAttributes` is explicit** —
[FETCHED](https://developer.apple.com/documentation/appkit/nslayoutmanager/settemporaryattributes(_:forcharacterrange:)):
> ⚠️ ***"Currently the only temporary attributes recognized are those that DO NOT AFFECT LAYOUT (colors,
> underlines, and so on)."***

✅ **And `kern` — the obvious trick for collapsing a marker — is documented as layout-affecting**
([FETCHED](https://developer.apple.com/documentation/foundation/nsattributedstring/key/kern)):
⚠️ ***"kern modifies character spacing and affects glyph advance metrics… it should not be applied as a
temporary rendering attribute."***

⚠️ **THE TEXTKIT 2 PAGE IS SILENT ON THIS.** ⛔ **FETCHED
[AppKit `setRenderingAttributes`](https://developer.apple.com/documentation/appkit/nstextlayoutmanager/setrenderingattributes(_:for:))
(macOS 12.0+) documents NO restriction at all** — ⚠️ **it says only *"Sets the rendering attributes for
the range you specify."*** ✅ **So the no-layout rule is INHERITED from the temporary-attributes lineage,
⛔ NOT stated for TextKit 2.**
⚠️ **THAT IS AN UNTESTED BOUNDARY AND THIS STUDY WILL NOT GUESS AT IT** — ✅ **it is the single most
valuable thing a spike could settle (§10 Q7), because ⛔ if rendering attributes cannot hide a marker,
Model B needs a DIFFERENT mechanism (a layout-fragment override), which is materially more work.**

### 4.2 The three models

| Model | What the writer sees | ⚠️ Cost |
| ----- | -------------------- | ------ |
| **A — markers visible, styled** | `**bold**` renders bold, ✅ **and the `**` is still there**, dimmed/smaller | ✅ **CHEAPEST, AND POSSIBLE TODAY with rendering attributes alone.** ⛔ Not the typeset look the user asked for |
| **B — markers hidden unless the caret is inside** (Obsidian / Bear / Typora) | `**bold**` shows as **bold**; ✅ **the `**` reappears when the caret enters** | ⛔ **BLOCKED ON §4.1.** ⚠️ Needs zero-width markers — a layout change. ✅ If rendering attributes cannot, this needs an `NSTextLayoutFragment` subclass |
| **C — markers never shown; separate source view** | true WYSIWYG | ⛔ **Two views of one document.** ⚠️ Largest build; ⛔ makes "where is the caret?" harder, not easier |

⚠️ **THE USER'S WORDS POINT AT B.** ✅ **An INCREMENTAL PATH EXISTS: ship A, then B per element type as
§4.1 is settled.** ⚠️ **Whole-LINE markers (`#`, `##`) are much easier to hide than INLINE ones (`**`),
because a line prefix can be handled by paragraph-level layout rather than glyph suppression.**

---

## 4A. ✅ **Q7 SPIKE — RUN 2026-09-28. BOTH HALVES ANSWERED.**

⚠️ **Throwaway spike, [EP-018]/[T-0191] precedent: it exists to kill or confirm a mechanism ON EVIDENCE.**
✅ **Four programs, built with `swiftc` against macOS 27.2 / Swift 6.4, measuring REAL `NSTextLayoutManager`
layout geometry — ⛔ not screenshots, ⛔ not documentation.**

---

### 4A.1 ⛔ **Q7(a): RENDERING ATTRIBUTES CANNOT HIDE A MARKER. §4.1's doubt is RESOLVED — against Model B.**

✅ **Method: identical storage in every case; only the decoration differs. Measure the laid-out width of
the line fragment.** ✅ **Text `"This is **emphasized**."` vs `"This is emphasized."`**

| Case | Width | Verdict |
| ---- | ----- | ------- |
| baseline, no markers (**the target**) | `152.69 pt` | — |
| baseline, `**` markers present | `184.83 pt` | ⚠️ **the 4 marker chars cost `32.14 pt`** |
| **C1** `setRenderingAttributes(.font 0.01)` | `184.83 pt` | ⛔ **NO EFFECT ON LAYOUT** |
| **C2** `setRenderingAttributes(.kern -100)` | `184.83 pt` | ⛔ **NO EFFECT ON LAYOUT** |
| **C3** `setRenderingAttributes(.foregroundColor)` | `184.83 pt` | ✅ **expected — colour is a no-layout attribute** |
| **C4 CONTROL** — the *same* `.font(0.01)` in **STORAGE** | `152.71 pt` | ✅ **COLLAPSES to the target** |

⚠️ **A NEGATIVE RESULT IS WORTHLESS UNLESS THE MECHANISM WAS PROVEN LIVE** — ✅ **so a second program
read the attributes back:**

```
run @8  len 2: ["NSColor", "NSFont"]  fontSize=0.01
run @20 len 2: ["NSColor", "NSFont"]  fontSize=0.01
measured advance of the `**` run: 16.07 pt      ← still FULL 13pt width
```

✅ **`enumerateRenderingAttributes` RETURNS the `.font(0.01)` that was set — ⛔ so the API accepted and
stored it and was NOT silently ignoring the call.** ⛔ **Layout simply does not consult it.**

✅ **THE RULE, NOW ESTABLISHED BY MEASUREMENT RATHER THAN INFERRED FROM THE TEXTKIT 1 PAGE:**
⚠️ **rendering attributes are applied at DRAW time; ⛔ GLYPH METRICS COME FROM STORAGE.**
✅ **This is exactly what TextKit 1's temporary-attribute documentation implies, ⚠️ and AppKit's TextKit 2
page never states — ⛔ so it was worth the hour.**

#### ⚠️ What CAN collapse a marker (addendum, measured)

| Approach | Width | Verdict |
| -------- | ----- | ------- |
| **S1** `.font(0.01)` as a **storage** attribute | `152.71 pt` | ✅ **works** |
| **S2** `.kern(-7)` as a **storage** attribute | `156.83 pt` | ⚠️ **partial — kerning is per-pair, not a width clamp** |
| **S3** replace the marker with a zero-size attachment | ⛔ **not tested — RULED OUT BY DESIGN** | ⛔ **it changes the CHARACTER COUNT, which breaks the substring save path (§2)** |

### 4A.2 ⛔ **THE CONSEQUENCE FOR MODEL B — it is not free, and it collides with an existing guard**

✅ **Model B is still REACHABLE: `.font(0.01)` in storage collapses the marker.** ⛔ **But storage
attributes are precisely what this app already goes out of its way to normalise:**

⚠️ **`ManuscriptTextView.swift:358-369` FORCES uniform attributes on the undo path,** ✅ **with the stated
reason *"so replaced text does not pick up typing attributes (e.g. bold)"*.** ⛔ **A storage-side
collapse attribute would be STRIPPED by that path**, ⚠️ **so Model B requires either re-applying after
every such write, or a `NSTextLayoutFragment` subclass that controls its own glyph advances.**

✅ **MODEL A IS UNAFFECTED AND WORKS TODAY** — ⚠️ **C3 proves colour/weight styling applies cleanly with
storage untouched, which is the whole point of the architecture.**

---

### 4A.3 ✅ **Q7(b): `AttributedString` IS THE RIGHT PARSER. The hand-written scanner LOST on correctness.**

⚠️ **§3A.3 suggested a hand-written scanner "normally wins" for a small closed subset.** ⛔ **THE SPIKE
DISPROVED THAT, and the disproof is the useful part.**

✅ **Both parsers were run over a realistic scene body containing the cases that break naive scanners.**

| Probe | ✅ `AttributedString` | ⛔ hand-written scanner |
| ----- | -------------------- | ---------------------- |
| `2 * 3 * 4` (arithmetic) | ✅ **no emphasis** | ⛔ **matched `*` across a paragraph boundary** |
| `5*6` | ✅ **no emphasis** | ⛔ same failure |
| `snake_case_word` | ✅ **no emphasis** | ✅ (underscores unhandled) |
| `\*literal\*` (escaped) | ✅ **renders `*literal*`, no emphasis** | ✅ handled |
| `` `code with **stars**` `` | ✅ **code run; ⛔ stars NOT emphasis** | ✅ handled |

⛔ **The scanner produced `italic @443 len 51 "*6.\n\nSome ~~struck~~ tex"`** — ⚠️ **a 51-character
"emphasis" span running across a blank line, from a decimal point.** ✅ **That is the exact class of
defect §3A.3 warned about, and it appeared in the FIRST 60 lines of scanner anyone would write.**

#### ✅ Source-position mapping is EXACT

| Measure | Result |
| ------- | ------ |
| emphasis runs found | **10** total intent runs |
| ⚠️ carrying a source position | **5** — ✅ **and the other 5 are `intent=64` (soft line breaks), NOT emphasis** |
| ✅ **runs mapped back to the ORIGINAL markered source exactly** | ✅ **5 / 5** |
| ⛔ mismatches | **0** |

✅ **The line/column → UTF-16 offset arithmetic (§3A.2's cost) WAS WRITTEN AND IT WORKS** — ⚠️ **~20
lines: precompute each line's UTF-8 start, add `column - 1` bytes, convert the prefix to UTF-16.**
⚠️ **`endColumn` is INCLUSIVE, so a range end needs `+1`** — ✅ **an off-by-one that a test would catch
and a reader would not.**

#### ⚠️ Speed — the scanner is faster, and it does not matter

| | 158 KB corpus | per viewport (~3 KB) |
| - | ------------- | -------------------- |
| `AttributedString(markdown:)` | `54.80 ms` | ✅ **`0.136 ms`** |
| hand-written scanner | `3.74 ms` | `0.022 ms` |
| ratio | ⚠️ **14.6×** | 6× |

⛔ **THE 14.6× IS A RED HERRING.** ✅ **§3.3's `renderingAttributesValidator` parses ONE FRAGMENT AT A
TIME, so the operative number is the viewport column: `0.136 ms`.** ⚠️ **That is ~2 ms per second at
60 fps of continuous scrolling — ⛔ irrelevant against [I-0206]'s measured `~59 ms` per keystroke.**
✅ **Correctness is the only axis that discriminates, and Apple's parser wins it outright.**

### 4A.4 ✅ **SPIKE VERDICT**

1. ⛔ **Q7(a): rendering attributes CANNOT hide markers — measured, with the mechanism proven live.**
   ✅ **Model A works today with storage untouched.** ⚠️ **Model B needs STORAGE attributes (and must
   then survive `:358-369`) or a layout-fragment subclass.** ⛔ **It is materially more work than v0.1
   assumed, and that is now a measured fact rather than a worry.**
2. ✅ **Q7(b): use `AttributedString(markdown:)` as the PARSER.** ⛔ **Do NOT hand-write a scanner** —
   ⚠️ **it lost on the first realistic input, and its speed advantage is invisible under per-fragment
   parsing.** ✅ **Source positions map back EXACTLY; the arithmetic is small and now written.**
3. ⚠️ **§3A.3's recommendation is WITHDRAWN.** ✅ **It reasoned that a small closed subset favours a
   hand-written scanner; ⛔ the spike shows the subset is not the difficulty — MARKDOWN'S EDGE CASES
   ARE, and they do not shrink with the feature set.**

⚠️ **SPIKE CODE IS THROWAWAY AND WAS NOT COMMITTED** (✅ `scratchpad/spike/q7a…q7d.swift`).
✅ **Its NUMBERS are recorded here; ⛔ its code is not meant to survive.**

---

## 5. ⚠️ "Multiple fonts" — the question that needs splitting

⚠️ **The user: *"I'd like to provide the ManuscriptView with the ability to display its text in multiple
fonts."*** ⛔ **This phrase covers three different features with three different costs, and the App Shape
study's objection applies to only ONE of them.**

| Reading | Where it persists | Verdict |
| ------- | ----------------- | ------- |
| **F1 — the writer picks the manuscript typeface** (one font for the surface) | ✅ **a preference** — `ProjectPreferences` already persists per-project display settings (`showChapterTitles`) | ✅ **NO FORMAT PROBLEM.** ⚠️ The `.md` is unaffected |
| **F2 — different ELEMENTS render in different fonts** (headings serif, body serif, code mono) | ✅ **nowhere — it is DERIVED from the Markdown structure** | ✅ **NO FORMAT PROBLEM.** ✅ This is just §3's rendering attributes with a font in them |
| **F3 — per-passage font choice by the writer** ("this paragraph in Courier") | ⛔ **NOWHERE.** ⚠️ Markdown has no syntax for it | ⛔ **THIS is what the App Shape study means by "does not fit."** ⚠️ It needs either a format extension or a sidecar, and both are real decisions |

⚠️ **F1 AND F2 ARE THE BULK OF WHAT "TYPESET" MEANS, AND NEITHER THREATENS THE FORMAT.**
⛔ **F3 IS A SEPARATE RULING** and should not be smuggled in with them.
✅ **The user has not yet said which they meant** — ⚠️ **§10 Q3 asks.**

---

## 6. ⛔ CORRECTION — `isRichText = false` is NOT the blocker the App Shape study says it is

⚠️ **`Scrivi_Apple_App_Shape_Trade_Study_v0_1.md` states that font/style toolbar controls *"DO NOT FIT"*
because `isRichText = false` and the canonical body is Markdown**, ✅ **and `CLAUDE.md` repeats it.**

✅ **FETCHED** —
[`NSTextView.isRichText`](https://developer.apple.com/documentation/appkit/nstextview/isrichtext):

> ⚠️ ***"A Boolean value that controls whether the text views sharing the receiver's layout manager allow
> THE USER to apply attributes to specific ranges of text."***

⛔ **IT GOVERNS USER-APPLIED FORMATTING ONLY.** ✅ **It does not restrict programmatic attributes on the
text storage, and it has nothing to say about rendering attributes.**

✅ **SO THE CORRECT STATEMENT IS NARROWER, AND IT IS THE ONE IN §5:**
⚠️ **what "does not fit" is F3 — CHARACTER-LEVEL STYLING THE WRITER APPLIES BY HAND, because Markdown
has nowhere to persist it.** ⛔ **NOT rendering, NOT headings, NOT bold-from-`**`, NOT multiple fonts in
senses F1/F2.**

⚠️ **THE EARLIER CONCLUSION WAS DIRECTIONALLY RIGHT AND MECHANICALLY WRONG** — ✅ **and it was wrong in a
way that would have ruled out this entire feature.** ⚠️ **`isRichText = false` should STAY** (it stops
paste-with-formatting and the font panel writing attributes nobody can persist), ✅ **but it is not an
argument against a renderer.**

---

## 7. ⚠️ What this collides with — measured, not guessed

| Constraint | ✅ Evidence | ⚠️ Implication |
| ---------- | ---------- | -------------- |
| ⛔ **Every attachment counts as a scene divider** | `:1556-1558` | ⚠️ **§2.3. Must be fixed FIRST. Data loss, not cosmetics** |
| ⚠️ **The undo path FORCES uniform attributes** | `:358-369` — *"so replaced text does not pick up typing attributes (e.g. bold)"* | ⛔ **An undo would STRIP rendering if rendering lived in storage.** ✅ **Another reason for §3's rendering attributes: they are reapplied by a renderer, not carried by storage** |
| ⚠️ **`rebuildStorage` is whole-document** | `:568-600`; ✅ [I-0196] already names it *"the prime suspect for the hang"* | ⛔ **Re-parsing Markdown for the WHOLE manuscript on every rebuild would re-earn [EP-039]'s hang.** ✅ **Parse must be viewport-scoped — see below** |
| ✅ **TextKit 2 supports viewport-only layout** | ✅ **FETCHED**: `NSTextViewportLayoutController` *"only lay out text that's visible in the viewport"* ([NSTextLayoutManager](https://developer.apple.com/documentation/uikit/nstextlayoutmanager)) | ✅ **The escape hatch: render attributes for the VISIBLE range only** |
| ⚠️ **[I-0206]: `setSelectedRange` is OFFSET-LINEAR** | `Issue-closed-0206.md` — `~59 ms`/keystroke, `57–81 ms` near the end of a 1.85 MB document; ⚠️ **CLOSED as not-a-defect with a RE-OPEN CONDITION** | ⛔ **Model B (§4) moves the caret more, not less.** ⚠️ **This feature is the most likely trigger of I-0206's re-open condition, and that should be said out loud before it is built** |
| ⚠️ **Byte-offset mapping is UTF-8 over scene-local text** | `:1933` `byteOffset(charOffset:in:)` | ⚠️ **Survives rendering attributes (storage unchanged); ⛔ would NOT survive hidden characters** |

---

## 8. ✅ THE [EP-032] QUESTION — ruled by this study, per the user

⚠️ **[EP-032] `[Cross]` Inline Object References in the Manuscript is 🔵 Draft with a FULL planning pass
retained — AC1–AC10, Q1–Q6, a code-verified claim table, and SP-107–SP-114 RESERVED.**

### 8.1 ⚠️ The overlap is real and it is at the FORMAT layer

| [EP-032] asks | ⚠️ This feature asks |
| ------------- | ------------------- |
| **Q1** *"What is the reference syntax in a Markdown body?"* | ⚠️ **What does the renderer parse, and what stays literal?** |
| **Q3** *"What does a reference look like to a non-Scrivi reader of the `.md` file?"* | ⚠️ **Identical question, different token** |
| **AC3** *"References render, resolved and live, in the Apple editor"* | ⚠️ **Identical mechanism — §3's rendering attributes / attachments** |

⛔ **TWO EPICS ANSWERING Q1/Q3 INDEPENDENTLY WILL DIVERGE**, ✅ **and divergence on an ON-DISK FORMAT
decision is the expensive kind** — ⚠️ **[EP-032]'s own AC1 requires a body containing a reference to be
*"still valid Markdown for every reader that does not understand it"*, which is exactly the property a
renderer must also preserve.**

### 8.2 ✅ RECOMMENDATION — the renderer is the GENERAL CASE; [EP-032] is a CLIENT of it

✅ **Sequence the renderer FIRST, and let [EP-032] build on it.** ⚠️ **Three reasons, each evidenced:**

1. ✅ **A reference is one more rendered token.** ⚠️ **Headings, emphasis and `* * *` are the general
   mechanism; an object reference is a special token inside it.** ⛔ **Building the special case first
   means building the general one underneath it later, at which point the special case is rewritten.**
2. ⛔ **[EP-032] CANNOT BE BUILT SAFELY ON TODAY'S CODE.** ✅ **§2.3: every attachment is a scene
   divider.** ⚠️ **[EP-032] would add reference attachments and silently corrupt `sceneBoundaries` — the
   save path.** ✅ **The renderer's first task fixes exactly that.**
3. ✅ **[EP-032] is `[Cross]` and gated on Linux parity (AC8); ⚠️ this work is Apple-first by
   necessity.** ⛔ **Coupling them makes the Apple work wait on a Linux renderer that does not exist.**

⚠️ **ONE THING [EP-032] SHOULD KEEP: its SP-107 design sprint owns Q1 (reference SYNTAX).** ✅ **That is
a DIFFERENT question from "what Markdown does the renderer understand" — ⚠️ but the two must be ruled
COMPATIBLY, and this study is where that compatibility is asserted.**

✅ **PROPOSED RULING (§10 Q1): renderer first; [EP-032] unblocked and improved by it; ⛔ its
SP-107–SP-114 reservation is NOT released and its planning is NOT discarded.**

---

## 9. ⚠️ Proposed shape — MULTIPLE Epics, as the user expected

⚠️ **The user: *"I believe we may be talking about multiple Epics as well. This is a large feature."***
✅ **Agreed. ⛔ This study does not create them; it proposes the split for ruling.**

| # | Candidate Epic | Scope | ⚠️ Why separate |
| - | -------------- | ----- | -------------- |
| **R1** | ⚠️ **The divider defect** — ⛔ **NOT an Epic; an ISSUE** | ✅ Restore divider visibility in Dark Mode | ⚠️ **The writer is blocked TODAY.** ⛔ Do not couple a live impediment to a multi-Epic feature |
| **E1** | ✅ **Foundations** `[Apple]` | ✅ **Fix §2.3 (typed attachments)** · ✅ the renderer seam · ✅ **`* * *` scene break** · ✅ heading typography (F2) | ⚠️ **Ships the user's stated functional need — "the writer can SEE she is at a boundary" — and pays the data-loss debt first** |
| **E2** | ✅ **Inline rendering** `[Apple]` | ✅ bold/italic/etc. via rendering attributes · ⚠️ **the §4 marker model, per element** | ⛔ **The largest uncertainty (§4 model B).** ✅ Separable because E1 stands without it |
| **E3** | ✅ **Typography & fonts** `[Apple]` | ✅ **F1** (writer picks the typeface) · ✅ F2 completion | ✅ **Independent of E2; ⚠️ pure preference + rendering** |
| **E4** | ⚠️ **`[Linux]` parity** | ✅ the same surface on `QPlainTextEdit`/Qt | ⚠️ **`feedback_linux_adopts_apple_shape` requires it; ⛔ Qt's text stack is NOT TextKit and the mechanism will differ** |
| — | ⚠️ **[EP-032]** | ✅ unchanged scope | ✅ **Follows E1/E2 (§8)** |

⚠️ **E4 IS NOT OPTIONAL AND IS NOT FREE.** ⛔ **`feedback_linux_adopts_apple_shape` says a shape change
on Apple must be made the same way on Linux IN THE SAME WORK.** ✅ **Naming it as its own Epic is how
that rule is honoured without blocking Apple on Qt** — ⚠️ **but it must be SCHEDULED, not assumed.**

---

## 10. ⚠️ Questions this study CANNOT answer — for user ruling

| # | Question | ⚠️ Why it is the user's |
| - | -------- | ---------------------- |
| **Q1** | ✅ **Does the renderer sequence BEFORE [EP-032]?** (§8 recommends **yes**) | ⚠️ **A priority call.** ✅ The study's job was to surface that they collide at Q1/Q3 — it does |
| **Q2** | ⚠️ **Which marker model (§4): A, B, or A-then-B per element?** | ⛔ **The single biggest cost driver.** ⚠️ B is what "WYSIWYG" usually means and where the engineering is |
| **Q3** | ⚠️ **Which "multiple fonts" (§5): F1, F2, F3 — or all three?** | ⛔ **F3 needs a format decision Markdown cannot express.** ✅ F1/F2 are nearly free |
| **Q4** | ⚠️ **Is `* * *` rendered from an EXISTING Markdown token (`***`/`---` thematic break) in the scene body, or a UI-only mark between scenes?** | ⛔ **Format decision.** ⚠️ Today the break is STRUCTURAL (scenes are separate files) and NOT in the text at all — ✅ so rendering it is free, ⛔ but authoring it as text would change the on-disk model |
| **Q5** | ⚠️ **Does [I-0206] re-open?** (§7) | ✅ **Its closure carries an explicit re-open condition, and this feature is the likeliest trigger** |
| **Q6** | ⚠️ **Fix the divider NOW (R1) or fold it into E1?** | ✅ **Study recommends NOW** — ⚠️ the user is impeded in Dark Mode today |
| ~~**Q7**~~ | ✅ **ANSWERED BY SPIKE, 2026-09-28 — §4A.** ⛔ **(a) rendering attributes CANNOT hide a marker** (measured; mechanism proven live) · ✅ **(b) use `AttributedString` as the parser** (the hand-written scanner lost on correctness) | ✅ **CLOSED.** ⚠️ **It changed two of this study's own recommendations — §3A.3 is WITHDRAWN and Model B is dearer than assumed** |
| **Q8** | ⚠️ **Is `#` a CHAPTER heading (§3A.5)?** ✅ **Study recommends NO** — chapter titles stay metadata; `#`/`##`/`###` are ordinary in-scene headings | ⛔ **`#` as chapter title creates TWO OWNERS of one fact** — ⚠️ the navigator title and the prose would disagree with no rule for which wins |
| **Q9** | ⚠️ **Does Enter insert `\n\n` (real paragraph) or render a single `\n` as spaced (§3A.6)?** | ⚠️ **Rendering it needs `NSParagraphStyle`, which affects LAYOUT and is stripped by the undo path.** ✅ Inserting `\n\n` is honest, ⛔ but changes Backspace and interacts with [EP-019] undo |

---

## 11. ✅ What was read, and what was not

✅ **READ:** `ManuscriptTextView.swift` (2,422 lines — storage build `:568-660`, save path `:788-820`,
undo path `:340-390`, `recomputeBoundaries` `:1545-1596`, `DividerTextAttachment` `:2362-2400`) ·
`Issue-verified-0111-0120.md` (I-0112) · `Issue-closed-0206.md` · `Epic-backlog.md` (EP-032, full entry)
· `Sprint-backlog.md` (SP-107–SP-114 reservation) · `Scrivi_Apple_App_Shape_Trade_Study_v0_1.md`.

✅ **FETCHED (10 Apple pages):** `NSTextView.isRichText` · `AttributedString` (Markdown init) ·
`AttributedString.MarkdownParsingOptions` · `AttributedString.MarkdownSourcePosition` ·
`…FoundationAttributes.markdownSourcePosition` · `…MarkdownSourcePositionAttribute` ·
`presentationIntent` / `inlinePresentationIntent` · `NSTextLayoutManager` (viewport layout) ·
`NSLayoutManager.setTemporaryAttributes` · `NSTextLayoutManager.setRenderingAttributes` (UIKit **and**
AppKit) · `NSTextLayoutManager.renderingAttributesValidator` · `NSAttributedString.Key.kern`.
⚠️ **Two fetches 404'd** (`foundation/markdownsourceposition`, and the `-swift.struct` spelling) —
✅ **the type was reached by its `attributedstring/markdownsourceposition` path instead.**

⚠️ **ONE CLAIM IN v0.1 WAS CORRECTED BY THIS PASS, AND IT MATTERS:** ⛔ **§2.1 first said
`AttributedString` *"CANNOT be used"*.** ✅ **That was too strong — it forbids `AttributedString` as the
DOCUMENT, not as a PARSER, and the user's "renderer not editor" framing is the distinction that makes it
work (§3A).** ⚠️ **A second, smaller correction: an early read suggested `MarkdownSourcePosition` carried
UTF-8 OFFSET properties; ⛔ it does not — it has ONLY `startLine`/`startColumn`/`endLine`/`endColumn`,
and the UTF-8 phrasing describes how COLUMNS are counted.**

⛔ **NOT READ / NOT DONE:**
- ⛔ **No screenshot of the divider in Dark Mode** — ⚠️ **§1.4's two causes are UNDISTINGUISHED.** ✅ **A
  live run should confirm before R1 is fixed** (`feedback_prove_code_is_reached`).
- ⛔ **No Linux/Qt investigation.** ⚠️ **E4 is named, NOT scoped** — ✅ Qt's rich-text stack is not
  TextKit and the mechanism will differ; ⛔ assuming parity is cheap would be the error
  `feedback_design_to_capability_not_lcd` warns about.
- ⛔ **No measurement of rendering-attribute cost on a 1.85 MB document.** ⚠️ **§7 argues viewport-scoping
  is required; ⛔ that is REASONED, not MEASURED.** ✅ **§3.3's `renderingAttributesValidator` is the
  mechanism that should make it cheap — ⛔ but that too is reasoned from docs, not run.**
- ✅ **§4.1 WAS THE BIGGEST UNKNOWN AND IT IS NOW RESOLVED BY MEASUREMENT** — ✅ **§4A, spike run
  2026-09-28.** ⛔ **Rendering attributes cannot hide a marker.** ⚠️ **The spike also CORRECTED this
  study twice: §3A.3's hand-written-scanner recommendation is WITHDRAWN, and Model B is dearer than
  v0.1 assumed.**
- ⛔ **STILL NOT MEASURED: the `renderingAttributesValidator` path END-TO-END in the real app.**
  ⚠️ **§4A.3 timed PARSING in isolation (`0.136 ms` per viewport); ⛔ it did NOT wire a validator into
  `ManuscriptTextView` and measure against a real 1.85 MB manuscript.** ✅ **E1 should, before E2
  commits** — ⚠️ **`project_read_amplification_class`: a cost measured in isolation is not a cost
  measured in place.**
- ⛔ **Model B's storage-attribute route was NOT tried against the undo path.** ⚠️ **§4A.2 reasons that
  `:358-369` would strip it; ✅ that is READ, not RUN.**
- ⛔ **No export impact assessed.** ⚠️ **[EP-032]'s Q5 already records that manuscript export has no
  existing path; ✅ the same gap applies here and is not re-litigated.**

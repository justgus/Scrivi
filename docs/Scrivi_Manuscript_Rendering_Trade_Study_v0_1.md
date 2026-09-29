# Scrivi — The Manuscript as a Rendered Surface: A Trade Study v0.1

**Status:** 🟢 **RULED — all of §10 answered by the user 2026-09-29.** ⚠️ **Still NOT an approved
design document: ✅ the DECISIONS are made, ⛔ the Epics they imply are not yet created.**
**Date:** 2026-09-28 · ✅ **rulings 2026-09-29**
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

⛔ **USER RULINGS 2026-09-29 — ✅ §10 IS NOW A RECORD, NOT A QUESTION LIST.** ⚠️ **Two of them were
VOLUNTEERED and are the largest in the study:**
1. ⛔ **ALL TYPED MARKDOWN RESERVED CHARACTERS ARE ESCAPED** — ✅ **§3A.0. ⚠️ Markup is authored ONLY by
   command; ⛔ §3A.4 is WITHDRAWN.**
2. ⛔ **THE CARET POSITION IS NOT THE FILE OFFSET** — ✅ **§3.4A. ⚠️ A SOURCE↔PRESENTED mapping is a
   REQUIRED E1 component.**
✅ **And the cost driver is settled: ⛔ Q2 = MODEL B (WYSIWYG), §4.3 — knowingly the dear one.**

✅ **THE Q10 SPIKE WAS RUN 2026-09-29 AT USER CHALLENGE — ✅ §4B.** ⚠️ ***"You need to determine if
`AttributedString(markdown:)` will also remove escapes. Let's not add work that isn't needed."***
⛔ **It answered FOUR things and CORRECTED THIS STUDY TWICE:** ⛔ **escapes ARE stripped and are NOT
located for you (⚠️ but a ~12-line scanner reproduces the parser exactly)** · ⛔ **markdown.org's 16 is
NOT Apple's set — ⚠️ Apple escapes all 32 ASCII punctuation marks, so WRITE and READ are DIFFERENT
LISTS** · ✅ **the single-`\n` paragraph merge is FREE (§3A.6's warning was unnecessary)** · ⛔ **and the
user's trailing-space hazard is REAL and reproduces exactly as they predicted.**
✅ **RULED: ⛔ escape ALL 32 ASCII punctuation marks (*"use the READ set"*) — ⚠️ one list, no drift.**

⛔ **A FURTHER SPIKE (§4C) FOUND THE ONE THING NOBODY WAS LOOKING FOR:** ⚠️ **interior spaces are NOT
conflated by the parser (⛔ the user's premise, measured false — ✅ eleven spaces stay eleven), ⛔ **and
≥4 LEADING spaces turn a paragraph into a `codeBlock`** — ⚠️ **a BLOCK-TYPE change a novelist can
trigger by indenting.**

✅ **⛔ AND §4D CLOSED IT THE SAME DAY, ⚠️ with the user turning a defect into a FEATURE:** ⛔ **a TAB is
WORSE than four spaces (⚠️ ONE tab trips it — measured), ✅ so indentation becomes a `paragraphIndent`
PROJECT PREFERENCE rendered via `firstLineHeadIndent`** — ⚠️ **ZERO characters in the `.md`, ✅ and the
writer never has a reason to type the hazardous sequence at all.**

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

### 1.4A ⛔ **THE COLOUR FIX SHIPPED AND DID NOT WORK — 2026-09-28**

⚠️ **USER, after the change was in: *"they are still invisible."*** ⛔ **So §1.3's diagnosis, though its
arithmetic was correct, ANSWERED THE WRONG QUESTION.**

✅ **WHAT IS STILL TRUE:** ⚠️ **`separatorColor` really did measure `1.34 : 1`, and the half-pixel
straddle really did halve it.** ⛔ **WHAT IS NOW KNOWN: fixing both changed nothing on screen**, ✅ **so
a cause UPSTREAM OF THE COLOUR is operative and was never tested.**

⚠️ **THE LEADING HYPOTHESIS — ⛔ AND IT IS ONLY THAT:** ✅ **`image(for:)` is the TextKit **1**-era
attachment hook.** ⚠️ **TextKit 2 renders attachments through `NSTextAttachmentViewProvider`, and an
attachment with neither a set `image` PROPERTY nor a custom view provider may draw NOTHING — while
`attachmentBounds` still reserves its 24 pt, producing a silent GAP between scenes rather than a line.**
⚠️ **That would also explain why the divider was never *reported* as working: it may never have drawn
since [T-0526] moved this class to TextKit 2.**

⛔ **A HARNESS BUILT TO TEST THIS RETURNED CONTRADICTORY RESULTS ACROSS RUNS** — ⚠️ **`image(for:)`
called 0 times, then 2, then 0 again with an opaque RED bar that never reached the canvas.** ✅ **A
harness that disagrees with itself is evidence about the harness, not the app**, ⛔ **so NOTHING is
concluded from it and the hypothesis stands unproven.**
✅ **IT MUST BE SETTLED IN THE RUNNING APP** (`feedback_prove_code_is_reached`).

⚠️ **THE LESSON, AND IT IS THIS STUDY'S SECOND OF THE SAME KIND:** ⛔ **§1.3 replaced one wrong theory
(the appearance bake) with another (the colour), and BOTH were measured carefully.** ✅ **Neither
measurement was of the thing that actually decides whether a writer sees a line.**
⚠️ **`feedback_prove_code_is_reached` exists for exactly this: "it didn't change anything" usually means
the code is not running** — ⛔ **and that check was skipped, twice.**

✅ **THIS IS NOW MOOT FOR THE PRODUCT, THOUGH NOT FOR THE LESSON:** ⚠️ **the user has ruled the scene
break becomes a CONFIGURABLE GLYPH (§1.6), which replaces this drawing code outright.**

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

### 3.4A ⛔ **USER RULING 2026-09-29 — THE CARET POSITION IS NO LONGER THE FILE OFFSET**

⚠️ **The user, 2026-09-29:** ***"The manuscript view cursor position is now no longer its position in
the .md file. It must be maintained independently because the rendered manuscript will be removing
tokens from the on disk text when the attributes are rendered."***

✅ **RULED. ⛔ This overturns an assumption §3.4 carried silently** — ⚠️ **§3.4 said "the `.md` bytes stay
in storage byte-for-byte", which is still TRUE of the FILE, ⛔ but the user is ruling on what the CARET
means, and that is a different axis.**

#### ⚠️ What the ruling requires

⚠️ **Once a marker is hidden (Q2 = Model B, §10), the manuscript presents FEWER positions than the file
has.** ✅ **There are now TWO coordinate spaces and they must be named and converted explicitly:**

| Space | What it counts | Who owns it |
| ----- | -------------- | ----------- |
| ⚠️ **SOURCE offset** | every character in the `.md`, ✅ **including hidden markers and escape backslashes** | ✅ **the save path (§2), `byteOffset(charOffset:in:)` `:1933`, [EP-019] undo, the core** |
| ✅ **PRESENTED offset** | only what the writer can see and land on | ✅ **the caret, selection, arrow keys, click targeting, `setSelectedRange`** |

⛔ **TODAY THESE ARE THE SAME NUMBER, AND EVERY CALL SITE ASSUMES SO.** ⚠️ **That assumption is what
the user has just withdrawn.**

#### ⛔ What this makes NON-OPTIONAL

1. ⛔ **A SOURCE↔PRESENTED mapping is now a REQUIRED component, not an implementation detail.**
   ⚠️ **It did not appear in §9's Epic split and it must** — ✅ **it belongs in **E1 Foundations**, beside
   the typed-attachment fix (§2.3), because E2 cannot be built without it and E1's `* * *` mark already
   needs it.**
2. ⚠️ **Hidden characters must be skipped by CARET MOVEMENT, not merely made invisible.** ⛔ **A
   zero-width `**` that the arrow keys still step through twice is a worse surface than a visible one** —
   ✅ **this is precisely the "dead zone" complaint writers make about half-built Markdown editors.**
3. ⛔ **[I-0206] IS NOW MORE LIKELY TO RE-OPEN, NOT LESS.** ⚠️ **§7 already flagged
   `setSelectedRange` as OFFSET-LINEAR at `~59 ms`/keystroke; ⛔ a mapping layer adds work to EVERY
   caret move, on top of a call that is already the measured cost.** ✅ **Q5 is ruled "no re-open, file a
   new Issue if it bites" — ⚠️ **but the mapping layer is the most probable trigger and E1 should
   MEASURE it, per `project_read_amplification_class` (a cost measured in isolation is not a cost
   measured in place).**
4. ✅ **The save path is UNAFFECTED and that is the good news.** ⚠️ **It slices SOURCE offsets out of
   storage (§2), and storage keeps every character** — ⛔ **so the mapping is a READ-SIDE concern only.**

⚠️ **⛔ NOT DESIGNED HERE.** ✅ **Whether the mapping is a per-fragment table, a run-length skip list, or
an `NSTextLayoutFragment` that owns its own presented geometry is an E1 design question** — ⚠️ **this
section records only that the SEPARATION IS RULED and that it has an owner.**

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

### 3A.0 ⛔ **USER RULING 2026-09-29 — TYPED MARKUP IS ALWAYS ESCAPED. THE USER ANSWERED THEIR OWN QUESTION, AND THE ANSWER IS "NO".**

⚠️ **The user, 2026-09-29:** ***"I'm going to answer my own question here and my answer is based on your
output determined during the Q7 spike. If the user types something like `2 * 3 * 4` markdown may
interpret that as a command to embolden the 3. Therefore I propose that ANY markdown reserved character
that the user types be automatically escaped and simply displayed in the manuscript. We can provide
controls for the user to make selected text "bold", "italic", "heading", "list" or other markdown
rendering capabilities, and we can therefore limit the capabilities we expose in version 1. So `**`
that the user types never becomes bold. `#` never becomes a heading."***

✅ **RULED, AND IT REVERSES §3A's PREMISE.** ⛔ **§3A.4 ("type the marker, see the effect") is
WITHDRAWN.** ⚠️ **The user is not narrowing that proposal — they are replacing it with the opposite
one, on evidence this study produced.**

#### ✅ THE RULE

| The writer... | ⚠️ What lands in the `.md` | ✅ What the manuscript shows |
| ------------- | ------------------------- | --------------------------- |
| ✅ **TYPES** `2 * 3 * 4` | ⚠️ `2 \* 3 \* 4` — **escaped at input** | ✅ **`2 * 3 * 4`, literally. ⛔ No emphasis, ever** |
| ✅ **TYPES** `**bold**` | ⚠️ `\*\*bold\*\*` | ✅ **`**bold**`, literally** |
| ✅ **TYPES** `# Chapter` | ⚠️ `\# Chapter` | ✅ **`# Chapter`, literally** |
| ✅ **SELECTS text, invokes BOLD** | ✅ `**bold**` — **unescaped, real Markdown, written by Scrivi** | ✅ **bold** |

⛔ **MARKUP IS NEVER AUTHORED BY TYPING. IT IS ONLY EVER AUTHORED BY A COMMAND.**

#### ✅ WHY THIS IS THE STRONGER DESIGN — ⚠️ three consequences, and one of them is large

1. ✅ **IT MAKES THE PARSE UNAMBIGUOUS BY CONSTRUCTION.** ⚠️ **§4A.3 measured that `2 * 3 * 4` and `5*6`
   break a naive scanner and that Apple's parser survives them.** ⛔ **Under this ruling NEITHER PARSER
   EVER SEES THOSE INPUTS** — ✅ **they arrive pre-escaped, and the only unescaped markers in the file
   are ones Scrivi itself wrote.**
2. ✅ **IT MAKES [I-0206]'s CLASS OF PROBLEM SMALLER, AND MODEL B SAFER.** ⚠️ **Hiding a `**` is only
   safe if every `**` means emphasis.** ⛔ **Under free typing it does not; ✅ under this ruling it does,
   because Scrivi is the sole author of unescaped markup.**
3. ⛔ **BUT IT ADDS CHARACTERS TO HIDE, IT DOES NOT REMOVE THEM.** ⚠️ **`\*` is TWO source characters
   rendering as ONE presented character.** ✅ **§3.4A's SOURCE↔PRESENTED mapping is therefore required
   for ESCAPES ALONE — ⛔ even under Model A, ⛔ even if no marker is ever hidden.**
   ✅ **⚠️ MEASURED 2026-09-29 (§4B), AT THE USER'S INSISTENCE — *"let's not add work that isn't
   needed"*:** ⛔ **the parser DOES remove escape backslashes and does NOT say where they were
   (a naive linear map misplaces up to 25 of 31 characters), ✅ BUT a ~12-line scanner reproduces the
   parser exactly.** ✅ **So the cost is REAL but SMALL — ⛔ ~12 lines, not a subsystem.**

#### ⚠️ WHAT THIS RULING DOES **NOT** SETTLE — ⛔ for E1 design, flagged, not decided

- ✅ **WHICH characters are "reserved" — ⛔ RULED + MEASURED 2026-09-29. ⚠️ IT IS TWO LISTS, NOT ONE.**
  ⚠️ **The user directed the escape set to [markdown.org's Escaping section](https://markdown.org/basics/)
  — ✅ *"Only Markdown commands should be escaped (commas are not part of that list)"*** — ✅ **fetched:
  **16 characters**, `` \ ` * _ { } [ ] ( ) # + - . ! | ``. ✅ **The user is right that `,` is absent.**
  ⛔ **BUT §4B.4 MEASURED APPLE'S PARSER AND IT ESCAPES ALL 32 ASCII PUNCTUATION MARKS** — ⚠️ **the 16
  are a strict SUBSET.** ✅ **So:** ✅ **WRITE set = the 16 (policy, keeps the `.md` readable);**
  ⛔ **READ set = all 32 (not a choice — it is what the parser does, and the caret mapping must match
  it).** ⚠️ **Treating them as one list is a defect — ✅ §4B.4's E10 probe caught exactly that.**
- ⛔ **PASTE.** ⚠️ **Typing is one door; ✅ pasting is another, and the ruling's words are *"that the user
  types"*.** ⚠️ **Pasting a block of Markdown from elsewhere must either be escaped identically (safe,
  and surprising to anyone pasting real Markdown) or offered as a choice.** ⛔ **UNRULED.**
- ⛔ **EXISTING MANUSCRIPTS.** ⚠️ **Scene files written before this rule contain unescaped `*` typed as
  arithmetic or emphasis.** ✅ **They will render as emphasis under the new renderer** — ⛔ **a silent
  appearance change to existing prose, with no migration.** ⚠️ **Whether that is acceptable, or needs a
  one-time escape pass, is a ruling this study owes but does not make.**
- ✅ **THE V1 SURFACE IS NOW THE USER'S TO SET, AND IT IS SMALL BY DESIGN** — ⚠️ **the user's own words:
  *"we can therefore limit the capabilities we expose in version 1"*.** ✅ **Their named list is
  **bold, italic, heading, list**; ⚠️ **§4A.3 also exercised strikethrough and code.** ⛔ **E1 must
  enumerate the exposed verbs EXPLICITLY rather than inherit "whatever CommonMark does"** —
  ✅ `feedback_design_to_capability_not_lcd`.

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

### 3A.4 ⛔ ~~"Type `**` and it becomes bold"~~ — **WITHDRAWN BY USER RULING 2026-09-29 (§3A.0)**

⛔ **THIS SECTION IS SUPERSEDED.** ✅ **The user has ruled that typed markup is ESCAPED and never
becomes formatting (§3A.0).** ⚠️ **Formatting is applied by COMMAND over a SELECTION.** ✅ **The text
below is retained as the record of the proposal that was considered and rejected, ⛔ not as a
recommendation.**

#### ~~What that actually required~~

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

✅ **THE CLEAN SPLIT — ⛔ RULED BY THE USER 2026-09-29 (Q8 = NO). ✅ THE STUDY'S RECOMMENDATION IS
ADOPTED:**
- ⛔ **`#` is NOT a chapter title.** ✅ **Chapter titles stay metadata, rendered by the view.**
  ⚠️ **They can still RENDER as a heading — that is F2 typography (§5) and needs no syntax at all.**
- ✅ **`##`/`###` (and `#` if the writer types it) are ORDINARY IN-SCENE HEADINGS** — ✅ **real Markdown,
  saved in the body, rendered as headings.** ⚠️ **A writer who wants a section break inside a scene gets
  one, and nothing competes for ownership.**

⛔ **RULED 2026-09-29: `#` IS NOT A CHAPTER HEADING.** ✅ **Chapter titles remain metadata, rendered by
the view, and stay toggleable.** ⚠️ **⛔ AND UNDER §3A.0 A TYPED `#` IS ESCAPED AND NEVER BECOMES A
HEADING AT ALL** — ✅ **so the two-owners hazard is closed twice over: once by this ruling, and once by
the escaping rule.** ⚠️ **In-scene headings exist, ⛔ but only via the Heading COMMAND.**

### 3A.6 ✅ "Enter means a new paragraph" — ⚠️ mostly free, one wrinkle

✅ **In Markdown, a paragraph break is a BLANK LINE (`\n\n`); a single `\n` is a soft break.**
⚠️ **So "Enter = new paragraph" means either:**
- ✅ **(a) RENDER a single `\n` with paragraph spacing** — ⚠️ **NOT possible via rendering attributes:
  paragraph spacing is `NSParagraphStyle`, which AFFECTS LAYOUT (§4.1).** ⛔ **It would have to be a real
  storage attribute, which the undo path (`:358-369`) currently strips.**
- ✅ **(b) INSERT `\n\n` on Enter** — ✅ **trivial, honest, and the file stays canonical Markdown.**
  ⚠️ **But it changes what the writer's Backspace does, and [EP-019]'s sentence-granular undo sees it.**

#### ⛔ **RULED 2026-09-29 — (b). ENTER INSERTS `\n\n`. ✅ AND THE USER RULED THE BACKSPACE CASE TOO.**

⚠️ **The user:** ***"Enter should insert `\n\n`. It would be disorientating if the user typed enter
only to find nothing happened until she types enter again. However, from the start of a paragraph,
Backspace should, in fact, delete only one `\n`, thus merging the paragraph with the previous one. The
single `\n` is rendered as whitespace and so a subsequent backspace will remove it correctly. That is
expected behavior, I think the cost is acceptable there."***

✅ **THE ASYMMETRY IS DELIBERATE AND IS THE RIGHT CALL.** ⚠️ **Enter is SYMMETRIC-BY-FEEL (one press,
one visible paragraph break); ⛔ Backspace is SYMMETRIC-BY-CHARACTER (one press, one character).**
✅ **The user has weighed that trade explicitly and accepted it.**

| Keystroke | ⚠️ Source effect | ✅ What the writer sees |
| --------- | ---------------- | ---------------------- |
| ✅ **Enter** | ⚠️ inserts **two** `\n` | ✅ **a new paragraph, immediately.** ⛔ Never "nothing happened" |
| ✅ **Backspace at paragraph start** | ⚠️ deletes **one** `\n` | ✅ **the paragraph merges upward** |
| ✅ **Backspace again** | ⚠️ deletes the remaining `\n` | ✅ **the soft break closes up** |

#### ✅ **⛔ MEASURED 2026-09-29 (§4B.5) — THE MERGE IS FREE, ⚠️ AND THE REAL HAZARD IS THE ONE THE USER NAMED**

⚠️ **v0.1 warned that E1 "must get right" that a single `\n` joins two lines.** ✅ **MEASURED: Apple's
parser ALREADY DOES.** ⛔ **`"first para.\nsecond para."` presents as `"first para.␣second para."` —
the `\n` becomes a literal SPACE (`INLINE(64)` soft break), one paragraph.** ✅ **So the user's point 3
is confirmed exactly: *"the writer is happy, ScriviCore is happy."*** ⚠️ **Nothing to build.**

⛔ **THE HAZARD IS THE USER'S, NOT THIS ONE — ✅ TWO TRAILING SPACES.** ⚠️ **`"first para.␣␣\nsecond
para."` presents as a HARD line break (`INLINE(128)`), ⛔ and so does a trailing `\`.**
✅ **RULED by the user and recorded in §4B.6** — ⚠️ **normalise trailing spaces on Enter (⛔ to AT MOST
ONE, not "delete one" — §4B.6's amendment), ✅ and collapse a trailing `\\` to `\` as a deliberate
hard break.**

⚠️ **[EP-019] INTERACTION IS UNCHANGED BUT MUST BE RE-CHECKED:** ⛔ **sentence-granular undo now sees a
two-character insert where the writer made one gesture.** ✅ **Whether Enter coalesces into one undo
step is an E1 question, ⛔ not answered here.**

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

### 4.3 ⛔ **RULED 2026-09-29 — MODEL B. ✅ "My inclination is to wysiwyg."**

⚠️ **The user, 2026-09-29, on §4.2:** ***"my inclination is to wysiwyg."*** ✅ **Q2 = B.**

⛔ **THIS IS THE EXPENSIVE ANSWER AND THE STUDY SAID SO BEFORE IT WAS GIVEN** — ✅ **§4A.1 measured that
rendering attributes CANNOT hide a marker, so B needs STORAGE attributes (and must survive `:358-369`)
or an `NSTextLayoutFragment` subclass.** ⚠️ **The ruling stands; ✅ what follows is what it now COSTS,
stated once, here.**

| ⚠️ What Model B now requires | ✅ Source | ⛔ Status |
| --------------------------- | -------- | --------- |
| ✅ **SOURCE↔PRESENTED offset mapping** | §3.4A (user ruling) | ⛔ **REQUIRED — moves into E1** |
| ✅ **Caret movement that SKIPS hidden runs** | §3.4A | ⛔ **not optional; a stepped-through zero-width marker is worse than a visible one** |
| ✅ **Hiding ESCAPE backslashes too** (`\*` → `*`) | §3A.0 | ⛔ **NEW — escaping made this universal, not marker-only** |
| ✅ **A storage-attribute route that survives the undo path** | §4A.2, `:358-369` | ⛔ **READ, NOT RUN — §11 still lists this as untested** |
| ⚠️ **Re-entry behaviour: markers reappear when the caret enters** | §4.2 Model B | ⛔ **UNDESIGNED** |

✅ **THE INCREMENTAL PATH SURVIVES THE RULING AND SHOULD BE TAKEN:** ⚠️ **ship **A** in E1 (it works
today, storage untouched, and it makes the renderer seam real), then **B** per element type in E2.**
⛔ **A is not a competing model under this ruling — it is B's first milestone.**

⚠️ **AND ONE ORDERING FACT IS NOW FIXED:** ✅ **whole-LINE markers (`#`, `##`, list bullets) are much
easier to hide than INLINE ones (`**`), because a line prefix can be handled by paragraph-level layout
rather than glyph suppression.** ✅ **E2 should take them in that order.**

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

## 4B. ✅ **Q10 SPIKE — RUN 2026-09-29. ⛔ THE ESCAPE RULING'S COST WAS MEASURED, AND IT IS SMALLER THAN §3A.0 FEARED.**

⚠️ **Occasioned by the user, 2026-09-29:** ***"You need to determine if `AttributedString(markdown:)`
will also remove escapes. Let's not add work that isn't needed."*** ✅ **Correct challenge — §3A.0
asserted a cost without measuring it.** ⛔ **Six programs, `swiftc -O`, Swift 6.4 / macOS 27.2.**

---

### 4B.1 ⛔ **Q10(a): YES — the parser REMOVES escape backslashes. Measured.**

✅ **Every escaped input loses its backslashes in the presented string:**

| Probe | Source | Presented | ⚠️ Δ |
| ----- | ------ | --------- | --- |
| **A1** | `2 \* 3 \* 4` (11) | ✅ `2 * 3 * 4` (9) | **−2** |
| **A2 CONTROL** | `2 * 3 * 4` (9) | ✅ `2 * 3 * 4` (9) | **0** — ⚠️ **no emphasis either way** |
| **A3** | `\*\*bold\*\*` (12) | ✅ `**bold**` (8) | **−4** — ⛔ **no emphasis** |
| **A4 CONTROL** | `**bold**` (8) | ⚠️ `bold` (4) | **−4** — ✅ **emphasis applied** |
| **A5** | `\# Heading` (10) | ✅ `# Heading` (9) | **−1** |
| **A6** | `a \\ b` (6) | ✅ `a \ b` (5) | **−1** |
| **A7** | `snake\_case\_word` | ✅ `snake_case_word` | **−2** |

✅ **A3 vs A4 IS THE WHOLE RULING, PROVEN IN TWO LINES:** ⚠️ **the same eight visible characters, and
escaping is what decides whether they are bold.** ✅ **§3A.0 works mechanically.**

### 4B.2 ⛔ **AND THE BACKSLASH IS NOT LOCATED FOR YOU — this is the real cost**

⚠️ **A run's `markdownSourcePosition` gives a SPAN, ⛔ with NO interior marker for where inside it a
backslash was removed.** ✅ **Measured — source columns exceed presented characters:**

| Probe | Run | ⚠️ cols vs chars |
| ----- | --- | ---------------- |
| **B1** | `"2 * 3 and "` | ⛔ **11 cols / 10 chars** |
| **B3** | `"a * b "` · `" d * e"` | ⛔ **7/6 each** |
| **B4** | `"*not* but "` | ⛔ **12 cols / 10 chars** |

⛔ **A NAIVE LINEAR MAP (`runStart + offsetInRun`) IS WRONG — MEASURED:**

| Probe | ⛔ characters landing on the WRONG source character |
| ----- | -------------------------------------------------- |
| **D1** | **8 / 26** |
| **D2** | **7 / 13** |
| **D3** | **10 / 13** |
| **D4** | ⛔ **25 / 31** |
| **D5 CONTROL** (no escapes) | ✅ **0 / 29** |

✅ **D5 IS WHY THIS MATTERS:** ⚠️ **with no escapes the naive map is PERFECT** — ⛔ **so this is a cost
that §3A.0's ruling CREATES, exactly as §3A.0's consequence 3 predicted.** ✅ **The prediction was right;
⚠️ what was unknown was how dear it is.**

### 4B.3 ✅ **⛔ BUT IT IS CHEAP — A 12-LINE SCANNER REPRODUCES THE PARSER EXACTLY**

⚠️ **An INDEPENDENT oracle — walk the source, skip a backslash before an escapable character — was
checked against Apple's parser on escape-only inputs:**

| Probe | Input | ✅ Oracle reproduces parser EXACTLY? |
| ----- | ----- | ----------------------------------- |
| **E1–E9** | `\*`, `\*\*`, `\#`, `\_`, `\\`, `` \` ``, `\[ \]`, `\( \{ \|`, `\+ \- \. \!` | ✅ **YES — 9/9** |
| **E10** | `Mr\. Smith said \"hi\"` | ⛔ **NO** — ✅ **and the disagreement is the ORACLE's fault, not the parser's (§4B.4)** |

✅ **SO THE MAPPING IS NOT A RESEARCH PROBLEM.** ⚠️ **Scrivi does NOT need the parser to locate escapes —
⛔ it can compute them itself, deterministically, in one pass over the fragment it is already scanning.**
✅ **THE ANSWER TO THE USER'S CHALLENGE: escaping DOES add work, ⛔ but ~12 lines of it, not a subsystem.**

⚠️ **⛔ ONE CAVEAT, AND IT IS THE `feedback_boundary_tests_not_facade` CLASS:** ✅ **the oracle agreeing
with the parser on 9 inputs is not the same as agreeing on all inputs.** ⚠️ **E10 found a disagreement
on the TENTH try.** ✅ **E1 must TEST the oracle against the parser over a corpus, ⛔ not assume it.**

### 4B.4 ⛔ **Q10(f): markdown.org's 16 IS NOT APPLE'S SET. ✅ APPLE ESCAPES ALL 32 ASCII PUNCTUATION MARKS.**

⚠️ **The user directed the escape list to markdown.org's Escaping section** — ✅ **fetched, and it lists
exactly 16: `` \ ` * _ { } [ ] ( ) # + - . ! | ``.** ⛔ **APPLE'S PARSER DOES NOT IMPLEMENT THAT LIST.**

✅ **MEASURED — every ASCII punctuation character, `x\<c>y` → is the backslash consumed?**

| | Result |
| - | ------ |
| ✅ **Backslash consumed** | ⛔ **32 / 32** — `` !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~ `` |
| ⛔ **Backslash NOT consumed** | ✅ **0** |
| ⚠️ **Escapable by Apple, ABSENT from markdown.org's 16** | ⛔ **16:** `` "$%&',/:;<=>?@^~ `` |
| ✅ **On markdown.org's 16 but NOT escapable by Apple** | ✅ **0 — the 16 are a strict SUBSET** |

⚠️ **THIS IS CommonMark's RULE, NOT A BUG** — ✅ **CommonMark escapes ALL ASCII punctuation; markdown.org
is describing ORIGINAL Markdown (2004), which Apple does not implement.**

#### ⛔ **THE CONSEQUENCE FOR THE RULING — ✅ it SPLITS into two different lists**

⚠️ **§3A.0 conflated two sets that the measurement now separates:**

| List | ⚠️ What it is | ✅ Size | ⚠️ Who it serves |
| ---- | ------------- | ------ | ---------------- |
| ✅ **WRITE set** — what Scrivi ESCAPES on input | ⚠️ **a POLICY choice** — only what would change the parse | ⚠️ **markdown.org's 16 is a defensible answer; ⛔ the user's "commas are not on that list" is CORRECT** | ✅ **the writer — keeps the `.md` readable** |
| ⛔ **READ set** — what Scrivi must UNESCAPE when mapping | ⛔ **NOT a choice — it is whatever the parser does** | ⛔ **ALL 32** | ✅ **the SOURCE↔PRESENTED mapping (§3.4A)** |

⛔ **E10 IS EXACTLY THIS CONFUSION, CAUGHT BY MEASUREMENT.** ⚠️ **The oracle used the 16-char list, hit
`\"`, and disagreed with the parser** — ✅ **because `"` is escapable in CommonMark and absent from
markdown.org's 16.** ⛔ **A reader who took markdown.org as the READ set would have shipped that
off-by-one into the caret mapping.**

#### ⛔ **RULED 2026-09-29 — ✅ USE THE READ SET. ONE LIST: ALL 32.**

⚠️ **The user, on being shown the split:** ***"use the READ set."***

✅ **RULED, AND IT COLLAPSES THE TWO LISTS BACK INTO ONE** — ⛔ **Scrivi escapes, and unescapes, ALL 32
ASCII punctuation characters.** ⚠️ **markdown.org's 16 is recorded as the list that was CONSIDERED, ⛔
not the list that is used.**

| | ✅ **RULED BEHAVIOUR** |
| - | -------------------- |
| ✅ **What Scrivi ESCAPES on input** | ⛔ **all 32:** `` !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~ `` |
| ✅ **What Scrivi UNESCAPES when mapping** | ⛔ **the same 32** |
| ✅ **Risk of the two drifting** | ✅ **NONE — ⚠️ there is only one list** |

✅ **WHY THIS IS THE RIGHT CALL, EVEN THOUGH IT ESCAPES MORE THAN MARKDOWN NEEDS:** ⚠️ **the WRITE set
is a policy choice but the READ set is NOT — ⛔ it is dictated by the parser.** ✅ **Matching the write
set to it makes the round trip EXACT by construction: ⚠️ every backslash Scrivi writes is one the parser
will consume, and every one the parser consumes is one Scrivi wrote.** ⛔ **The alternative — escaping
16 and unescaping 32 — is correct only so long as two lists stay in agreement, ✅ and this project has
filed that class of defect repeatedly (the `kAllStorableKinds` standing rule).**

⚠️ **⛔ THE ACCEPTED COST, STATED PLAINLY:** ✅ **a typed `Mr. Smith, in "quotes" — really?` is stored as
`Mr\. Smith\, in \"quotes\" — really\?`.** ⛔ **The `.md` is NOISIER to a human reading the raw file
than it would be under the 16.** ⚠️ **It is still VALID CommonMark and renders identically** — ✅ **the
cost is legibility of the source, ⛔ not correctness.** ⚠️ **The user has the information and has ruled.**

#### ✅ **⛔ AND THE RULING WAS RE-MEASURED, NOT ASSUMED — 10/10**

⚠️ **§4B.3's oracle failed ONE probe (E10, `Mr\. Smith said \"hi\"`) because it used the 16.**
✅ **The oracle was re-run with the RULED all-32 set:**

| | ⚠️ Oracle with the 16 | ✅ Oracle with the ruled 32 |
| - | --------------------- | -------------------------- |
| ✅ **Probes reproducing Apple's parser EXACTLY** | ⚠️ **9 / 10** | ✅ **10 / 10** |
| ⛔ **E10 (`\"` — escapable in CommonMark, absent from the 16)** | ⛔ **oracle 30 vs parser 28** | ✅ **28 vs 28** |

✅ **SO THE RULING IS NOT MERELY TIDIER — ⚠️ IT FIXED THE ONE MEASURED DISAGREEMENT.**
⛔ **The §4B.3 caveat still stands in reduced form:** ⚠️ **10 probes is not a proof, ✅ and E1 still owes
a corpus test** — ⛔ **but the known failure is gone, not argued away.**

### 4B.5 ✅ **Q10(c): THE USER'S TRAILING-SPACE CORNER CASE IS REAL — ⛔ MEASURED, AND WORSE THAN STATED**

⚠️ **The user, 2026-09-29:** ***"Markdown will render two trailing spaces followed by one `\n` as an
"in paragraph" line break… If the writer ends sentences with two spaces (which is proper), then this may
become a problem because a backspace that eliminates the first `\n` of a `\n\n` will create exactly this
issue."***

✅ **CONFIRMED BY MEASUREMENT, at block level (`interpretedSyntax: .full`):**

| Source | ✅ Presented | ⚠️ Structure |
| ------ | ----------- | ------------ |
| `"first para.\n\nsecond para."` | `"first para.second para."` | ✅ **TWO `paragraph` blocks** |
| ⚠️ `"first para.\nsecond para."` | ✅ **`"first para. second para."`** | ✅ **ONE paragraph** — ⚠️ **the `\n` became a SPACE, `INLINE(64)` soft break** |
| ⛔ **`"first para.  \nsecond para."`** | ⛔ **`"first para.\nsecond para."`** | ⛔ **ONE paragraph, ⚠️ but `INLINE(128)` — a HARD LINE BREAK** |
| ⛔ **`"first line.\\\nsecond line."`** | ⛔ **`"first line.\nsecond line."`** | ⛔ **`INLINE(128)` — ⚠️ identical to the two-space form** |

✅ **THE USER'S DIAGNOSIS IS EXACTLY RIGHT, AND BOTH FORMS THEY NAMED BEHAVE IDENTICALLY** — ⚠️ **two
trailing spaces and a trailing backslash both produce `INLINE(128)`.**

#### ⛔ **THIS CORRECTS §3A.6 — the study said the soft break "renders as whitespace"; ✅ it renders as a SPACE, which is stronger**

⚠️ **§3A.6 warned that E1 "must get right" that a single `\n` joins two lines.** ✅ **MEASURED: Apple's
parser ALREADY DOES THIS** — ⛔ **`"first para.\nsecond para."` presents as `"first para. second para."`,
with the `\n` replaced by a literal SPACE.** ✅ **So the user's point 3 is confirmed: *"the writer is
happy, ScriviCore is happy"* — ⚠️ the merge is free, not a thing E1 must build.**

⛔ **THE HAZARD IS NARROWER THAN §3A.6 IMPLIED, AND SHARPER:** ⚠️ **it is not "will the lines join" —
✅ they will — ⛔ it is "did the writer leave two spaces before the `\n`", in which case they DON'T.**

### 4B.6 ✅ **THE USER'S RULING ON THE CORNER CASE — ⛔ ADOPTED, with one measured amendment**

⚠️ **The user:** ***"if the user has typed two spaces and then types enter (for which we will insert
`\n\n`), we should silently eliminate one of those spaces. It can, however, become part of the history
if necessary. If the user types a backslash (escaped as `\\`) followed by enter (`\n\n`), we should
silently convert it to a single backslash (`\`) which will remove it from the rendering, and leave the
paragraph "broken" at that character. That is a deliberate sequence the writer typed herself and so will
be expecting the behavior."***

✅ **RULED. ⚠️ Both halves are sound and the measurement supports them:**

| ⚠️ Writer types | ✅ Scrivi writes | ⚠️ After a Backspace merges the paragraphs | ✅ Why |
| --------------- | ---------------- | ------------------------------------------ | ----- |
| `…text.` + `␣␣` + **Enter** | ⛔ **`…text.␣\n\n`** — ✅ **ONE space dropped** | ✅ **`…text.␣\n` → renders as one paragraph** | ⛔ **Two spaces would have become `INLINE(128)`, a hard break the writer never asked for** |
| `…text.` + `\\` + **Enter** | ⛔ **`…text.\\n\n`** — ✅ **`\\` collapsed to `\`** | ⚠️ **`…text.\\n` → `INLINE(128)`, a DELIBERATE hard break** | ✅ **The writer typed a backslash on purpose; ✅ the break is the expected result** |

✅ **THE ASYMMETRY IS PRINCIPLED:** ⚠️ **two spaces are an ARTEFACT of sentence-spacing habit (the writer
does not mean "line break"); ⛔ a trailing backslash is UNAMBIGUOUS INTENT.** ✅ **The ruling reads
intent correctly in both cases.**

#### ⚠️ **⛔ ONE AMENDMENT THE MEASUREMENT FORCES — the rule must be "≥2 spaces", not "two"**

⛔ **MEASURED: `"first line. \nsecond line."` (ONE trailing space) is NOT a hard break** — ✅ **it stays
a soft break.** ⚠️ **So dropping to exactly one space is SAFE, ✅ which is what the ruling does.**
⛔ **BUT CommonMark's hard-break rule is TWO OR MORE spaces** — ⚠️ **a writer who typed three (or a
stray trailing space after two) still produces `INLINE(128)` after the merge.** ✅ **E1 must implement
*"reduce trailing spaces to at most one"*, ⛔ NOT *"delete one space"*.**

⚠️ **⛔ AND THE INVERSE CASE IS UNRULED:** ✅ **the ruling covers spaces present when Enter is pressed.**
⛔ **It does not cover a writer who presses Enter FIRST and later adds spaces at the end of the previous
line** — ⚠️ **the hazard reappears and nothing catches it.** ✅ **E1 should decide whether the
normalisation runs on Enter only, or on the BACKSPACE-MERGE as well** — ⛔ **the merge is where the
damage actually manifests, and normalising there catches every route.**

✅ **[EP-019] NOTE, per the user's *"it can become part of the history if necessary"*:** ⚠️ **the dropped
space is a real edit and should be undoable** — ⛔ **but coalesced into the Enter, not as a separate
step, or Undo after Enter will restore a space the writer never saw.**

---

### 4B.7 ⚠️ **SPIKE PROVENANCE**

✅ **Six programs, `swiftc -O`, Apple Swift 6.4 / macOS 27.2** (`scratchpad/spike2/q10a…q10f.swift`).
⚠️ **THROWAWAY AND NOT COMMITTED**, ✅ **[EP-018]/[T-0191] precedent, same as §4A.**
⛔ **Their NUMBERS are recorded above; ⚠️ their code is not meant to survive** — ✅ **but the ORACLE in
`q10d`/`q10e` is the ~12 lines E1 will re-write, and §4B.3's caveat applies: ⛔ it is verified on 9
inputs, not proven.**

⚠️ **ONE METHOD NOTE, recorded because it cost a build:** ⛔ **`#"…"#` raw strings in Swift treat `\#`
as an escape sequence**, ✅ **so probes containing `\#` need `##"…"##`.** ⚠️ **A study about escaping
was itself bitten by escaping.**

---

## 4C. ⛔ **Q13/Q14 SPIKE — RUN 2026-09-29. ⚠️ THE "SPACES COLLAPSE" PREMISE IS FALSE, AND ONE CASE IS A BLOCK-TYPE CHANGE.**

⚠️ **The user, 2026-09-29, reasoning forward from §4B.6:** ***"let's say the writer goes to the last
valid character of the paragraph before the break, enters 3 or six or eleven spaces and then starts
typing. Well, AttributedString will simply conflate that to a single space and move on. Writer happy,
scrivicore happy."***

⛔ **MEASURED, AND IT DOES NOT.** ✅ **The reasoning was sound — ⚠️ it is what HTML does, and what most
Markdown renderers do — ⛔ but Apple's parser does not collapse interior spaces.**

### 4C.1 ⛔ **Q13: INTERIOR SPACES ARE PRESERVED, NOT CONFLATED**

| Probe | Source | ⛔ `.full` output | ⚠️ Spaces |
| ----- | ------ | ----------------- | -------- |
| **I1** | `word␣␣␣word` | ⛔ `word␣␣␣word` | **3 → 3 PRESERVED** |
| **I2** | `word␣␣␣␣␣␣word` | ⛔ `word␣␣␣␣␣␣word` | **6 → 6 PRESERVED** |
| **I3** | ⚠️ **`word` + ELEVEN spaces + `word`** | ⛔ **unchanged** | **11 → 11 PRESERVED** |
| **I4** | `Sentence one.␣␣Sentence two.` | ✅ `Sentence one.␣␣Sentence two.` | **PRESERVED** |

⛔ **SO THE WRITER'S ELEVEN SPACES STAY ELEVEN SPACES, ON SCREEN, MID-PARAGRAPH.**
✅ **THE USER'S OWN CLOSING SENTENCE IS THEREFORE THE CORRECT READ OF THE SITUATION** — ⚠️ ***"the writer
will not be certain about the number of spaces at the end of her paragraph, or that there may be eleven
spaces somewhere in the middle of her paragraph."*** ⛔ **She will not be certain, AND the spaces will be
visible. ⚠️ The parser does not rescue this.**

⚠️ **⛔ AND I4 IS THE ONE THAT MAKES NORMALISATION UNSAFE AS A GENERAL RULE:** ✅ **two spaces after a
period is PROPER sentence spacing and the writer means it.** ⛔ **A blanket "collapse runs of spaces"
would silently rewrite her prose everywhere, ⚠️ which is a far worse defect than the one it fixes.**
✅ **This is why §4B.6's normalisation is scoped to the END OF A LINE ON ENTER and nowhere else.**

### 4C.2 ✅ **WHERE SPACES *ARE* COLLAPSED — ⚠️ and `.full` vs `.inlineOnlyPreservingWhitespace` DISAGREE**

| Probe | Source | ⚠️ `.full` | ✅ `.inlineOnlyPreservingWhitespace` |
| ----- | ------ | ---------- | ----------------------------------- |
| **I5** | `end of para.` + 11 trailing | ⛔ **trailing spaces DROPPED** | ✅ **PRESERVED** |
| **I8** | 2 LEADING spaces | ⛔ **DROPPED** | ✅ **PRESERVED** |
| **I9** | ⛔ **11 LEADING spaces** | ⛔ **`"       eleven leading\n"`** | ✅ **PRESERVED** |

⛔ **THE TWO SYNTAX MODES BEHAVE DIFFERENTLY ON WHITESPACE, AND THE STUDY HAS BEEN QUOTING BOTH.**
⚠️ **§4A.3 and §4B used `.inlineOnlyPreservingWhitespace`; ✅ §4B.5's paragraph work used `.full`.**
⛔ **E1 MUST PICK ONE AND STATE IT** — ⚠️ **the whitespace answers are not the same, and a study that
mixes them will mislead.**

### 4C.3 ⛔ **Q14: ELEVEN LEADING SPACES IS NOT A COSMETIC SURPRISE — ⚠️ IT SILENTLY BECOMES A CODE BLOCK**

⚠️ **I9's output (`"       eleven leading\n"` — SEVEN spaces and a trailing newline) had the shape of an
indented code block.** ✅ **Confirmed directly by reading `presentationIntent`:**

| Leading spaces | ⚠️ Block type | Output |
| -------------- | ------------- | ------ |
| **0** | ✅ `paragraph` | `normal paragraph` |
| **1** | ✅ `paragraph` | ✅ space dropped |
| **3** | ✅ `paragraph` | ✅ spaces dropped |
| ⛔ **4** | ⛔ **`codeBlock`** | ⛔ **`"four leading spaces\n"`** |
| ⛔ **11** (the user's number) | ⛔ **`codeBlock`** | ⛔ **`"       eleven leading\n"`** |

⛔ **CommonMark's INDENTED CODE BLOCK rule is FOUR OR MORE LEADING SPACES**, ✅ **and Apple implements
it.** ⚠️ **So a writer who indents a paragraph with spaces — ⛔ a thing writers do by reflex — turns her
prose into a code block:** ⛔ **monospace, no emphasis rendering, no wrapping.**

✅ **THE GOOD NEWS, MEASURED (P1):** ⚠️ **an indented SECOND line of an existing paragraph is safe** —
⛔ `"First line.\n␣␣␣␣indented second line."` stays ONE `paragraph`, ✅ **because CommonMark's lazy
continuation absorbs it.** ⚠️ **The hazard is a line that STARTS a block.**

### 4C.4 ⛔ **WHAT THIS ADDS TO THE RULING — ✅ one new normalisation, and it is NOT the one the user proposed**

⚠️ **§4B.6 normalises TRAILING spaces on Enter. ⛔ Q14 shows LEADING spaces are the sharper hazard,
because they change the BLOCK TYPE rather than merely inserting a break.**

| ⚠️ Hazard | ⛔ Consequence | ✅ Proposed handling |
| --------- | -------------- | ------------------- |
| ✅ **≥2 TRAILING spaces before `\n`** | ⚠️ unwanted hard line break (`INLINE(128)`) | ✅ **RULED §4B.6** — reduce to at most one on Enter |
| ⛔ **≥4 LEADING spaces on a line** | ⛔ **the paragraph becomes a `codeBlock`** | ✅ **RULED 2026-09-29 — §4D.4: suppress unexposed block intents + a `paragraphIndent` preference** |
| ⚠️ **3–11 INTERIOR spaces** | ⚠️ **visible, preserved, untidy** | ⛔ **DO NOTHING — ✅ I4 proves collapsing them would destroy proper sentence spacing** |

⚠️ **⛔ THE STUDY RECOMMENDS, BUT DOES NOT RULE, ON LEADING SPACES:** ✅ **under §3A.0 the writer's typed
text is escaped anyway — ⛔ but a SPACE IS NOT AN ESCAPABLE CHARACTER, so §3A.0 does NOT cover this.**
⚠️ **Three options, each with a real cost:**

1. ✅ **Normalise leading spaces to at most three on a line that starts a block.** ⛔ **Silently changes
   what she typed, ⚠️ and §4C.1's I4 lesson says silent whitespace rewriting is dangerous.**
2. ✅ **Leave the source alone and suppress the `codeBlock` INTENT in the renderer.** ⚠️ **Scrivi does not
   expose code blocks in v1 anyway (the user's list is bold/italic/heading/list), ⛔ so an intent it
   never renders is harmless.** ✅ **THIS IS THE STUDY'S PREFERENCE — it touches no bytes.**
3. ⛔ **Do nothing and let indented prose render as code.** ⚠️ **Honest to Markdown, ⛔ astonishing to a
   novelist.**

✅ **OPTION 2 GENERALISES:** ⚠️ **the v1 verb list is CLOSED (§3A.0), so ANY block intent Scrivi does not
expose — `codeBlock`, `blockQuote`, `table` — can be rendered as ordinary prose rather than fought at
the byte level.** ⛔ **That is a design decision E1 owes, ✅ and it is cheaper than every alternative.**

---

## 4D. ✅ **Q14 RULED + Q15/Q16 SPIKE — 2026-09-29. ⛔ THE TAB IS THE WORST OPTION, ✅ AND THE BEST ONE COSTS ZERO CHARACTERS.**

⚠️ **The user, 2026-09-29:** ***"according to copilot, in markdown a codeblock is surrounded by three
single tics. However, it looks like apple is treating 4 or more spaces as a codeblock… we do Q14 option
2, we normalise a paragraph indent. In fact, we can add another project preference with regard to
paragraph indentations that would apply globally to all paragraphs… should we insert a tab character in
place of spaces? or is it possible to simply format the paragraphs with leading spaces? That way, the
writer has set her preference at the beginning and does not need to type leading spaces at each
paragraph. Reducing the temptation to enter a character sequence that would lead to unintended
results."***

### 4D.1 ✅ **BOTH THE USER AND COPILOT ARE RIGHT — ⚠️ CommonMark HAS TWO CODE-BLOCK FORMS**

⛔ **THIS IS NOT A DISAGREEMENT TO RESOLVE; ✅ they are describing different constructs.**

| Form | Syntax | ✅ Measured (§4C, §4D.2) |
| ---- | ------ | ----------------------- |
| ✅ **FENCED** — what Copilot described | ` ``` ` … ` ``` ` | ✅ **`codeBlock` — CONFIRMED (probe F1)** |
| ⛔ **INDENTED** — what Apple applied to the user's prose | ⚠️ **≥4 leading spaces, or ONE TAB** | ⛔ **`codeBlock` — CONFIRMED (§4C.3)** |

✅ **THE INDENTED FORM IS THE OLDER ONE** (Markdown 1.0, 2004) ⚠️ **and CommonMark kept it for
compatibility.** ⛔ **It is the one that ambushes prose, ✅ and the user's read of WHY is exactly right:**
⚠️ ***"This would make sense if you were writing technical documentation with inserts. In Scrivi's case,
not so much."*** ✅ **Scrivi's v1 verb list (§3A.0) exposes NEITHER form.**

### 4D.2 ⛔ **Q15: A TAB IS *WORSE* THAN FOUR SPACES — ⚠️ ONE TAB IS ENOUGH. MEASURED.**

⚠️ **The user asked whether to *"insert a tab character in place of spaces"*.** ⛔ **MEASURED, AND THE
ANSWER IS NO:**

| Probe | Source | ⛔ Block type |
| ----- | ------ | ------------- |
| ⛔ **T1** | ⚠️ **ONE tab** | ⛔ **`codeBlock`** |
| ⛔ **T2** | two tabs | ⛔ **`codeBlock`** (⚠️ and the second tab survives into the output) |
| ⛔ **S4** | four spaces | ⛔ **`codeBlock`** |
| ✅ **S3** | three spaces | ✅ **`paragraph`** |
| ⛔ **T3** | ⚠️ **a tab on a LATER paragraph** | ⛔ **`codeBlock`** |

✅ **CommonMark COUNTS A TAB AS FOUR COLUMNS OF INDENTATION**, ⚠️ **so a single tab meets the indented-code
threshold on its own.** ⛔ **THE TAB IS THEREFORE THE MOST DANGEROUS OF THE THREE OPTIONS, NOT THE
SAFEST** — ⚠️ **it reaches the hazard in ONE keystroke where spaces need four.**

⛔ **RULED OUT: ✅ Scrivi must NOT insert a tab for paragraph indentation.**

### 4D.3 ✅ **Q16: THE USER'S THIRD OPTION WORKS — ⛔ AND IT PUTS ZERO CHARACTERS IN THE FILE**

⚠️ **The user asked: *"or is it possible to simply format the paragraphs with leading spaces?"***
✅ **YES — `NSParagraphStyle.firstLineHeadIndent`, MEASURED in a real TextKit 2 layout:**

| Configuration | line 1 x | line 2 x | ⚠️ Verdict |
| ------------- | -------- | -------- | --------- |
| baseline (no indent) | `5.00` | `5.00` | — |
| ✅ **STORAGE `firstLineHeadIndent = 28`** | ✅ **`33.00`** | ✅ **`5.00`** | ✅ **FIRST LINE ONLY — ⚠️ exactly how a novel indents** |
| ⛔ **RENDERING `firstLineHeadIndent = 28`** | ⛔ `5.00` | `5.00` | ⛔ **NO EFFECT — ✅ §4A.1's rule holds again** |

✅ **THIS IS THE ANSWER TO THE USER'S QUESTION AND IT IS THE BEST OF THE THREE:**

| Option | ⚠️ Characters in the `.md` | ⛔ Code-block risk | ✅ Wrapped lines |
| ------ | -------------------------- | ------------------ | ---------------- |
| ⛔ **Tab** | ⚠️ 1 per paragraph | ⛔ **YES — immediately** | ⛔ **indents the WRAP too** |
| ⚠️ **Leading spaces** | ⚠️ 3 max (⛔ 4 trips it) | ⚠️ **at 4+** | ⛔ **indents the WRAP too** |
| ✅ **`firstLineHeadIndent`** | ✅ **ZERO** | ✅ **NONE — ⛔ nothing to parse** | ✅ **wrap stays at the margin** |

⛔ **AND THE THIRD COLUMN IS NOT A TIE-BREAKER, IT IS DISQUALIFYING FOR THE OTHER TWO:** ⚠️ **leading
characters indent only the FIRST line because the rest is soft-wrapped** — ✅ **which happens to look
right** — ⛔ **but only until the writer edits earlier in the paragraph and the wrap moves.** ⚠️ **The
characters do not follow. `firstLineHeadIndent` is defined in terms of the paragraph, ✅ so it always
follows.**

### 4D.4 ✅ **RULED — Q14 OPTION 2, ⚠️ AND THE PREFERENCE IS THE BETTER HALF OF THE RULING**

✅ **THE USER RULED Q14 = OPTION 2** (suppress block intents Scrivi does not expose, §4C.4)
⚠️ **AND ADDED A PROJECT PREFERENCE FOR PARAGRAPH INDENTATION.** ⛔ **The two are separate mechanisms
and both are needed:**

| | ⚠️ What it does | ✅ Why it is needed |
| - | --------------- | ------------------- |
| ✅ **(a) Suppress unexposed block intents** | ⚠️ **the renderer ignores `codeBlock`/`blockQuote`/`table` and draws prose** | ⛔ **DEFENSIVE — ✅ catches indentation that already exists in files, or arrives by paste** |
| ✅ **(b) `paragraphIndent` preference + `firstLineHeadIndent`** | ✅ **the writer never types an indent at all** | ✅ **PREVENTIVE — ⚠️ exactly the user's reasoning: *"reducing the temptation to enter a character sequence that would lead to unintended results"*** |

✅ **(b) IS THE STRONGER IDEA AND IT GENERALISES BEYOND THIS DEFECT.** ⚠️ **It is the same move §3A.0
made: ⛔ do not ask the writer to type structure, ✅ give her a control and let Scrivi own the bytes.**
⚠️ **⛔ BUT (a) IS STILL REQUIRED** — ✅ **a preference cannot retroactively fix a scene file that already
has four leading spaces in it.**

#### ✅ **WHERE THE PREFERENCE LIVES — ⚠️ the pattern already exists**

✅ **`ProjectPreferences` ALREADY persists per-project display settings** (⚠️ `showChapterTitles`, §5's
F1 row) — ⛔ **so this is not new machinery.** ✅ **`paragraphIndent` joins it as a points value,
⚠️ defaulting to `0` so no existing project changes appearance.**

⛔ **⚠️ AND IT IS F1'S SIBLING, WHICH MATTERS FOR SCOPE:** ✅ **§5.1 ruled Q3 = F1 (the writer picks the
typeface).** ⚠️ **Paragraph indent is the SAME KIND of thing — ✅ a display preference that never touches
the `.md`** — ⛔ **so it belongs in **E3**, not E1, ⚠️ except that E1 needs (a) regardless.**

#### ⛔ **ONE CODE-LEVEL CONSTRAINT, READ NOT GUESSED**

⚠️ **`firstLineHeadIndent` must live in STORAGE (§4D.3), ⛔ and this app STRIPS storage attributes on
undo.** ✅ **READ — `ManuscriptTextView.swift:366-369` sets exactly two attributes:**

```swift
let attrs: [NSAttributedString.Key: Any] = [
    .font: NSFont.monospacedSystemFont(ofSize: NSFont.systemFontSize, weight: .regular),
    .foregroundColor: NSColor.textColor
]
storage.replaceCharacters(in: range, with: NSAttributedString(string: change.newText, attributes: attrs))
```

⛔ **A `.paragraphStyle` WOULD BE DROPPED BY EVERY UNDO/REDO** — ⚠️ **the same guard §4A.2 flagged for
Model B.** ✅ **THE FIX IS CHEAP HERE, unlike Model B's:** ⚠️ **the indent is UNIFORM across the
document (it is a preference, not a per-run decision), ⛔ so it is one more entry in this dictionary,
not a re-derivation.** ✅ **E1/E3 must add it; ⚠️ omitting it means paragraphs lose their indent after
an undo, which reads as a rendering bug and is not.**

⚠️ **⛔ ALSO NOT CHECKED: `rebuildStorage` (`:568-600`) builds body attributes too** — ✅ **the indent
must be applied there as well, ⛔ or it will vanish on the next rebuild rather than on undo.**
⚠️ **Both sites, or neither.**

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

### 5.1 ⛔ **RULED 2026-09-29 — Q3 = F1 ONLY.**

✅ **The user ruled `f1`.** ⚠️ **So "multiple fonts" means ONE THING: ✅ the writer picks the manuscript
typeface, as a preference. ⛔ F3 is OUT — no per-passage font choice, and the format question it would
have forced does not arise.**

⚠️ **⛔ ONE THING THE RULING LEAVES AMBIGUOUS, AND IT IS WORTH A SENTENCE:** ✅ **F2 (headings render
differently from body) is not really a separate FEATURE — it is what "render Markdown" MEANS.**
⛔ **A heading that renders in the body face at body size is not rendered at all.** ⚠️ **So E1's
heading typography proceeds as the mechanism of rendering; ✅ F1 is the ruled user-facing capability,
and F3 is closed.** ⛔ **If the user intends F2 to be excluded as well, that would empty §9's E1/E3 of
their content and should be said** — ⚠️ **the study reads `f1` as "the font FEATURE is F1", not as "do
not style headings".**

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

⛔ **RULED 2026-09-29 (Q1 = YES): renderer first; [EP-032] unblocked and improved by it; ⛔ its
SP-107–SP-114 reservation is NOT released and its planning is NOT discarded.**

⚠️ **AND §3A.0 STRENGTHENS THE CASE THE RULING ACCEPTS:** ✅ **under the escaping rule, an [EP-032]
reference token can only ever be written BY SCRIVI, never typed by accident** — ⛔ **so [EP-032]'s AC1
("still valid Markdown for every reader that does not understand it") gains a guarantee it did not have
when it was written.** ⚠️ **[EP-032]'s SP-107 still owns the reference SYNTAX; ✅ it must now also
honour the escape rule, which is a constraint SP-107 did not previously carry.**

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

### 9.1 ⛔ **WHAT THE 2026-09-29 RULINGS CHANGE ABOUT THIS SPLIT**

⚠️ **The table above predates the rulings. ✅ It survives, ⛔ but three of its rows change content:**

| Epic | ⚠️ What moved | ✅ Why |
| ---- | ------------- | ----- |
| **E1** | ✅ **GAINS the SOURCE↔PRESENTED offset mapping** (§3.4A) · ✅ **GAINS the input-escaping layer** (§3A.0) · ⚠️ **`* * *` is re-read as "make the divider visible" + "a scene-SPLIT command"** (§10.1) | ⛔ **Escapes need the mapping even under Model A, so it cannot wait for E2** |
| **E2** | ⚠️ **NARROWS to marker hiding + the formatting COMMANDS** | ✅ **No input-side parsing of typed markup — §3A.0 removed that whole problem** |
| **E3** | ⛔ **SHRINKS to F1 — ✅ then REGAINS the `paragraphIndent` preference** (§4D.4) | ✅ **Q3 ruled F1; ⚠️ F2 is E1's rendering, not a separate Epic. ✅ Indent is F1's sibling: a display preference that never touches the `.md`** |
| **E1** ⚠️ *(second entry)* | ✅ **ALSO GAINS "suppress unexposed block intents"** (§4C.4 / §4D.4a) | ⛔ **DEFENSIVE and cannot wait for E3 — ⚠️ a preference cannot fix scene files that ALREADY contain four leading spaces** |

⚠️ **⛔ AND ONE ITEM HAS NO OWNER IN THE TABLE:** ✅ **the formatting COMMANDS themselves (Bold, Italic,
Heading, List, Scene Break) are now the ONLY way markup is authored (§3A.0)** — ⛔ **so they are not a
nicety in E2, they are the feature's entire input surface.** ⚠️ **`project_scrivi_app_shape` already
found that *"the toolbar is mostly a SURFACING job: the verbs already exist in the menu bar as callable
closures"*** — ✅ **but these particular verbs do NOT exist yet, and that study's cheapness finding does
not transfer to them.**

⚠️ **E4 IS NOT OPTIONAL AND IS NOT FREE.** ⛔ **`feedback_linux_adopts_apple_shape` says a shape change
on Apple must be made the same way on Linux IN THE SAME WORK.** ✅ **Naming it as its own Epic is how
that rule is honoured without blocking Apple on Qt** — ⚠️ **but it must be SCHEDULED, not assumed.**

---

## 10. ✅ **RULED BY THE USER 2026-09-29 — ⚠️ SIXTEEN QUESTIONS, ✅ ALL ANSWERED**

⚠️ **This section was "Questions this study CANNOT answer."** ✅ **It no longer is. ⛔ Every question is
ruled; the table below is the RECORD, not a proposal.** ⚠️ **Q1–Q9 were the study's own; ✅ Q10–Q14 arose
from the user's 2026-09-29 challenges and were settled by MEASUREMENT (§4B, §4C), not by ruling alone.**
✅ **Q14 was OPENED and CLOSED in the same pass** — ⚠️ **it was not known to exist before the spike;
⛔ it is a block-type change, not a cosmetic one; ✅ and the user's ruling on it (§4D.4) removes the
writer's REASON to type the hazard rather than merely tolerating it.**

| # | Question | ✅ **RULING** | ⚠️ Where it lands |
| - | -------- | ------------ | ----------------- |
| **Q1** | Does the renderer sequence BEFORE [EP-032]? | ✅ **YES** | ✅ §8.2. ⚠️ [EP-032] keeps SP-107–SP-114 and its planning; ⛔ its SP-107 must now honour §3A.0's escape rule |
| **Q2** | Which marker model — A, B, or A-then-B? | ✅ **B (WYSIWYG)** — ⚠️ *"my inclination is to wysiwyg"* | ✅ §4.3. ⛔ **The expensive answer, knowingly taken.** ✅ A ships in E1 as B's first milestone, not as a rival |
| **Q3** | Which "multiple fonts" — F1, F2, F3? | ✅ **F1** | ✅ §5.1. ⛔ **F3 closed** — no per-passage fonts, no format extension needed. ⚠️ F2 read as part of rendering, not as a separate capability |
| **Q4** | Is `* * *` an existing Markdown token in the body, or a UI-only mark? | ⛔ **NEITHER, AS ASKED — a COMMAND** | ✅ §10.1 below |
| **Q5** | Does [I-0206] re-open? | ⛔ **NO** — ⚠️ *"no, let's create a new issue if necessary"* | ✅ §10.2 below |
| **Q6** | Fix the divider NOW (R1) or fold it into E1? | ✅ **NOW** | ⚠️ §9's R1 stands as a standalone ISSUE, ⛔ not an Epic. ✅ **But see §1.4A — the colour fix already shipped and DID NOT WORK; the cause is still unidentified** |
| ~~**Q7**~~ | Marker hiding + parser choice | ✅ **CLOSED BY SPIKE 2026-09-28** — §4A | ⛔ **(a) rendering attributes cannot hide a marker** · ✅ **(b) `AttributedString` is the parser** |
| **Q8** | Is `#` a CHAPTER heading? | ✅ **NO** | ✅ §3A.5. ⚠️ **Doubly closed: chapter titles stay metadata, ⛔ and a typed `#` is escaped anyway (§3A.0)** |
| **Q9** | Enter → `\n\n`, or render a single `\n` as spaced? | ✅ **YES — Enter inserts `\n\n`** | ✅ §3A.6. ⚠️ **Plus a ruling not asked for: Backspace at paragraph start deletes ONE `\n`** |
| ✅ **NEW** | ⚠️ **Does typed markup become formatting?** | ⛔ **NO — ALL typed reserved characters are ESCAPED** | ✅ **§3A.0. ⚠️ The user's own ruling, volunteered, on Q7 spike evidence. ⛔ It is the largest ruling in this pass** |
| ✅ **NEW** | ⚠️ **Is the caret position the file offset?** | ⛔ **NO — they are separate spaces** | ✅ **§3.4A. ⚠️ Volunteered. ⛔ It makes a SOURCE↔PRESENTED mapping a REQUIRED E1 component** |
| ✅ **Q10** | ⚠️ **Does `AttributedString(markdown:)` also remove ESCAPES?** — ⛔ *"let's not add work that isn't needed"* | ✅ **YES, it removes them; ⛔ and it does NOT say where they were** | ✅ **§4B.1–4B.3, MEASURED 2026-09-29. ⚠️ Cost is REAL but ~12 lines, ⛔ not a subsystem** |
| ✅ **Q11** | ⚠️ **Which characters get escaped?** | ⛔ **ALL 32 — *"use the READ set"*** | ✅ **§4B.4. ⚠️ ONE list, not two; ⛔ accepted cost is a noisier raw `.md`. ✅ Re-measured: the oracle goes 9/10 → 10/10** |
| ✅ **Q12** | ⚠️ **Trailing spaces + Enter → an unwanted hard break?** | ✅ **CONFIRMED. ⛔ Normalise on Enter** | ✅ **§4B.5–4B.6. ⚠️ User ruled both halves; ⛔ study amends "delete one space" → "reduce to at most one"** |
| ⛔ **Q13** | ⚠️ **Does the parser conflate interior runs of spaces?** | ⛔ **NO — they are PRESERVED** | ✅ **§4C.1, MEASURED. ⚠️ The user's forward reasoning was sound but the parser does not do it; ⛔ eleven spaces stay eleven** |
| ✅ **Q14** | ⚠️ **Are leading spaces merely untidy?** | ⛔ **NO — ≥4 (or ONE TAB) makes it a `codeBlock`.** ✅ **RULED: OPTION 2 + an indent PREFERENCE** | ✅ **§4C.3, §4D. ⚠️ Suppress unexposed block intents (defensive) ✅ AND give the writer a `paragraphIndent` preference (preventive)** |
| ⛔ **Q15** | ⚠️ **Insert a TAB for paragraph indentation?** | ⛔ **NO — ⚠️ ONE tab triggers a code block** | ✅ **§4D.2, MEASURED. ⛔ The tab is the WORST option — it reaches the hazard in one keystroke where spaces need four** |
| ✅ **Q16** | ⚠️ **Can indentation be pure formatting, no characters?** | ✅ **YES — `firstLineHeadIndent` in STORAGE** | ✅ **§4D.3, MEASURED. ✅ ZERO characters in the `.md`, ⚠️ first line only, ✅ wrap stays at the margin** |

### 10.1 ⚠️ **Q4 ruled — `* * *` IS A STRUCTURAL SCENE BREAK, AUTHORED BY COMMAND**

⚠️ **The user:** ***"No. The `* * *` is a scene break, which has a structural definition in Scrivi
external to Markdown. If the writer wants to insert a break we should provide the capability as with
"Bold" "Italic" etc."***

✅ **⛔ THE QUESTION AS POSED OFFERED A FALSE CHOICE, AND THE USER REJECTED BOTH HORNS.** ⚠️ **Q4 asked
"body token OR UI-only mark"; ✅ the answer is that the break is STRUCTURE — it already exists in Scrivi
as the boundary between scene FILES — ⛔ and the writer's need is a COMMAND that creates that structure,
not a glyph.**

| ⚠️ What Q4 assumed | ✅ What was ruled |
| ------------------ | ---------------- |
| ⛔ the writer would type `***` or `---` into the prose | ✅ **the writer invokes a SCENE BREAK command** |
| ⛔ the renderer's job is to recognise a token | ✅ **the renderer's job is to DRAW the structural boundary that already exists** |
| ⚠️ it was a FORMAT decision | ✅ **it is not — ⛔ nothing new goes into the `.md` at all** |

✅ **THIS IS CONSISTENT WITH §2.2 AND COSTS NOTHING NEW:** ⚠️ **scene dividers ALREADY render outside
`sceneBoundaries` and are already invisible to the save path.** ✅ **The divider IS the `* * *`.**
⛔ **So §9's E1 item *"`* * *` scene break"* is re-read: it is **(a)** make the existing divider VISIBLE
(that is R1 / §1.4A, still unsolved) and **(b)** add a command that SPLITS a scene at the caret.**

⚠️ **⛔ AND (b) IS NOT A RENDERING FEATURE — it is a scene-split operation in the core.** ✅ **Scrivi
already has the inverse (`scrivi_merge_scene`, `project_sp074_merge_endpoints`).** ⛔ **Whether a split
endpoint exists was NOT CHECKED in this pass and E1 must check it before scoping.**

### 10.2 ⚠️ **Q5 ruled — [I-0206] STAYS CLOSED; ✅ a NEW Issue if it bites**

⚠️ **The user:** ***"no, lets create a new issue if necessary."***

✅ **RULED, AND IT IS THE RIGHT LAYER DISCIPLINE** — ⚠️ **re-opening a closed Issue on a PREDICTION
would make its record say something the evidence did not.** ✅ **A new Issue, filed against measured
behaviour under the renderer, is the honest artefact.**

⛔ **BUT THE STUDY OWES ONE WARNING IT WILL NOT SOFTEN:** ⚠️ **[I-0206] measured `~59 ms` per keystroke
at `setSelectedRange`, offset-linear, on a 1.85 MB document.** ⛔ **Q2 = B and §3.4A's mapping BOTH add
work to that exact path.** ✅ **E1 must MEASURE the caret path against a real 1.85 MB manuscript before
E2 commits** — ⚠️ **`project_read_amplification_class`: a cost measured in isolation is not a cost
measured in place.** ✅ **If that measurement is skipped, the "new Issue if necessary" will be filed by
the user against their own manuscript rather than by a test.**

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
- ✅ **ANSWERED 2026-09-29 (§4B) — THE ESCAPE COST WAS MEASURED, at the user's challenge.** ⛔ **The
  parser DOES strip escapes and does NOT locate them; ✅ a ~12-line scanner reproduces it exactly.**
  ⚠️ **WRITE set = markdown.org's 16 (user-directed); ⛔ READ set = all 32 ASCII punctuation (measured).**
  ⛔ **STILL OWED: the oracle is verified on 9 inputs, ⚠️ and its 10th disagreed** — ✅ **E1 must test it
  against the parser over a CORPUS (`feedback_boundary_tests_not_facade`), ⛔ not assume it.**
- ⛔ **NEW 2026-09-29 — PASTE IS UNRULED.** ⚠️ **The ruling's words are *"that the user types"*; ✅ paste
  is a second door and is not covered.**
- ⛔ **NEW 2026-09-29 — EXISTING MANUSCRIPTS ARE UNMIGRATED.** ⚠️ **Scene files already contain
  unescaped `*` typed as arithmetic or emphasis; ✅ under the new renderer they will silently change
  appearance.** ⛔ **Whether a one-time escape pass is owed is not ruled.**
- ⛔ **NEW 2026-09-29 — A SCENE-SPLIT ENDPOINT WAS NOT LOOKED FOR.** ⚠️ **§10.1 makes the scene-break
  COMMAND a core operation; ✅ the merge side exists (`scrivi_merge_scene`), ⛔ the split side was not
  checked.**
- ⛔ **NEW 2026-09-29 — `rebuildStorage` WAS NOT READ FOR THE INDENT.** ⚠️ **§4D.4 READ the undo path
  (`:366-369`) and confirmed a `.paragraphStyle` would be stripped; ⛔ it did NOT read `rebuildStorage`
  (`:568-600`), which builds body attributes too.** ✅ **Both sites must apply the indent, ⛔ or it
  vanishes on rebuild instead of on undo.**
- ⛔ **NEW 2026-09-29 — THE TWO PARSING MODES DISAGREE ON WHITESPACE AND THE STUDY QUOTES BOTH.**
  ⚠️ **§4A.3/§4B used `.inlineOnlyPreservingWhitespace`; §4B.5/§4C/§4D used `.full`.** ⛔ **Trailing and
  leading spaces are DROPPED by one and PRESERVED by the other (§4C.2).** ✅ **E1 must pick ONE and
  state it** — ⚠️ **a study that mixes them will mislead, and this one currently does.**
- ⛔ **No export impact assessed.** ⚠️ **[EP-032]'s Q5 already records that manuscript export has no
  existing path; ✅ the same gap applies here and is not re-litigated.**

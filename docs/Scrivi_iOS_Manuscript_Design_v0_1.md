# Scrivi — The Manuscript on iOS — Design v0.1

**Status:** ✅ **Q1 RULED (c)** (user, 2026-10-30), and no undo on iOS in S1 — written 2026-10-30 for [SP-198] / [T-0632] ([EP-055] AC1).
Implemented in S1: `ManuscriptEditing.swift` (A, B, C1, C2) and `ManuscriptUITextView.swift` (D).
**Scope:** the manuscript editor's HOST — what `ManuscriptTextView.swift` and its companions do, which parts iOS can share,
and which it must re-implement. The rest of the app's surfaces are audited before EP-055 S3.
**Standing rulings it works under:** [EP-055] Q8 / [SP-198] R3: size classes and the system's defaults only, nothing
device- or keyboard-specific. `feedback_linux_adopts_apple_shape`: the Mac is the reference; iOS must not drift from it.

---

## 1. What the Mac host is (read 2026-10-30)

`ManuscriptTextView.swift` is 3,718 lines in four parts:

| Part | Lines | What it does |
| ---- | ----- | ------------ |
| `ManuscriptTextView: NSViewRepresentable` | `:37–284` | builds the `NSTextView` + `NSScrollView`, wires the presenter, the session's menu actions, the scroll/frame observers; `updateNSView` rebuilds storage when the scene list, the titles toggle, a chapter title or the typography changes |
| `Coordinator: NSTextViewDelegate` | `:287–2609` | the model sync (`textDidChange`, `textViewDidChangeSelection`), rendering (`renderedManuscript`, `renderRun`), autosave, history capture/undo apply, structural edits, cross-scene cut/copy/paste, buffers, boundary navigation, scroll restore and centring, TextKit 2 geometry, the fork popover and branch marks |
| `ManuscriptNSTextView: NSTextView` | `:2614–3412` | **the write path**: `shouldChangeText` (`:2648`), `insertText` (`:2839`), `insertNewline` (`:2863`), Option-Return (`:2897`), `deleteBackward`/`deleteForward` (`:3305,3374`); pasteboard read/write; Find; Format commands; key bindings (`keyDown` `:3223`); undo routing |
| Shared types | `:3414–3697` | `SceneDivider`, `StructuredClipboard`, `SceneBoundaryTable`, `DividerTextAttachment` |

Companions: `ManuscriptPresenter` (584, TextKit 2 delegates), `ManuscriptPresentedMap` (483), `ManuscriptTypography` (260),
`ManuscriptAccessibility` (296), `ManuscriptFind` (192), `ManuscriptCommands` (100), and the already-neutral
`MarkdownBlocks`, `MarkdownEmphasis`, `ManuscriptEscapes`.

## 2. The finding that decides the shape

⚠️ **What reaches the `.md` file is decided inside `ManuscriptNSTextView`'s AppKit overrides,** not in shared code:

- a typed character is stored **escaped** (`insertText` → `MarkdownEscapes.escape`), with the empty pending-pair rule;
- Return stores a **paragraph** (`\n\n`), trims trailing spaces, collapses a trailing `\\`, continues a list;
- Option-Return stores a **hard break** (`\` + `\n`);
- ⌫ / ⌦ treat markers as **atomic**, join paragraphs with **one space**, never remove a divider;
- `shouldChangeText` **refuses** an edit that touches a chapter title or a divider ([I-0270]: deleting a divider saved one
  scene's text into another's file), **snaps** an escape pair, and **re-issues** an unbalancing edit as a balanced one.

A `UITextView` reaches the same moments through `UIKeyInput` (`insertText(_:)`, `deleteBackward()`; Return arrives as
`insertText("\n")`) and `textView(_:shouldChangeTextIn:replacementText:)`. **If iOS re-wrote these, the two platforms
would write different bytes for the same keystrokes** — exactly what [EP-049] had to repair on Linux.

## 3. Dispositions

| # | Disposition | What |
| - | ----------- | ---- |
| **A** | **Shared as is** (compile for iOS, no change) | `MarkdownBlocks`, `MarkdownEmphasis`, `ManuscriptEscapes`; `SceneDivider`, `SceneBoundaryTable`, `StructuredClipboard`, `DividerRenderState`, the `.scriviHeading` / `.scriviDivider` keys |
| **B** | **Shared after a font / colour / image swap** | `ManuscriptPresenter`, `ManuscriptPresentedMap`, `ManuscriptTypography`'s drawing half (`:129–260`), `DividerTextAttachment` (its TextKit 2 API is the same on both; it draws with `NSImage`/`NSBezierPath`/`NSColor`) |
| **C1** | **Moved out of the host into shared code — the write path** | every decision in §2, as functions of (storage, selection, presenter, input) returning *let the system do it* / *refuse* / *replace this range with this stored text and put the caret here*. The Mac overrides call them and apply the result exactly as today. |
| **C2** | **Moved out of the host — the model sync** | the render (`renderedPrefix`, `renderedManuscript`), and what an edit and a caret move do to the loader: extract the scene's text, `updateText`, history `noteEdit` / `caretMoved`, the 1 s autosave, the live title, the viewport scene and cursor offsets. None of it needs the view, only the storage and the selection. |
| **D** | **Re-implemented per host** | the view and scrolling (`NSScrollView` + clip view vs `UITextView`, which *is* the scroll view); TextKit 2 geometry in view coordinates; restoring and centring the writer's place; focus; pasteboard (`NSPasteboard` vs `UIPasteboard`); menus and key commands; the fork popover, branch marks and alerts; the Find bar (`NSTextFinder` vs `UIFindInteraction`); accessibility (`NSAccessibility` vs `UIAccessibility`) |

## 4. What S1 needs, and what waits

- **S1 ([SP-198]):** A, B, C1, C2, and D's view, scrolling and place-restore. The writer reads, scrolls, types with the Mac's
  storage rules, and the scene autosaves; dividers and chapter titles show and cannot be edited away.
- **Later:** structural edits (⌘↩ etc.), undo/redo and the fork chooser, cross-scene cut/copy/paste, buffers, Find, the
  Format commands, accessibility, key commands → EP-055 S2/S3, each moving its part out of the Mac host the same way when
  it is reached.
- **Undo in S1:** the Mac turns AppKit's undo off (`allowsUndo = false`) because Scrivi's history owns undo. iOS does the
  same, so **S1 has no undo** until the history apply path is shared (S2). Shake-to-undo and the keyboard's undo key do
  nothing rather than undo behind the history's back.
- **Smart quotes:** the Mac forces them off ([I-0283]: they corrupted escapes). iOS sets `smartQuotesType = .no` for the
  same reason. Smart dashes are measured on the simulator, not assumed.

## 5. Q1 — the shape

| Option | What | Verdict |
| ------ | ---- | ------- |
| (a) | A `UITextView` subclass that re-implements §2 and a slim iOS coordinator | ⛔ Two write paths that must stay byte-identical by hand: the EP-049 defect, built in |
| (b) | Make the whole `Coordinator` and text view compile on both, via typealiases | ⛔ Half of the Coordinator is geometry and AppKit UI; `#if` would land in most functions |
| **(c)** | **Move C1 and C2 into shared files; a new, thin `UITextView` host; the Mac host calls the same code** | ✅ **Recommended.** One write path. ⚠️ It edits the Mac's write path, which is guarded by the interop suite (325 tests, 55 of them driving `ManuscriptNSTextView`'s typing paths) and a Mac live pass |

## 6. Tests

[T-0635]'s iOS target runs the shared C1/C2 code on the iOS simulator: typed text stored escaped, Return a paragraph, a
divider never deleted, a heading refused, the scene text reaching the loader. The Mac's interop tests keep running the
same functions through `ManuscriptNSTextView`.

## 7. Measured while building it (2026-10-30)

- ⚠️ **UIKit does not ask `textView(_:shouldChangeTextIn:replacementText:)` for every `insertText` / `deleteBackward`.** The iOS
  overrides pass `ManuscriptEditRules.verdict` themselves before calling `super` (an escape pair was half-deleted otherwise).
- ⚠️ **`UITextView(usingTextLayoutManager:)` on a subclass left its stored properties uninitialised** (they read as nil — the
  pasteboard, the undo manager). `ManuscriptUITextView` has a designated `init()`; with no container it is TextKit 2.
- ⚠️ `disableUndoRegistration()` did not stop UITextView registering; the iOS view's undo manager refuses instead.

## 8. Panels, gestures and the toolbar (ruled 2026-10-31)

The user, 2026-10-31: the iPhone and Duo passes wait until the Navigator, Inspector and Timeline can be hidden, because
at compact width *"the display isn't good for anything but the Manuscript view."* The rulings below came from four questions,
and each one took the recommended option.

**Size classes** (Apple has no "expanded"; each axis is compact or regular). iPhone portrait is width compact; iPhone
landscape is width compact except Plus/Max; a full-screen iPad is regular/regular; a narrow iPad window is width compact.
The Duo is to be measured on the simulator. ⚠️ The layout branches on **horizontal size class only**, so a narrow iPad
window behaves like a phone.

| | Width compact | Width regular |
| - | - | - |
| **Layout** | One navigation stack: **Navigator → Manuscript → Inspector**, the Inspector a **pushed page** (G3), not the system sheet `.inspector` gives (Apple: *"adapt to a sheet in a horizontally compact size class"*). It opens on the Manuscript. | A two- or three-panel split: the Navigator sidebar, the Manuscript, and the Inspector as the `.inspector` trailing column (the Mac's shape). ⏳ Which one looks best is to be judged on the simulator. |
| **Swipe right** | Previous page (the system back swipe) | From the left edge: show the Navigator. Ending at the right edge: hide the Inspector. |
| **Swipe left** | Next page (custom: there is no system forward swipe) | From the right edge: show the Inspector. Ending at the left edge: hide the Navigator. |
| **Full-width swipe** | — | ✅ **G1**: the edge rules combine. A swipe right across the screen shows the Navigator and hides the Inspector; a swipe left across does the reverse. |
| **Timeline** | ✅ **G2**: a grabber strip above the bottom edge. Swiping up on it shows the Timeline and swiping down hides it. | same |

- ⚠️ **Why G2 is a grabber.** A one-finger vertical swipe scrolls the manuscript, and so does a two-finger one. The bottom edge
  is Home and the app switcher, and the top edge opens Notification Center and Control Center.
- ✅ **G4, the way without gestures:** the iPad menu bar's View menu with the Mac's shortcuts (⌥⌘I, ⌥⌘T), VoiceOver actions,
  and Show Inspector and Show Timeline in the toolbar's ⋯ menu. ⛔ No show/hide buttons on the toolbar. A gesture is never
  the only way (porting outline §4.7).
- **The toolbar** carries the Mac's buttons (`ProjectWindowManager.swift`: the Scene and Chapter groups, Worlds, Project
  Settings) at the **system's default placement**. It sits at the top on iPad and iPhone and down the right side on the Duo,
  wherever the system puts it. ⛔ There is no placement code ([SP-198] R3).

*Last Updated: 2026-10-31 (§8: panels, gestures and the toolbar ruled.)*

*Last Updated: 2026-10-30 (Q1 ruled; §7 what the build measured.)*

*Last Updated: 2026-10-30 (v0.1 written for T-0632.)*

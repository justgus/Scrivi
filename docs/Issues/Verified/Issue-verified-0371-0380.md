# Verified Issues: I-0371 – I-0380

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0361–I-0370 are in [`Issue-verified-0361-0370.md`](Issue-verified-0361-0370.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0371** | `[Apple]` The object detail sheet on macOS showed only Save and Close: Undo Changes and SP-239's ⋯ (Change to Artifact/Item, Delete) were missing (user, 2026-11-23, SP-239 live pass step 2). **Cause:** the sheet draws `.cancellationAction` items in a **bottom** button bar, which shows one view per placement: the group showed only Save, and so did three separate items (2nd try). **Fix:** one `ToolbarItem` holding an `HStack` (`Scrivi/Views/Detail/ObjectDetailSheet.swift:452`). macOS build green; user saw all four buttons, Mac, iPad and iPhone. | Medium | [SP-239] | ✅ **Verified** (2026-11-23) |
| **I-0372** | `[Apple]` The object detail sheet's bottom bar (Save · Undo Changes · ⋯ · Close) was missing on every object opened FROM the sheet — a related object pushed by double-click showed no buttons until Back returned to the first (user, 2026-11-23: *"in step 6 when I double clicked L, L appeared, but there were no bottom bar buttons.  Same for the subsequent double click on test key.  Back button twice and it showed."*). **Cause:** `.toolbar { detailToolbar }` was attached to the `NavigationStack`'s root view only; toolbar items belong to the view that declares them, so each `navigationDestination` level had none. Likely since [T-0547] (2026-09-23). **Fix:** the destination's `editorSurface` declares the same toolbar (`Scrivi/Views/Detail/ObjectDetailSheet.swift:216`). re-pass: the bar on every pushed level, also with L → test key loaded twice and the second deleted (user, 2026-11-23). | Medium | [SP-240] | ✅ **Verified** (2026-11-23) |
| **I-0373** | `[Apple]` **iOS: an overflowed toolbar menu shows TWO headers.** Found in [SP-243]'s session (user, 2026-11-24: *"when the phone is unfolded and the toolbar buttons (now menus) are selected, then the menu has two title items."*). [SP-242] added `menuHeader` (`ProjectToolbar.swift`) to every toolbar menu on the Mac and iPad (width regular); when the system moves an item into the ⋯ overflow it adds ITS OWN header, so the menu shows both. Folded (width compact) is right: `menuHeader` is left out there. Predicted in [SP-242]'s record (*"An iPad item the system overflows into ⋯ shows both"*). ⚠️ **Research reopened** (user, 2026-11-24: *"I didn't decide that! I said research a way. … I'm certain there is a way for a toolbar to know if it is displaying a button or an overflow menu"*): a first keyword pass over six doc pages was NOT research, and the Mac-only fallback built on it was reverted the same day. **Measured 2026-11-24** (a probe build in a scratch copy, iPad mini A20 Pro sim, iOS 27.2, six dummy menus forcing overflow): ① **16 environment values** read inside each menu's content (`toolbarLabelStyle`, `labelsVisibility`, `menuIndicatorVisibility`, `labelReservedIconWidth`, both size classes, `controlSize`, `isPresented`, `imageScale`, `symbolVariants`, `dynamicTypeSize`, `menuOrder`, `isEnabled`, `buttonSizing`, `font`, `labelIconToTitleSpacing`) are IDENTICAL for an item on the bar and one in the overflow — SwiftUI builds every menu's content up front, before any placement; ② the toolbar **labels' `onAppear`/`onDisappear`/geometry** are identical too (all rendered off-window, then handed to UIKit). ⚠️ **On the iPad an overflowed menu opens as a CASCADING submenu with NO system header** — only ours shows; the double header is the **iPhone-idiom** overflow (the Duo unfolded), where the system pulls the item into the top of the menu. **Duo simulator, unfolded, portrait** (user, 2026-11-24: *"Lets look at it on the Duo simulator unfolded in portrait orientation"*; iOS 27.2): reproduced — an overflowed Merge shows the system header ("Merge ⌄") above ours; View on the bar shows ours alone. The log shows every menu's content AND every label evaluated ONCE, together, when the project opened (16:15:36), with identical environments for bar and overflow items; **opening View from the bar (16:15:43) and Merge from ⋯ (16:16) evaluated NOTHING** — SwiftUI hands UIKit finished menus, so menu content cannot react to where it is shown. **Fix** (user, 2026-11-24: *"Can you log a boolean when the overflow button is clicked? can it be cleared when the overflow menu is dismissed?"*): a window recognizer that never takes the touch (`ToolbarOverflowWatchInstaller`, `ProjectToolbar.swift`, installed in `EditorView` beside `PanelSwipeInstaller`) looks at every touch in the navigation bar: the touched button whose image IS (`===`) one of the bar's own items' `UIBarButtonItem.image` (public `UINavigationBar.topItem` item groups) is ours, any other is the overflow button; `ToolbarOverflowWatch.overflowOpen` follows, and `ToolbarMenuHeader` drops our header while it is set — SwiftUI rebuilt the menus in 14 ms, before the submenu opened. iPhone-type devices only (the iPad's cascade has no system header). ⚠️ The button's title label cannot tell them apart (empty for every icon-only button) — tried first, failed. **Verified on the Duo simulator** (unfolded, portrait, real build): overflowed Merge shows ONE header (the system's); View from the bar shows ours; the cycle repeats. macOS + iOS build; iOS 38 + interop 355 green. **Verify:** the same on Remus. | Low | — | ✅ **Verified** 2026-11-24 (user, 2026-11-24: *"Steps 1-4 pass on Remus."*) |
| **I-0374** | `[Apple]` **iPad: typed asterisks become formatting** — `**two*` turned italic and the last `*` was stored escaped; `**two**` typed elsewhere turned bold. The escape ruling says a typed `*` is always literal (the Mac and Linux keep it). Found in [SP-244]'s live pass → [details](#i-0374) | Medium | [SP-244] | ✅ **Verified** |

*Last Updated: 2026-11-24 (**I-0374** archived with [SP-244]'s close (user, 2026-11-24: *"close SP-244"*).)*

*Last Updated: 2026-11-24 (**I-0373** archived — verified on Remus (user, 2026-11-24: *"Steps 1-4 pass on Remus."*).)*

*Last Updated: 2026-11-23 (**I-0372** archived with [SP-240]'s close.)*

*Last Updated: 2026-11-23 (**I-0371** archived with [SP-239]'s close.)*

---

## I-0374: `[Apple]` iPad: typed asterisks become formatting

**Reported:** the user, 2026-11-24, [SP-244]'s live pass step 5 on the iPad (that step wrongly told the writer to TYPE `**two**`;
under the escape ruling a typed `*` is literal, which the Mac and Linux did): *"On Opt-Return I don't see a backslash. but the
cursor advances to the next line. **two* changes the text to italic and advances another line. The final asterisk has the
backslash in front of it. Oddly enough if, on the iPad, I just go to a random position and type **two** then the two becomes
bold. same for *italic*."*
**Expected:** typed `*` is stored escaped (`\*`) and shown as an asterisk, as on the Mac and Linux (`ManuscriptCommands.swift:5`,
the escape ruling); formatting enters only through Format commands.
**Read from the code, NOT a diagnosis:** `ManuscriptUITextView.insertText` escapes typed text (`MarkdownEscapes.escape`, `:834`,
`:839`) — EXCEPT while the keyboard holds marked text, which goes to `super.insertText` unescaped (`:815`). A keyboard that
composes (predictive text, a hardware keyboard's dead keys, Smart Punctuation) could take that path. The extra line advance is
not explained. ⚠️ Reproduce and log (`[SCRIVI-CARET] insertText … marked=`) before naming a cause.
**Also seen:** Option-Return on the iPad is an ordinary Return — iOS has no hard-break key (the Mac's is
`insertNewlineIgnoringFieldEditor:`, Linux's Alt-Return). Not a defect unless ruled one.
**Cause (2026-11-24, [SP-244]):** text UIKit puts in by ITSELF — autocorrect, predictive and inline completions, text
replacements — arrives at `UITextInput.replace(_:withText:)`, not `insertText`, and the view did not override it: the
delegate's `shouldChangeTextIn` ran the rules and returned `true`, so UIKit inserted the text AS GIVEN, unescaped. Shown by a
test, red before the fix: `tv.replace(range, withText: "**two**")` stored `**two**`. (Not proved to be the iPad's exact
path — the device log will say: `[SCRIVI-CARET] replace …` in Debug.) The extra line advance is not explained; re-check it.
**Fixed:** `ManuscriptUITextView.replace(_:withText:)` escapes and stores through the rules, as `insertText` does (marked
text still goes to UIKit, as before). **Test:** iOS `replacementEscapes`. The Mac already escapes there
(`insertText(_:replacementRange:)`).
**Re-reported (user, 2026-11-24, hardware keyboard):** typing `**test**`, a space appeared between the last two `*`, and
arrowing back showed half a formatting hint. **The device log named the real route:** each `*` reached the delegate's
`shouldChangeTextIn` with NO `insertText` or `replace` after it, and storage grew by 1 (unescaped). A hardware key asks the
delegate with the TYPED text BEFORE `insertText`; the rules judged a bare `*` as markup (`balancedEdit`), stored it
themselves and returned `false`, so `insertText` never ran. The space is that balanced edit's work.
**Fixed (2):** the delegate judges typed text escaped, as `insertText` will store it — `ManuscriptUITextView.shouldChangeTyped`.
**Test:** iOS `hardwareKeyEscapes` (the key path: verdict, then `insertText`). The `replace` fix stays. Diagnostic overrides removed.
**Status:** ✅ **Verified 2026-11-24** (user, 2026-11-24: *"Ok this now works on both the iPhone with a software keyboard and the iPad with the hardware keyboard."*). Archived when [SP-244] closes.

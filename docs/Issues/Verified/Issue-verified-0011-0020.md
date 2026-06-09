# Verified Issues — I-0011 to I-0020

---

## I-0011: Chapter title headings editable; text leaks into scene content on toggle

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `ManuscriptTextView.swift`
**Severity:** High
**Sprint:** SP-033

**Description:**
When "Show Chapter Titles in Manuscript" is enabled, chapter title headings were fully editable. Typing or deleting in a heading modified `NSTextStorage`, and when the toggle was turned off and `rebuildStorage` called again, the modified heading text was interpreted as scene content and permanently written into the manuscript. Scene boundaries could also be destroyed by deleting the separator attachment character.

**Root Cause Analysis:**
Heading text was appended to `NSTextStorage` as plain `NSAttributedString` with no edit protection. `recomputeBoundaries` only scans for `NSTextAttachment` characters, so heading text was invisible to boundary tracking. Additionally, `deleteBackward:` had duplicate and incorrect guard logic allowing the separator attachment character itself to be deleted, destroying scene boundary structure.

**Resolution:**
- Custom `.scriviHeading` attribute applied to all heading characters in `rebuildStorage`.
- `shouldChangeText(in:replacementString:)` override blocks any edit touching a `.scriviHeading` range.
- `deleteBackward:`/`deleteForward:` rewritten using a single `isSeparatorPosition(_:in:)` helper that correctly blocks deletion of both the attachment character and its trailing `\n`.

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift`

**Verification:**
- ✅ Typing in a chapter heading is blocked
- ✅ Delete in a chapter heading is blocked
- ✅ Toggle off after placing cursor in heading — scene text unchanged
- ✅ Cursor at scene divider, delete backward/forward — divider not deleted

---

## I-0012: First chapter title never shown when "Show Chapter Titles" is enabled

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `ManuscriptTextView.swift`
**Severity:** Medium
**Sprint:** SP-033

**Description:**
When "Show Chapter Titles in Manuscript" is enabled, the title for the first chapter was never rendered. Only Chapter 2 and beyond showed headings.

**Root Cause Analysis:**
In `rebuildStorage`, the heading insertion block was nested inside `if i > 0`, unconditionally skipping the first segment. Chapter headings were only inserted at transitions between segments, never before the first scene.

**Resolution:**
`isChapterBoundary` is now computed for all `i` (`i == 0 || segments[i-1].chapterID != seg.chapterID`). The divider insertion remains inside `if i > 0`. The heading insertion fires whenever `showTitles && isChapterBoundary`. For `i == 0`, the heading string omits the leading `\n` so Chapter 1's title sits flush at the document top.

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift`

**Verification:**
- ✅ Chapter 1 heading appears at top of manuscript when toggle is enabled
- ✅ Chapter 2+ headings still appear correctly
- ✅ Toggle off — no heading text leaks into scene content

---

## I-0013: Project settings (toggle, title, subtitle) do not persist between app launches

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `AppEnvironment.swift`, `ProjectSettingsSheet.swift`, `ProjectPreferences.swift`
**Severity:** High
**Sprint:** SP-033

**Description:**
Per-project settings (Show Chapter Titles toggle, project title, project subtitle) were not saved to disk and reset on every launch. Window position and size were also not restored. Window maximized state restoration is split to I-0017 (backlog).

**Root Cause Analysis:**
`showChapterTitles` was held in transient `@State` in `ManuscriptEditorView` with no persistence layer.

**Resolution:**
Introduced `ProjectPreferences` — `@Observable`, loads/saves to `UserDefaults` keyed by `projectID`. Holds `showChapterTitles`, `projectTitle`, `projectSubtitle`. Created by `AppEnvironment` on `openProject`. `WindowFrameAutosave` saves/restores window frame via `NSWindowDelegate`. Window maximized state (I-0017) remains open.

**Files Affected:**
- `Scrivi/App/ProjectPreferences.swift` (new)
- `Scrivi/App/WindowFrameAutosave.swift` (new)
- `Scrivi/App/AppEnvironment.swift`
- `Scrivi/App/ScriviApp.swift`
- `Scrivi/Views/EditorView.swift`

**Verification:**
- ✅ "Show Chapter Titles" toggle persists across launches
- ✅ Project title persists across launches
- ✅ Project subtitle persists across launches
- ✅ Window position and size restored on relaunch

---

## I-0014: Navigation panel shows internal project ID instead of project title

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `SceneNavigatorView.swift`, `EditorView.swift`
**Severity:** High
**Sprint:** SP-033

**Description:**
The navigation panel header showed "Scrivi" as the title and a raw internal UUID as the subtitle. Internal IDs must never be shown to the writer.

**Root Cause Analysis:**
`EditorView` toolbar read `env.openProjectResult?.projectID` directly. No project title was surfaced through `OpenProjectResult` or stored in a persistent Swift-side layer.

**Resolution:**
`SceneNavigatorView` now shows a `projectHeader` above the scene list: `prefs.projectTitle` (fallback "Untitled") and `prefs.projectSubtitle` if set. Raw `projectID` display removed from the toolbar.

**Files Affected:**
- `Scrivi/Views/SceneNavigatorView.swift`
- `Scrivi/Views/EditorView.swift`

**Verification:**
- ✅ Navigator header shows project title (or "Untitled" if not set)
- ✅ Subtitle shown when set, hidden when empty
- ✅ No internal IDs visible anywhere in the UI

---

## I-0015: No way to edit project title or subtitle

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `ProjectSettingsSheet.swift`
**Severity:** Medium
**Sprint:** SP-033

**Description:**
No UI existed to set or change project title or subtitle.

**Root Cause Analysis:**
`ProjectSettingsSheet` only contained the chapter titles toggle.

**Resolution:**
`ProjectSettingsSheet` rewritten with a "Project" section containing Title and Subtitle text fields, both bound to `ProjectPreferences` via `Bindable`. Changes persist immediately to `UserDefaults`.

**Files Affected:**
- `Scrivi/Views/ProjectSettingsSheet.swift`

**Verification:**
- ✅ Project Settings shows Title and Subtitle fields
- ✅ Changes persist immediately and survive app relaunch
- ✅ Title shows in Navigator header after setting

---

## I-0020: Project name and subtitle should appear in the window title bar

**Status:** ✅ Resolved - Verified
**Platform:** macOS, iPadOS
**Component:** `EditorView.swift`, `SceneNavigatorView.swift`
**Severity:** Low
**Sprint:** SP-035

**Description:**
Project title and subtitle only appeared in the Scene Navigator sidebar header. The window title bar showed the generic app name.

**Root Cause Analysis:**
`ManuscriptEditorView` did not call `.navigationTitle()` or `.navigationSubtitle()`. The project title was only rendered inside `SceneNavigatorView.projectHeader`.

**Resolution:**
- Added `.navigationTitle` and `.navigationSubtitle` on the detail pane `VStack` in `ManuscriptEditorView` (`EditorView.swift`).
- Simplified `projectHeader` in `SceneNavigatorView` to show only the project title (subtitle removed; it now appears in the title bar subtitle slot).

**Files Affected:**
- `Scrivi/Views/EditorView.swift`
- `Scrivi/Views/SceneNavigatorView.swift`

**Verification:**
- ✅ Project title appears in window title bar when a project is open
- ✅ Subtitle appears in title bar subtitle slot
- ✅ Landing View title bar shows app default (no project open)

---

## I-0021: Chapter headers in Scene Navigator are visually indistinct from scenes

**Status:** ✅ Resolved - Verified
**Platform:** macOS, iPadOS, iOS
**Component:** `SceneNavigatorView.swift`
**Severity:** Low
**Sprint:** SP-035

**Description:**
Chapter header rows used `.caption` + `.secondary` — smaller and lighter than scene rows — making chapters appear subordinate to scenes. Visual hierarchy was inverted.

**Root Cause Analysis:**
`chapterHeaderRow(for:)` used `.font(.caption).foregroundStyle(.secondary)`. Scene rows used `.callout` + `.secondary`/`.primary`, making them appear heavier than their chapter container.

**Resolution:**
- Chapter headers updated to `.font(.subheadline.weight(.semibold))` with `.foregroundStyle(.primary)` and `.padding(.top, 6)`.
- Scene rows given `.padding(.leading, 8)` indent to nest visually under their chapter.

**Files Affected:**
- `Scrivi/Views/SceneNavigatorView.swift`

**Verification:**
- ✅ Chapter names appear bold and dominant above scene rows
- ✅ Scene rows are indented, visually nested under their chapter
- ✅ Reorder drag still works correctly

---

## I-0022: No way to open a project from the menu bar

**Status:** ✅ Resolved - Verified
**Platform:** macOS, iPadOS
**Component:** `ScriviApp.swift`, `AppEnvironment.swift`, `LandingView.swift`
**Severity:** Medium
**Sprint:** SP-035

**Description:**
No Open Project command existed in the menu bar. The only entry point was the Landing View button, inaccessible when a project was already open.

**Root Cause Analysis:**
The `Project` `CommandMenu` had no Open item. The open-panel logic was private to `LandingView`.

**Resolution:**
- Extracted open-panel logic into `AppEnvironment.presentOpenProjectPanel()`.
- Added `Open Project… ⌘O` to the top of the `Project` `CommandMenu` in `ScriviApp.swift`, always enabled.
- `LandingView` updated to call `env.presentOpenProjectPanel()` instead of its own private method.

**Files Affected:**
- `Scrivi/App/AppEnvironment.swift`
- `Scrivi/App/ScriviApp.swift`
- `Scrivi/Views/LandingView.swift`

**Verification:**
- ✅ Project → Open Project… appears in menu bar at all times
- ✅ Selecting it opens the folder picker and loads the chosen project
- ✅ Accessible when a project is already open (replaces it in the same window)

---

## I-0023: About panel uses SF Symbol placeholder instead of app icon

**Status:** ✅ Resolved - Verified
**Platform:** macOS, iPadOS, iOS
**Component:** `AboutView.swift`
**Severity:** Low
**Sprint:** SP-035

**Description:**
The About panel displayed an SF Symbol (`pencil.and.outline`) instead of the real application icon.

**Root Cause Analysis:**
`AboutView` used `Image(systemName: "pencil.and.outline")` as a placeholder since no custom app icon existed yet.

**Resolution:**
- Replaced SF Symbol with `NSApp.applicationIconImage` on macOS (displays the bundle's `.icns`).
- Falls back to `UIImage(named: "AppIcon")` on iOS/iPadOS, with SF Symbol fallback if neither is available.
- Icon displayed at 80×80 with `cornerRadius(16)`.

**Files Affected:**
- `Scrivi/Views/AboutView.swift`

**Verification:**
- ✅ About panel shows the macOS default app icon (generic until a custom icon is added)
- ✅ No SF Symbol placeholder visible

---

---

## I-0024: Chapter title text written into scene content on app relaunch

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `ManuscriptTextView.swift`
**Severity:** High
**Sprint:** SP-035

**Description:**
After closing and reopening the app, the chapter title appeared both as the styled heading and as editable text at the start of scene content. The writer had to manually delete the duplicated line. Additionally, typing at the very start of a scene was blocked when "Show chapter titles" was enabled.

**Root Cause Analysis:**
Two related bugs in `ManuscriptNSTextView`:

1. `recomputeBoundaries` scanned for divider attachment characters and assumed each segment started immediately after the divider. When chapter titles are shown, heading text (`.scriviHeading` attributed) sits between the divider and the scene text. Every `sceneBoundaries[i].location` pointed into the heading rather than the scene, so `textDidChange` extracted heading characters into `seg.text` and saved them to disk.

2. `shouldChangeText(in:replacementString:)` shifted the heading check range one character backward for insertions (`affectedCharRange.length == 0`). At the start of a scene, `location - 1` landed inside the trailing `\n` of the heading string (which carries `.scriviHeading`), causing the insertion to be blocked.

**Resolution:**
- `recomputeBoundaries` rewritten to call `skipHeading(from:)` at the document start and after each divider, advancing past any contiguous `.scriviHeading` run before recording the segment start. Boundaries now match what `rebuildStorage` produces.
- `shouldChangeText` now distinguishes pure insertions (`length == 0`, non-empty replacement) from deletions/backspace. Insertions use a zero-length check range, bypassing the heading scan entirely. Only deletions still look at the preceding character.

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift`

**Verification:**
- ✅ Editing near start of scene, quit, relaunch — chapter title not present in scene text or on disk
- ✅ Typing at the start of a chapter with "Show chapter titles" enabled works normally
- ✅ Typing in heading text is still correctly blocked

---

*Last Updated: 2026-06-09 (I-0024 verified; batch 2 complete)*

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

*Last Updated: 2026-06-08 (I-0013, I-0014, I-0015 verified; SP-033)*

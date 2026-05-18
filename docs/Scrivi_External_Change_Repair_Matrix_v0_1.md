# Scrivi External Change Repair Matrix v0.1

**Project:** Scrivi  
**Document:** External Change Repair Matrix  
**Version:** 0.1  
**Status:** Draft / Approved Direction Pending Review  
**Date:** 2026-05-18

---

## 1. Purpose

This document defines how Scrivi detects, stages, and repairs changes made to a `.scrivi` project package outside the normal Scrivi editing flow.

External changes may come from:

```text
manual file edits
external text editors
file manager operations
Git checkout
Git merge
cloud sync
backup restore
drag/drop
scripted modifications
partial copy/restore
```

The purpose of this matrix is to prevent silent data loss while allowing Scrivi projects to remain transparent, inspectable, Git-friendly, and externally editable where appropriate.

---

## 2. Approved Context

This matrix assumes the following approved backend decisions:

```text
Project format: .scrivi package/folder
Manuscript format: visible .md + .meta.json pairs
Metadata: visible, text-based, app-managed
IDs: UUID v7-style opaque IDs
Git: optional Git-backed snapshots in v1
Indexes: app-local and rebuildable
SQLite: not required in v1
External editing: manuscript text and assets supported; metadata hand-editing not officially supported
Repair pattern: Detect → Stage → Review → Commit
```

---

## 3. Core Repair Principles

### 3.1 Preserve data first

When in doubt, Scrivi should preserve all available files and ask the user how to proceed.

### 3.2 Do not silently overwrite external changes

If Scrivi has unsaved in-app changes and the file on disk changed externally, Scrivi must preserve both versions or ask the user.

### 3.3 Do not silently delete unknown files

Unknown files may be staged, ignored, or moved to an inbox, but not deleted without confirmation.

### 3.4 IDs beat filenames

File names and slugs are convenience. Stable IDs remain authoritative.

### 3.5 Metadata is visible but app-managed

Users can see metadata. Scrivi should not assume they safely understand how to edit it by hand.

### 3.6 Repair should be reversible where possible

Before destructive repair actions, Scrivi should create a backup or recommend a Git snapshot when Git is enabled.

---

## 4. Detection Levels

Scrivi may detect external changes at different confidence levels.

### 4.1 Passive detection

Occurs during normal filesystem watching.

Examples:

```text
file changed
file deleted
file added
folder renamed
```

### 4.2 Open-project validation

Occurs when opening a project.

Examples:

```text
required files missing
metadata path points to missing content
chapter list points to missing scene metadata
```

### 4.3 Git-aware detection

Occurs when Git is enabled or a Git repository is present.

Examples:

```text
git status
git rename detection
checkout changed files
merge conflict markers
uncommitted changes
```

### 4.4 Manual refresh

Triggered by the user.

Examples:

```text
Refresh Project
Rebuild Project Index
Scan for External Changes
```

---

## 5. External Change States

Scrivi should classify external change states into one of these categories:

```text
clean
safe external edit
new unregistered file
missing content
missing metadata
possible rename
orphan metadata
schema issue
corrupt metadata
Git state change
merge conflict
unsupported project version
unknown issue
```

---

## 6. Repair Matrix

---

### 6.1 Normal pair: `.md` and `.meta.json` both exist and are valid

**Condition**

```text
scene.md exists
scene.meta.json exists
scene.meta.json content.path points to scene.md
scene ID is valid
chapter scene list references the scene metadata
```

**Classification**

```text
clean
```

**Scrivi behavior**

```text
Open normally
Index or re-index if needed
No repair UI required
```

**Allowed automatic action**

```text
Update app-local index
Update derived stats if file content changed through Scrivi
```

**Do not**

```text
Rewrite metadata unnecessarily
```

---

### 6.2 Markdown changed externally, metadata still valid

**Condition**

```text
scene.md changed on disk outside Scrivi
scene.meta.json remains valid
content.path still matches
```

**Classification**

```text
safe external edit
```

**Scrivi behavior**

If no unsaved in-app edits exist:

```text
Reload file or notify user
Update derived stats
Mark index dirty
Set modifiedBy to external or unknown
```

If unsaved in-app edits exist:

```text
Stage conflict
Preserve in-app buffer
Preserve on-disk version
Ask user to compare/choose/merge
```

**Suggested user actions**

```text
Reload external version
Keep current Scrivi version
Save current version as copy
Compare changes
```

**Git behavior**

If Git is enabled:

```text
Show as unsnapshotted change
```

**Do not**

```text
Silently overwrite external edit
Attribute external edit to current Scrivi identity
```

---

### 6.3 Markdown exists, metadata missing

**Condition**

```text
scene.md exists
paired scene.meta.json is missing
chapter list may or may not reference missing metadata
```

**Classification**

```text
new unregistered file
or missing metadata
```

**Scrivi behavior**

Stage as:

```text
Unregistered manuscript file found
```

or, if the file appears to be a known scene with lost metadata:

```text
Manuscript file exists but metadata is missing
```

**Suggested user actions**

```text
Import as new scene
Attach to existing scene
Regenerate metadata
Move to inbox
Ignore
Delete after confirmation
```

**Automatic repair allowed?**

Only if confidence is extremely high and the action is reversible.

Example high confidence signals:

```text
chapter list references missing metadata
file has expected paired name
Git history shows metadata deletion
content hash matches known previous scene
```

**Do not**

```text
Create metadata that reuses an existing scene ID unless confirmed
Delete the Markdown file
```

---

### 6.4 Metadata exists, Markdown missing

**Condition**

```text
scene.meta.json exists
content.path points to missing .md file
chapter list references the metadata
```

**Classification**

```text
missing content
```

**Scrivi behavior**

Stage as:

```text
Scene metadata exists, but manuscript file is missing
```

**Suggested user actions**

```text
Restore from Git snapshot
Relink to another Markdown file
Create empty manuscript file
Mark scene as missing
Remove scene from project after confirmation
Ignore for now
```

**Automatic repair allowed?**

No, except possibly marking the scene as missing in app-local repair state.

**Do not**

```text
Delete metadata automatically
Create empty manuscript file without user confirmation
Remove scene from chapter order automatically
```

---

### 6.5 Markdown renamed externally

**Condition**

```text
metadata content.path points to missing old filename
similar new .md file exists in same folder
metadata file still exists
```

**Classification**

```text
possible rename
```

**Confidence signals**

```text
same folder
similar file name
Git reports rename
content hash matches previous version
near-identical content
same heading/title
timestamp relationship suggests rename
```

**Scrivi behavior**

Stage as:

```text
Possible renamed manuscript file detected
```

**Suggested user actions**

```text
Relink metadata to new file
Import new file as separate scene
Restore old filename
Ignore
```

**Automatic repair allowed?**

Only if Git reports a clear rename and no conflicting candidates exist.

**Do not**

```text
Guess among multiple candidates
Change scene ID
```

---

### 6.6 Metadata renamed externally

**Condition**

```text
chapter.meta.json references old scene metadata path
old metadata path missing
similar new .meta.json exists nearby
sceneID inside new metadata matches expected sceneID
```

**Classification**

```text
possible metadata rename
```

**Scrivi behavior**

Stage as:

```text
Possible scene metadata rename detected
```

**Suggested user actions**

```text
Relink chapter scene entry to renamed metadata
Restore old metadata filename
Ignore
```

**Automatic repair allowed?**

Yes, if the sceneID matches exactly and only one candidate exists.

**Do not**

```text
Create duplicate scene entries
Change scene ID
```

---

### 6.7 Markdown and metadata both renamed together

**Condition**

```text
old metadata path missing
old content path missing
new .md and new .meta.json pair found
sceneID matches expected scene or Git detects rename pair
```

**Classification**

```text
possible paired rename
```

**Scrivi behavior**

Stage as:

```text
Possible scene file pair rename detected
```

**Suggested user actions**

```text
Accept rename
Restore previous names
Import as new scene
Ignore
```

**Automatic repair allowed?**

Possible if:

```text
sceneID matches
content path inside metadata matches new Markdown filename
only one candidate exists
```

**Do not**

```text
Treat as a new scene if it clearly preserves sceneID
```

---

### 6.8 Chapter folder renamed externally

**Condition**

```text
manuscript.meta.json references old chapter path
old chapter folder missing
similar new folder exists
new folder contains chapter.meta.json with expected chapterID
```

**Classification**

```text
possible chapter folder rename
```

**Scrivi behavior**

Stage as:

```text
Possible chapter folder rename detected
```

**Suggested user actions**

```text
Relink manuscript chapter path
Restore old folder name
Ignore
```

**Automatic repair allowed?**

Yes, if chapterID matches exactly and only one candidate exists.

**Do not**

```text
Reorder manuscript based on folder name
Change chapter ID
```

---

### 6.9 Chapter metadata missing

**Condition**

```text
chapter folder exists
chapter.meta.json missing
manuscript.meta.json references missing chapter metadata
```

**Classification**

```text
missing metadata
```

**Scrivi behavior**

Stage as:

```text
Chapter metadata is missing
```

**Suggested user actions**

```text
Restore from Git snapshot
Regenerate chapter metadata from folder contents
Move chapter folder to inbox
Ignore for now
Remove chapter from manuscript after confirmation
```

**Automatic repair allowed?**

No, unless user selects regeneration.

**Do not**

```text
Delete chapter folder
Remove scenes automatically
```

---

### 6.10 Manuscript metadata missing

**Condition**

```text
manuscript/manuscript.meta.json missing
manuscript folder exists
chapter folders may exist
```

**Classification**

```text
missing root manuscript metadata
```

**Scrivi behavior**

Open repair mode.

Stage as:

```text
Manuscript structure file is missing
```

**Suggested user actions**

```text
Restore from Git snapshot
Rebuild manuscript order from chapter folders
Create new manuscript metadata
Open project read-only
Cancel open
```

**Automatic repair allowed?**

No.

**Do not**

```text
Invent order silently
Overwrite existing chapter metadata
```

---

### 6.11 Project manifest missing

**Condition**

```text
project.json missing
.scrivi folder/package selected
```

**Classification**

```text
missing project manifest
```

**Scrivi behavior**

Do not open as a valid project without repair.

**Suggested user actions**

```text
Restore from Git snapshot
Attempt project recovery from package contents
Open containing folder
Cancel
```

**Automatic repair allowed?**

No.

**Do not**

```text
Create a new project.json over existing content without confirmation
```

---

### 6.12 New Markdown dropped into manuscript folder

**Condition**

```text
new .md file appears
no paired metadata exists
not referenced by chapter metadata
```

**Classification**

```text
new unregistered file
```

**Scrivi behavior**

Stage as:

```text
New manuscript file found
```

**Suggested user actions**

```text
Import as new scene in current chapter
Import as new chapter
Move to inbox
Ignore
Delete after confirmation
```

**Recommended default**

```text
Import as new scene
```

But only after user confirmation.

---

### 6.13 New asset file added

**Condition**

```text
new file added under assets/
no paired asset metadata exists
```

**Classification**

```text
new asset
```

**Scrivi behavior**

Stage as:

```text
New asset found
```

**Suggested user actions**

```text
Create asset metadata
Leave unmanaged
Move to inbox
Ignore
Delete after confirmation
```

**Automatic repair allowed?**

Scrivi may create asset metadata automatically only if project settings allow automatic asset registration.

Default should be staged review.

---

### 6.14 Asset file missing, metadata exists

**Condition**

```text
asset metadata exists
asset file path missing
```

**Classification**

```text
missing asset content
```

**Scrivi behavior**

Stage as:

```text
Asset metadata exists, but asset file is missing
```

**Suggested user actions**

```text
Restore from snapshot
Relink to replacement file
Mark asset missing
Remove asset metadata after confirmation
Ignore
```

---

### 6.15 Metadata JSON corrupt or unreadable

**Condition**

```text
JSON parse failure
invalid encoding
truncated file
```

**Classification**

```text
corrupt metadata
```

**Scrivi behavior**

Stage as:

```text
Metadata file cannot be read
```

**Suggested user actions**

```text
Restore from Git snapshot
Open file externally
Move corrupt file aside and regenerate where possible
Ignore for now
Cancel open if critical
```

**Automatic repair allowed?**

No destructive repair without confirmation.

**Do not**

```text
Overwrite corrupt metadata without backup
```

---

### 6.16 Unsupported newer schema version

**Condition**

```text
schema version newer than supported by this Scrivi build
```

**Classification**

```text
unsupported project version
```

**Scrivi behavior**

If safe:

```text
Open read-only
Warn user
Avoid writing unsupported files
```

If not safe:

```text
Refuse to open writable
Show version warning
```

**Suggested user actions**

```text
Update Scrivi
Open read-only
Cancel
```

**Do not**

```text
Downgrade schema silently
Rewrite newer schema files
```

---

### 6.17 Merge conflict markers in Markdown

**Condition**

```text
Markdown file contains Git conflict markers
```

Example:

```text
<<<<<<< HEAD
...
=======
...
>>>>>>> branch
```

**Classification**

```text
merge conflict
```

**Scrivi behavior**

Stage as:

```text
Merge conflict detected in manuscript
```

**Suggested user actions**

```text
Open conflict resolver
Open as plain text
Restore one version
Resolve externally
```

**V1 scope**

Full merge UI is out of scope.

Scrivi may offer safe plain-text visibility and warnings.

---

### 6.18 Merge conflict markers in metadata JSON

**Condition**

```text
metadata JSON contains conflict markers
```

**Classification**

```text
merge conflict / corrupt metadata
```

**Scrivi behavior**

Stage as critical repair item.

**Suggested user actions**

```text
Restore from snapshot
Resolve externally
Open repair guidance
Cancel open if critical
```

**Do not**

```text
Attempt to parse conflicted JSON as normal metadata
```

---

### 6.19 Git checkout changed currently open scene

**Condition**

```text
open scene changed on disk due to Git checkout
```

**Scrivi behavior**

If no unsaved in-app edits exist:

```text
Reload scene
Restore cursor if possible
Mark indexes dirty
```

If unsaved edits exist:

```text
Preserve in-app version
Preserve checked-out version
Stage conflict
Ask user to choose/merge/save copy
```

**Do not**

```text
Overwrite either version silently
```

---

### 6.20 Unknown file at project root

**Condition**

```text
unrecognized file appears at project root
```

**Classification**

```text
unknown issue or unmanaged file
```

**Scrivi behavior**

Ignore by default unless file appears dangerous or conflicts with required names.

Optional staged action:

```text
Move to inbox
Ignore
Delete after confirmation
```

---

## 7. Repair UI Severity Levels

Scrivi may classify repair items by severity.

### 7.1 Info

Does not block writing.

Examples:

```text
new asset found
unknown file at root
unindexed external edit
```

### 7.2 Warning

May affect project structure, but writing can continue.

Examples:

```text
unregistered Markdown file
missing asset
possible rename
```

### 7.3 Blocking

Project cannot safely continue in normal editing mode until resolved.

Examples:

```text
project.json missing
manuscript.meta.json missing
currently open scene has unsaved conflict
critical metadata corrupt
unsupported newer schema requiring read-only
```

---

## 8. Repair Action Policy

### 8.1 Safe automatic actions

Scrivi may perform these automatically:

```text
mark indexes dirty
rebuild app-local cache
reload clean external edit if no unsaved changes exist
recognize exact ID-preserving metadata rename
record repair issue in app-local state
```

### 8.2 Confirmation-required actions

Scrivi must ask before:

```text
deleting files
removing scenes from chapter order
creating replacement metadata
creating empty missing content file
relinking ambiguous files
overwriting metadata
moving files out of their current location
discarding unsaved edits
```

### 8.3 Backup-required actions

Scrivi should preserve a backup before:

```text
regenerating metadata
rewriting corrupt metadata
repairing project manifest
mass relinking after Git checkout
```

---

## 9. Git Interaction

### 9.1 Git-enabled projects

If Git-backed snapshots are enabled, repair UI should prefer Git-aware options.

Examples:

```text
Restore file from snapshot
Compare with last snapshot
Show unsnapshotted changes
Show file deleted since last snapshot
```

### 9.2 Git not enabled

If Git is not enabled, repair options should not depend on snapshot history.

Scrivi may suggest:

```text
Enable Git-backed snapshots
```

But must not require it.

### 9.3 Git conflict scope

Full Git conflict resolution is out of scope for v1.

Scrivi should detect conflicts and avoid worsening them.

---

## 10. Index/Cache Interaction

External repairs may require app-local indexes to be rebuilt.

Scrivi should mark caches dirty when:

```text
Markdown content changes
metadata changes
chapter order changes
manuscript order changes
objects/assets/comments change
Git checkout changes files
repair action modifies project structure
```

Caches are disposable and should never block repair unless needed for UI display.

---

## 11. Success Criteria

The External Change Repair Matrix is successful if:

1. External manuscript edits are accepted safely.
2. Missing `.md` and missing `.meta.json` files are handled explicitly.
3. Renames are detected when possible without unsafe guessing.
4. Scrivi does not silently overwrite external changes.
5. Scrivi does not silently delete unknown files.
6. Metadata corruption is recoverable where possible.
7. Git checkouts are treated as normal external project changes.
8. Unsaved in-app edits are preserved during disk conflicts.
9. The writer can continue working when noncritical issues exist.
10. Critical issues open repair mode instead of causing data loss.
11. App-local indexes can be rebuilt after repair.
12. The repair model remains compatible with future encryption and signing.

---

## 12. Open Issues for Later

Future specs should define:

1. Repair UI design.
2. File comparison/merge UX.
3. Git conflict handling UX.
4. Asset metadata repair behavior after asset schema approval.
5. Comment anchor repair behavior.
6. Object relationship repair behavior.
7. Encryption-aware repair behavior.
8. Signing-aware tamper detection.
9. Cloud sync conflict behavior.
10. Automated repair confidence thresholds.

# Scrivi External Change Repair Matrix v0.2

**Project:** Scrivi  
**Document:** External Change Repair Matrix  
**Version:** 0.2  
**Status:** Approved Direction  
**Date:** 2026-05-21  
**Supersedes:** `Scrivi_External_Change_Repair_Matrix_v0_1.md`

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

This matrix reflects the following approved architecture decisions:

```text
Architecture: Scrivi_Architecture_v0_3.md (supersedes v0.2)
Project format: .scrivi package/folder
Manuscript format: visible .md + .meta.json pairs
Metadata: visible, text-based, app-managed
IDs: UUID v7-style opaque IDs
Git: optional Git-backed snapshots in v1
Indexes: app-local and rebuildable
SQLite: not required in v1
External editing: manuscript text and assets supported; metadata hand-editing not officially supported
Repair pattern: Detect → Stage → Review → Commit
Boundary protocol: JSON-over-std::string (all repair issue data travels by value to the platform wrapper)
```

The repair model itself is unchanged from v0.1. The architecture update (v0.3) does not affect the conditions, classifications, or actions in this matrix. The only architectural consequence for this document is that `RepairIssue` lists are serialized into the JSON result envelope by `ScriviCoreAdapter` and decoded by the platform wrapper — they do not cross the boundary as C++ types.

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

### 4.1 Passive detection

Filesystem watching: file changed, file deleted, file added, folder renamed.

### 4.2 Open-project validation

Required files missing, metadata path points to missing content, chapter list points to missing scene metadata.

### 4.3 Git-aware detection

`git status`, rename detection, checkout-changed files, merge conflict markers, uncommitted changes.

### 4.4 Manual refresh

User-triggered: Refresh Project, Rebuild Project Index, Scan for External Changes.

---

## 5. External Change States

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

**Classification:** `clean`

**Behavior:** Open normally. Index or re-index if needed. No repair UI required.

**Automatic:** Update app-local index, update derived stats if content changed through Scrivi.

**Do not:** Rewrite metadata unnecessarily.

---

### 6.2 Markdown changed externally, metadata still valid

**Classification:** `safe external edit`

**Behavior (no unsaved in-app edits):** Reload file or notify user. Update derived stats. Mark index dirty. Set `modifiedBy` to external or unknown.

**Behavior (unsaved in-app edits exist):** Stage conflict. Preserve in-app buffer. Preserve on-disk version. Ask user to compare/choose/merge.

**Suggested user actions:** Reload external version, Keep current Scrivi version, Save current version as copy, Compare changes.

**Git behavior:** Show as unsnapshotted change if Git is enabled.

**Do not:** Silently overwrite external edit. Attribute external edit to current Scrivi identity.

---

### 6.3 Markdown exists, metadata missing

**Classification:** `new unregistered file` or `missing metadata`

**Behavior:** Stage as "Unregistered manuscript file found" or "Manuscript file exists but metadata is missing."

**Suggested user actions:** Import as new scene, Attach to existing scene, Regenerate metadata, Move to inbox, Ignore, Delete after confirmation.

**Automatic repair:** Only if confidence is extremely high and the action is reversible (chapter list references missing metadata, file has expected paired name, Git history shows metadata deletion, content hash matches known previous scene).

**Do not:** Create metadata that reuses an existing scene ID unless confirmed. Delete the Markdown file.

---

### 6.4 Metadata exists, Markdown missing

**Classification:** `missing content`

**Behavior:** Stage as "Scene metadata exists, but manuscript file is missing."

**Suggested user actions:** Restore from Git snapshot, Relink to another Markdown file, Create empty manuscript file, Mark scene as missing, Remove scene from project after confirmation, Ignore for now.

**Automatic repair:** No, except possibly marking the scene as missing in app-local repair state.

**Do not:** Delete metadata automatically. Create empty manuscript file without user confirmation. Remove scene from chapter order automatically.

---

### 6.5 Markdown renamed externally

**Classification:** `possible rename`

**Confidence signals:** Same folder, similar file name, Git reports rename, content hash matches, near-identical content, same heading/title, timestamp relationship.

**Behavior:** Stage as "Possible renamed manuscript file detected."

**Suggested user actions:** Relink metadata to new file, Import new file as separate scene, Restore old filename, Ignore.

**Automatic repair:** Only if Git reports a clear rename and no conflicting candidates exist.

**Do not:** Guess among multiple candidates. Change scene ID.

---

### 6.6 Metadata renamed externally

**Classification:** `possible metadata rename`

**Confidence signal:** `sceneID` inside new metadata matches expected `sceneID`.

**Behavior:** Stage as "Possible scene metadata rename detected."

**Suggested user actions:** Relink chapter scene entry to renamed metadata, Restore old metadata filename, Ignore.

**Automatic repair:** Yes, if the `sceneID` matches exactly and only one candidate exists.

**Do not:** Create duplicate scene entries. Change scene ID.

---

### 6.7 Markdown and metadata both renamed together

**Classification:** `possible paired rename`

**Automatic repair:** Possible if `sceneID` matches, content path inside metadata matches new Markdown filename, and only one candidate exists.

**Behavior:** Stage as "Possible scene file pair rename detected."

**Suggested user actions:** Accept rename, Restore previous names, Import as new scene, Ignore.

**Do not:** Treat as a new scene if it clearly preserves `sceneID`.

---

### 6.8 Chapter folder renamed externally

**Classification:** `possible chapter folder rename`

**Automatic repair:** Yes, if `chapterID` matches exactly and only one candidate exists.

**Behavior:** Stage as "Possible chapter folder rename detected."

**Suggested user actions:** Relink manuscript chapter path, Restore old folder name, Ignore.

**Do not:** Reorder manuscript based on folder name. Change chapter ID.

---

### 6.9 Chapter metadata missing

**Classification:** `missing metadata`

**Behavior:** Stage as "Chapter metadata is missing."

**Suggested user actions:** Restore from Git snapshot, Regenerate chapter metadata from folder contents, Move chapter folder to inbox, Ignore for now, Remove chapter from manuscript after confirmation.

**Automatic repair:** No, unless user selects regeneration.

**Do not:** Delete chapter folder. Remove scenes automatically.

---

### 6.10 Manuscript metadata missing

**Classification:** `missing root manuscript metadata`

**Behavior:** Open repair mode. Stage as "Manuscript structure file is missing."

**Suggested user actions:** Restore from Git snapshot, Rebuild manuscript order from chapter folders, Create new manuscript metadata, Open project read-only, Cancel open.

**Automatic repair:** No.

**Do not:** Invent order silently. Overwrite existing chapter metadata.

---

### 6.11 Project manifest missing

**Classification:** `missing project manifest`

**Behavior:** Do not open as a valid project without repair.

**Suggested user actions:** Restore from Git snapshot, Attempt project recovery from package contents, Open containing folder, Cancel.

**Automatic repair:** No.

**Do not:** Create a new `project.json` over existing content without confirmation.

---

### 6.12 New Markdown dropped into manuscript folder

**Classification:** `new unregistered file`

**Behavior:** Stage as "New manuscript file found."

**Suggested user actions:** Import as new scene in current chapter, Import as new chapter, Move to inbox, Ignore, Delete after confirmation.

**Recommended default:** Import as new scene — but only after user confirmation.

---

### 6.13 New asset file added

**Classification:** `new asset`

**Behavior:** Stage as "New asset found."

**Suggested user actions:** Create asset metadata, Leave unmanaged, Move to inbox, Ignore, Delete after confirmation.

**Automatic repair:** Only if project settings explicitly allow automatic asset registration. Default: staged review.

---

### 6.14 Asset file missing, metadata exists

**Classification:** `missing asset content`

**Behavior:** Stage as "Asset metadata exists, but asset file is missing."

**Suggested user actions:** Restore from snapshot, Relink to replacement file, Mark asset missing, Remove asset metadata after confirmation, Ignore.

---

### 6.15 Metadata JSON corrupt or unreadable

**Classification:** `corrupt metadata`

**Behavior:** Stage as "Metadata file cannot be read."

**Suggested user actions:** Restore from Git snapshot, Open file externally, Move corrupt file aside and regenerate where possible, Ignore for now, Cancel open if critical.

**Automatic repair:** No destructive repair without confirmation.

**Do not:** Overwrite corrupt metadata without backup.

---

### 6.16 Unsupported newer schema version

**Classification:** `unsupported project version`

**Behavior (safe):** Open read-only. Warn user. Avoid writing unsupported files.

**Behavior (not safe):** Refuse to open writable. Show version warning.

**Suggested user actions:** Update Scrivi, Open read-only, Cancel.

**Do not:** Downgrade schema silently. Rewrite newer schema files.

---

### 6.17 Merge conflict markers in Markdown

**Classification:** `merge conflict`

**Behavior:** Stage as "Merge conflict detected in manuscript."

**Suggested user actions:** Open conflict resolver, Open as plain text, Restore one version, Resolve externally.

**V1 scope:** Full merge UI is out of scope. Scrivi may offer safe plain-text visibility and warnings.

---

### 6.18 Merge conflict markers in metadata JSON

**Classification:** `merge conflict / corrupt metadata`

**Behavior:** Stage as critical repair item.

**Suggested user actions:** Restore from snapshot, Resolve externally, Open repair guidance, Cancel open if critical.

**Do not:** Attempt to parse conflicted JSON as normal metadata.

---

### 6.19 Git checkout changed currently open scene

**Behavior (no unsaved in-app edits):** Reload scene. Restore cursor if possible. Mark indexes dirty.

**Behavior (unsaved edits exist):** Preserve in-app version. Preserve checked-out version. Stage conflict. Ask user to choose/merge/save copy.

**Do not:** Overwrite either version silently.

---

### 6.20 Unknown file at project root

**Classification:** `unknown issue` or unmanaged file

**Behavior:** Ignore by default unless file appears dangerous or conflicts with required names.

**Optional staged action:** Move to inbox, Ignore, Delete after confirmation.

---

### 6.21 History store corrupt, missing, or out of sync (`history/`) — *added 2026-07-06 (EP-019, T-0200)*

The `history/` directory holds the undo/redo history and copy buffers
(`scrivi.history.v1` / `scrivi.buffers.v1` — see
`Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §6 and Appendix A). It is
**app-managed derived state, not canonical content**: the manuscript is never reconstructed from it,
and no history repair may modify any manuscript or metadata file.

**Classification:** `history issue`

**Behavior (by sub-condition):**

```text
history/ absent entirely          → initialize fresh history silently (Info). Normal for
                                    older projects and copies made before EP-019.
Torn final line in active log     → truncate the torn line, continue (Info). Worst case
                                    loses the last uncommitted event.
state.json missing or corrupt     → rebuild the checkpoint by replaying the log segments (Info).
Log segment unparseable           → reset history (fresh scrivi.history.v1), warn the user that
                                    undo history was lost (Warning).
Scene head-hash mismatch          → scene file changed outside the recorded history (external
                                    edit, or crash between undo-apply and save). Append an
                                    `externalChange` barrier for that scene and re-seed its
                                    cached text from disk (Info; surfaced in the history UI as
                                    a barrier, not a repair dialog).
buffers.json missing or corrupt   → reset to empty buffer set, warn (Warning).
```

**Do not:** modify, restore, or delete any manuscript/metadata/object file as part of history
repair; block writing (history conditions are never Blocking); silently keep history that no
longer matches the scene files.

**Severity:** Info or Warning only.

---

## 7. Repair UI Severity Levels

### 7.1 Info

Does not block writing. Examples: new asset found, unknown file at root, unindexed external edit.

### 7.2 Warning

May affect project structure, but writing can continue. Examples: unregistered Markdown file, missing asset, possible rename.

### 7.3 Blocking

Project cannot safely continue in normal editing mode until resolved. Examples: `project.json` missing, `manuscript.meta.json` missing, currently open scene has unsaved conflict, critical metadata corrupt, unsupported newer schema requiring read-only.

---

## 8. Repair Action Policy

### 8.1 Safe automatic actions

```text
mark indexes dirty
rebuild app-local cache
reload clean external edit if no unsaved changes exist
recognize exact ID-preserving metadata rename
record repair issue in app-local state
```

### 8.2 Confirmation-required actions

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

```text
regenerating metadata
rewriting corrupt metadata
repairing project manifest
mass relinking after Git checkout
```

---

## 9. Git Interaction

### 9.1 Git-enabled projects

Repair UI should prefer Git-aware options: Restore file from snapshot, Compare with last snapshot, Show unsnapshotted changes, Show file deleted since last snapshot.

### 9.2 Git not enabled

Repair options must not depend on snapshot history. Scrivi may suggest enabling Git-backed snapshots but must not require it.

### 9.3 Git conflict scope

Full Git conflict resolution is out of scope for v1. Scrivi detects conflicts and avoids worsening them.

---

## 10. Index/Cache Interaction

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
13. `RepairIssue` data travels by value to the platform wrapper via the JSON result envelope.

---

## 12. Open Issues for Later

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
11. Repair action operations in the facade API (`applyRepair`, `relinkScene`, etc.).

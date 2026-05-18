# Scrivi Project Creation and Open Flow v0.1

**Project:** Scrivi  
**Document:** Project Creation and Open Flow  
**Version:** 0.1  
**Status:** Draft / Approved Direction Pending Review  
**Date:** 2026-05-18

---

## 1. Purpose

This document defines the first user-visible and backend-facing flows for creating and opening Scrivi projects.

It translates the approved architecture, project package structure, schema set, and backend behavior spec into concrete lifecycle flows.

The central requirement is:

> A writer should be able to open Scrivi and resume writing immediately, without being forced through project-management complexity.

---

## 2. Scope

### In scope

This document covers:

```text
first app startup
author identity creation
author persona creation
new project creation
optional Git-backed snapshots
opening an existing project
resuming last writing surface
fallback to first scene
workspace state behavior
first-save behavior
open failure handling
```

### Out of scope

This document does not cover:

```text
full UI layout
visual design
advanced Git UI
remote Git hosting
real-time collaboration
asset import flows
comments UI
object/worldbuilding editor flows
export flows
identity transfer flow details
encryption
signing
```

---

## 3. Flow Principles

### 3.1 Start writing first

The default path should lead to a writing surface as quickly as possible.

### 3.2 Power features are optional

Git-backed snapshots, project repair tools, and advanced metadata should be available but not required to begin writing.

### 3.3 No online account required

Scrivi may create a local author identity and certificate-ready identity material, but the user does not need a cloud login.

### 3.4 Project files are canonical

The `.scrivi` package contains canonical project source. App-local workspace state and caches are not canonical.

### 3.5 Safe fallbacks

If last workspace state is missing or invalid, Scrivi should fall back to the first valid scene.

---

## 4. First App Startup Flow

### 4.1 Trigger

This flow runs when Scrivi launches and no local author identity exists.

### 4.2 User-facing flow

Scrivi displays a lightweight author setup screen:

```text
Welcome to Scrivi

How should Scrivi identify you as an author?

This name will appear on your writing history, comments, and shared projects.

[ Author Name ]

Continue
```

Optional secondary text:

```text
You can use a real name, pen name, initials, or pseudonym.
```

### 4.3 User decisions

Required:

```text
Author display name
```

Not required:

```text
email address
username
password
cloud account
Git credentials
```

### 4.4 Backend actions

Scrivi creates:

```text
device identity
Scrivi identity ID
default author persona
public/private key pair or certificate-ready identity material
local secure storage entry
```

### 4.5 Created local records

App-local secure/private storage:

```text
identity private key
identity local configuration
device identity
```

App-local nonprivate storage may include:

```text
default persona display name
identity ID
public key summary
```

### 4.6 Exit condition

The flow completes when:

```text
local identity exists
default persona exists
Scrivi can create or open projects
```

### 4.7 Error handling

If secure key storage fails, Scrivi should:

```text
warn user
offer retry
offer local fallback only if secure enough
avoid creating project identity inconsistently
```

---

## 5. New Project Creation Flow

### 5.1 Trigger

User chooses:

```text
New Project
```

### 5.2 Minimum user-facing fields

Recommended minimum:

```text
Project title
Project location
```

Optional:

```text
Author persona
Enable Git-backed snapshots
Initial chapter title
Initial scene title
```

### 5.3 Default values

If the user only enters a project title, Scrivi may default:

```text
Initial chapter title: Chapter 1
Initial scene title: Opening Scene
Initial scene filename: 001-opening-scene.md
Author persona: current default persona
Git-backed snapshots: off
```

### 5.4 Optional Git choice

User-facing control:

```text
Version History

[ ] Track this project with Git-backed snapshots
```

Expanded explanation:

```text
Creates local snapshots using Git. No remote account is required.
```

### 5.5 Backend creation sequence

Scrivi performs these steps:

```text
1. Generate project ID.
2. Generate manuscript ID.
3. Generate first chapter ID.
4. Generate first scene ID.
5. Create .scrivi package/folder.
6. Create project.json.
7. Create manuscript/manuscript.meta.json.
8. Create manuscript/chapter-001/.
9. Create manuscript/chapter-001/chapter.meta.json.
10. Create manuscript/chapter-001/001-opening-scene.md.
11. Create manuscript/chapter-001/001-opening-scene.meta.json.
12. Create identities/project-members.json.
13. Create identities/project-personas.json.
14. Create app-local workspace-state.json.
15. Initialize app-local index/cache placeholder.
16. Optionally initialize Git.
17. Open the first scene.
```

### 5.6 Minimum created package

```text
MyNovel.scrivi/
  project.json

  manuscript/
    manuscript.meta.json

    chapter-001/
      chapter.meta.json
      001-opening-scene.md
      001-opening-scene.meta.json

  identities/
    project-members.json
    project-personas.json
```

### 5.7 Git-enabled package additions

If Git-backed snapshots are enabled:

```text
MyNovel.scrivi/
  .git/
  .gitignore
  snapshots/
    scrivi-snapshots.json
```

### 5.8 Initial Git snapshot

If enabled, Scrivi creates an initial snapshot after the minimum project files are written.

User-facing snapshot label:

```text
Initial project
```

Git author mapping:

```text
<active persona display name> <identity_ID@scrivi.author>
```

### 5.9 Open created project

After creation, Scrivi immediately opens:

```text
manuscript/chapter-001/001-opening-scene.md
```

Cursor position:

```text
start of document
```

The writing surface should appear even if background indexing has not completed.

### 5.10 Failure handling

If project creation fails midway, Scrivi should:

```text
preserve diagnostic information
avoid leaving partially-created project as valid unless all required files exist
offer cleanup of its own partial files
avoid deleting user files not created by Scrivi
```

### 5.11 Success criteria

Project creation is successful if:

1. The minimum package exists and validates.
2. The first scene opens.
3. The cursor is placed in the writing surface.
4. Authorship/persona metadata is present.
5. Git is initialized only if requested.
6. No online account is required.

---

## 6. Enable Git Later Flow

### 6.1 Trigger

User chooses:

```text
Project Settings → Version History → Enable Git-backed snapshots
```

### 6.2 Backend behavior

Scrivi should:

```text
1. Check whether project is already in a Git repository.
2. If no repository exists, initialize Git.
3. Generate or update .gitignore.
4. Create snapshots/scrivi-snapshots.json if missing.
5. Create initial snapshot of current project state.
6. Record snapshot metadata.
```

### 6.3 User-facing language

Avoid raw Git unless in advanced details.

Recommended:

```text
Enable local snapshots for this project?
```

Expanded:

```text
Scrivi will use Git-compatible version history. No remote account is required.
```

### 6.4 Out of scope

```text
remote setup
GitHub login
GitLab login
pull/push
branch UI
merge UI
```

---

## 7. Open Existing Project Flow

### 7.1 Trigger

User chooses an existing `.scrivi` project.

### 7.2 Fast path

Scrivi should perform the minimum work needed to open the last writing surface.

Sequence:

```text
1. Read project.json.
2. Validate schema and project ID.
3. Read identities/project-members.json.
4. Read identities/project-personas.json.
5. Read app-local workspace-state.json for this project, if present.
6. Resolve lastWritingSurface.sceneID.
7. Read the target scene.meta.json.
8. Read the target scene.md.
9. Restore cursor/selection/scroll if valid.
10. Display writing surface.
11. Start background validation/indexing.
```

### 7.3 Background tasks after editor display

```text
validate manuscript structure
scan for external changes
rebuild dirty app-local indexes
scan comments
scan objects/assets
check Git status if enabled
load project explorer
```

### 7.4 Success criteria

Open existing project is successful if:

1. The last writing surface appears quickly.
2. The cursor is restored when valid.
3. Missing app-local cache does not block opening.
4. Full indexing is not required before editing.
5. Repair issues are staged without destroying data.

---

## 8. Open Fallback Flow

### 8.1 Trigger

Fallback is used when workspace state is missing, stale, invalid, or points to a missing scene.

Examples:

```text
workspace-state.json missing
last scene ID not found
last content path missing
cursor range invalid
project opened on a new device
```

### 8.2 Fallback sequence

Scrivi should:

```text
1. Read manuscript/manuscript.meta.json.
2. Resolve first chapter in structure.chapters.
3. Read first chapter.meta.json.
4. Resolve first scene in chapter.scenes.
5. Read first scene.meta.json.
6. Read scene content path.
7. Display scene.
8. Place cursor at document start.
9. Create or update workspace-state.json.
```

### 8.3 If first scene is invalid

Scrivi should continue searching in canonical order for the first valid scene.

If no valid scene exists, open repair mode.

### 8.4 Success criteria

Fallback is successful if:

1. The project opens even without workspace state.
2. The first available scene is found by metadata order, not folder sorting.
3. Invalid scene references are staged for repair.
4. The project is not modified unnecessarily.

---

## 9. Resume Writing Flow

### 9.1 Trigger

Occurs after opening or creating a project.

### 9.2 Required loaded data

To show the writing surface, Scrivi needs:

```text
scene metadata
Markdown content
active identity/persona
workspace cursor/scroll state if available
```

It does not need:

```text
full object graph
full search index
all assets
all comments
all relationship data
Git status
```

### 9.3 Cursor rules

If cursor anchor/focus are valid:

```text
restore exact selection
```

If invalid but same scene exists:

```text
place cursor at nearest valid offset or document start
```

If scene changed significantly:

```text
restore scene only; place cursor safely
```

### 9.4 First empty scene

For a newly-created project, the scene file may be empty.

Scrivi opens it with:

```text
cursor at offset 0
scroll position 0
```

### 9.5 Success criteria

Resume writing is successful if the writer can begin typing without opening a project browser or repair UI unless the active scene is structurally unsafe.

---

## 10. First Save Flow

### 10.1 Trigger

User edits a scene and Scrivi saves.

Save may be:

```text
autosave
manual save
save before close
save before snapshot
```

### 10.2 Backend sequence

Scrivi should:

```text
1. Write scene.md safely.
2. Update scene.meta.json modifiedAt.
3. Update scene.meta.json modifiedBy.
4. Update derived stats if enabled.
5. Update workspace-state.json.
6. Mark app-local indexes dirty.
7. If Git is enabled, show unsnapshotted changes.
```

### 10.3 Safe write behavior

Preferred:

```text
write temporary file
flush/sync when appropriate
atomic replace
preserve UTF-8
```

### 10.4 Metadata write behavior

Scrivi should update only necessary metadata fields.

Do not rewrite unrelated metadata just because the scene text changed.

### 10.5 Success criteria

First save is successful if:

1. The Markdown content is durable.
2. Metadata reflects modification time and author/persona.
3. Workspace state is updated.
4. Indexes are marked dirty.
5. Git state reflects changes if enabled.

---

## 11. Close Project Flow

### 11.1 Trigger

User closes project or exits app.

### 11.2 Backend sequence

Scrivi should:

```text
1. Save unsaved manuscript edits if autosave is enabled.
2. Update workspace-state.json.
3. Flush minimal app-local state.
4. Avoid blocking close on full indexing.
5. Leave background tasks safely cancelable.
```

### 11.3 Git-enabled behavior

If unsnapshotted changes exist, Scrivi may remind the user:

```text
You have unsnapshotted changes.
```

But it should not force a snapshot on close.

### 11.4 Success criteria

Close is successful if:

1. Last writing position is recoverable.
2. Manuscript changes are not lost.
3. Project is not left in a partially-written state.
4. Noncritical indexing can resume later.

---

## 12. Open With External Changes

### 12.1 Trigger

Project files changed while Scrivi was closed or inactive.

### 12.2 Behavior

Scrivi should:

```text
1. Open the last writing surface if safe.
2. Stage nonblocking repair issues.
3. Open repair mode if the active writing surface is unsafe.
4. Rebuild app-local indexes as needed.
```

### 12.3 Examples of safe-to-open conditions

```text
last scene .md changed externally but metadata valid
new asset added
new unregistered file found elsewhere
unknown root file
```

### 12.4 Examples of repair-before-edit conditions

```text
last scene .md missing
last scene metadata corrupt
project.json missing
manuscript.meta.json missing
merge conflict in active scene
unsupported newer schema requiring read-only
```

### 12.5 Success criteria

The writer should not be blocked by unrelated noncritical repair issues.

But Scrivi must not open an unsafe active scene as if everything is normal.

---

## 13. Open Modes

Scrivi may need different open modes depending on project condition.

### 13.1 Normal edit mode

Used when required files are valid.

### 13.2 Edit mode with warnings

Used when noncritical issues exist.

Examples:

```text
new unregistered asset
unknown file
stale index
```

### 13.3 Repair mode

Used when project structure needs user decision.

Examples:

```text
missing scene content
missing scene metadata
possible rename
corrupt noncritical metadata
```

### 13.4 Read-only mode

Used when writing could damage unsupported data.

Examples:

```text
newer unsupported schema
incomplete encryption support
permissions prevent safe write
```

### 13.5 Cannot open

Used when root project identity is not recoverable.

Examples:

```text
project.json missing and recovery declined
package unreadable
critical permissions failure
```

---

## 14. Project Creation Pseudocode

Illustrative only.

```text
createProject(input):
  ensureLocalIdentityExists()
  activePersona = resolveActivePersona(input.persona)

  ids = generateProjectIDs()
  root = createProjectPackage(input.location, input.title)

  writeProjectJson(root, ids, activePersona, input)
  writeProjectMembers(root, activePersona.identity)
  writeProjectPersonas(root, activePersona)

  createDirectory(root/manuscript)
  writeManuscriptMeta(root, ids)

  createDirectory(root/manuscript/chapter-001)
  writeChapterMeta(root, ids)
  writeEmptySceneMarkdown(root, ids)
  writeSceneMeta(root, ids, activePersona)

  writeWorkspaceState(appLocal, ids, activePersona)

  if input.enableGit:
    initializeGit(root)
    writeGitIgnore(root)
    writeSnapshotMetadata(root)
    createInitialSnapshot(root, activePersona)

  openScene(ids.firstSceneID)
```

---

## 15. Open Project Pseudocode

Illustrative only.

```text
openProject(root):
  project = readProjectJson(root)
  validateSupportedProject(project)

  members = readProjectMembers(project)
  personas = readProjectPersonas(project)

  workspace = readWorkspaceState(project.projectID)

  if workspace has valid lastWritingSurface:
    result = tryOpenScene(workspace.lastWritingSurface.sceneID)
    if result.success:
      displayEditor(result.scene, workspace.cursor, workspace.scroll)
      startBackgroundValidation()
      return

  fallback = openFirstValidScene(project)
  if fallback.success:
    displayEditor(fallback.scene, cursorStart, scrollTop)
    writeWorkspaceState(fallback.scene)
    startBackgroundValidation()
    return

  openRepairMode(project)
```

---

## 16. Success Criteria

The Project Creation and Open Flow is successful if:

1. First app startup creates identity/persona without account login.
2. New project creation produces the approved minimum file set.
3. A new project immediately opens to an editable scene.
4. Git-backed snapshots can be enabled at creation.
5. Git-backed snapshots can be enabled later.
6. Existing projects open to the last writing surface when possible.
7. Missing workspace state falls back to the first valid scene.
8. Full indexing does not block the editor.
9. External changes are staged according to the repair matrix.
10. Active-scene structural problems open repair mode instead of causing data loss.
11. App-local workspace state is separate from canonical project source.
12. The flow remains compatible with future identity transfer, signing, group personas, and encryption.

---

## 17. Open Issues for Later Specs

Future documents should define:

1. Detailed first-run UI.
2. New project UI.
3. Project browser/recent projects behavior.
4. Repair UI design.
5. Git snapshot UI design.
6. Identity transfer UI/protocol.
7. App-local storage paths per platform.
8. Background index lifecycle.
9. Autosave cadence.
10. Multi-window behavior.
11. iPadOS/visionOS/macOS package behavior.
12. Windows/Linux folder behavior.

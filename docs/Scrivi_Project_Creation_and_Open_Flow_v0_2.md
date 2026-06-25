# Scrivi Project Creation and Open Flow v0.2

**Project:** Scrivi  
**Document:** Project Creation and Open Flow  
**Version:** 0.2  
**Status:** Approved Direction  
**Date:** 2026-05-21  
**Supersedes:** `Scrivi_Project_Creation_and_Open_Flow_v0_1.md`

---

## 1. Purpose

This document defines the first user-visible and backend-facing flows for creating and opening Scrivi projects.

It translates the approved architecture (v0.3), project package structure, schema set, and backend behavior spec into concrete lifecycle flows.

The central requirement is:

> A writer should be able to open Scrivi and resume writing immediately, without being forced through project-management complexity.

**Change from v0.1:** The architecture update (v0.3) confirms that scene Markdown content travels by value across the C++/Swift boundary — it is serialized into the JSON result string by `ScriviCoreAdapter` and decoded in Swift. The flows themselves are unchanged. This document updates the approved context references and notes where this boundary behavior is relevant.

---

## 2. Scope

### In scope

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

### 3.6 Markdown travels by value

When the platform layer (Swift) receives `openProject` results, the full scene Markdown content is included in the JSON result string — not a pointer to C++ memory. The platform wrapper decodes the Markdown from the JSON value; it does not make a subsequent call to fetch content. This is the permanent boundary design.

---

## 4. First App Startup Flow

### 4.1 Trigger

This flow runs when Scrivi launches and no local author identity exists.

### 4.2 User-facing flow

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

`ScriviCore::ensureLocalIdentity()` creates:

```text
device identity
Scrivi identity ID
default author persona
public/private key pair or certificate-ready identity material
local secure storage entry
```

### 4.5 Created local records

App-local secure storage:

```text
identity private key
identity local configuration
device identity
```

App-local non-private storage:

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

If secure key storage fails, Scrivi should warn the user, offer retry, and avoid creating project identity inconsistently.

---

## 5. New Project Creation Flow

### 5.1 Trigger

User chooses New Project.

### 5.2 Minimum user-facing fields

```text
Project title     (required)
Project location  (required)
```

Optional:

```text
Author persona
Enable Git-backed snapshots
Initial chapter title
Initial scene title
```

### 5.3 Default values

If the user enters only a project title, Scrivi defaults:

```text
Initial chapter title: Chapter 1
Initial scene title: Opening Scene
Initial scene filename: 001-opening-scene.md
Author persona: current default persona
Git-backed snapshots: off
```

### 5.4 Optional Git choice

```text
Version History

[ ] Track this project with Git-backed snapshots

Creates local snapshots using Git. No remote account is required.
```

### 5.5 Backend creation sequence

`ScriviCore::createProject()` performs these steps:

```text
1.  Generate project ID.
2.  Generate manuscript ID.
3.  Generate first chapter ID.
4.  Generate first scene ID.
5.  Create .scrivi package/folder.
6.  Create project.json.
7.  Create manuscript/manuscript.meta.json.
8.  Create manuscript/chapter-001/.
9.  Create manuscript/chapter-001/chapter.meta.json.
10. Create manuscript/chapter-001/001-opening-scene.md.
11. Create manuscript/chapter-001/001-opening-scene.meta.json.
12. Create identities/project-members.json.
13. Create identities/project-personas.json.
14. Create app-local workspace-state.json.
15. Initialize app-local index/cache placeholder.
16. Optionally initialize Git.
17. Return CreateProjectResult with first scene paths.
```

The platform wrapper opens the first scene using the paths from the result.

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

```text
MyNovel.scrivi/
  .git/
  .gitignore
  snapshots/
    scrivi-snapshots.json
```

### 5.8 Initial Git snapshot

Snapshot label: `Initial project`

Git author mapping: `<active persona display name> <identity_ID@scrivi.author>`

### 5.9 Open created project

After creation, the platform wrapper calls `openProject` to load the scene content. The `OpenProjectResult.activeScene.markdown` field contains the initial (empty) scene Markdown by value. The writing surface appears even if background indexing has not completed.

### 5.10 Failure handling

If project creation fails midway:

```text
Preserve diagnostic information.
Avoid leaving a partially-created project as valid unless all required files exist.
Offer cleanup of its own partial files.
Avoid deleting user files not created by Scrivi.
```

### 5.11 Success criteria

1. The minimum package exists and validates.
2. The first scene opens.
3. The cursor is placed in the writing surface.
4. Authorship/persona metadata is present.
5. Git is initialized only if requested.
6. No online account is required.

---

## 6. Enable Git Later Flow

### 6.1 Trigger

User chooses: Project Settings → Version History → Enable Git-backed snapshots.

### 6.2 Backend behavior

`ScriviCore::enableGitSnapshots()`:

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

```text
Enable local snapshots for this project?

Scrivi will use Git-compatible version history. No remote account is required.
```

### 6.4 Out of scope

Remote setup, GitHub/GitLab login, pull/push, branch UI, merge UI.

---

## 7. Open Existing Project Flow

### 7.1 Trigger

User chooses an existing `.scrivi` project.

### 7.2 Fast path

`ScriviCore::openProject()`:

```text
1.  Read project.json.
2.  Validate schema and project ID.
3.  Read identities/project-members.json.
4.  Read identities/project-personas.json.
5.  Read app-local workspace-state.json for this project, if present.
6.  Resolve lastWritingSurface.sceneID.
7.  Read the target scene.meta.json.
8.  Read the target scene.md.
9.  Serialize scene Markdown into result JSON string.
10. Return OpenProjectResult with activeScene.markdown by value.
```

The platform wrapper decodes the result JSON. `activeScene.markdown` contains the full scene content as a Swift `String`. No subsequent call is needed to fetch Markdown. Cursor and scroll are restored from `restoredSelection` and `restoredScroll`.

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

1. The last writing surface appears quickly.
2. The cursor is restored when valid.
3. Missing app-local cache does not block opening.
4. Full indexing is not required before editing.
5. Repair issues are staged without destroying data.

---

## 8. Open Fallback Flow

### 8.1 Trigger

Workspace state is missing, stale, invalid, or points to a missing scene.

Examples:

```text
workspace-state.json missing
last scene ID not found
last content path missing
cursor range invalid
project opened on a new device
```

### 8.2 Fallback sequence

```text
1. Read manuscript/manuscript.meta.json.
2. Resolve first chapter in structure.chapters.
3. Read first chapter.meta.json.
4. Resolve first scene in chapter.scenes.
5. Read first scene.meta.json.
6. Read scene content path.
7. Serialize scene Markdown into result JSON.
8. Return OpenProjectResult with mode=normalEdit.
9. Place cursor at document start.
10. Create or update workspace-state.json.
```

### 8.3 If first scene is invalid

Continue searching in canonical order for the first valid scene. If no valid scene exists, return `mode=repairRequired`.

### 8.4 Success criteria

1. The project opens even without workspace state.
2. The first available scene is found by metadata order, not folder sorting.
3. Invalid scene references are staged for repair.
4. The project is not modified unnecessarily.

---

## 9. Resume Writing Flow

### 9.1 Trigger

Occurs after opening or creating a project. The writing surface displays from `OpenProjectResult.activeScene.markdown` (already decoded by the platform wrapper from the JSON result).

### 9.2 Required loaded data

```text
activeScene.markdown (from openProject result — already in Swift String)
activeScene.sceneID, metadataPath, contentPath
restoredSelection, restoredScroll
```

The platform wrapper does not need:

```text
full object graph
full search index
all assets
all comments
all relationship data
Git status
```

### 9.3 Cursor rules

If cursor anchor/focus are valid: restore exact selection.

If invalid but same scene exists: place cursor at nearest valid offset or document start.

If scene changed significantly: restore scene only; place cursor safely.

### 9.4 First empty scene

For a newly-created project, `activeScene.markdown` is empty. Scrivi opens with cursor at offset 0, scroll position 0.

### 9.5 Success criteria

The writer can begin typing without opening a project browser or repair UI unless the active scene is structurally unsafe.

### 9.6 Scope of restoration (window vs. project)

This flow restores state **within a single project** — active scene, cursor (`restoredSelection`), and scroll (`restoredScroll`). It is silent on the **window↔project mapping** and on restoring *which projects* are open.

That window-level model — multiple distinct projects open at once, one per window, non-reentrant per project, and restoring the full set of open project windows on relaunch — is specified separately in **`Scrivi_PerWindow_Project_Model_Design_v0_1.md`** (approved 2026-06-24, EP-018). On relaunch the per-window model reopens each previously-open project window; this Resume Writing Flow then runs *inside* each window to restore that project's scene/cursor/scroll. The two compose: per-window restore selects the projects; this flow restores position within each.

---

## 10. First Save Flow

### 10.1 Trigger

User edits a scene and Scrivi saves (autosave, manual save, save before close, save before snapshot).

### 10.2 Backend sequence

`ScriviCore::saveScene()`:

```text
1. Write scene.md safely (atomic write).
2. Update scene.meta.json modifiedAt.
3. Update scene.meta.json modifiedBy.
4. Update derived stats if enabled.
5. Update workspace-state.json.
6. Mark app-local indexes dirty.
7. If Git is enabled, note unsnapshotted changes.
8. Return SaveSceneResult with saved=true and wordCount.
```

### 10.3 Safe write behavior

```text
Write temporary file.
Flush/sync as appropriate.
Atomic replace.
Preserve UTF-8.
```

### 10.4 Metadata write behavior

Update only necessary fields. Do not rewrite unrelated metadata because scene text changed.

### 10.5 Success criteria

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

```text
1. Save unsaved manuscript edits if autosave is enabled.
2. Update workspace-state.json.
3. Flush minimal app-local state.
4. Avoid blocking close on full indexing.
5. Leave background tasks safely cancelable.
```

### 11.3 Git-enabled behavior

If unsnapshotted changes exist, Scrivi may remind the user. It must not force a snapshot on close.

### 11.4 Success criteria

1. Last writing position is recoverable.
2. Manuscript changes are not lost.
3. Project is not left in a partially-written state.
4. Noncritical indexing can resume later.

---

## 12. Open With External Changes

### 12.1 Trigger

Project files changed while Scrivi was closed or inactive.

### 12.2 Behavior

```text
1. Open the last writing surface if safe.
2. Stage nonblocking repair issues.
3. Open repair mode if the active writing surface is unsafe.
4. Rebuild app-local indexes as needed.
```

### 12.3 Safe-to-open conditions

```text
last scene .md changed externally but metadata valid
new asset added
new unregistered file found elsewhere
unknown root file
```

### 12.4 Repair-before-edit conditions

```text
last scene .md missing
last scene metadata corrupt
project.json missing
manuscript.meta.json missing
merge conflict in active scene
unsupported newer schema requiring read-only
```

### 12.5 Success criteria

The writer should not be blocked by unrelated noncritical repair issues. Scrivi must not open an unsafe active scene as if everything is normal.

---

## 13. Open Modes

### 13.1 Normal edit mode

Required files are valid; writing may proceed.

### 13.2 Edit mode with warnings

Noncritical issues exist (new unregistered asset, unknown file, stale index); writing may proceed with the issue staged.

### 13.3 Repair mode

Project structure needs user decision (missing scene content, missing scene metadata, possible rename, corrupt noncritical metadata).

### 13.4 Read-only mode

Writing could damage unsupported data (newer unsupported schema, incomplete encryption support, permissions prevent safe write).

### 13.5 Cannot open

Root project identity is not recoverable (`project.json` missing and recovery declined, package unreadable, critical permissions failure).

---

## 14. Project Creation Pseudocode

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

  // Return result with firstScene paths.
  // Platform wrapper calls openProject() to load Markdown by value.
```

---

## 15. Open Project Pseudocode

```text
openProject(root):
  project = readProjectJson(root)
  validateSupportedProject(project)

  members = readProjectMembers(project)
  personas = readProjectPersonas(project)

  workspace = readWorkspaceState(project.projectID)

  if workspace has valid lastWritingSurface:
    sceneMeta = readSceneMeta(workspace.lastWritingSurface.sceneID)
    markdown   = readSceneMarkdown(sceneMeta.contentPath)
    // Serialize markdown into JSON result string.
    // Platform wrapper decodes it as a Swift String.
    return OpenProjectResult {
        mode: normalEdit,
        activeScene: sceneMeta,
        activeSceneMarkdown: markdown,
        restoredSelection: workspace.lastWritingSurface.selection,
        restoredScroll: workspace.lastWritingSurface.scroll
    }

  // Fallback: first valid scene.
  fallback = openFirstValidScene(project)
  if fallback.success:
    markdown = readSceneMarkdown(fallback.scene.contentPath)
    writeWorkspaceState(fallback.scene)
    return OpenProjectResult {
        mode: normalEdit,
        activeScene: fallback.scene,
        activeSceneMarkdown: markdown
    }

  return OpenProjectResult { mode: repairRequired, repairIssues: [...] }
```

---

## 16. Success Criteria

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
12. Scene Markdown travels by value from C++ to the platform wrapper via the JSON result envelope; no pointer to C++ memory is returned.
13. The flow remains compatible with future identity transfer, signing, group personas, and encryption.

---

## 17. Open Issues for Later Specs

1. Detailed first-run UI.
2. New project UI.
3. Project browser/recent projects behavior.
4. Repair UI design.
5. Git snapshot UI design.
6. Identity transfer UI/protocol.
7. App-local storage paths per platform (Windows, Linux, Android).
8. Background index lifecycle.
9. Autosave cadence.
10. ~~Multi-window behavior.~~ — **Resolved:** specified in `Scrivi_PerWindow_Project_Model_Design_v0_1.md` (EP-018). See §9.6.
11. iPadOS/visionOS/macOS package behavior.
12. Windows/Linux folder behavior.
13. Multi-scene `openProject` result (scene list for project explorer).

# Scrivi Backend Behavior Spec v0.2

**Project:** Scrivi  
**Document:** Backend Behavior Spec  
**Version:** 0.2  
**Status:** Approved Direction  
**Date:** 2026-05-21  
**Supersedes:** `Scrivi_Backend_Behavior_Spec_v0_1.md`

---

## 1. Purpose

This document defines the operational behavior layer for the Scrivi backend.

The approved documents define:

1. Backend architecture direction (`Scrivi_Architecture_v0_3.md`).
2. Project package structure (`Scrivi_Project_Package_Structure_v0_1.md`).
3. Minimum schema set (`Scrivi_Minimum_Schema_Set_v0_1.md`).

This document defines how those pieces behave during the core lifecycle:

```text
first app startup
create author identity/persona
create project
open project
resume writing
save writing
detect external changes
repair mismatches
optional Git-backed snapshots
rebuild app-local indexes
```

The central product requirement is:

> When a writer opens a Scrivi project, the app should return her to the last writing surface quickly, with the cursor restored, before full project graph/indexing work completes.

**Change from v0.1:** The architecture update (v0.3) formalizes the pass-by-value principle and the JSON-over-`std::string` boundary protocol. Scene Markdown content travels by value from `ScriviCore` through `ScriviCoreAdapter` to the platform wrapper — it is in the result JSON, not a reference into C++ memory. This document updates approved-context references and adds clarifying notes where this principle affects described behavior.

---

## 2. Scope

### In scope for v0.2

- First app startup.
- Author identity and persona creation.
- New project creation.
- Opening an existing project.
- Resuming writing.
- Saving manuscript changes.
- Updating workspace state.
- Detecting external changes.
- Staging and repairing file/metadata mismatches.
- Optional Git-backed snapshots.
- Rebuildable app-local indexes/caches.
- Basic failure handling rules.

### Out of scope for v0.2

- Full object schemas for characters, locations, timelines, or worldbuilding objects.
- Real-time collaboration.
- Pull request workflows.
- Remote Git hosting integration.
- Full branch/merge/conflict resolution UI.
- Project encryption implementation.
- Full cryptographic signing implementation.
- Group/shared persona implementation.
- Rich text format beyond Markdown/markup source.
- Export rendering behavior.

---

## 3. Approved Baseline

```text
Canonical manuscript text: Markdown / markup-oriented source
Canonical metadata and objects: JSON
Assets: original native formats where possible
Project structure: .scrivi package/folder
Manuscript pairing: visible .md + .meta.json files
IDs: UUID v7-style opaque IDs with optional human-friendly slugs
Git: optional Git-backed snapshots in v1
Indexes: app-local, rebuildable; no mandatory SQLite in v1
Roles: owner, editor, reader
Identity: certificate-backed identity + flexible author personas
Security: attribution in v1; signing-ready later
Encryption: future-compatible metadata hooks only
Boundary protocol: JSON-over-std::string at all cross-language boundaries (permanent)
Pass-by-value: all data transmission between layers produces an independent copy in the receiver
```

---

## 4. Core Behavioral Principles

### 4.1 Writing-first launch

Scrivi should not behave like a database browser at launch. On open, the app does the minimum work required to display the last active writing surface and restore the cursor. Full project indexing, graph construction, thumbnail generation, and noncritical validation occur afterward.

### 4.2 Do not silently destroy data

When Scrivi detects external changes or mismatches, it stages them and presents safe repair choices.

Pattern: **Detect → Stage → Review → Commit**

### 4.3 Metadata is visible but app-managed

Visible metadata files are part of the canonical project package. They are inspectable and Git-visible. Arbitrary hand-editing of metadata JSON is not a supported authoring workflow in v1.

### 4.4 IDs are authoritative

File names, titles, slugs, and folder names are human-facing convenience. Stable IDs are the source of truth.

### 4.5 App-local indexes are disposable

Indexes/caches may be deleted and rebuilt. They are not canonical data.

### 4.6 Git is optional

A writer must be able to start writing without Git credentials or Git setup. Git-backed snapshots are optional and may be enabled at project creation or later.

### 4.7 Data moves by value

All data transmission between ScriviCore, the adapter, and the platform wrapper produces an independent, owned copy in the receiver. No layer holds a reference into another layer's memory after a call returns. In practice, this means scene Markdown is included in the JSON result string — the platform wrapper decodes it from its own Swift `String`, not from a C++ pointer.

---

## 5. First App Startup

### 5.1 Goal

On first launch, create the minimum local identity information needed to attribute writing and support future collaboration/security features. The user must not be forced into an account, username, password, or cloud login.

### 5.2 User-facing flow

```text
How should Scrivi identify you as an author?

This name will appear on your writing history, comments, and shared projects.

[ Author Name ]

Continue
```

The name may be a real name, pen name, pseudonym, initials, or other author-facing identity.

### 5.3 Backend behavior

`ScriviCore::ensureLocalIdentity()` creates:

1. Local device identity.
2. Human-controlled Scrivi identity.
3. Default author persona.
4. Public/private key pair or certificate-ready identity material.
5. Secure local storage record.

### 5.4 Storage rules

The private key must not be stored inside any `.scrivi` project package.

Private identity material belongs in platform secure storage:

```text
macOS / iOS / iPadOS / visionOS: Keychain
Windows: Credential Manager / DPAPI
Linux: Secret Service / KWallet / GNOME Keyring, with fallback
Android: Android Keystore
```

MVP uses `PrototypeSecureStore` (in-memory `unordered_map`). This must be replaced with `KeychainSecureStore` before real user testing.

### 5.5 Success criteria

1. The user enters only an author-facing display name.
2. No account is required.
3. No email address is required.
4. A stable identity and persona are created.
5. Private key material remains outside project packages.
6. The user can immediately create or open a project.

---

## 6. New Project Creation

### 6.1 Goal

Create the smallest complete Scrivi project package that can open directly to a writing surface.

### 6.2 Minimum file set

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

If Git-backed snapshots are enabled:

```text
  .git/
  .gitignore
  snapshots/
    scrivi-snapshots.json
```

### 6.3 Project creation steps

`ScriviCore::createProject()`:

1. Create the `.scrivi` package/folder.
2. Generate all required IDs.
3. Create `project.json`.
4. Create `manuscript/manuscript.meta.json`.
5. Create the first chapter folder.
6. Create `chapter.meta.json`.
7. Create the first empty `.md` scene file.
8. Create the paired `scene.meta.json`.
9. Create `identities/project-members.json`.
10. Create `identities/project-personas.json`.
11. Create app-local workspace state.
12. Optionally initialize Git-backed snapshots.
13. Return `CreateProjectResult` with first scene paths.

The platform wrapper then calls `openProject()` to load the scene content by value.

### 6.4 Git option at creation

If enabled, `ScriviCore` (or `enableGitSnapshots()`) initializes a Git repository, generates `.gitignore`, creates `snapshots/scrivi-snapshots.json`, creates the initial snapshot/commit, and maps the active persona to Git author metadata.

### 6.5 Git credentials

Git credentials are not required for local snapshots. Remote Git hosting, pushing, pulling, and authentication are out of scope for v1.

### 6.6 Success criteria

1. A complete minimal project exists on disk.
2. The writer sees an editable scene immediately.
3. The project opens without needing indexing.
4. Authorship metadata is present.
5. Git is optional.
6. A project without Git remains fully valid.

---

## 7. Open Existing Project Flow

### 7.1 Goal

Open a Scrivi project quickly and safely.

### 7.2 Minimum open sequence

`ScriviCore::openProject()`:

1. Locate and read `project.json`.
2. Validate basic project format/version.
3. Read `identities/project-members.json`.
4. Read `identities/project-personas.json`.
5. Load app-local workspace state if present.
6. Resolve last writing surface from workspace state.
7. If valid, load that scene metadata.
8. Load that scene Markdown file.
9. Serialize scene Markdown into the JSON result string.
10. Return `OpenProjectResult` — platform wrapper decodes `activeScene.markdown` from the result JSON.

The platform wrapper restores cursor and scroll from `restoredSelection` and `restoredScroll` in the result.

### 7.3 Fallback path

If workspace state is missing or invalid:

1. Read `manuscript/manuscript.meta.json`.
2. Resolve the first chapter in canonical order.
3. Read that chapter's `chapter.meta.json`.
4. Resolve the first scene in canonical order.
5. Read that scene's metadata.
6. Load the scene Markdown file.
7. Serialize Markdown into result JSON.
8. Return result; platform wrapper places cursor at document start.

### 7.4 What should not block writing surface display

```text
full project graph construction
full-text indexing
relationship indexing
thumbnail generation
asset validation
comment indexing
timeline indexing
Git status scan
deep schema validation of every object
```

### 7.5 Success criteria

1. The writer reaches the last scene quickly.
2. The cursor and scroll position are restored when available.
3. Missing workspace state does not prevent opening.
4. Full indexing can finish after the editor appears.
5. Serious structural errors are surfaced without data loss.

---

## 8. Resume Writing Flow

### 8.1 Goal

Return the writer to the exact writing context where she left off.

### 8.2 Workspace state source

Workspace state is app-local, not canonical project data.

Example location:

```text
Scrivi App Support/
  state/
    projects/
      <projectID>/
        workspace-state.json
```

### 8.3 Workspace state may include

```text
project ID
device ID
identity ID
active persona ID
last scene ID
last content path
cursor anchor/focus
scroll position
last opened timestamp
visible panels
window/layout hints
recent navigation
```

### 8.4 Cursor restoration rules

Scrivi should restore the cursor if:

1. The scene ID exists.
2. The content path exists.
3. The Markdown file can be opened.
4. The cursor range is still valid for the current text length.

If the cursor range is invalid, place the cursor at the nearest valid location or the start of the document.

### 8.5 Note on content delivery

The writing surface is populated from `OpenProjectResult.activeSceneMarkdown`, which the platform wrapper decoded from the JSON result string. This is a Swift `String` value — a complete, owned copy of the scene content. No subsequent fetch call is needed.

### 8.6 Success criteria

1. The active scene is displayed.
2. Cursor/selection are restored when valid.
3. Scroll position is restored when valid.
4. Invalid workspace state falls back safely.
5. The project itself is not modified just because workspace state is missing.

---

## 9. Save Scene Flow

### 9.1 Goal

Persist manuscript edits safely while updating only the necessary metadata.

### 9.2 Save steps

`ScriviCore::saveScene()`:

1. Write the `.md` content safely (atomic write).
2. Update `modifiedAt` in the paired `.meta.json`.
3. Update `modifiedBy`.
4. Update derived stats if enabled.
5. Update app-local workspace state.
6. Mark app-local indexes dirty.
7. If Git is enabled, mark project as having unsnapshotted changes.
8. Return `SaveSceneResult`.

The platform wrapper passes `markdown` as a `const char*` parameter (via `withCString`). The Markdown string is consumed synchronously by the adapter; ownership never transfers to C++.

### 9.3 Safe write behavior

```text
Write temporary file.
Flush/sync as appropriate.
Atomically replace target file.
Preserve encoding as UTF-8.
```

### 9.4 Metadata update

`modifiedBy` identifies the active identity/persona:

```json
{
  "modifiedAt": "2026-05-21T12:00:00Z",
  "modifiedBy": {
    "identityID": "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8",
    "personaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8",
    "displayNameAtModification": "Rhozwyn Darius"
  }
}
```

### 9.5 External edit attribution

If Scrivi detects an external edit, it must not falsely attribute that change to the current Scrivi identity:

```json
{ "modifiedBy": { "source": "external" } }
```

or:

```json
{ "modifiedBy": null }
```

### 9.6 Derived stats

Scene metadata may include non-authoritative derived stats: word count, character count, last calculated timestamp. These may be recalculated.

### 9.7 Success criteria

1. Manuscript text is durable.
2. Only necessary metadata changes are made.
3. App-local workspace state is updated.
4. Derived stats can be recalculated.
5. External edits are not falsely attributed.
6. Git-enabled projects clearly show unsnapshotted changes.

---

## 10. External Change Detection

### 10.1 Goal

Respond safely when users or external tools modify the project package.

### 10.2 Detection sources

```text
filesystem watching
file timestamps
content hashing
Git status
periodic validation
manual refresh
open-project validation
```

### 10.3 Detection policy

Distinguish:

1. Safe external manuscript edit.
2. Missing manuscript file.
3. Missing metadata file.
4. New orphan manuscript file.
5. New orphan metadata file.
6. Renamed file/folder.
7. Asset added/replaced/deleted.
8. Git checkout/merge changed many files.
9. Schema/version mismatch.
10. Corrupt or unreadable JSON.

### 10.4 Success criteria

1. Common external manuscript edits are accepted safely.
2. Mismatches are staged for review.
3. Scrivi does not silently delete unknown data.
4. The writer can repair project structure without hand-editing JSON.
5. Git checkouts are treated as normal possible external changes.

---

## 11. Repair and Staging Matrix

### 11.1 General rule

When project structure is ambiguous, Scrivi stages the issue and asks for a repair action. It does not guess silently unless confidence is very high and the operation is reversible.

See `Scrivi_External_Change_Repair_Matrix_v0_2.md` for the full 20-case matrix.

---

### 11.2 `.md` exists, `.meta.json` exists, both valid

**Behavior:** Open normally. Index/update as needed.

---

### 11.3 `.md` changed externally, metadata still valid

**Behavior:** Reload or prompt depending on unsaved in-app changes. Update derived stats. Mark `modifiedBy` as external or unknown. Mark indexes dirty. If Git enabled, show unsnapshotted change.

**Do not:** Overwrite external changes silently. Attribute external changes to current identity.

---

### 11.4 `.md` exists, `.meta.json` missing

**Stage as:** Unregistered manuscript file found.

**Actions:** Import as new scene, Attach to existing scene, Move to inbox, Ignore, Delete after confirmation.

---

### 11.5 `.meta.json` exists, `.md` missing

**Stage as:** Scene metadata exists, but manuscript file is missing.

**Actions:** Restore from Git snapshot if available, Create empty manuscript file, Relink to another Markdown file, Mark scene as missing, Remove scene from project after confirmation, Ignore for now.

---

### 11.6 `.md` renamed externally, metadata still points to old path

**Stage as:** Possible renamed manuscript file detected.

**Actions:** Relink metadata to new file, Import new file as separate scene, Restore old file path, Ignore.

---

### 11.7 `.meta.json` renamed externally

**Behavior:** Stage possible metadata rename. Offer relink. Do not silently rewrite chapter order unless confident.

---

### 11.8 Chapter folder renamed externally

**Behavior:** Stage possible chapter folder rename. Offer relink chapter path. Preserve chapter ID.

---

### 11.9 New files dropped into `assets/`

**Behavior:** Stage as new asset. Offer to create paired asset metadata or ignore.

---

### 11.10 New files dropped into `inbox/dropped-files/`

**Behavior:** Present import/review queue. Actions: Import as asset, Import as manuscript scene, Import as research document, Ignore, Delete after confirmation.

---

### 11.11 Git checkout changed project

**Behavior:** Pause unsafe writes. Refresh project manifest. Reload affected open files. Validate current writing surface. Rebuild dirty indexes. Stage unresolved mismatches. If the currently open scene changed on disk while unsaved in-app edits exist, preserve both versions.

---

### 11.12 Corrupt JSON metadata

**Behavior:** Stage as corrupt metadata. Show file path. Offer restore from snapshot if Git enabled. Offer move aside and regenerate where possible.

**Do not:** Overwrite corrupt metadata without backup. Delete corrupt metadata automatically.

---

### 11.13 Unsupported schema version

**Behavior:** Open read-only if safe. Warn user. Avoid rewriting unsupported files.

---

## 12. Optional Git-Backed Snapshot Behavior

### 12.1 Goal

Provide writer-friendly local version history using Git without requiring the user to understand Git.

### 12.2 Enable Git at project creation

`ScriviCore` (via `createProject` with `enableGitSnapshots=true` or via `enableGitSnapshots()`):

1. Initializes Git.
2. Creates `.gitignore`.
3. Creates snapshot metadata.
4. Creates initial commit/snapshot.

### 12.3 Enable Git later

1. Checks whether project is already in a Git repository.
2. If not, initializes Git.
3. Creates `.gitignore`.
4. Creates snapshot metadata.
5. Creates initial snapshot representing current project state.

### 12.4 Snapshot creation

`ScriviCore::createSnapshot()`:

```text
git add canonical project files
git commit with Scrivi-generated message
record friendly snapshot metadata
```

### 12.5 Git author mapping

Git author name: active persona display name.

Git email: stable Scrivi identity metadata.

Example: `Rhozwyn Darius <identity_01J8Z7Y9@scrivi.author>`

### 12.6 Snapshot metadata

`snapshots/scrivi-snapshots.json` maps Scrivi-friendly snapshot data to Git commits:

```text
snapshot ID
Git commit hash
label
createdAt
createdBy identity
persona ID
display name at snapshot
optional notes
```

### 12.7 Out of scope for v1

Remote hosting setup, GitHub/GitLab integration, pull requests, full branch UI, complex merge UI, automatic conflict resolution, multi-author remote sync.

### 12.8 Success criteria

1. Git is optional.
2. Local snapshots work without remote credentials.
3. Snapshots use Scrivi author/persona attribution.
4. Non-Git projects remain fully valid.
5. Git-generated artifacts do not pollute canonical project state.

---

## 13. Index and Cache Behavior

### 13.1 v1 default

No mandatory SQLite. App-local indexes are optional and rebuildable.

### 13.2 App-local cache

Indexes live outside the project package:

```text
Scrivi App Support/
  cache/
    projects/
      project_01J.../
        search.index
        graph.index
        thumbnails/
```

### 13.3 Rebuild behavior

Indexes may be rebuilt when: project opens, metadata changes, manuscript changes, external changes are detected, Git checkout changes files, user requests repair/rebuild, cache version changes.

### 13.4 Success criteria

1. Project remains valid without cache.
2. Cache can be deleted and rebuilt.
3. Cache is not committed to Git.
4. Search/navigation can improve over time without changing canonical format.
5. Opening the last writing surface does not wait on full indexing.

---

## 14. App-Local Workspace State Behavior

### 14.1 Workspace state includes

```text
last active scene
cursor
selection
scroll position
active persona
last opened timestamp
visible panels
window/layout hints
recent navigation
```

### 14.2 Workspace state does not include

```text
canonical manuscript text
canonical object definitions
project membership
private keys
Git history
```

### 14.3 Multi-device behavior

Each device may have its own workspace state. Workspace state remains local unless sync behavior is explicitly designed.

### 14.4 Success criteria

1. It restores the writing surface.
2. It can be missing without damaging the project.
3. It can be deleted and recreated.
4. It does not overwrite another user's project content.
5. It separates device state from authorship identity.

---

## 15. Identity Transfer Behavior

### 15.1 Approved direction

Preferred model: QR transfer (preferred), encrypted transfer file (fallback).

### 15.2 QR transfer

QR payload establishes a local encrypted transfer session:

```text
transfer session ID
local endpoint or rendezvous hint
short-lived public key
expiration timestamp
verification code
```

### 15.3 Encrypted transfer file fallback

File name: `<persona-or-author-name>.scriviauthor`. Must be encrypted with a passphrase.

### 15.4 Security warning

```text
This transfer allows another device to write as this Scrivi identity.
Only transfer it to a device you control.
```

### 15.5 Success criteria

1. Author identity can move to another device.
2. Transfer does not require cloud account login.
3. Private key material is encrypted in transit.
4. QR is the preferred UX.
5. Transfer file is available as a reliable fallback.

---

## 16. Failure Handling Rules

### 16.1 General failure rule

Scrivi should prefer preserving data over preserving clean state. If something is ambiguous, stage it.

### 16.2 Never silently overwrite external changes

If Scrivi has unsaved in-app changes and the file changed on disk, it must preserve both versions or ask the user.

### 16.3 Never delete unknown files automatically

Unknown files should be ignored, staged, or moved only with user confirmation.

### 16.4 Never rewrite unsupported newer schemas

If a file uses a newer schema Scrivi cannot understand, avoid rewriting it.

### 16.5 Prefer repair actions over fatal errors

A project with missing metadata or missing manuscript files should open into a repair mode where possible.

### 16.6 Keep backups before destructive repair

Before regenerating, removing, or replacing metadata, Scrivi should preserve a backup or require a Git snapshot where appropriate.

---

## 17. Minimum Backend Loop

The first backend implementation proves this loop:

```text
1.  First launch creates local identity/persona.
2.  User creates project.
3.  Scrivi creates minimum file set.
4.  ScriviCoreAdapter serializes openProject result JSON.
5.  Platform wrapper decodes activeScene.markdown by value.
6.  User writes text.
7.  Platform wrapper passes markdown as const char* to saveScene.
8.  Scrivi saves Markdown and updates metadata.
9.  Scrivi records workspace state.
10. User closes project.
11. User reopens project.
12. Scrivi restores last scene and cursor (Markdown by value in result JSON).
13. User edits scene externally.
14. Scrivi detects external edit and reloads/stages safely.
15. Optional: user creates Git snapshot.
```

This loop is the first real backend success target.

---

## 18. Success Criteria

1. A writer can start Scrivi for the first time without creating an online account.
2. A writer can create a new project and immediately start writing.
3. A project opens to the last writing surface quickly.
4. Cursor and scroll position are restored when valid.
5. The app does not need to load the full project graph before showing prose.
6. Manuscript text saves safely.
7. Visible paired metadata remains synchronized during normal Scrivi edits.
8. External edits are detected without silent data loss.
9. Missing or mismatched files are staged for repair.
10. Git-backed snapshots are optional and local-first.
11. Projects remain valid without app-local caches.
12. SQLite is not required in v1.
13. Private identity keys are never stored inside project packages.
14. Scene Markdown crosses the C++/platform boundary by value — in the JSON result string, not as a raw pointer.
15. Future signing, group personas, and encryption remain possible without replacing the project model.

---

## 19. Open Issues for Later Specs

1. Asset metadata schema and behavior.
2. Comment schema and comment anchoring behavior.
3. Object schemas for characters, locations, items, rules, and timelines.
4. Relationship graph model.
5. Git snapshot UI design.
6. External change repair UI design.
7. Identity transfer UI and protocol.
8. Future signing behavior.
9. Future encryption behavior.
10. Export behavior.
11. Import behavior.
12. Search/index behavior if SQLite becomes necessary.
13. `KeychainSecureStore` implementation sprint.
14. `deviceID` stable identifier mechanism per platform.
15. `JsonDoc` double support for `ScrollPosition`.

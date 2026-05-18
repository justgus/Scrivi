# Scrivi Backend Behavior Spec v0.1

**Project:** Scrivi  
**Document:** Backend Behavior Spec  
**Version:** 0.1  
**Status:** Draft / Approved Direction Pending Review  
**Role Context:** App Designer supporting Project Lead  
**Date:** 2026-05-18

---

## 1. Purpose

This document defines the first operational behavior layer for the Scrivi backend.

Previous approved documents define:

1. Backend architecture direction.
2. Project package structure.
3. Minimum schema set.

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

---

## 2. Scope

### In scope for v0.1

This behavior spec covers:

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

### Out of scope for v0.1

This document does not define:

- Full object schemas for characters, locations, timelines, or worldbuilding objects.
- Real-time collaboration.
- Pull request workflows.
- Remote Git hosting integration.
- Google Docs integration.
- Full branch/merge/conflict resolution UI.
- Project encryption implementation.
- Full cryptographic signing implementation.
- Group/shared persona implementation.
- Rich text format beyond Markdown/markup source.
- Export rendering behavior.

---

## 3. Approved Baseline

This behavior spec assumes the following approved backend decisions:

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
```

---

## 4. Core Behavioral Principles

### 4.1 Writing-first launch

Scrivi should not behave like a database browser at launch.

On open, the app should do the minimum work required to display the last active writing surface and restore the cursor.

Full project indexing, graph construction, thumbnail generation, and noncritical validation may occur afterward.

### 4.2 Do not silently destroy data

When Scrivi detects external changes or mismatches, it should stage them and present safe repair choices.

The preferred pattern is:

```text
Detect → Stage → Review → Commit
```

### 4.3 Metadata is visible but app-managed

Visible metadata files are part of the canonical project package. They are inspectable and Git-visible.

However, arbitrary hand-editing of metadata JSON is not a supported authoring workflow in v1.

### 4.4 IDs are authoritative

File names, titles, slugs, and folder names are human-facing convenience.

Stable IDs are the source of truth.

### 4.5 App-local indexes are disposable

Indexes/caches may be deleted and rebuilt. They are not canonical data.

### 4.6 Git is optional

A writer must be able to start writing without Git credentials or Git setup.

Git-backed snapshots are optional and may be enabled at project creation or later.

---

## 5. First App Startup

### 5.1 Goal

On first launch, Scrivi creates the minimum local identity information needed to attribute writing and support future collaboration/security features.

The user should not be forced into an account, username, password, or cloud login.

### 5.2 User-facing flow

Scrivi asks:

```text
How should Scrivi identify you as an author?

This name will appear on your writing history, comments, and shared projects.

[ Author Name ]

Continue
```

The name may be a real name, pen name, pseudonym, initials, or other author-facing identity.

### 5.3 Backend behavior

Scrivi creates:

1. Local device identity.
2. Human-controlled Scrivi identity.
3. Default author persona.
4. Public/private key pair or certificate-ready identity material.
5. Secure local storage record.

### 5.4 Storage rules

The private key must not be stored inside any `.scrivi` project package.

Private identity material belongs in platform secure storage where available:

```text
macOS / iOS / iPadOS / visionOS: Keychain
Windows: Credential Manager / DPAPI
Linux: Secret Service / KWallet / GNOME Keyring, with fallback
Android: Android Keystore
```

### 5.5 Success criteria

First app startup is successful if:

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

A new project with one chapter and one scene contains:

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
MyNovel.scrivi/
  .git/
  .gitignore
  snapshots/
    scrivi-snapshots.json
```

### 6.3 Project creation steps

Scrivi should:

1. Create the `.scrivi` package/folder.
2. Generate a project ID.
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
13. Open the new scene with cursor at the start.

### 6.4 Git option at creation

The project creation UI may include:

```text
Version History

[ ] Track this project with Git-backed snapshots
```

If unchecked, no Git setup occurs.

If checked, Scrivi:

1. Initializes a Git repository.
2. Generates `.gitignore`.
3. Creates `snapshots/scrivi-snapshots.json`.
4. Creates the initial snapshot/commit.
5. Maps the active persona to Git author metadata.

### 6.5 Git credentials

Git credentials are not required for local snapshots.

Remote Git hosting, pushing, pulling, and authentication are out of scope for v1.

### 6.6 Success criteria

New project creation is successful if:

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

Scrivi should:

1. Locate and read `project.json`.
2. Validate basic project format/version.
3. Read identity/persona paths from `project.json`.
4. Read `identities/project-members.json`.
5. Read `identities/project-personas.json`.
6. Load app-local workspace state if present.
7. Resolve last writing surface from workspace state.
8. If valid, load that scene metadata.
9. Load that scene Markdown file.
10. Restore cursor and scroll position.
11. Display writing surface.
12. Begin noncritical background validation/indexing.

### 7.3 Fallback path

If workspace state is missing or invalid:

1. Read `manuscript/manuscript.meta.json`.
2. Resolve the first chapter in canonical order.
3. Read that chapter's `chapter.meta.json`.
4. Resolve the first scene in canonical order.
5. Read that scene's metadata.
6. Open the scene Markdown file.
7. Place cursor at document start.

### 7.4 What should not block writing surface display

The following should not block the initial writing surface unless required for safety:

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

Open project is successful if:

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

Example app-local location:

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

If the cursor range is invalid, Scrivi should place the cursor at the nearest valid location or the start of the document.

### 8.5 Success criteria

Resume writing is successful if:

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

When Scrivi saves a scene, it should:

1. Write the `.md` content safely.
2. Update `modifiedAt` in the paired `.meta.json`.
3. Update `modifiedBy`.
4. Update derived stats if enabled.
5. Update app-local workspace state.
6. Mark app-local indexes dirty.
7. If Git is enabled, mark project as having unsnapshotted changes.

### 9.3 Safe write behavior

Scrivi should use a safe write strategy appropriate to the platform.

Preferred behavior:

```text
write temporary file
flush/sync as appropriate
atomically replace target file
preserve encoding as UTF-8
```

### 9.4 Metadata update

When Scrivi performs the edit, `modifiedBy` should identify the active identity/persona.

Example:

```json
{
  "modifiedAt": "2026-05-18T18:30:00Z",
  "modifiedBy": {
    "identityID": "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8",
    "personaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8",
    "displayNameAtModification": "Rhozwyn Darius"
  }
}
```

### 9.5 External edit attribution

If Scrivi detects that an external editor changed the `.md` file, Scrivi must not falsely attribute that change to the current Scrivi identity.

Recommended metadata behavior:

```json
{
  "modifiedBy": {
    "source": "external"
  }
}
```

or:

```json
{
  "modifiedBy": null
}
```

The approved recommendation is to identify external source rather than invent authorship.

### 9.6 Derived stats

Scene metadata may include derived stats such as:

```text
word count
character count
last calculated timestamp
```

These stats are non-authoritative and may be recalculated.

### 9.7 Success criteria

Save scene is successful if:

1. Manuscript text is durable.
2. Only necessary metadata changes are made.
3. App-local workspace state is updated.
4. Derived stats can be recalculated.
5. External edits are not falsely attributed.
6. Git-enabled projects clearly show unsnapshotted changes.

---

## 10. External Change Detection

### 10.1 Goal

Scrivi should respond safely when users or external tools modify the project package.

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
```

### 10.2 Detection sources

Scrivi may use:

```text
filesystem watching
file timestamps
content hashing
Git status
periodic validation
manual refresh
open-project validation
```

No single method should be assumed perfect.

### 10.3 Detection policy

Scrivi should distinguish:

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

External change detection is successful if:

1. Common external manuscript edits are accepted safely.
2. Mismatches are staged for review.
3. Scrivi does not silently delete unknown data.
4. The writer can repair project structure without hand-editing JSON.
5. Git checkouts are treated as normal possible external changes.

---

## 11. Repair and Staging Matrix

### 11.1 General rule

When project structure is ambiguous, Scrivi should stage the issue and ask for a repair action.

It should not guess silently unless confidence is very high and the operation is reversible.

---

### 11.2 `.md` exists, `.meta.json` exists, both valid

**Condition:** Normal paired manuscript file.

**Behavior:**

```text
Open normally
Index/update as needed
```

---

### 11.3 `.md` changed externally, metadata still valid

**Condition:** The Markdown file changed outside Scrivi.

**Behavior:**

```text
Reload or prompt depending on unsaved in-app changes
Update derived stats
Mark modifiedBy as external or unknown
Mark indexes dirty
If Git enabled, show unsnapshotted change
```

**Do not:**

```text
Overwrite external changes silently
Attribute external changes to current identity
```

---

### 11.4 `.md` exists, `.meta.json` missing

**Condition:** Unregistered manuscript file or lost metadata.

**Stage as:**

```text
Unregistered manuscript file found
```

**Possible actions:**

```text
Import as new scene
Attach to existing scene
Move to inbox
Ignore
Delete after confirmation
```

**Recommended default:** Stage and ask.

---

### 11.5 `.meta.json` exists, `.md` missing

**Condition:** Scene metadata exists, but content file is missing.

**Stage as:**

```text
Scene metadata exists, but manuscript file is missing
```

**Possible actions:**

```text
Restore from Git snapshot if available
Create empty manuscript file
Relink to another Markdown file
Mark scene as missing
Remove scene from project after confirmation
Ignore for now
```

**Recommended default:** Stage and ask.

---

### 11.6 `.md` renamed externally, metadata still points to old path

**Condition:** Old content path missing; new similar Markdown file found nearby.

**Stage as:**

```text
Possible renamed manuscript file detected
```

**Possible actions:**

```text
Relink metadata to new file
Import new file as separate scene
Restore old file path
Ignore
```

**Recommended default:** Offer relink only when confidence is strong.

Confidence signals may include:

```text
similar filename
same folder
same content hash or near hash
same title heading
recent timestamp relationship
Git rename detection
```

---

### 11.7 `.meta.json` renamed externally

**Condition:** Chapter scene list points to old metadata path; similar metadata file found.

**Behavior:**

```text
Stage possible metadata rename
Offer relink
Do not silently rewrite chapter order unless confident
```

---

### 11.8 Chapter folder renamed externally

**Condition:** `manuscript.meta.json` points to old chapter path; similar folder exists.

**Behavior:**

```text
Stage possible chapter folder rename
Offer relink chapter path
Preserve chapter ID
```

---

### 11.9 New files dropped into `assets/`

**Condition:** New native asset file appears.

**Behavior:**

```text
Stage as new asset
Offer to create paired asset metadata
Offer to ignore
```

If an asset metadata schema is not yet defined, Scrivi may list it as an unmanaged asset.

---

### 11.10 New files dropped into `inbox/dropped-files/`

**Condition:** User intentionally dropped files into inbox.

**Behavior:**

```text
Present import/review queue
```

Possible actions:

```text
Import as asset
Import as manuscript scene
Import as research document
Ignore
Delete after confirmation
```

---

### 11.11 Git checkout changed project

**Condition:** Many project files changed at once due to Git.

**Behavior:**

```text
Pause unsafe writes
Refresh project manifest
Reload affected open files
Validate current writing surface
Rebuild dirty indexes
Stage unresolved mismatches
```

If the currently open scene changed on disk while unsaved in-app edits exist, Scrivi must not overwrite either version silently.

---

### 11.12 Corrupt JSON metadata

**Condition:** JSON cannot be parsed.

**Behavior:**

```text
Stage as corrupt metadata
Show file path
Offer restore from snapshot if Git enabled
Offer move aside and regenerate where possible
Offer open containing folder
```

**Do not:**

```text
Overwrite corrupt metadata without backup
Delete corrupt metadata automatically
```

---

### 11.13 Unsupported schema version

**Condition:** Metadata schema version is newer than this Scrivi build supports.

**Behavior:**

```text
Open read-only if safe
Warn user
Avoid rewriting unsupported files
```

---

## 12. Optional Git-Backed Snapshot Behavior

### 12.1 Goal

Provide writer-friendly local version history using Git without requiring the user to understand Git.

### 12.2 Enable Git at project creation

If selected, Scrivi:

1. Initializes Git.
2. Creates `.gitignore`.
3. Creates snapshot metadata.
4. Creates initial commit/snapshot.

### 12.3 Enable Git later

If Git is enabled later, Scrivi:

1. Checks whether project is already in a Git repository.
2. If not, initializes Git.
3. Creates `.gitignore`.
4. Creates snapshot metadata.
5. Creates initial snapshot representing current project state.

### 12.4 Snapshot creation

User-facing language:

```text
Create Snapshot
```

Backend behavior:

```text
git add canonical project files
git commit with Scrivi-generated message
record friendly snapshot metadata
```

### 12.5 Git author mapping

Git author name should use the active persona display name.

Git email may use stable Scrivi identity metadata rather than a real email.

Example:

```text
Rhozwyn Darius <identity_01J8Z7Y9@scrivi.author>
```

### 12.6 Snapshot metadata

`snapshots/scrivi-snapshots.json` maps Scrivi-friendly snapshot data to Git commits.

Example responsibilities:

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

```text
remote hosting setup
GitHub/GitLab integration
pull requests
full branch UI
complex merge UI
automatic conflict resolution
multi-author remote sync
```

### 12.8 Success criteria

Git snapshot behavior is successful if:

1. Git is optional.
2. Local snapshots work without remote credentials.
3. Snapshots use Scrivi author/persona attribution.
4. Non-Git projects remain fully valid.
5. Git-generated artifacts do not pollute canonical project state.

---

## 13. Index and Cache Behavior

### 13.1 Goal

Support fast enough navigation/search without making indexes canonical.

### 13.2 v1 default

Approved v1 behavior:

```text
D1 initially: no mandatory SQLite
D2 later if needed: app-local SQLite cache
```

### 13.3 App-local cache

Indexes live outside the project package.

Example:

```text
Scrivi App Support/
  cache/
    projects/
      project_01J.../
        search.index
        graph.index
        thumbnails/
```

### 13.4 Rebuild behavior

Indexes may be rebuilt when:

```text
project opens
metadata changes
manuscript changes
external changes are detected
Git checkout changes files
user requests repair/rebuild
cache version changes
```

### 13.5 SQLite policy

SQLite is allowed later as an implementation of app-local cache.

It is not required in v1.

SQLite databases should not be canonical project data.

### 13.6 Success criteria

Index/cache behavior is successful if:

1. Project remains valid without cache.
2. Cache can be deleted and rebuilt.
3. Cache is not committed to Git.
4. Search/navigation can improve over time without changing canonical format.
5. Opening the last writing surface does not wait on full indexing.

---

## 14. App-Local Workspace State Behavior

### 14.1 Goal

Persist per-device/per-identity working state without polluting canonical project source.

### 14.2 Workspace state includes

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

### 14.3 Workspace state does not include

```text
canonical manuscript text
canonical object definitions
project membership
private keys
Git history
```

### 14.4 Multi-device behavior

Each device may have its own workspace state.

Future identity transfer may allow author identity to move across devices, but workspace state remains local unless later sync behavior is explicitly designed.

### 14.5 Success criteria

Workspace state behavior is successful if:

1. It restores the writing surface.
2. It can be missing without damaging the project.
3. It can be deleted and recreated.
4. It does not overwrite another user's project content.
5. It separates device state from authorship identity.

---

## 15. Identity Transfer Behavior

### 15.1 Approved direction

Preferred model:

```text
C4: QR transfer preferred, encrypted transfer file fallback
C3 design: QR establishes secure local transfer session
```

### 15.2 QR transfer

The QR code should not need to contain the full identity bundle.

Instead, it should establish a local encrypted transfer session.

Possible QR payload:

```text
transfer session ID
local endpoint or rendezvous hint
short-lived public key
expiration timestamp
verification code
```

### 15.3 Encrypted transfer file fallback

Fallback file:

```text
<persona-or-author-name>.scriviauthor
```

The transfer file must be encrypted with a passphrase.

### 15.4 Security warning

The UI should communicate:

```text
This transfer allows another device to write as this Scrivi identity.
Only transfer it to a device you control.
```

### 15.5 Success criteria

Identity transfer is successful if:

1. Author identity can move to another device.
2. Transfer does not require cloud account login.
3. Private key material is encrypted in transit.
4. QR is the preferred UX.
5. Transfer file is available as a reliable fallback.

---

## 16. Failure Handling Rules

### 16.1 General failure rule

Scrivi should prefer preserving data over preserving clean state.

If something is ambiguous, stage it.

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

The first backend implementation should prove this loop:

```text
1. First launch creates local identity/persona.
2. User creates project.
3. Scrivi creates minimum file set.
4. Scrivi opens first scene.
5. User writes text.
6. Scrivi saves Markdown and updates metadata.
7. Scrivi records workspace state.
8. User closes project.
9. User reopens project.
10. Scrivi restores last scene and cursor.
11. User edits scene externally.
12. Scrivi detects external edit and reloads/stages safely.
13. Optional: user creates Git snapshot.
```

This loop is the first real backend success target.

---

## 18. Success Criteria

The backend behavior described here is successful if:

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
14. Future signing, group personas, and encryption remain possible without replacing the project model.

---

## 19. Open Issues for Later Specs

The following require separate future specs:

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

---

## 20. Recommended Next Document

The next backend/design document should be:

```text
Scrivi External Change Repair Matrix v0.1
```

This may either expand Section 11 of this document or become a standalone implementation-facing spec.

A second good next document would be:

```text
Scrivi Project Creation and Open Flow v0.1
```

That would translate the behavior in this document into step-by-step app flows and backend function boundaries.

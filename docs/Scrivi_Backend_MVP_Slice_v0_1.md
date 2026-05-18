# Scrivi Backend MVP Slice v0.1

**Project:** Scrivi  
**Document:** Backend MVP Slice  
**Version:** 0.1  
**Status:** Draft / Approved Direction Pending Review  
**Date:** 2026-05-18

---

## 1. Purpose

This document defines the first implementable backend slice for Scrivi.

The goal is not to build the whole Scrivi backend. The goal is to prove the smallest useful end-to-end loop:

```text
create identity
create project
open project
resume writing
save writing
reopen project
restore cursor
detect simple external changes
optionally create Git-backed snapshots
```

This MVP slice converts the approved backend architecture into a concrete implementation target.

---

## 2. Design Context

This MVP slice is based on the approved direction from:

```text
Scrivi Backend Architecture v0.2
Scrivi Project Package Structure v0.1
Scrivi Minimum Schema Set v0.1
Scrivi Backend Behavior Spec v0.1
Scrivi External Change Repair Matrix v0.1
Scrivi Project Creation and Open Flow v0.1
```

Approved baseline:

```text
Project format: .scrivi package/folder
Manuscript text: Markdown / markup-oriented source
Metadata: visible JSON files
Manuscript pairing: .md + .meta.json
IDs: UUID v7-style opaque IDs with optional slugs
Indexes: app-local, rebuildable, no mandatory SQLite in MVP
Git: optional local Git-backed snapshots
Identity: local certificate-ready identity + personas
Encryption: future-compatible metadata hooks only
```

---

## 3. MVP Goal

The MVP backend is successful when Scrivi can:

1. Create a local author identity and default persona.
2. Create a new `.scrivi` project.
3. Create the approved minimum project file set.
4. Open the project.
5. Display the first or last active writing surface.
6. Save Markdown text.
7. Update scene metadata.
8. Store app-local workspace state.
9. Close and reopen the project.
10. Restore the last scene and cursor position.
11. Detect simple external manuscript edits.
12. Detect missing `.md` or `.meta.json` files at open.
13. Optionally enable local Git-backed snapshots.

---

## 4. Non-Goals

The MVP slice must not expand into the full product.

The following are explicitly out of scope:

```text
full character/location/item/rule/timeline object model
relationship graph
comments UI or comment anchoring
assets import/management
image/video/audio/document metadata
SQLite indexing
full-text search
advanced app-local indexing
real-time collaboration
Google Docs integration
remote Git hosting
GitHub/GitLab integration
pull requests
branch UI
merge/conflict resolution UI
identity transfer
group/shared personas
project encryption
cryptographic signing enforcement
exports to PDF/EPUB/DOCX
rich-text editor engine
platform-specific visual design
```

Some out-of-scope areas may have placeholder fields if already approved as future-compatible hooks, but they should not be implemented in this MVP.

---

## 5. Required Project Files

A minimum valid project created by the MVP contains:

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

---

## 6. Required App-Local Files

The MVP requires app-local state outside the project package.

Minimum app-local records:

```text
Scrivi App Support/
  identity/
    local-identity.json or secure-store equivalent

  state/
    projects/
      <projectID>/
        workspace-state.json

  cache/
    projects/
      <projectID>/
        placeholder or simple rebuildable index state
```

Actual private key material should use platform secure storage where available.

For the MVP, if secure storage abstraction is not yet finalized, the implementation must still separate private identity material from project packages.

---

## 7. Core Backend Operations

The MVP backend should expose or internally support these operations.

Names are illustrative, not final API.

---

### 7.1 `ensureLocalIdentity()`

#### Purpose

Ensure that the current installation has a local Scrivi identity and at least one author persona.

#### Input

```text
optional author display name
```

#### Output

```text
identity ID
default persona ID
display name
public key or certificate-ready identity metadata
```

#### Behavior

If identity exists:

```text
return existing identity/persona
```

If identity does not exist:

```text
create device identity
create Scrivi identity
create key pair or certificate-ready identity material
create default persona
store private material outside project packages
return identity/persona
```

#### MVP constraints

No cloud account.

No email address.

No username/password required.

---

### 7.2 `createProject()`

#### Purpose

Create a new minimum Scrivi project package.

#### Input

```text
project title
project location
active identity ID
active persona ID
optional initial chapter title
optional initial scene title
optional Git enabled flag
```

#### Output

```text
project ID
project root path
first chapter ID
first scene ID
first scene content path
```

#### Behavior

Create:

```text
project.json
manuscript/manuscript.meta.json
chapter.meta.json
opening scene Markdown
opening scene metadata
project members
project personas
workspace state
```

If Git is enabled:

```text
initialize Git repository
write .gitignore
create snapshots/scrivi-snapshots.json
create initial snapshot
```

#### Success condition

After creation, Scrivi can immediately open the first scene and place the cursor in the writing surface.

---

### 7.3 `openProject()`

#### Purpose

Open an existing project and resolve the writing surface to display.

#### Input

```text
project root path
current identity ID if available
```

#### Output

```text
project model summary
active scene metadata
active scene Markdown content
cursor/selection/scroll state
repair warnings if any
```

#### Fast path behavior

```text
read project.json
read project-members.json
read project-personas.json
read app-local workspace-state.json
resolve last scene
read scene.meta.json
read scene.md
return editor-ready scene
start background validation/indexing
```

#### Fallback behavior

If last scene is invalid:

```text
read manuscript.meta.json
resolve first chapter
read chapter.meta.json
resolve first scene
read scene.meta.json
read scene.md
return first valid scene
```

#### Repair behavior

If no valid scene can be opened, return repair mode state instead of crashing or silently rewriting files.

---

### 7.4 `saveScene()`

#### Purpose

Persist scene Markdown changes and update required metadata/workspace state.

#### Input

```text
project ID
scene ID
scene metadata path
scene content path
new Markdown content
cursor/selection/scroll state
active identity/persona
```

#### Behavior

```text
write Markdown safely
update modifiedAt
update modifiedBy
update derived stats if enabled
write workspace-state.json
mark app-local index dirty
mark Git working tree dirty if Git enabled
```

#### Safe write requirement

Preferred:

```text
write temp file
flush/sync as appropriate
atomic replace
preserve UTF-8
```

#### MVP constraint

Do not rewrite unrelated metadata fields.

---

### 7.5 `enableGitSnapshots()`

#### Purpose

Enable optional local Git-backed snapshots for an existing project.

#### Input

```text
project root path
active identity/persona
```

#### Behavior

```text
detect existing Git repo
initialize Git if needed
write or update .gitignore
create snapshots/scrivi-snapshots.json if missing
create initial snapshot of current state
```

#### Out of scope

```text
remote origin setup
push/pull
GitHub login
GitLab login
branch UI
merge UI
pull request UI
```

---

### 7.6 `createSnapshot()`

#### Purpose

Create a local writer-facing snapshot.

#### Input

```text
project root path
snapshot label
optional snapshot note
active identity/persona
```

#### Behavior

```text
git add canonical project files
git commit
record snapshot metadata
```

#### Author mapping

Use active persona display name as Git author name.

Use stable Scrivi identity-derived email-like value.

Example:

```text
Rhozwyn Darius <identity_01J8Z7Y9@scrivi.author>
```

---

### 7.7 `scanForExternalChanges()`

#### Purpose

Detect simple external project changes.

#### MVP detection scope

Must detect:

```text
scene.md changed externally
scene.md missing
scene.meta.json missing
scene.meta.json corrupt/unreadable
workspace last scene invalid
new unregistered .md in manuscript folder
```

May detect:

```text
possible rename
Git checkout changed open scene
unknown files
```

#### Output

```text
repair issue list
severity level
suggested repair actions
```

---

## 8. Minimum Repair Behavior Included in MVP

The MVP does not need a complete repair UI, but it must classify and avoid destructive behavior.

### 8.1 External `.md` edit

If scene Markdown changed externally and there are no unsaved in-app edits:

```text
reload or notify
update derived stats
mark modifiedBy as external or unknown
mark index dirty
```

If unsaved in-app edits exist:

```text
preserve both versions
stage conflict
do not overwrite
```

### 8.2 `.md` missing

If metadata exists but content file is missing:

```text
stage as missing content
do not create empty file automatically
do not remove scene automatically
```

Allowed MVP actions:

```text
show repair issue
open fallback scene if available
offer create empty file only with confirmation
```

### 8.3 `.meta.json` missing

If Markdown exists but metadata is missing:

```text
stage as unregistered manuscript file or missing metadata
do not import automatically
```

Allowed MVP actions:

```text
show repair issue
offer import as new scene
offer ignore
```

### 8.4 Corrupt metadata

If metadata JSON cannot be parsed:

```text
stage as corrupt metadata
avoid rewriting
open repair mode if critical
```

### 8.5 Workspace state invalid

If workspace state points to an invalid scene:

```text
fallback to first valid scene
update workspace state after successful open
```

---

## 9. MVP Git Scope

Git support is optional but included in MVP.

### 9.1 Must support

```text
enable Git at project creation
enable Git later
generate .gitignore
create initial snapshot
create named snapshot
record snapshot metadata
map persona to Git author
show whether project has unsnapshotted changes
```

### 9.2 Must not require

```text
Git credentials
remote repository
GitHub account
GitLab account
network access
```

### 9.3 Out of scope

```text
push
pull
fetch
clone
remote setup
branch UI
merge UI
pull request UI
conflict resolution UI
```

### 9.4 Implementation note

The MVP runtime trade study should decide whether Git support uses:

```text
system git executable
embedded library
runtime-specific Git package
```

This decision is intentionally pending.

---

## 10. MVP Index/Cache Scope

### 10.1 Required

The MVP should maintain only enough app-local state to support:

```text
open recent project
restore last writing surface
mark scene metadata/content dirty
simple validation result cache if useful
```

### 10.2 Not required

```text
SQLite
full-text search
relationship graph
thumbnail cache
timeline index
object graph index
```

### 10.3 Policy

Any MVP cache must be:

```text
app-local
rebuildable
safe to delete
not canonical
not required to open project
```

---

## 11. MVP Identity Scope

### 11.1 Required

```text
local identity creation
default persona creation
project membership creation
project persona creation
authorship attribution in metadata
persona display name at creation/modification
```

### 11.2 Not required

```text
identity transfer
QR transfer
encrypted .scriviauthor transfer file
group personas
shared credentials
signature validation
project encryption
```

### 11.3 Future hooks

The schema may include:

```text
public key
personaKind
controlledBy
encryption: none
```

But MVP should not implement advanced behavior behind these fields.

---

## 12. MVP Test Project

A successful MVP implementation should be able to create this project:

```text
TestNovel.scrivi/
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

Initial scene content may be empty:

```markdown

```

Or may contain test prose:

```markdown
It was raining when the door beneath the hill opened.
```

---

## 13. Acceptance Tests

The MVP should pass these tests.

---

### 13.1 First launch identity test

**Given** no local identity exists  
**When** Scrivi starts  
**Then** Scrivi creates a local identity and default persona after the user supplies an author display name.

Pass criteria:

```text
identity ID exists
persona ID exists
private material is not stored in project package
default persona display name is available
```

---

### 13.2 Create project test

**Given** a valid local identity/persona  
**When** user creates a new project  
**Then** Scrivi writes the minimum project file set and opens the first scene.

Pass criteria:

```text
project.json exists
manuscript.meta.json exists
chapter.meta.json exists
scene.md exists
scene.meta.json exists
project-members.json exists
project-personas.json exists
scene opens
cursor is at document start
```

---

### 13.3 Save and reopen test

**Given** a new project is open  
**When** user types text and saves/closes/reopens  
**Then** the same scene reopens with saved text and restored cursor.

Pass criteria:

```text
Markdown content persists
scene metadata modifiedAt updated
scene metadata modifiedBy updated
workspace state updated
cursor restored
```

---

### 13.4 Workspace fallback test

**Given** workspace-state.json is missing  
**When** project opens  
**Then** Scrivi opens the first valid scene from manuscript/chapter metadata order.

Pass criteria:

```text
project opens
first scene opens
workspace state recreated
no canonical project files are unnecessarily modified
```

---

### 13.5 External edit test

**Given** a scene `.md` file is edited outside Scrivi  
**When** Scrivi reopens or refreshes the project  
**Then** Scrivi detects the external edit and does not overwrite it.

Pass criteria:

```text
external content visible or reload offered
modifiedBy is external/unknown, not falsely current identity
indexes/cache marked dirty if present
no data lost
```

---

### 13.6 Missing Markdown test

**Given** `scene.md` is deleted outside Scrivi  
**When** Scrivi opens the project  
**Then** Scrivi stages a missing content repair issue.

Pass criteria:

```text
metadata not deleted
empty scene file not created automatically
repair issue reported
fallback scene opens if available
```

---

### 13.7 Missing metadata test

**Given** `scene.meta.json` is deleted outside Scrivi  
**When** Scrivi opens the project  
**Then** Scrivi stages an unregistered/missing metadata repair issue.

Pass criteria:

```text
Markdown not deleted
file not imported silently
repair issue reported
```

---

### 13.8 Corrupt metadata test

**Given** `scene.meta.json` contains invalid JSON  
**When** Scrivi opens the project  
**Then** Scrivi stages a corrupt metadata issue and does not overwrite the file.

Pass criteria:

```text
corrupt file preserved
repair issue reported
project opens fallback or repair mode as appropriate
```

---

### 13.9 Git at creation test

**Given** user enables Git-backed snapshots at project creation  
**When** project is created  
**Then** Scrivi initializes Git and creates an initial snapshot.

Pass criteria:

```text
.git exists
.gitignore exists
snapshots/scrivi-snapshots.json exists
initial commit exists
snapshot metadata maps to commit
```

---

### 13.10 Git enable later test

**Given** a non-Git project exists  
**When** user enables Git-backed snapshots  
**Then** Scrivi initializes Git and creates an initial snapshot of current state.

Pass criteria:

```text
existing project files preserved
.git exists
initial snapshot exists
snapshot metadata exists
```

---

### 13.11 Snapshot test

**Given** Git-backed snapshots are enabled  
**When** user creates a named snapshot  
**Then** Scrivi commits canonical project files and records snapshot metadata.

Pass criteria:

```text
Git commit created
snapshot label stored
identity/persona attribution stored
project remains valid
```

---

## 14. Implementation Runtime Decision Pending

This MVP slice deliberately does not choose the backend implementation runtime.

The next trade study must evaluate options such as:

```text
Swift-first backend
Rust shared core
C++ shared core
Kotlin Multiplatform
TypeScript/JavaScript core
platform-specific implementations following shared specs
```

The runtime must be evaluated against this MVP slice, not against the entire future product.

Key evaluation criteria:

```text
cross-platform reach
file I/O quality
JSON support
Markdown/text handling
Git integration options
secure key storage integration
Apple platform integration
Windows/Linux viability
Android viability
iPadOS/visionOS viability
developer productivity
long-term maintainability
minimum code required for MVP
```

---

## 15. Success Criteria for This Document

This MVP slice is successful if:

1. It is small enough to implement.
2. It proves the writing-first backend loop.
3. It does not require speculative systems.
4. It preserves the approved project structure.
5. It includes optional Git snapshots without requiring Git for casual writing.
6. It avoids mandatory SQLite.
7. It avoids implementing future collaboration/encryption/signing prematurely.
8. It gives the runtime trade study a concrete target.

---

## 16. Recommended Next Document

The next document should be:

```text
Scrivi Backend Runtime Trade Study v0.1
```

That trade study should choose the best implementation approach for this MVP slice while preserving the approved multi-platform product direction.

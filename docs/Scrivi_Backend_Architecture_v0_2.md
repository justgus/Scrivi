# Scrivi Backend Architecture

**Version:** 0.2  
**Status:** Draft / Approved Direction Incorporated  
**Project:** Scrivi  
**Role Context:** App Designer supporting Project Lead  

---

## 1. Purpose

Scrivi is a cross-platform creative writing application. Its backend must support a writing-first experience while also allowing a project to behave like a structured creative codebase.

A Scrivi project should open directly to the writer’s last active writing surface, with the cursor at the place where the writer left off. At the same time, the project must support structured organization, metadata, worldbuilding objects, assets, comments, indexing, optional Git-backed history, identity, authorship, and future collaboration.

This document describes the approved backend architecture direction for Scrivi as of revision 0.2.

---

## 2. Design Principles

Scrivi backend design follows these project principles:

1. Do not assume. Do not hide confusion. Surface tradeoffs.
2. Use the minimum code that solves the problem.
3. Do nothing speculative.
4. Touch only what must be touched.
5. Clean up only your own mess.
6. Define success criteria.
7. Loop until approved.
8. The project lead can make mistakes; design review should push back when needed.

---

## 3. Core Product Principle

> Scrivi is an IDE for a writing project, but it opens like a manuscript.

This means Scrivi has two simultaneous responsibilities:

1. **Default writer experience:** resume writing immediately.
2. **Project architecture:** maintain a structured, inspectable, recoverable, versionable, cross-platform creative project.

The writer should not be forced to think about the project structure during ordinary writing, but the structure must exist and remain understandable.

---

## 4. Platform Direction

Scrivi is intended to support:

- macOS
- Windows
- Linux, initially likely Debian
- iPadOS
- visionOS
- iPhone
- Android

The backend must therefore be platform-neutral and must not depend on Apple-only technologies such as SwiftData as the canonical storage layer.

Apple-specific technologies may be used in Apple UI shells, but the canonical project format must remain cross-platform.

---

## 5. Approved Backend Foundation

The approved foundation is:

```text
Canonical manuscript text: Markdown / markup-oriented source
Canonical metadata and objects: JSON
Derived indexes: app-local rebuildable indexes
SQLite: allowed later if justified, not mandatory for v1
Assets: original native file formats where practical
Project model: structured .scrivi project package/folder
Git: optional v1 support
Identity: certificate-backed human identity plus flexible author personas
```

Scrivi should not begin with a single monolithic document or an opaque database as the canonical project format.

---

## 6. Project Package / Folder Model

A Scrivi project uses a `.scrivi` project package.

Approved behavior:

- On Apple platforms, `.scrivi` may appear as a document package.
- On Windows and Linux, `.scrivi` may appear as an ordinary folder.
- Internally, the structure remains the same across platforms.
- The project structure must remain Git-compatible and inspectable.

This gives normal users a document-like experience while preserving power-user transparency and cross-platform recoverability.

---

## 7. Initial Project Structure Direction

A representative project structure is:

```text
MyNovel.scrivi/
  project.json

  manuscript/
    chapter-001/
      chapter.meta.json
      scene-001.md
      scene-001.meta.json
      scene-002.md
      scene-002.meta.json

  objects/
    characters/
      ada.json
      thomas.json
    locations/
      old-watchtower.json
    items/
      brass-key.json
    rules/
      magic-system.json
    timelines/

  assets/
    images/
    video/
    documents/
    audio/

  comments/
    scene-001.comments.json

  identities/
    project-members.json

  inbox/

  .git/                 optional, if Git is enabled
  .gitignore            optional, if Git is enabled
```

This structure is not yet a final schema. It is the approved architectural direction for project organization.

---

## 8. Canonical, Derived, and Local State

Scrivi files fall into three broad categories.

### 8.1 Canonical source

Canonical source is the creative truth of the project.

Examples:

```text
project.json
manuscript/
objects/
assets/
comments/
identities/
```

Canonical source should be durable, platform-neutral, inspectable, recoverable, and suitable for backup and versioning.

### 8.2 Derived indexes and caches

Derived data can be rebuilt from canonical source.

Approved decision:

> Scrivi v1 should not require SQLite. Use in-memory or simple app-local indexes initially. Add app-local SQLite later only if profiling and feature requirements justify it.

Indexes should live outside the project by default, in app-local support/cache storage.

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

SQLite is allowed as an implementation detail for app-local rebuildable indexes later, but should not be required in v1.

### 8.3 Personal/local state

Personal or local state describes how a specific device or identity last interacted with a project.

Examples:

- last opened scene
- cursor position
- selection
- scroll position
- visible panels
- workspace layout
- recent navigation history

This state must be separated from canonical manuscript content.

---

## 9. Manuscript Format

Approved decision:

> Scrivi uses a hybrid Markdown plus metadata model.

The manuscript body is stored as readable markup, initially Markdown or a Markdown-like text format. Metadata is stored separately as visible JSON files.

Example:

```text
manuscript/
  chapter-001/
    chapter.meta.json
    scene-001.md
    scene-001.meta.json
    scene-002.md
    scene-002.meta.json
```

The `.md` file is the writing surface. The `.meta.json` file stores app-owned structure such as:

- stable object ID
- title
- slug
- order
- chapter membership
- scene status
- point of view
- tags
- timeline placement
- content path
- encryption state hook
- comment references
- relationship references

---

## 10. Metadata Visibility

Approved decision:

> Metadata files are fully visible and paired with manuscript files.

Scrivi should not hide metadata in an internal folder if the writer can directly modify manuscript files. Hidden metadata can create invisible failure states when users delete, rename, or replace `.md` files outside the app.

Metadata is therefore:

- visible
- text-based
- inspectable
- Git-visible
- app-managed

However, Scrivi does **not** promise to support arbitrary hand-editing of metadata JSON in v1. Metadata is visible for transparency and recovery, not because it is a stable public authoring interface.

---

## 11. External Editing

Approved decision:

> Scrivi supports external editing of manuscript text and assets, but not arbitrary external editing of metadata.

Users may edit `.md` files in external tools such as VS Code, Vim, Notepad, or other text editors. Scrivi should detect changes and re-index as needed.

Assets may be added, removed, or replaced externally.

Metadata JSON files are inspectable, but hand-editing them is not an officially supported workflow in v1.

---

## 12. External Change Repair Rules

Scrivi must handle external file changes safely and explicitly.

### 12.1 `.md` exists, metadata missing

Scrivi should detect an unregistered manuscript file.

Possible actions:

- Import as new scene
- Attach to existing scene
- Ignore
- Move to inbox

### 12.2 Metadata exists, `.md` missing

Scrivi should detect orphaned scene metadata.

Possible actions:

- Restore manuscript from snapshot
- Create empty manuscript file
- Mark scene as missing
- Remove scene from project
- Ignore for now

### 12.3 `.md` replaced externally

If the manuscript file still maps to the same metadata file, Scrivi treats this as an external edit and re-indexes it.

If Git is enabled, this is a normal working-tree change.

### 12.4 `.md` renamed externally

Scrivi may detect a likely rename when one metadata file is orphaned and one new manuscript file appears.

Scrivi should offer a repair action instead of silently guessing, unless confidence is very high.

### 12.5 General rule

> Detect → stage → review → commit.

Scrivi should avoid silently mutating the project graph in response to uncertain external changes.

---

## 13. Object Identity

Approved decision:

> Scrivi uses UUID v7-style opaque stable IDs, with optional human-friendly slugs.

Example:

```json
{
  "id": "scene_01HYX8K7X4N9R2J4M6V9Q2F3AB",
  "slug": "the-door-beneath-the-hill",
  "title": "The Door Beneath the Hill"
}
```

The stable ID is authoritative. The slug and title are for display, readability, and convenience.

IDs must survive:

- rename
- file move
- title change
- Git branch changes
- project sharing
- collaboration
- future encryption

Object identity must not depend on plaintext file path or filename.

---

## 14. Writing-First Launch Path

When opening a project, Scrivi should load only what is needed to resume writing.

Minimum launch path:

1. Load `project.json`.
2. Resolve the local device and active author/persona context.
3. Load project state needed to find the last active writing surface.
4. Load the target manuscript `.md` file and its paired metadata.
5. Display the writing surface with cursor, selection, and scroll position restored where possible.
6. Continue indexing and deeper project loading after the writing surface is available.

Scrivi must not require loading the entire object graph before showing the writing surface.

---

## 15. Identity, Authorship, Personas, and Device State

Scrivi separates several concepts that must not be collapsed.

### 15.1 Device identity

A device identity is generated silently on install.

Used for:

- device-specific preferences
- local workspace state
- local cache ownership
- this device’s last cursor position

Device identity does not represent authorship.

### 15.2 Human-controlled Scrivi identity

A Scrivi identity represents authorship authority and project access.

It is backed by a locally generated key/certificate. It does not require a cloud account, email address, username, or password for ordinary use.

The private key should be stored in the platform’s secure storage where possible.

Examples:

- Apple platforms: Keychain
- Windows: Credential Manager / DPAPI
- Linux: Secret Service, KWallet, GNOME Keyring, or fallback encrypted file
- Android: Android Keystore

### 15.3 Author personas

A single human-controlled identity may own multiple author personas.

Examples:

- Rhozwyn Darius
- Julie Stoddard
- J. M. Stoddard
- Anonymous

Different projects may use different personas from the same identity.

Changing a persona name must not require a new identity certificate.

### 15.4 Project membership

Project membership records which identities have access to a project and which role they have.

Approved initial roles:

```text
owner
editor
reader
```

---

## 16. First Startup Authorship Flow

On very first startup, Scrivi should ask the user to identify herself as an author, but not require account creation.

Example prompt:

```text
How should Scrivi identify you as an author?

This name will appear on your writing history, comments, and shared projects.

[ Julie Stoddard ]
```

Behind the scenes, Scrivi creates:

- device identity
- human-controlled Scrivi identity
- public/private key pair
- initial author persona

No cloud account is required. No email address is required. No password is required for ordinary first use.

---

## 17. Author Persona Rename and Historical Attribution

Approved decision:

> Scrivi stores both stable persona identity and contribution-time display name.

This supports persona renaming without losing historical attribution.

Example:

```json
{
  "identityID": "identity_01HZ...",
  "personaID": "persona_01A",
  "displayNameAtContribution": "Rhozwyn Darius"
}
```

The UI may default to showing the current persona name, but history, review, and audit views may show the contribution-time display name when appropriate.

---

## 18. Identity Transfer

Approved decision:

> Scrivi should support QR-based identity transfer using QR as a session handshake, with encrypted transfer file fallback.

Preferred flow:

1. Source device begins identity transfer.
2. Source device displays a QR code.
3. Receiving device scans QR code.
4. QR establishes a secure temporary transfer session.
5. The encrypted identity bundle transfers device-to-device.
6. User confirms transfer.

Fallback:

- export encrypted `.scriviauthor` file
- protect with transfer passphrase
- import on another device

The QR should not need to contain the entire identity bundle. It should usually contain only enough information to establish the transfer session.

---

## 19. Git Support

Approved decision:

> Scrivi v1 includes optional Git-backed snapshots. Git is never required to start writing. Git can be enabled at project creation or later.

Supported v1 scope:

- create project without Git
- create project with Git enabled
- enable Git later
- initialize Git repository
- generate `.gitignore`
- create writer-facing snapshots
- show snapshot history
- map Scrivi persona to Git author

Out of scope for v1:

- remote hosting setup
- GitHub/GitLab integration
- pull requests
- full branch UI
- complex merge/conflict resolution UI
- full collaborative review workflow

Writer-facing terms should be preferred:

| Git concept | Scrivi-facing concept |
|---|---|
| commit | snapshot |
| branch | version / experiment |
| diff | changes |
| pull request | review proposal |
| merge conflict | conflicting edits |
| tag | milestone / draft version |

Git support must not block the casual writer who simply wants to start writing.

---

## 20. Git Identity Mapping

Scrivi should map active author personas to Git author information.

Example:

```text
Author: Rhozwyn Darius <identity_01HZ@scrivi.author>
```

If the same identity later uses a different persona:

```text
Author: Julie Stoddard <identity_01HZ@scrivi.author>
```

This preserves a stable identity token while allowing project-facing author names to change.

No real email address should be required.

---

## 21. Comments

Approved decision:

> Comments are included in the v1 architecture. Suggested changes are deferred.

Comments may initially attach to scenes or objects. Later, comments may attach to paragraph or text-range anchors.

Comments should not corrupt or directly modify manuscript text.

Representative direction:

```json
{
  "id": "comment_01J...",
  "target": {
    "objectID": "scene_01J...",
    "anchor": null
  },
  "author": {
    "identityID": "identity_01J...",
    "personaID": "persona_01J...",
    "displayNameAtContribution": "Julie Stoddard"
  },
  "body": "This reveal may come too early.",
  "createdAt": "2026-05-18T15:42:00Z",
  "status": "open"
}
```

Suggested edits, tracked changes, and full review proposals are out of scope for v1.

---

## 22. Collaboration Direction

Approved decision:

> Scrivi begins with local-file/Git-compatible collaboration, not Google Docs-style real-time collaboration.

The architecture should support:

- multiple project members
- authorship attribution
- comments
- optional Git-backed snapshots
- local sharing
- Git-based sharing
- future review workflows

The architecture should not require in v1:

- real-time multi-user editing
- live cursors
- remote presence
- cloud accounts
- hosted Scrivi collaboration server
- CRDT or operational transform engine

Real-time collaboration should remain possible later, but should not define v1.

---

## 23. Shared Credentials and Group Personas

Shared credentials are out of scope for v1.

However, the architecture should leave hooks for a later model based on shared personas or groups.

Preferred future direction:

> Prefer shared personas/groups over shared private keys.

Example future model:

```json
{
  "personaID": "persona_review_board",
  "displayName": "Review Board",
  "personaKind": "group",
  "controlledBy": [
    "identity_01J...",
    "identity_01K..."
  ]
}
```

This would allow the UI to display “Review Board” while retaining the ability to know which underlying identity actually performed an action.

V1 should avoid assuming that every persona must be permanently one-to-one with one identity.

---

## 24. Security Direction

Approved decision:

> V1 implements authorship attribution and is signing-ready. Full project encryption is deferred.

Security levels:

### Level 1 — Attribution

Scrivi knows who is writing, commenting, and taking snapshots.

Approved for v1.

### Level 2 — Signing

Scrivi can cryptographically verify that metadata, snapshots, or changes came from an author identity.

Design-ready, not necessarily fully implemented in v1.

### Level 3 — Encryption/access control

Scrivi prevents unauthorized people from reading or editing project content.

Deferred.

---

## 25. Future Encryption Compatibility

Approved decision:

> V1 does not implement project encryption, but the architecture must remain compatible with future file-level or object-level encryption.

Preferred future-compatible designs:

- F2: canonical files may be encrypted individually
- F3: selected sensitive objects may be encrypted

Not preferred:

- whole-project opaque encrypted container as the only model, because it harms Git, external editing, partial sync, file watching, and the creative-codebase model

Metadata should reserve encryption fields even when encryption is not implemented.

Example v1-compatible metadata hook:

```json
{
  "content": {
    "path": "scene-001.md",
    "encryption": "none"
  }
}
```

Future encrypted form:

```json
{
  "content": {
    "path": "scene-001.md.enc",
    "encryption": "object-key"
  }
}
```

Architectural guardrails:

1. Do not assume all files are plaintext forever.
2. Store content access through metadata, not only hardcoded paths.
3. Keep stable object IDs independent of filenames.
4. Do not require indexes to contain plaintext.
5. Make indexes rebuildable and encryption-aware.
6. Separate identity/signing keys from content-encryption keys.
7. Allow project manifest to declare encryption policy later.

---

## 26. Assets

Assets such as images, maps, audio, video, PDFs, and reference documents should retain their original formats where practical.

Examples:

```text
assets/
  images/
  video/
  documents/
  audio/
```

Scrivi may create thumbnails, previews, transcripts, or derived metadata in app-local cache. These derived artifacts are not canonical source.

---

## 27. Inbox and Dropped Files

Scrivi should support files being dropped into a project structure.

Examples:

- images
- PDFs
- maps
- scene drafts
- research notes
- videos

Dropped files should generally follow:

> Detect → stage → review → commit.

The `inbox/` folder may be used as a safe place for unclassified files.

Scrivi should avoid silently mutating the canonical project graph when intent is unclear.

---

## 28. Export Direction

Approved decision:

> Exports are generated to user-selected destinations, with export history metadata retained.

Exports are build products, not canonical source.

Scrivi should support remembering recent export destinations and settings, but should not automatically store exports inside the project package unless the user chooses that.

Potential export formats later include:

- PDF
- EPUB
- DOCX
- Markdown bundles
- plain text
- HTML

The backend architecture does not depend on final export format decisions.

---

## 29. Non-Goals for V1 Backend

The following are out of scope for v1 backend implementation:

- Google Docs integration
- real-time collaboration
- live cursors
- hosted Scrivi sync service
- cloud account system
- full GitHub/GitLab integration
- pull requests
- full branch management UI
- complex merge/conflict resolution UI
- suggested changes/tracked edits
- shared private-key credentials
- full project encryption
- CRDT or operational transform editing engine
- SQLite as a mandatory backend component
- SwiftData as canonical storage

---

## 30. Initial Success Criteria

The Scrivi backend architecture is successful if:

1. A project opens directly to the last writing surface.
2. A writer can start writing without configuring Git, cloud accounts, or passwords.
3. A project can optionally enable Git-backed snapshots in v1.
4. A project with a few thousand objects does not require full graph loading before writing can begin.
5. Canonical data is platform-neutral and inspectable.
6. Manuscript text is readable, versionable, and recoverable outside Scrivi.
7. Metadata is visible and paired with manuscript files.
8. Metadata is app-managed and not required to be hand-edited.
9. External manuscript edits are detected safely.
10. Missing, renamed, or replaced files can be staged for repair.
11. Derived indexes are app-local and rebuildable.
12. SQLite can be added later if needed without changing canonical storage.
13. Assets retain original formats where practical.
14. Exports do not pollute canonical project history unless the user chooses that.
15. A project can have multiple members with owner/editor/reader roles.
16. Authorship can be attributed without requiring real names, emails, or cloud accounts.
17. One human identity can use multiple personas.
18. Persona names can change without replacing identity certificates.
19. Git author mapping can use Scrivi personas and stable identity tokens.
20. QR-based identity transfer and encrypted transfer-file fallback are supported by design.
21. Collaboration can grow from local-file/Git-compatible foundations.
22. Future group personas are not blocked.
23. Future signing and encryption are not blocked.
24. The project format can support macOS, Windows, Linux, iPadOS, visionOS, iPhone, and Android.

---

## 31. Remaining Open Questions

The following remain open for later trade studies or schema design:

1. Exact Markdown dialect or Scrivi markup subset.
2. Exact JSON schemas for `project.json`, `chapter.meta.json`, `scene.meta.json`, object files, comments, and membership.
3. Exact UUID v7-style ID representation.
4. Whether chapter folders should be ordered by filename, metadata, or both.
5. How to represent manuscript ordering across chapters and scenes.
6. How to anchor comments to text robustly.
7. How much Git snapshot UI is required for v1.
8. How restore-from-snapshot should behave safely.
9. How to handle Git conflicts when they occur outside Scrivi.
10. How identity transfer works across networks that block local device discovery.
11. What app-local index format should be used before SQLite.
12. Whether assets need canonical metadata sidecars.
13. How project repair UI should classify external changes.
14. How user/device local state should be stored across multiple devices.
15. How project membership should work when a project is copied rather than shared.
16. Whether project packages should include a formal format-version migration log.
17. What minimal schemas are needed before first implementation.

---

## 32. Recommended Next Document

The next backend design artifact should be:

# Scrivi Project Structure v0.1

It should define:

1. Exact top-level project folder layout.
2. Required files for a minimal valid project.
3. Required files for a minimal valid chapter.
4. Required files for a minimal valid scene.
5. Initial `project.json` fields.
6. Initial `chapter.meta.json` fields.
7. Initial `scene.meta.json` fields.
8. Initial comment file placement.
9. Initial identity and membership file placement.
10. App-local cache conventions.
11. Git ignore rules.
12. External-change repair cases.


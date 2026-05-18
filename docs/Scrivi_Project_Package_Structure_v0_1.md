# Scrivi Project Package Structure v0.1

**Status:** Approved baseline  
**Project:** Scrivi  
**Document type:** Backend / project package structure  
**Purpose:** Define the physical on-disk structure of a `.scrivi` project package.

---

## 1. Design Goal

A Scrivi project package should be:

1. Easy for a writer to understand.
2. Safe for external manuscript editing.
3. Friendly to Git.
4. Recoverable if files are moved, deleted, or replaced.
5. Platform-neutral.
6. Able to open quickly to the last writing surface.
7. Ready for future collaboration, signing, and encryption without requiring a redesign.

Scrivi uses a **creative codebase** model: the project is structured and inspectable like a software project, but the app opens directly to the writer’s last writing surface.

---

## 2. Approved Foundation

The Scrivi backend architecture has already approved the following foundation:

- Canonical manuscript text uses Markdown / markup-oriented source files.
- Canonical metadata and project objects use JSON.
- Assets retain their original native formats where possible.
- Indexes are rebuildable app-local caches, not canonical project files.
- SQLite is not required for v1, but may be introduced later as an app-local cache implementation if profiling justifies it.
- Git-backed snapshots are optional in v1.
- Git is never required to start writing.
- Identity, authorship, personas, and project membership are represented in project metadata, but private keys are never stored inside the project package.
- Future encryption must remain possible without replacing the package model.

---

## 3. Package Behavior Across Platforms

A `.scrivi` project should have one canonical internal structure.

Platform presentation may vary:

- On Apple platforms, `.scrivi` may appear as a document package.
- On Windows and Linux, `.scrivi` may appear as an ordinary folder.

The app must not depend on Apple-only package behavior. The internal folder structure remains platform-neutral.

---

## 4. Approved Package Layout

A representative Scrivi project package:

```text
MyNovel.scrivi/
  project.json

  manuscript/
    manuscript.meta.json

    chapter-001/
      chapter.meta.json
      scene-001.md
      scene-001.meta.json
      scene-002.md
      scene-002.meta.json

    chapter-002/
      chapter.meta.json
      scene-001.md
      scene-001.meta.json

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
      main-timeline.json

  assets/
    images/
      old-watchtower-map.png
      old-watchtower-map.meta.json

    video/
      reference-clip.mov
      reference-clip.meta.json

    audio/
      dialect-note.m4a
      dialect-note.meta.json

    documents/
      research-paper.pdf
      research-paper.meta.json

  comments/
    scenes/
      scene_01J8X.comments.json

    objects/
      character_01J8Y.comments.json

  identities/
    project-members.json
    project-personas.json

  inbox/
    dropped-files/

  snapshots/
    scrivi-snapshots.json

  .gitignore
```

If Git-backed snapshots are enabled, the package also contains:

```text
  .git/
```

The `.git/` directory is present only when Git is enabled.

---

## 5. Root-Level Files and Folders

### 5.1 `project.json`

`project.json` is the project manifest.

It contains high-level project identity and configuration only. It should not contain the entire project graph.

Expected responsibilities include:

- project ID
- project title
- project/package format version
- creation date
- creator identity/persona reference
- manuscript root path
- active project structure version
- feature flags
- Git support state
- encryption policy placeholder

`project.json` should remain small enough to load quickly during project open.

---

## 6. `manuscript/`

The `manuscript/` folder is the writer-facing prose area.

Approved model:

```text
scene-001.md
scene-001.meta.json
```

The manuscript text and metadata are paired visibly.

### 6.1 Why visible pairing is approved

Visible pairing avoids hidden mismatch traps.

If a writer externally deletes, renames, or replaces a `.md` file, the companion metadata file is visible. The user is not left unaware that the manuscript file has associated app metadata.

The pairing is intentionally obvious:

```text
scene-001.md
scene-001.meta.json
```

Metadata is text-based and inspectable, but it is still app-managed. Scrivi does not promise to support arbitrary hand-editing of metadata JSON in v1.

---

## 7. `manuscript/manuscript.meta.json`

`manuscript.meta.json` exists and defines the intended manuscript-level structure.

Folder order and filename order are useful, but they are not the authoritative source of manuscript order.

Expected responsibilities include:

- manuscript ID
- manuscript title
- chapter order
- front matter references
- back matter references
- default export order
- optional manuscript-level status/configuration

This allows folders and files to remain human-readable without making path order the only source of truth.

---

## 8. Chapter Folders

A chapter folder represents a natural writing unit.

Example:

```text
manuscript/
  chapter-001/
    chapter.meta.json
    scene-001.md
    scene-001.meta.json
```

### 8.1 `chapter.meta.json`

`chapter.meta.json` contains chapter identity and ordering metadata.

Expected responsibilities include:

- chapter ID
- title
- slug
- chapter number/display label
- order
- status
- included scene references
- created/modified attribution

---

## 9. Scene Files

### 9.1 `scene-001.md`

The `.md` file is canonical prose.

It should be readable and editable outside Scrivi.

Scrivi may support Markdown or a Markdown-compatible markup profile. The exact manuscript markup profile is a later schema/design decision.

### 9.2 `scene-001.meta.json`

The `.meta.json` file contains scene-level metadata.

Expected responsibilities include:

- scene ID
- title
- slug
- chapter ID
- order within chapter
- status
- POV character reference
- timeline references
- tags
- word count cache, if used
- content path
- encryption placeholder
- created/modified attribution

Example pattern:

```json
{
  "id": "scene_01J8X...",
  "title": "The Door Beneath the Hill",
  "slug": "the-door-beneath-the-hill",
  "content": {
    "path": "scene-001.md",
    "format": "markdown",
    "encryption": "none"
  }
}
```

Future encryption-compatible pattern:

```json
{
  "content": {
    "path": "scene-001.md.enc",
    "format": "markdown",
    "encryption": "object-key"
  }
}
```

Object identity must not depend on plaintext file path or filename.

---

## 10. File Naming Direction

Scene and chapter files should be human-readable where practical.

Recommended naming pattern:

```text
001-the-door-beneath-the-hill.md
001-the-door-beneath-the-hill.meta.json
```

The file name is a convenience for humans and external tools. The stable ID inside metadata is authoritative.

If a title changes, Scrivi may offer to rename files, but renaming should not be required to preserve identity.

---

## 11. `objects/`

The `objects/` folder contains structured world/project objects.

Example categories:

```text
objects/
  characters/
  locations/
  items/
  rules/
  timelines/
```

Examples:

```text
objects/characters/ada.json
objects/locations/old-watchtower.json
objects/items/brass-key.json
```

Worldbuilding objects are canonical creative source. They are visible, inspectable, and Git-visible.

Each object file contains its own stable UUID v7-style ID. File names are human conveniences only.

Example pattern:

```json
{
  "id": "character_01J8X...",
  "type": "character",
  "slug": "ada",
  "displayName": "Ada",
  "createdAt": "2026-05-18T00:00:00Z"
}
```

---

## 12. `assets/`

Assets retain their original formats where possible.

Approved asset categories include:

```text
assets/
  images/
  video/
  audio/
  documents/
```

Examples:

```text
assets/images/map.png
assets/video/reference-clip.mov
assets/audio/dialect-note.m4a
assets/documents/research-paper.pdf
```

### 12.1 Asset metadata

Asset metadata should be paired visibly with the asset file, matching the manuscript metadata approach.

Example:

```text
assets/images/old-watchtower-map.png
assets/images/old-watchtower-map.meta.json
```

This avoids hidden metadata mismatch problems and preserves external visibility.

---

## 13. `comments/`

Comments are part of the v1 architecture. Suggested changes/tracked edits are deferred.

Comments should live outside manuscript prose so they do not clutter or corrupt the canonical writing text.

Approved structure:

```text
comments/
  scenes/
  objects/
  assets/
```

Examples:

```text
comments/scenes/scene_01J8X.comments.json
comments/objects/character_01J8Y.comments.json
```

V1 must support scene-level comments. Richer anchoring to paragraphs or text ranges may be added later.

---

## 14. `identities/`

The `identities/` folder stores project-level public identity and membership metadata.

Approved files:

```text
identities/
  project-members.json
  project-personas.json
```

Expected responsibilities include:

- member identity IDs
- public keys
- roles
- personas used in the project
- persona display names
- contribution-time display names where needed

Private keys do **not** belong in the project package.

Private author keys belong in device/app secure storage.

### 14.1 Future shared persona hook

Shared/group personas are out of scope for v1, but the model should not prevent them.

The schema should avoid assuming every persona permanently maps one-to-one with one identity.

Future-compatible example:

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

This is a future design hook, not a v1 implementation requirement.

---

## 15. `inbox/`

The `inbox/` folder is used for externally dropped or discovered files.

Approved structure:

```text
inbox/
  dropped-files/
```

Scrivi should follow this workflow:

```text
Detect → stage → review → commit
```

Scrivi should not silently mutate the project graph in response to every dropped file.

Possible import actions include:

- import as asset
- import as research document
- create scene from Markdown
- ignore
- delete

---

## 16. `snapshots/`

Git-backed snapshots are optional in v1.

If Git is enabled, Git stores the actual history. Scrivi may store UI metadata over Git commits.

Approved file:

```text
snapshots/scrivi-snapshots.json
```

Example responsibilities:

- Scrivi snapshot ID
- Git commit hash
- writer-facing label
- created by identity/persona
- created date

Example pattern:

```json
{
  "snapshots": [
    {
      "snapshotID": "snap_01J...",
      "gitCommit": "abc123...",
      "label": "Draft before editor review",
      "createdBy": "identity_01J...",
      "personaID": "persona_01J..."
    }
  ]
}
```

No independent Scrivi snapshot system is approved for v1.

If Git is disabled, `snapshots/` may be absent or empty.

---

## 17. `.gitignore`

If Git is enabled, Scrivi generates `.gitignore`.

Defensive default:

```gitignore
# Scrivi app-local caches should not be inside project, but exclude just in case
.cache/
indexes/
*.sqlite
*.sqlite-shm
*.sqlite-wal

# OS noise
.DS_Store
Thumbs.db

# temporary/editor files
*.tmp
*.swp
~$*
```

Because approved indexes are app-local by default, `.gitignore` is mostly defensive.

---

## 18. What Does Not Belong Inside the Project Package

### 18.1 App-local cache

Not inside project:

```text
Scrivi App Support/
  cache/
    projects/
      project_01J.../
        search.index
        graph.index
        thumbnails/
```

Indexes are rebuildable and app-local. SQLite may be used later if profiling proves it necessary.

### 18.2 Private keys

Not inside project:

```text
Secure Store/
  author private keys
  transfer secrets
```

Private keys remain in platform secure storage.

### 18.3 Routine exports

Routine exports are not stored inside the project by default.

Example external destination:

```text
Documents/Exports/My Novel Draft 1.pdf
```

The project may remember export history, but exports are build products, not canonical project source.

---

## 19. External File Change and Repair Rules

Scrivi must expect users and external tools to change project files.

### 19.1 `.md` exists, metadata missing

Scrivi should stage this as an unregistered manuscript file.

Possible actions:

- import as new scene
- attach to existing scene
- ignore
- move to inbox

### 19.2 Metadata exists, `.md` missing

Scrivi should stage this as missing manuscript content.

Possible actions:

- restore manuscript from snapshot, if Git is enabled
- create empty manuscript file
- mark scene as missing
- remove scene from project
- ignore for now

### 19.3 `.md` replaced externally

If the file still maps to the same metadata file, Scrivi treats it as an external edit and re-indexes it.

If Git is enabled, this is a normal file change.

### 19.4 `.md` renamed externally but metadata still points to old path

Scrivi should detect an orphaned metadata file and a new unregistered manuscript file.

It may offer a repair action:

```text
This looks like scene-001.md may have been renamed to chapter-opening.md.
Relink metadata?
```

Scrivi should not silently guess unless confidence is very high.

---

## 20. Success Criteria

The Scrivi project package structure is successful if:

1. A writer can find and edit manuscript text externally.
2. Companion metadata is visible and understandable as related.
3. File deletion/replacement can be detected and repaired.
4. Git diffs remain meaningful.
5. Project identity, personas, and membership are represented without private keys.
6. Assets remain in native formats.
7. App-local indexes are rebuildable and not part of the canonical package.
8. The package can support future object/file encryption.
9. The app can open quickly without scanning every asset.
10. The structure is understandable on macOS, Windows, Linux, iPadOS, visionOS, iPhone, and Android.

---

## 21. Next Design Step

The next backend design layer should define the minimum schema set:

- `project.json`
- `manuscript.meta.json`
- `chapter.meta.json`
- `scene.meta.json`
- asset `.meta.json`
- `project-members.json`
- `project-personas.json`

Schemas should remain minimal and v1-focused. They should include compatibility hooks for future signing, group personas, and object/file encryption, but should not implement those systems prematurely.

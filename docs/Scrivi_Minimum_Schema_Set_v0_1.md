# Scrivi Minimum Schema Set v0.1

**Project:** Scrivi  
**Document:** Minimum Schema Set  
**Version:** 0.1  
**Status:** Approved baseline  

---

## 1. Purpose

This document defines the minimum schema set required for a valid Scrivi project package.

The goal is not to define every future Scrivi object. The goal is to define the smallest useful backend contract needed to:

1. Create a new project.
2. Open an existing project.
3. Resume writing at the last writing surface.
4. Maintain manuscript order.
5. Attribute authorship through identity and persona metadata.
6. Support visible manuscript/metadata pairing.
7. Leave hooks for future signing, group personas, encryption, comments, assets, snapshots, and richer object systems.

This schema set follows the approved Scrivi backend direction:

- Canonical manuscript text is stored as Markdown or Markdown-like markup.
- Canonical metadata is stored as JSON.
- Manuscript files and metadata files are visibly paired.
- IDs are authoritative; filenames, slugs, and titles are human-facing conveniences.
- Git-backed snapshots are optional in v1.
- SQLite is not required in v1; indexes begin as rebuildable app-local caches.
- Private keys are never stored inside the project package.

---

## 2. Minimum Required Project Files

A minimal Scrivi project contains:

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

The minimum required schema files are:

```text
project.json
manuscript/manuscript.meta.json
manuscript/<chapter>/chapter.meta.json
manuscript/<chapter>/<scene>.meta.json
identities/project-members.json
identities/project-personas.json
```

There is also one important app-local schema that is not part of the project package:

```text
workspace-state.json
```

---

## 3. General Schema Rules

### 3.1 IDs Are Authoritative

All durable Scrivi objects use stable opaque IDs, preferably UUID v7-style or ULID-style identifiers.

Examples:

```text
project_01J8Z7V6B5W4Q3R2N1M0K9J8H7
manuscript_01J8Z8D4C3B2A1N0M9K8J7H6G5
chapter_01J8Z8H9K7M6N5B4V3C2X1Z0A9
scene_01J8Z8P4N3M2K1J0H9G8F7E6D5
identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8
persona_01J8Z801V6B5N4M3K2J1H0G9F8
```

The ID is authoritative. File paths, filenames, slugs, and titles are not identity.

### 3.2 Paths

Paths stored inside project metadata are relative to the file or package root as specified by the schema.

For this minimum schema set:

- Paths in `project.json` are relative to the project package root.
- Paths in `manuscript.meta.json` are relative to the `manuscript/` folder.
- Paths in `chapter.meta.json` are relative to the chapter folder containing that file.
- Paths in `scene.meta.json` are relative to the scene metadata file.

### 3.3 Metadata Visibility

Manuscript files use visible paired metadata:

```text
001-opening-scene.md
001-opening-scene.meta.json
```

The metadata file is visible, inspectable, text-based JSON. However, Scrivi does not promise to support arbitrary hand-editing of metadata JSON.

The metadata is app-managed but not hidden.

### 3.4 Authorship and Persona Metadata

Scrivi distinguishes:

- **Identity**: the certificate-backed human-controlled authority.
- **Persona**: the project-facing author name or pseudonym.
- **Project membership**: the role that an identity has in a project.

Authored records should store both stable IDs and contribution-time display names.

This supports persona renaming without destroying historical attribution.

### 3.5 Private Keys

Private keys are never stored inside the project package.

The project may contain public keys and identity IDs, but private key material belongs in platform secure storage.

---

## 4. `project.json`

### 4.1 Purpose

`project.json` is the root project manifest.

It identifies the project, declares the format version, records high-level configuration, and points Scrivi to the manuscript and identity metadata.

It should not contain every scene, character, location, comment, or asset.

### 4.2 Example

```json
{
  "schema": "scrivi.project.v1",
  "projectID": "project_01J8Z7V6B5W4Q3R2N1M0K9J8H7",
  "title": "The Door Beneath the Hill",
  "slug": "the-door-beneath-the-hill",
  "createdAt": "2026-05-18T18:00:00Z",
  "createdBy": {
    "identityID": "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8",
    "personaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8",
    "displayNameAtCreation": "Rhozwyn Darius"
  },
  "format": {
    "version": 1,
    "minimumScriviVersion": "0.1"
  },
  "manuscript": {
    "path": "manuscript/manuscript.meta.json"
  },
  "identities": {
    "membersPath": "identities/project-members.json",
    "personasPath": "identities/project-personas.json"
  },
  "features": {
    "gitSnapshots": {
      "enabled": false,
      "metadataPath": null
    },
    "comments": {
      "enabled": true,
      "path": "comments/"
    },
    "encryption": {
      "state": "none",
      "method": null
    }
  }
}
```

### 4.3 Required Fields

```text
schema
projectID
title
createdAt
createdBy
format
manuscript.path
identities.membersPath
identities.personasPath
features.encryption.state
```

### 4.4 Notes

- `createdBy` records the active persona at the time of project creation.
- `projectID` must not depend on the project folder name.
- `features.encryption` is a future-compatibility hook only in v1.
- Git-backed snapshots are optional and may be enabled at project creation or later.

---

## 5. `manuscript/manuscript.meta.json`

### 5.1 Purpose

`manuscript.meta.json` defines the manuscript as an ordered writing unit.

It records the canonical chapter order and leaves room for future front matter and back matter.

### 5.2 Example

```json
{
  "schema": "scrivi.manuscript.v1",
  "manuscriptID": "manuscript_01J8Z8D4C3B2A1N0M9K8J7H6G5",
  "title": "The Door Beneath the Hill",
  "createdAt": "2026-05-18T18:01:00Z",
  "createdBy": {
    "identityID": "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8",
    "personaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8",
    "displayNameAtCreation": "Rhozwyn Darius"
  },
  "structure": {
    "frontMatter": [],
    "chapters": [
      {
        "chapterID": "chapter_01J8Z8H9K7M6N5B4V3C2X1Z0A9",
        "path": "chapter-001/chapter.meta.json"
      }
    ],
    "backMatter": []
  },
  "exportDefaults": {
    "includeFrontMatter": true,
    "includeBackMatter": true,
    "includeSceneTitles": false
  }
}
```

### 5.3 Required Fields

```text
schema
manuscriptID
title
structure.chapters
```

### 5.4 Notes

- The order of `structure.chapters` is the canonical chapter order.
- Folder order is not authoritative.
- A chapter folder may be renamed without changing the chapter ID.

---

## 6. `chapter.meta.json`

### 6.1 Example Path

```text
manuscript/chapter-001/chapter.meta.json
```

### 6.2 Purpose

`chapter.meta.json` defines a chapter and the ordered list of scenes inside it.

### 6.3 Example

```json
{
  "schema": "scrivi.chapter.v1",
  "chapterID": "chapter_01J8Z8H9K7M6N5B4V3C2X1Z0A9",
  "title": "Chapter 1",
  "slug": "chapter-001",
  "displayLabel": "Chapter 1",
  "status": "draft",
  "createdAt": "2026-05-18T18:02:00Z",
  "createdBy": {
    "identityID": "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8",
    "personaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8",
    "displayNameAtCreation": "Rhozwyn Darius"
  },
  "scenes": [
    {
      "sceneID": "scene_01J8Z8P4N3M2K1J0H9G8F7E6D5",
      "metadataPath": "001-opening-scene.meta.json"
    }
  ]
}
```

### 6.4 Required Fields

```text
schema
chapterID
title
displayLabel
status
scenes
```

### 6.5 Status Values

The approved v0.1 chapter status values are:

```text
draft
revised
final
archived
```

### 6.6 Notes

- The order of `scenes` is the canonical scene order.
- File order is not authoritative.
- Scene file names may change if the corresponding metadata is updated.

---

## 7. `scene.meta.json`

### 7.1 Example Paired Files

```text
manuscript/chapter-001/001-opening-scene.md
manuscript/chapter-001/001-opening-scene.meta.json
```

### 7.2 Purpose

`scene.meta.json` defines a scene and points to its manuscript text file.

### 7.3 Example

```json
{
  "schema": "scrivi.scene.v1",
  "sceneID": "scene_01J8Z8P4N3M2K1J0H9G8F7E6D5",
  "title": "Opening Scene",
  "slug": "001-opening-scene",
  "status": "draft",
  "createdAt": "2026-05-18T18:03:00Z",
  "createdBy": {
    "identityID": "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8",
    "personaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8",
    "displayNameAtCreation": "Rhozwyn Darius"
  },
  "modifiedAt": "2026-05-18T18:03:00Z",
  "modifiedBy": {
    "identityID": "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8",
    "personaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8",
    "displayNameAtModification": "Rhozwyn Darius"
  },
  "content": {
    "path": "001-opening-scene.md",
    "format": "markdown",
    "encoding": "utf-8",
    "encryption": "none"
  },
  "classification": {
    "kind": "scene",
    "povCharacterID": null,
    "timelineEventIDs": [],
    "tags": []
  },
  "stats": {
    "wordCount": 0,
    "characterCount": 0,
    "lastCalculatedAt": null
  }
}
```

### 7.4 Required Fields

```text
schema
sceneID
title
status
content.path
content.format
content.encryption
```

### 7.5 Status Values

The approved v0.1 scene status values are:

```text
draft
revised
final
archived
```

### 7.6 Stats

`stats` are approved in scene metadata, but they are derived and non-authoritative.

Scrivi may recalculate them at any time.

Approved derived stats:

```text
wordCount
characterCount
lastCalculatedAt
```

### 7.7 External Edits

If Scrivi detects that the `.md` file was modified outside Scrivi, it must not falsely attribute the edit to the current Scrivi identity.

The approved behavior is to represent the modification source as external.

Example:

```json
{
  "modifiedAt": "2026-05-18T19:20:00Z",
  "modifiedBy": {
    "source": "external"
  }
}
```

If Git is enabled, Git may also provide author information for the change, but Scrivi should not infer authorship unless it has reliable identity mapping.

---

## 8. `identities/project-members.json`

### 8.1 Purpose

`project-members.json` defines which identities have membership in the project and what role each identity has.

This file contains public/project-safe identity information only.

It may contain public keys in v1 because author certificates are generated and public keys support future signing.

It must not contain private keys.

### 8.2 Example

```json
{
  "schema": "scrivi.projectMembers.v1",
  "members": [
    {
      "identityID": "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8",
      "role": "owner",
      "publicKey": {
        "algorithm": "ed25519",
        "value": "PUBLIC_KEY_PLACEHOLDER"
      },
      "joinedAt": "2026-05-18T18:00:00Z",
      "status": "active",
      "defaultPersonaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8"
    }
  ]
}
```

### 8.3 Required Fields

```text
schema
members
identityID
role
status
```

### 8.4 Role Values

The approved v1 roles are:

```text
owner
editor
reader
```

### 8.5 Status Values

The approved v0.1 member status values are:

```text
active
removed
```

### 8.6 Notes

- `publicKey` is allowed and recommended in v1.
- Private keys belong in secure local storage, never in the project package.
- Group/shared credentials are out of scope for v1, but the schema should not prevent later group persona support.

---

## 9. `identities/project-personas.json`

### 9.1 Purpose

`project-personas.json` defines project-facing author personas.

A persona is the name, byline, or pseudonym under which an identity writes.

One identity may use multiple personas across different projects. A persona name may change without changing the underlying identity.

### 9.2 Example

```json
{
  "schema": "scrivi.projectPersonas.v1",
  "personas": [
    {
      "personaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8",
      "displayName": "Rhozwyn Darius",
      "personaKind": "individual",
      "controlledBy": [
        "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8"
      ],
      "createdAt": "2026-05-18T18:00:00Z",
      "status": "active",
      "nameHistory": [
        {
          "displayName": "Rhozwyn Darius",
          "startedAt": "2026-05-18T18:00:00Z",
          "endedAt": null
        }
      ]
    }
  ]
}
```

### 9.3 Required Fields

```text
schema
personas
personaID
displayName
personaKind
controlledBy
status
```

### 9.4 Persona Kind Values

The approved v1 value is:

```text
individual
```

The future reserved value is:

```text
group
```

`group` is a design hook for later support of shared/group personas, such as a review board. Group personas are not implemented in v1.

### 9.5 Persona Rename Behavior

Scrivi stores both:

1. The stable persona ID.
2. The display name at the time of contribution.

This allows the current persona name to change without destroying historical attribution.

---

## 10. App-Local `workspace-state.json`

### 10.1 Purpose

`workspace-state.json` is not part of the canonical project package, but it is required for the writing-first launch experience.

It tells Scrivi which writing surface to open and where to place the cursor.

### 10.2 Example App-Local Path

```text
Scrivi App Support/
  state/
    projects/
      project_01J8Z7V6B5W4Q3R2N1M0K9J8H7/
        workspace-state.json
```

### 10.3 Example

```json
{
  "schema": "scrivi.workspaceState.v1",
  "projectID": "project_01J8Z7V6B5W4Q3R2N1M0K9J8H7",
  "deviceID": "device_01J8Z9A8B7C6D5E4F3G2H1J0K9",
  "identityID": "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8",
  "activePersonaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8",
  "lastWritingSurface": {
    "sceneID": "scene_01J8Z8P4N3M2K1J0H9G8F7E6D5",
    "contentPath": "manuscript/chapter-001/001-opening-scene.md",
    "cursor": {
      "anchor": 0,
      "focus": 0
    },
    "scrollPosition": 0.0
  },
  "lastOpenedAt": "2026-05-18T18:10:00Z"
}
```

### 10.4 Notes

- This file is app-local state, not canonical project data.
- It may vary per device.
- It may be synced later by user choice, but sync is not required for v1.
- It should not be committed into project Git history.

---

## 11. Minimum Open-Project Algorithm

Scrivi can open a minimum valid project with the following sequence:

```text
1. Read project.json.
2. Read identities/project-members.json.
3. Read identities/project-personas.json.
4. Read app-local workspace-state.json, if present.
5. If workspace state names a valid scene, open that scene.
6. Otherwise read manuscript/manuscript.meta.json.
7. Resolve the first chapter.
8. Read chapter.meta.json.
9. Resolve the first scene.
10. Read scene.meta.json.
11. Open scene.content.path.
12. Place cursor from workspace state, or at document start.
```

This satisfies the writing-first launch requirement without loading the full project graph.

---

## 12. Minimum New-Project File Set

A new Scrivi project with one chapter and one scene should create:

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

The initial `.md` file should be empty prose.

Titles belong in metadata, not as required inline Markdown content.

---

## 13. Deferred Schemas

The following schemas are not part of the minimum set and should be defined later:

```text
asset metadata
comments
snapshot metadata
object schemas
relationship schemas
timeline schemas
export history
repair/staging records
Git snapshot metadata
```

These are important but not required for the first project-open/write loop.

---

## 14. Approved Open-Question Decisions

The following decisions were approved before finalizing this document:

1. `stats` are allowed in `scene.meta.json` but are derived and non-authoritative.
2. External manuscript edits must not be falsely attributed to the current Scrivi identity; use `modifiedBy.source = "external"` or equivalent.
3. `project-members.json` may contain public keys in v1.
4. `displayNameAtCreation` and `displayNameAtModification` are included for authored objects where appropriate.
5. Comments are not included in the minimum schema set, even though comments are part of the v1 architecture.

---

## 15. Success Criteria

The minimum schema set is successful if:

1. Scrivi can create a valid project with one chapter and one scene.
2. Scrivi can open that project and place the cursor on the last writing surface.
3. The manuscript remains readable and editable outside Scrivi.
4. Metadata is visibly paired with manuscript files.
5. Stable IDs survive renames and moves.
6. Authorship can be attributed through identities and personas.
7. Persona renaming does not destroy historical attribution.
8. Public keys can support future signing without redesign.
9. Future encryption is not blocked.
10. Comments, assets, snapshots, and richer object models can be added without replacing the project format.

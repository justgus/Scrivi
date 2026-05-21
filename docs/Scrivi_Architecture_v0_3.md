# Scrivi Architecture v0.3

**Project:** Scrivi  
**Document:** Core Architecture  
**Version:** 0.3  
**Status:** Approved direction — supersedes all v0.1 and v0.2 architecture documents  
**Supersedes:**
- `Scrivi_Backend_Architecture_v0_2.md`
- `Scrivi_Cpp24_Core_API_Sketch_v0_2.md`
- `Scrivi_Apple_Wrapper_Design_v0_1.md`
- `Scrivi_Swift_Interop_Trade_Study_v0_1.md`

---

## 1. Purpose of This Document

This document re-evaluates the Scrivi architecture after the T-0011 Swift interop prototype. The prototype confirmed the feasibility of the C++/Swift stack and revealed one architectural principle that was not explicit in earlier documents: **all data transmission in the system must be by value, not by reference**.

That principle — and its consequences — are the basis for this revision.

Everything in the earlier architecture that holds up is carried forward unchanged. Anything that conflicts with the pass-by-value principle, or that was designed without it in mind, is revised here.

---

## 2. What the Prototype Proved

T-0011 implemented `ensureLocalIdentity`, `createProject`, `openProject`, and `saveScene` through a C++ adapter into Swift. Four tests passed out of five. The one failure was instructive.

**What worked:**
- C++ static library building and linking via CMake
- Swift Package Manager importing a C++ module
- `ScriviAdapter` as a reference type (`import_reference`) with atomic retain/release
- `const char*` input parameters from Swift `withCString`
- Flat output structs with fixed `char[]` arrays for small fixed-size results
- The full pipeline: identity → project creation → open → save → reopen

**What broke:**
- `openProject` could not return variable-length Markdown content across the boundary. The cause was not a logic error — it was a memory model mismatch. Swift and C++ have fundamentally different ownership semantics. Any design that passes a pointer from C++ to Swift, and expects Swift to read memory owned by C++, will fail because Swift cannot guarantee that memory is still valid or properly managed.

**The insight:**
The prototype was designed to transfer data conveniently across the boundary, using the same internal C++ result structs that were designed for C++-internal use. Those structs hold `std::string` members — heap-allocated, with interior pointers. Swift cannot safely receive interior pointers. The fix is not a workaround at the boundary. It is a design principle applied consistently through the whole system: **data moves between layers by value, as a complete copy**.

---

## 3. The Pass-by-Value Principle

**Definition:** Every transmission of data between components in the Scrivi system must produce an independent, owned copy of the data in the receiver. No component should depend on memory owned by another component remaining valid after the call returns.

This principle applies at every boundary in the system:

| Boundary | Mechanism | Why it is pass-by-value |
|---|---|---|
| Filesystem → C++ | `readTextFile()` returns `std::string` | The string is a copy of file contents owned by the caller |
| C++ → Filesystem | `atomicWriteTextFile(std::string_view)` | The string_view is consumed synchronously; no shared ownership |
| C++ internal → result types | Result structs hold `std::string` members | Each result is a value, moved or copied out of the call |
| C++ → Swift boundary | `std::string` JSON returned by value | Swift receives a copy; C++ can free its memory immediately |
| Swift → C++ boundary | `const char*` via `withCString` | C++ reads the string synchronously; lifetime is guaranteed by `withCString` |
| Swift → UI | Swift structs decoded from JSON | `Codable` structs are value types; full ownership in Swift |

The mechanism at the C++/Swift boundary is JSON-over-`std::string`. The adapter serializes every result to a `std::string` JSON value and returns it. Swift receives the string by value, copies it into a Swift `String`, and decodes it with `Codable`. No pointers cross in either direction after the call returns.

This is not an optimization. It is the only design that is safe across the Swift/C++ boundary. JSON-over-string is therefore the permanent boundary protocol, not a temporary workaround.

---

## 4. What Does Not Change

The following decisions from earlier documents are confirmed and carried forward unchanged.

### 4.1 Runtime: C++23 shared backend core

C++ is the implementation language for ScriviCore. The project lead is fluent in C++. The backend is file-I/O heavy, cross-platform, and has no UI concerns. C++ is well-suited to all of these. No other runtime is introduced.

The earlier runtime trade study (v0.2) reached this conclusion through systematic comparison. It stands.

### 4.2 On-disk format: Markdown + JSON

Manuscript content is stored as plain Markdown (`.md`) files. Metadata is stored as JSON (`.meta.json`, `project.json`, etc.). Files are human-readable, visibly paired, and manageable with standard tools outside Scrivi. This is not renegotiated.

### 4.3 Project package structure

A Scrivi project is a directory with a `.scrivi` extension. The canonical structure is:

```
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
  snapshots/
    scrivi-snapshots.json      (if Git enabled)
  .gitignore                   (if Git enabled)
  .git/                        (if Git enabled)
```

App-local state (workspace state, indexes, cache) lives outside the project package in the platform app support directory. It is rebuildable and not canonical.

### 4.4 Stable opaque IDs

All durable Scrivi objects are identified by stable opaque IDs with typed prefixes:

```
project_<ulid>
manuscript_<ulid>
chapter_<ulid>
scene_<ulid>
identity_<ulid>
persona_<ulid>
snapshot_<ulid>
```

IDs are authoritative. Filenames, slugs, and titles are human-facing conveniences that can change without affecting identity.

### 4.5 Identity and persona model

Scrivi distinguishes the **identity** (the stable device-bound authority) from the **persona** (the project-facing author name or pseudonym). An identity may have multiple personas across projects. Authorship attribution records both the stable ID and the display name at the time of the contribution, so persona renames do not destroy historical attribution.

Private key material never enters the project package. The project may contain public keys. Private keys belong in platform secure storage (Keychain on Apple platforms).

### 4.6 Service injection

ScriviCore depends on platform services through injected interfaces: `FileSystem`, `Clock`, `UUIDProvider`, `SecureStore`, `GitProvider`, `Logger`. No service is instantiated by ScriviCore itself. Platform wrappers wire the services at startup. Test code wires mock services.

This design is confirmed. The `CoreServices` struct is the correct injection container.

### 4.7 Public facade

ScriviCore exposes a single facade class with coarse-grained methods. No tiny getters or setters. No interior state is exposed. Each method takes a request struct and returns a result struct wrapped in `Result<T>`.

Current operations:

```cpp
Result<EnsureIdentityResult>     ensureLocalIdentity(const EnsureIdentityRequest&);
Result<CreateProjectResult>      createProject(const CreateProjectRequest&);
Result<OpenProjectResult>        openProject(const OpenProjectRequest&);
Result<SaveSceneResult>          saveScene(const SaveSceneRequest&);
Result<ExternalChangeScanResult> scanForExternalChanges(const ExternalChangeScanRequest&);
Result<EnableGitResult>          enableGitSnapshots(const EnableGitRequest&);
Result<CreateSnapshotResult>     createSnapshot(const CreateSnapshotRequest&);
```

This shape is confirmed.

### 4.8 Error model

All operations return `Result<T>` — either `Result<T>::success(value)` or `Result<T>::failure(error)`. Exceptions are not used. The `Error` struct carries a typed `ErrorCode` and a human-readable `message` string.

This model is confirmed.

### 4.9 Repair model

Scrivi uses a Detect → Stage → Review → Commit repair pattern. ScriviCore detects and classifies repair conditions and returns `RepairIssue` lists in results. It does not apply repairs automatically without caller instruction. The UI presents repair options. The caller invokes the appropriate repair action.

This model is confirmed.

### 4.10 JSON schema evolution: flat and additive

On-disk JSON schemas evolve by addition only. New fields are added; existing fields are not renamed or removed without a schema version bump. Readers ignore unknown fields. This keeps the format compatible across Scrivi versions without requiring migration logic.

The `"schema"` field in each JSON file is retained as a version hook for future use. In MVP, schema version checking is informational only — no migration logic is implemented.

---

## 5. What Changes

### 5.1 The boundary protocol is JSON-over-`std::string`, permanently

Earlier documents (API sketch v0.2, §27) designated direct Swift/C++ struct interop as the primary Apple strategy, with JSON-over-string as a possible future C ABI option. That is reversed.

**JSON-over-`std::string` is the primary and permanent boundary protocol for all platform wrappers.**

The reason is the pass-by-value principle: it is the only mechanism that safely transfers variable-length data across a language boundary without shared memory ownership. It applies not just to Swift but to every future platform (Android/JNI, Windows, Linux tooling, scripting). Any boundary that can use JSON-over-string should.

The C++ adapter layer (`ScriviCoreAdapter`) serializes every result to a `std::string` JSON value. The platform wrapper (Swift, or any future language) receives the string by value and deserializes it with the platform's native JSON tools.

### 5.2 The adapter layer is the canonical C++/platform boundary

The `ScriviCoreAdapter` shim is not a temporary workaround. It is a permanent, load-bearing layer with a well-defined contract:

- **Inputs:** `const char*` parameters. Simple, safe, universally compatible.
- **Outputs:** `std::string` JSON values, returned by value.
- **No pointers cross as return values.** Ever.
- **No ScriviCore types are exposed through the adapter header.** The adapter header depends only on `<string>`, `<memory>`, and `<atomic>`.

The adapter owns the service wiring for its platform. On Apple, `ScriviAdapter::Impl` instantiates `LocalFileSystem`, `SystemUUIDProvider`, `PrototypeClock`, and the platform `SecureStore`. On future platforms, the equivalent adapter instantiates platform-appropriate services.

### 5.3 The JSON envelope is the result contract

Every adapter method returns one of two JSON envelopes:

**Success:**
```json
{ "ok": true, "result": { ... } }
```

**Failure:**
```json
{ "ok": false, "error": { "code": 7, "message": "..." } }
```

The `code` value maps to `scrivi::ErrorCode` cast to `int`. The `message` is the human-readable error string from `scrivi::Error`.

The platform wrapper checks `ok` first. On failure, it constructs a platform error type from `code` and `message`. On success, it decodes the nested `result` object.

### 5.4 Result JSON schemas align with on-disk schemas

The JSON returned in the `result` field uses the same field names as the corresponding on-disk schemas where possible. This is not a strict requirement — the result schemas are driven by what the caller needs, not by the full on-disk schema — but unnecessary divergence is avoided.

The approved result schemas for the current four adapter methods are defined in `Scrivi_Apple_Wrapper_Design_v0_1.md` §4. Those schemas are the reference. They will be updated as the API evolves.

### 5.5 `OpenProjectResult` carries Markdown by value

The prototype's failure was specifically that Markdown content could not cross the C++/Swift boundary as a pointer. Under the new design, `openProject` serializes the full active scene Markdown into the JSON result string, and Swift receives it as a value. There is no `getLastOpenMarkdown()` method, no `lastOpenMarkdown` buffer in the adapter, and no fixed-size char array. The Markdown is part of the JSON.

This is correct regardless of content size. JSON strings can hold arbitrary UTF-8 content. The overhead of serializing scene Markdown into JSON is negligible compared to disk I/O.

### 5.6 `SaveSceneRequest` carries cursor and scroll

The `SaveSceneRequest` includes `TextSelection selection` and `ScrollPosition scroll` fields. These are workspace-state values that the platform UI provides on save. The adapter passes them through to `ScriviCore::saveScene`, which updates `workspace-state.json`.

On the Swift side, `saveScene` currently omits these fields (they default to zero). This is acceptable for MVP. The adapter and Swift type definitions will be extended when the UI editor is implemented and cursor tracking is live.

### 5.7 `deviceID` is hardcoded "device-local" in MVP

`WorkspaceState.deviceID` is currently hardcoded to `"device-local"` in `SceneWriter.cpp`. This is a known limitation. A production implementation will use a stable device-specific identifier. This is deferred until the workspace state and sync design is revisited.

---

## 6. Layer Map

```
┌─────────────────────────────────────────────────┐
│  Platform UI (SwiftUI / future Android / etc.)  │
│  Not in ScriviCore scope                        │
└────────────────┬────────────────────────────────┘
                 │  Swift/Kotlin/etc. native types
┌────────────────▼────────────────────────────────┐
│  Platform Engine (ScriviEngine.swift / equiv.)  │
│  • withCString input conversion                 │
│  • JSON envelope decode (Codable/etc.)          │
│  • Platform error throw                         │
└────────────────┬────────────────────────────────┘
                 │  const char* inputs / std::string JSON outputs
┌────────────────▼────────────────────────────────┐
│  ScriviCoreAdapter (C++)                        │
│  • import_reference / retain-release (Apple)   │
│  • Service wiring (platform-specific)           │
│  • Delegates to ScriviCore                      │
│  • Serializes results to JSON std::string       │
└────────────────┬────────────────────────────────┘
                 │  C++ public API (Request/Result structs)
┌────────────────▼────────────────────────────────┐
│  ScriviCore (C++23 static library)              │
│  • Facade: ScriviCore.hpp                       │
│  • Identity, project, manuscript, repair, git   │
│  • Service interfaces (injected)                │
│  • nlohmann/json behind schema wrappers         │
└────────────────┬────────────────────────────────┘
                 │  FileSystem service (injected)
┌────────────────▼────────────────────────────────┐
│  Filesystem / SecureStore / Clock / Git         │
│  Platform service implementations               │
└─────────────────────────────────────────────────┘
```

Data flows upward and downward as values. No layer holds a reference into another layer's memory after a call returns.

---

## 7. JSON Schema Reference

The following tables summarize the approved JSON schema field sets for each adapter operation. These are the result schemas — the data that crosses the C++/Swift boundary. On-disk schemas (the full file formats) are documented in `Scrivi_Minimum_Schema_Set_v0_1.md` and are not repeated here.

### 7.1 `ensureLocalIdentity`

| Field | Type | Notes |
|---|---|---|
| `identityID` | string | `identity_<ulid>` |
| `defaultPersonaID` | string | `persona_<ulid>` |
| `displayName` | string | |
| `createdNewIdentity` | bool | false if identity already existed |

### 7.2 `createProject`

| Field | Type | Notes |
|---|---|---|
| `projectID` | string | `project_<ulid>` |
| `firstScene.sceneID` | string | `scene_<ulid>` |
| `firstScene.metadataPath` | string | relative to project root |
| `firstScene.contentPath` | string | relative to project root |

### 7.3 `openProject`

| Field | Type | Notes |
|---|---|---|
| `projectID` | string | |
| `activeScene.sceneID` | string | |
| `activeScene.metadataPath` | string | relative to project root |
| `activeScene.contentPath` | string | relative to project root |
| `activeScene.markdown` | string | full UTF-8 scene content |

### 7.4 `saveScene`

| Field | Type | Notes |
|---|---|---|
| `sceneID` | string | |
| `saved` | bool | false if content was unchanged |
| `wordCount` | int | derived, non-authoritative |

---

## 8. Service Wiring by Platform

| Service | Apple MVP | Apple Production | Future Android |
|---|---|---|---|
| `FileSystem` | `LocalFileSystem` | same | platform equivalent |
| `UUIDProvider` | `SystemUUIDProvider` | same | platform equivalent |
| `Clock` | `PrototypeClock` (chrono) | same | same |
| `SecureStore` | `PrototypeSecureStore` (in-memory) | `KeychainSecureStore` | Android Keystore |
| `GitProvider` | `nullptr` | `SystemGitProvider` | same |
| `Logger` | `nullptr` | platform logger | platform logger |

`PrototypeSecureStore` is an in-memory `unordered_map`. It works for MVP but does not survive process restart. Replacing it with `KeychainSecureStore` is a sprint-level task, not an architectural question.

---

## 9. Data Model Status

The on-disk data model (the JSON schemas in `Scrivi_Minimum_Schema_Set_v0_1.md`) is directionally correct and not revised by this document. The schema set has not been exercised extensively yet, so fine-grained revision would be premature. Specific observations:

**Confirmed correct:**
- Stable ID scheme with typed prefixes
- Paired `.md` / `.meta.json` files
- `createdBy` / `modifiedBy` with both stable ID and display-name-at-time fields
- `workspace-state.json` living outside the project package
- `stats` block in `scene.meta.json` as derived/non-authoritative

**Known gaps to address in later sprints:**
- `deviceID` in `workspace-state.json` is hardcoded; needs a real stable device identifier
- `scrollPosition` in `WorkspaceStateData` is stored as `int * 1000` (no `setDouble` in `JsonDoc`); `JsonDoc` should gain a `setDouble`/`getDouble` method
- `SaveSceneRequest.selection` and `SaveSceneRequest.scroll` are not yet wired from Swift; defaults to zero
- `PrototypeSecureStore` does not persist across process restarts; production `KeychainSecureStore` needed before shipping

**Not yet designed:**
- Objects (characters, locations, items, rules, timelines) — deferred
- Assets — deferred
- Comments — deferred
- Export — deferred

---

## 10. Open Questions

These are unresolved questions that will affect future design decisions. They are not blockers for the current sprint.

1. **Device identity.** What is the correct mechanism for generating a stable `deviceID` on Apple platforms? On macOS, `IOPlatformExpertDevice` serial number or `SecKeychainItem` UUID are options. This must be settled before workspace state sync is designed.

2. **`JsonDoc` double support.** `ScrollPosition.value` is a `double`. The current `JsonDoc` wrapper has no `setDouble`/`getDouble`. Either add them or represent scroll position as a fixed-point integer. This is a small implementation gap, not an architectural question.

3. **`KeychainSecureStore`.** When does the production Keychain implementation replace `PrototypeSecureStore`? This should happen before any user testing — in-memory secrets do not survive restart, which means identity is lost on every restart. This is a sprint-level task.

4. **Cursor and scroll from Swift.** `ScriviEngine.saveScene` currently passes zero for `selection` and `scroll`. When the SwiftUI editor is implemented, these must be wired. The adapter already accepts them; only the Swift call site needs updating.

5. **`appSupportRoot` bootstrap on other platforms.** `AppSupportLayout` bootstraps the app support directory structure. On Apple, the path is the platform Application Support directory. On other platforms this will need a platform-specific determination. The mechanism (injected path, environment variable, etc.) is not yet defined for non-Apple targets.

6. **Multi-scene projects.** The current `openProject` result returns a single `activeScene`. A future `openProject` result will need to return enough information for the UI to render a scene list (manuscript order, chapter/scene titles, statuses). The schema for that richer result is not yet designed.

7. **Repair actions.** `scanForExternalChanges` returns `RepairIssue` lists. The API does not yet have operations for applying repair actions. These will be needed before the repair UI is designed.

---

## 11. Architectural Principles (Consolidated)

These are the governing principles of the Scrivi architecture. Future design decisions that contradict any of these require explicit re-evaluation.

1. **Pass by value.** All data transmission between layers produces an independent, owned copy in the receiver. No layer depends on memory owned by another layer remaining valid after a call returns.

2. **JSON is the transmission medium.** JSON-over-`std::string` is the protocol at every cross-language boundary. It is safe, ubiquitous, human-readable, and machine-parseable. It is not a workaround.

3. **Flat and additive schemas.** JSON schemas evolve by adding fields. Existing fields are not renamed or removed without a version bump. Readers tolerate unknown fields.

4. **IDs are authoritative.** Filenames, slugs, and titles are conveniences. Stable opaque IDs are identity.

5. **No backend logic in platform wrappers.** Platform wrappers (Swift, future Android, etc.) convert types and decode results. They do not reimplement ScriviCore behavior.

6. **Service injection.** ScriviCore depends on platform services through injected interfaces. No service is instantiated by ScriviCore. Tests wire mock services.

7. **Explicit error model.** Operations return `Result<T>`. Errors are typed and carry human-readable messages. Exceptions are not used.

8. **Coarse API surface.** ScriviCore exposes coarse-grained operations, not fine-grained object accessors. The facade is the contract.

9. **Canonical data lives on disk.** The project package (Markdown + JSON files on disk) is the source of truth. In-process state is a cache. App-local state (workspace, indexes) is rebuildable.

10. **Repair without destruction.** When Scrivi detects an inconsistency, it stages a repair issue and presents options. It does not silently overwrite, delete, or repair without caller instruction.

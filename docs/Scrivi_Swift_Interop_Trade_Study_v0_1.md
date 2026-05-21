# Scrivi Swift/C++ Interop Trade Study v0.1

**Project:** Scrivi  
**Document:** Swift/C++ Interop Trade Study  
**Version:** 0.1  
**Status:** Approved — replaces Section 27 direction in `Scrivi_Cpp24_Core_API_Sketch_v0_2.md`  
**Supersedes:** `Scrivi_Cpp24_Core_API_Sketch_v0_2.md` §27 (Direct Swift/C++ interop approval)

---

## 1. Purpose

This trade study documents the Swift/C++ interop boundary options evaluated during T-0011 (Swift Interop Prototype). It records what was attempted, what blocked each approach, and why the approved boundary design changed.

The approved API sketch (v0.2, §27) favored direct Swift/C++ interop with the ScriviCore public headers. T-0011 implementation revealed that the public API as designed is not directly importable by Swift without changes that would either constrain the C++ API or require unsafe workarounds. This document records the findings and updates the direction.

---

## 2. Background: What Swift/C++ Direct Interop Actually Means

Swift 5.9+ supports direct import of C++ types and functions via module maps and the `-cxx-interoperability-mode=default` compiler flag. Unlike Objective-C bridging, no `.h` wrapper is written by hand — Swift reads C++ headers directly through Clang.

The important constraints are:

**Value types** — C++ structs/classes are imported as Swift value types (copied on assignment) if they are *trivially movable* and contain no interior pointers visible at the boundary. Structs with only primitive fields and fixed-size arrays are imported cleanly.

**Reference types** — C++ classes with `unique_ptr` or other move-only members cannot be value types in Swift. They must be annotated as reference types using `SWIFT_SHARED_REFERENCE(retain, release)` (from `<swift/bridging>`) and managed via ARC-style retain/release hooks. The `ScriviAdapter` class falls into this category.

**Interior pointers** — Swift refuses to import C++ methods returning types that contain `char*` (raw pointer) members because Swift cannot reason about the pointer's lifetime. This includes returning structs that embed a `const char*` field.

**`std::string` and STL types** — `std::string` crosses the boundary as a Swift value type: `String(cxxString)` constructs a Swift String copy; `std.string(swiftString)` is valid Swift→C++. However, structs containing `std::string` members are only importable if the compiler can prove they are self-contained (no interior pointers to other allocations that escape the value).

---

## 3. Approaches Evaluated

Three boundary designs were evaluated. All three were attempted or analyzed against the actual ScriviCore API shape.

---

### Option A: Direct C++ struct interop with ScriviCore public headers

**Description:**  
Import `ScriviCore.hpp`, `Requests.hpp`, `Results.hpp` directly into Swift. Call `ScriviCore` methods from Swift using the public C++ types.

**Analysis:**

The ScriviCore public API uses `std::string` fields throughout: `EnsureIdentityRequest.requestedDisplayName`, `CreateProjectRequest.title`, `OpenProjectResult.activeSceneMarkdown`, and all path fields. A result struct containing `std::string activeSceneMarkdown` is not self-contained from Swift's perspective — `std::string` is a heap-allocated type with an interior pointer to its character data. Swift cannot guarantee the copied struct owns its storage independently in all configurations.

Additionally, `ScriviCore` itself requires `CoreServices` wiring at construction time. `CoreServices` holds raw `Service*` pointers. Constructing a `ScriviCore` from Swift would require Swift to instantiate and manage service objects (clock, UUID provider, file system, secure store). Those services are currently concrete C++ types defined in `src/platform/` — not public API. Wiring them from Swift would require exposing private headers or providing a factory function.

**Result: Blocked.** `OpenProjectResult` contains `activeSceneMarkdown` as `std::string`, which Swift cannot import as a self-contained value type. The `ScriviCore` constructor requires service wiring that is not part of the public API.

**Verdict: Not viable without fundamental changes to ScriviCore's public API.**

---

### Option B: Thin C++ adapter with fixed char arrays and `const char*`

**Description:**  
Write a `ScriviAdapter` C++ shim that wraps `ScriviCore`, accepts `const char*` parameters, and returns flat structs with primitive fields and fixed-size `char[]` arrays. Markdown content too long for a fixed array is returned via a separate `getLastOpenMarkdown() const char*` method.

This was the approach actually implemented during T-0011.

**What worked:**
- `ScriviAdapter` annotated as `import_reference` with `retain`/`release` hooks imported cleanly into Swift.
- Structs with `bool`, `int`, `uint64_t`, and `char[N]` fields imported cleanly.
- `EnsureIdentityOut`, `CreateProjectOut`, `SaveSceneOut` all imported successfully.
- `ensureLocalIdentity`, `createProject`, `saveScene` all worked end-to-end through the boundary.

**What blocked:**

`openProject` was blocked at the Swift import stage. The active scene Markdown cannot go into `OpenProjectOut` as a `char[]` member — the Markdown content is variable-length and can be arbitrarily large (full manuscript scene text). A 64 KB fixed buffer would cover most scenes but is too large for Swift to import as a value type: Swift structs above a few hundred bytes cannot be passed in registers and the compiler rejects them at the module import boundary.

The `getLastOpenMarkdown() const char*` workaround — returning a pointer to a `std::string` stored inside `Impl` — was blocked because Swift refuses to import any method returning `const char*` without explicit `SWIFT_RETURNS_INDEPENDENT_VALUE` annotation. That annotation requires `<swift/bridging>`, and even with it, returning a pointer into interior storage is unsafe if the pointer outlives the next `openProject` call.

**Attempted fixes:**

- `SWIFT_RETURNS_INDEPENDENT_VALUE` attribute via raw `__attribute__((swift_attr(...)))` — rejected; the raw string syntax does not activate the bridging macro behavior, which requires including `<swift/bridging>`.
- Including `<swift/bridging>` — available in the Apple SDK but requires the `SWIFT_BRIDGING` feature macro to be defined; adding it to the adapter header introduces SDK version coupling.
- Storing Markdown in a separately accessed wrapper type — requires a second crossing, complicating the API.

**Result: Blocked on `openProject` markdown return.** All other operations proved the boundary works. The pattern of flat structs with fixed arrays is sound for small fixed-size results but cannot extend to variable-length content.

**Verdict: Viable for operations without variable-length output. Not extensible.**

---

### Option C: JSON-over-`std::string` boundary (approved direction)

**Description:**  
The `ScriviAdapter` methods return `std::string` by value, where the string contains a JSON-encoded result (or a JSON-encoded error). Swift receives the `std::string` as a value type copy (Swift `String`), then deserializes with `Codable`.

Request parameters may be passed as individual `const char*` (per current design) or as a single JSON `const char*` request string. Either way, the important constraint is that all returns are `std::string` by value — not structs with interior pointers, not raw `char*`.

**Why this works:**

`std::string` is the one STL type that Swift's C++ interop handles reliably as a value. `String(cxxString)` copies the string data into a Swift-managed allocation. The copy happens at the boundary, so the C++ side's storage can be freed immediately after the call returns. There is no pointer lifetime problem.

JSON serialization is already present in ScriviCore (`nlohmann/json` is the approved JSON library). Adding `toJson()` serialization methods to result structs is a small, contained change. The JSON schemas must be defined — and are defined in `Scrivi_Minimum_Schema_Set_v0_1.md` — so the serialization round-trip has a known, documented contract.

**Tradeoffs:**

- Added serialize/deserialize step at the boundary. Cost is proportional to payload size — for project metadata and scene content the overhead is negligible on modern hardware.
- JSON schema for API results must be defined and maintained. This is discussed in Section 5.
- Error returns must also be serialized. A simple `{"ok": false, "error": {"code": 1, "message": "..."}}` envelope covers all cases.
- Requires Swift `Codable` structs for each result type. These are small and mechanical to write.

**What this does not require:**

- Any changes to ScriviCore's C++ public API or internal types.
- Exposing private service headers to Swift.
- `<swift/bridging>` or platform SDK macros.
- Fixed-size buffer decisions for variable-length content.
- `getLastOpenMarkdown()` workaround.

**Result: Unblocked on all operations including `openProject` with full Markdown content.**

**Verdict: Approved.**

---

## 4. Comparison Summary

| Criterion | Option A: Direct | Option B: Flat structs | Option C: JSON-over-string |
|---|---|---|---|
| `openProject` works | No | No (blocked) | Yes |
| Variable-length content | No | No | Yes |
| Requires ScriviCore API changes | Yes (large) | No | No |
| Requires `<swift/bridging>` | Yes | Yes (workaround) | No |
| Type safety at boundary | High | Medium | Low (string-typed) |
| Codable structs required | No | No | Yes (small, mechanical) |
| Serialize cost | None | None | Negligible |
| Extensible | No | No | Yes |

---

## 5. JSON Schema Strategy

### 5.1 Principle: API schemas are sub-objects of on-disk schemas

The on-disk schemas (`project.json`, `scene.meta.json`, `workspace-state.json`, etc.) are the canonical format documented in `Scrivi_Minimum_Schema_Set_v0_1.md`. The API result JSON returned across the boundary should reuse these same schema shapes wherever possible.

The API does not invent new JSON vocabularies. Where an API result returns project-level data, it returns exactly the same field names and structure as `project.json`. Where it returns scene metadata, it returns the same field names as `scene.meta.json`.

### 5.2 Envelope format

All adapter methods return a top-level envelope:

```json
{
  "ok": true,
  "result": { ... }
}
```

On error:

```json
{
  "ok": false,
  "error": {
    "code": 7,
    "message": "project.json not found at path"
  }
}
```

Swift decodes the envelope first. If `ok` is `false`, it throws `ScriviError`. If `ok` is `true`, it decodes the nested `result`.

### 5.3 `ensureLocalIdentity` result

Returns identity data from app-local storage. No on-disk project schema equivalent — this is identity-layer data.

```json
{
  "ok": true,
  "result": {
    "identityID": "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8",
    "defaultPersonaID": "persona_01J8Z801V6B5N4M3K2J1H0G9F8",
    "displayName": "Rhozwyn Darius",
    "createdNewIdentity": true
  }
}
```

### 5.4 `createProject` result

Returns the project manifest fields needed to open the first scene. Structure mirrors the `project.json` schema top level plus scene resolution fields.

```json
{
  "ok": true,
  "result": {
    "projectID": "project_01J8Z7V6B5W4Q3R2N1M0K9J8H7",
    "firstScene": {
      "sceneID": "scene_01J8Z8P4N3M2K1J0H9G8F7E6D5",
      "metadataPath": "manuscript/chapter-001/001-opening-scene.meta.json",
      "contentPath": "manuscript/chapter-001/001-opening-scene.md"
    }
  }
}
```

### 5.5 `openProject` result

Returns project identity plus the active writing surface — mirrors `workspace-state.json` `lastWritingSurface` fields, plus the scene Markdown content.

```json
{
  "ok": true,
  "result": {
    "projectID": "project_01J8Z7V6B5W4Q3R2N1M0K9J8H7",
    "activeScene": {
      "sceneID": "scene_01J8Z8P4N3M2K1J0H9G8F7E6D5",
      "metadataPath": "manuscript/chapter-001/001-opening-scene.meta.json",
      "contentPath": "manuscript/chapter-001/001-opening-scene.md",
      "markdown": "It was raining when the door beneath the hill opened."
    }
  }
}
```

The `markdown` field is the full scene text. This is the field that was blocked in Options A and B. It crosses cleanly as part of a JSON `std::string`.

### 5.6 `saveScene` result

Mirrors `scene.meta.json` `stats` fields plus save confirmation.

```json
{
  "ok": true,
  "result": {
    "sceneID": "scene_01J8Z8P4N3M2K1J0H9G8F7E6D5",
    "saved": true,
    "wordCount": 11
  }
}
```

---

## 6. Impact on ScriviCore

ScriviCore's C++ public API (`ScriviCore.hpp`, `Requests.hpp`, `Results.hpp`) does not change. The JSON serialization of results lives entirely in the adapter layer (`ScriviCoreAdapter.cpp`), which already depends on nlohmann/json transitively through ScriviCore. The adapter may include nlohmann/json directly for result serialization.

No changes to `ScriviCore.cpp`, `ProjectCreator.cpp`, `ProjectOpener.cpp`, `SceneWriter.cpp`, or any schema file are required for this boundary redesign.

---

## 7. Impact on the Adapter Layer

`ScriviCoreAdapter.hpp` changes:

- Remove all `Out` structs (`EnsureIdentityOut`, `CreateProjectOut`, `OpenProjectOut`, `SaveSceneOut`, `AdapterError`).
- Remove `getLastOpenMarkdown()`.
- All four public methods return `std::string` by value.
- The `lastOpenMarkdown` member of `Impl` is removed (no longer needed separately).

`ScriviCoreAdapter.cpp` changes:

- Each method serializes its result (or error) to a JSON string and returns it.
- `nlohmann/json` is used directly in the adapter for this serialization.

`ScriviEngine.swift` changes:

- Remove `string(from:)` helper (no longer needed for fixed char arrays).
- Each method receives a `std::string`, converts to Swift `String`, decodes `Codable` struct.
- `ScriviError` is thrown from the decoded envelope's error field.

---

## 8. Approved Direction

**Boundary design:** JSON-over-`std::string`. The `ScriviAdapter` C++ shim serializes all results to `std::string` JSON. Swift receives by value and deserializes with `Codable`.

**Request passing:** Individual `const char*` parameters (existing approach). This is simpler than a JSON request string for the current four operations, and avoids a second layer of request serialization. This may be revisited if operation count grows substantially.

**Error model:** JSON envelope with `ok` bool and nested `error` object. Swift throws `ScriviError(code:message:)` on `ok == false`.

**Schema alignment:** API result JSON reuses field names from on-disk schemas. No new JSON vocabularies invented at the boundary.

This direction supersedes Section 27 of `Scrivi_Cpp24_Core_API_Sketch_v0_2.md`.

---

## 9. What Was Proved by T-0011 Attempts

Despite the direction change, T-0011 proved:

1. The `ScriviAdapter` reference-type pattern (`import_reference` + retain/release) works.
2. `const char*` parameters from Swift `withCString` work for all input parameters.
3. SPM can compile the C++ adapter and link `libScriviCore.a` against it.
4. The Swift Testing framework tests for the full pipeline (`ensureLocalIdentity` → `createProject` → `openProject` → `saveScene`) are structurally correct.
5. `ScriviCore` builds cleanly as a static library for macOS.

The tests and the adapter structure are preserved. Only the return-value representation changes.

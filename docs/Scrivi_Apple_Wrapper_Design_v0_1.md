# Scrivi Apple Wrapper Design v0.1

**Project:** Scrivi  
**Document:** Apple Platform Wrapper Design  
**Version:** 0.1  
**Status:** Approved baseline  
**Depends on:** `Scrivi_Swift_Interop_Trade_Study_v0_1.md` (boundary design rationale)

---

## 1. Purpose

This document defines the approved architecture for the Apple platform wrapper around ScriviCore. It covers:

- The layer structure between ScriviCore and Swift.
- The C++ adapter shim design (JSON-over-`std::string` boundary).
- The Swift engine design (decoding, error model, service wiring).
- The JSON envelope and schema contracts.
- The SPM package structure.

This document supersedes the Swift/C++ interop section (§27) of `Scrivi_Cpp24_Core_API_Sketch_v0_2.md`.

---

## 2. Layer Structure

```
┌──────────────────────────────────────┐
│        SwiftUI (future)              │
│  Platform UI — not in this doc       │
└──────────────┬───────────────────────┘
               │
┌──────────────▼───────────────────────┐
│        ScriviEngine.swift            │
│  Swift entry point                   │
│  • Converts Swift types → const char*│
│  • Decodes JSON result strings       │
│  • Throws ScriviError on ok=false    │
└──────────────┬───────────────────────┘
               │  std::string (JSON, by value)
┌──────────────▼───────────────────────┐
│     ScriviCoreAdapter (C++)          │
│  Thin shim — no logic                │
│  • Accepts const char* parameters    │
│  • Delegates to ScriviCore           │
│  • Serializes result to JSON string  │
│  • Returns std::string by value      │
└──────────────┬───────────────────────┘
               │  C++ public API
┌──────────────▼───────────────────────┐
│         ScriviCore (C++)             │
│  Static library — unchanged          │
│  • All project logic                 │
│  • nlohmann/json schema I/O          │
└──────────────────────────────────────┘
```

The boundary between the adapter and the Swift engine is: `std::string` by value carrying JSON. No raw pointers cross the boundary in either direction as return values.

---

## 3. ScriviCoreAdapter (C++)

### 3.1 Purpose

`ScriviCoreAdapter` is a thin C++ shim with two responsibilities:

1. Accept `const char*` parameters and convert them to ScriviCore request types.
2. Serialize ScriviCore results (or errors) to `std::string` JSON and return by value.

No logic lives in the adapter. No ScriviCore types are exposed to Swift.

### 3.2 Class declaration

```cpp
// ScriviCoreAdapter.hpp

#pragma once
#include <atomic>
#include <memory>
#include <string>

namespace scrivi::apple {
    class ScriviAdapter;
}

void scriviAdapterRetain(scrivi::apple::ScriviAdapter* p) noexcept;
void scriviAdapterRelease(scrivi::apple::ScriviAdapter* p) noexcept;

namespace scrivi::apple {

class __attribute__((swift_attr("import_reference")))
      __attribute__((swift_attr("retain:scriviAdapterRetain")))
      __attribute__((swift_attr("release:scriviAdapterRelease")))
ScriviAdapter {
public:
    static ScriviAdapter* create();

    void retain()  noexcept;
    void release() noexcept;

    std::string ensureLocalIdentity(
        const char* requestedDisplayName,
        const char* appSupportRoot);

    std::string createProject(
        const char* projectRootPath,
        const char* appSupportRoot,
        const char* title,
        const char* slug,
        const char* identityID,
        const char* personaID,
        const char* displayName);

    std::string openProject(
        const char* projectRootPath,
        const char* appSupportRoot,
        const char* identityID);

    std::string saveScene(
        const char* projectID,
        const char* projectRootPath,
        const char* appSupportRoot,
        const char* sceneID,
        const char* sceneMetadataPath,
        const char* sceneContentPath,
        const char* markdown,
        const char* identityID,
        const char* personaID,
        const char* displayName);

private:
    ScriviAdapter();
    ~ScriviAdapter();

    ScriviAdapter(const ScriviAdapter&) = delete;
    ScriviAdapter& operator=(const ScriviAdapter&) = delete;

    std::atomic<int> refCount_{1};
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace scrivi::apple
```

### 3.3 Impl struct (in .cpp, not exposed to Swift)

```cpp
struct ScriviAdapter::Impl {
    scrivi::platform::LocalFileSystem    fileSystem;
    scrivi::platform::SystemUUIDProvider uuidProvider;
    PrototypeClock                       clock;
    PrototypeSecureStore                 secureStore;
    std::unique_ptr<scrivi::ScriviCore>  core;
};
```

`PrototypeClock` and `PrototypeSecureStore` are defined in the `.cpp` file only, not in the header. They implement the `scrivi::Clock` and `scrivi::SecureStore` interfaces respectively. `PrototypeSecureStore` uses an in-memory `std::unordered_map` for MVP; the production Apple implementation will use the system Keychain.

### 3.4 Return format

All four methods return `std::string`. On success:

```json
{ "ok": true, "result": { ... } }
```

On failure:

```json
{ "ok": false, "error": { "code": 7, "message": "project.json not found" } }
```

The `code` values correspond to `scrivi::ErrorCode` enum values cast to `int`.

### 3.5 Serialization

The adapter includes `nlohmann/json` directly for result serialization. `nlohmann/json` is already a dependency of ScriviCore (linked in via the static library) and is available transitively, but the adapter includes it explicitly for clarity.

---

## 4. JSON Envelope Schemas

### 4.1 `ensureLocalIdentity`

**Request parameters (individual `const char*`):**
- `requestedDisplayName` — user-supplied display name string
- `appSupportRoot` — absolute path to app support directory

**Result JSON:**

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

Swift type:

```swift
public struct IdentityResult: Decodable, Sendable {
    public let identityID:         String
    public let defaultPersonaID:   String
    public let displayName:        String
    public let createdNewIdentity: Bool
}
```

---

### 4.2 `createProject`

**Request parameters (individual `const char*`):**
- `projectRootPath`, `appSupportRoot`, `title`, `slug`
- `identityID`, `personaID`, `displayName` — authorship ref

**Result JSON:**

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

Swift types:

```swift
public struct CreateProjectResult: Decodable, Sendable {
    public let projectID:  String
    public let firstScene: SceneRef
}

public struct SceneRef: Decodable, Sendable {
    public let sceneID:       String
    public let metadataPath:  String
    public let contentPath:   String
}
```

---

### 4.3 `openProject`

**Request parameters (individual `const char*`):**
- `projectRootPath`, `appSupportRoot`, `identityID`

**Result JSON:**

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

Swift types:

```swift
public struct OpenProjectResult: Decodable, Sendable {
    public let projectID:    String
    public let activeScene:  ActiveSceneResult
}

public struct ActiveSceneResult: Decodable, Sendable {
    public let sceneID:       String
    public let metadataPath:  String
    public let contentPath:   String
    public let markdown:      String
}
```

---

### 4.4 `saveScene`

**Request parameters (individual `const char*`):**
- `projectID`, `projectRootPath`, `appSupportRoot`
- `sceneID`, `sceneMetadataPath`, `sceneContentPath`, `markdown`
- `identityID`, `personaID`, `displayName` — authorship ref

**Result JSON:**

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

Swift type:

```swift
public struct SaveSceneResult: Decodable, Sendable {
    public let sceneID:    String
    public let saved:      Bool
    public let wordCount:  Int
}
```

---

## 5. ScriviEngine (Swift)

### 5.1 Purpose

`ScriviEngine` is the Swift-side entry point. It:

- Holds the `ScriviAdapter` reference type (ARC-managed via retain/release hooks).
- Converts `String` parameters to `const char*` via `withCString`.
- Calls the adapter method, receives a `std::string` result by value.
- Converts to Swift `String` via `String(cxxString)`.
- Decodes the JSON envelope; throws `ScriviError` on `ok == false`.
- Returns the decoded result struct to callers.

### 5.2 Envelope decoding

A private generic helper decodes the envelope:

```swift
private struct Envelope<T: Decodable>: Decodable {
    let ok:     Bool
    let result: T?
    let error:  ErrorPayload?
}

private struct ErrorPayload: Decodable {
    let code:    Int
    let message: String
}

private func decode<T: Decodable>(_ cxxString: std.string) throws -> T {
    let json = String(cxxString)
    let data = Data(json.utf8)
    let envelope = try JSONDecoder().decode(Envelope<T>.self, from: data)
    if !envelope.ok {
        let e = envelope.error ?? ErrorPayload(code: -1, message: "unknown error")
        throw ScriviError(code: e.code, message: e.message)
    }
    guard let result = envelope.result else {
        throw ScriviError(code: -1, message: "ok=true but result missing")
    }
    return result
}
```

### 5.3 Public interface (no change to callers)

The four public methods on `ScriviEngine` remain:

```swift
public func ensureLocalIdentity(displayName: String, appSupportRoot: String) throws -> IdentityResult
public func createProject(projectRootPath: String, appSupportRoot: String, title: String, slug: String, authorshipRef: AuthorshipRef) throws -> CreateProjectResult
public func openProject(projectRootPath: String, appSupportRoot: String, identityID: String) throws -> OpenProjectResult
public func saveScene(projectID: String, projectRootPath: String, appSupportRoot: String, sceneID: String, sceneMetadataPath: String, sceneContentPath: String, markdown: String, authorshipRef: AuthorshipRef) throws -> SaveSceneResult
```

The `AuthorshipRef` helper struct and `ScriviError` type are unchanged.

`OpenProjectResult` is updated to use `ActiveSceneResult` instead of flat `firstScene*` fields, and `CreateProjectResult` uses a nested `SceneRef` instead of flat fields.

---

## 6. SPM Package Structure

The SPM package at `platforms/apple/` has three targets:

### 6.1 `ScriviCoreAdapter` — C++ target

- Sources: `Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp`
- Public headers: `Sources/ScriviCoreAdapter/` (includes `ScriviCoreAdapter.hpp` and `module.modulemap`)
- CXX flags: `-std=c++2b`, `-I <ScriviCore includes>`, `-I <ScriviCore src>`
- Linker: links `libScriviCore.a`

### 6.2 `Scrivi` — Swift target

- Sources: `Sources/Scrivi/ScriviEngine.swift`
- Dependencies: `ScriviCoreAdapter`
- Swift settings: `.interoperabilityMode(.Cxx)`, `-Xcc -I<includes>` for both ScriviCore include dirs

### 6.3 `ScriviInteropTests` — test target

- Sources: `Tests/ScriviInteropTests/ScriviInteropTests.swift`
- Dependencies: `Scrivi`
- Framework: Swift Testing (`import Testing`)
- Swift settings: same as `Scrivi` target

No changes to `Package.swift` are required from the current implementation — the target structure was already correct.

---

## 7. Service Wiring

For MVP (T-0011), all services are wired inside `ScriviAdapter::Impl`:

| Service | MVP implementation |
|---|---|
| `FileSystem` | `LocalFileSystem` (production, real disk I/O) |
| `UUIDProvider` | `SystemUUIDProvider` (production, real UUIDs) |
| `Clock` | `PrototypeClock` (real UTC via `std::chrono`) |
| `SecureStore` | `PrototypeSecureStore` (in-memory `unordered_map`) |
| `GitProvider` | `nullptr` (Git not in T-0011 scope) |
| `Logger` | `nullptr` |

The production Apple implementation will replace `PrototypeSecureStore` with a `KeychainSecureStore` that calls the macOS/iOS Keychain API. This replacement is out of scope for SP-003 and will be addressed in a later sprint.

---

## 8. Removed from Adapter Header

The following are removed compared to the Option B design:

- `AdapterError` struct
- `EnsureIdentityOut` struct
- `CreateProjectOut` struct
- `OpenProjectOut` struct
- `SaveSceneOut` struct
- `getLastOpenMarkdown() const char*`
- `lastOpenMarkdown` field in `Impl`

The `string(from:)` fixed-array helper in `ScriviEngine.swift` is also removed.

---

## 9. Constraints and Non-Goals

- The adapter does **not** own any project state between calls. Each call is stateless from the adapter's perspective (ScriviCore manages all state).
- The adapter does **not** validate JSON. It serializes correct results; parsing errors are a bug, not a runtime condition.
- The adapter does **not** expose nlohmann/json types to Swift.
- The adapter does **not** buffer Markdown or manage file handles.
- Swift does **not** implement any backend logic. ScriviEngine is pure type conversion and decoding.

---

## 10. Acceptance Criteria for T-0011

The interop prototype (T-0011) is complete when:

1. `swift test` passes all five tests in `ScriviInteropTests`.
2. `ensureLocalIdentity` returns decoded `IdentityResult` with valid `identity_` / `persona_` prefixed IDs.
3. `createProject` returns decoded `CreateProjectResult` with non-empty projectID and `firstScene` fields.
4. `openProject` returns decoded `OpenProjectResult` with `activeScene.markdown` containing scene text.
5. `saveScene` returns `saved == true` and `wordCount > 0`, and reopening the project returns the saved markdown.
6. `openProject` on a nonexistent path throws `ScriviError`.

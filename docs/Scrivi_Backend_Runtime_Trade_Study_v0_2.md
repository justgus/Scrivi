# Scrivi Backend Runtime Trade Study v0.2

**Project:** Scrivi  
**Document:** Backend Runtime Trade Study  
**Version:** 0.2  
**Status:** Approved Direction: C++24 Shared Backend Core  
**Date:** 2026-05-18

---

## 1. Purpose

This document updates the Scrivi backend runtime trade study based on the project lead's implementation context and decision.

The prior trade study identified Swift-first MVP with Rust as a possible long-term shared-core candidate. That recommendation is superseded by the following project fact:

> The project lead is fluent in C++24 and does not know Rust.

Because Scrivi is intended to be a long-lived, multi-platform writing system, backend maintainability by the project lead is more important than an abstract language preference.

---

## 2. Decision

Scrivi will use:

```text
C++24 shared backend core from the start
```

with thin platform wrappers and native UI shells.

This means:

```text
C++24 core: shared backend behavior
Swift wrapper: macOS / iPadOS / iPhone / visionOS
Windows/Linux wrapper: desktop shells
Android wrapper: JNI/NDK boundary
```

The UI remains platform-specific or platform-appropriate. The backend core must not own UI presentation.

---

## 3. Design Context

This decision is evaluated against the approved **Scrivi Backend MVP Slice v0.1**, whose first backend loop is:

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

The backend core must support the approved Scrivi project model:

```text
.scrivi package/folder
Markdown manuscript files
visible JSON metadata files
native asset files
optional Git-backed snapshots
app-local rebuildable caches
certificate-ready identity metadata
future-compatible hooks for signing/encryption/group personas
```

---

## 4. Scope of Runtime Decision

This trade study selects the runtime for the **shared backend core**, not the entire app.

The backend core owns:

```text
project package creation
schema read/write
Markdown file persistence
metadata synchronization
workspace state logic
external change scanning
repair classification
Git snapshot abstraction
identity/persona project metadata
app-local cache/index policy
```

The backend core does not own:

```text
SwiftUI/AppKit/UIKit/visionOS presentation
Windows UI
Linux UI
Android UI
rich text editor presentation
project visual design
native menus/toolbars/windows
platform navigation patterns
```

---

## 5. Evaluation Criteria

| Criterion | Meaning |
|---|---|
| Cross-platform reach | Can the backend run across Apple, Windows, Linux, and Android targets? |
| Project lead fluency | Can the project lead personally build, debug, and maintain it? |
| Apple integration | Can it be wrapped cleanly for Swift UI shells? |
| Windows/Linux integration | Can it serve desktop implementations cleanly? |
| Android integration | Can it be called through JNI/NDK or equivalent wrapper? |
| File I/O quality | Strong filesystem support, package/folder manipulation, atomic writes |
| JSON support | Mature JSON parsing/encoding available |
| Markdown/text handling | Good Unicode/text handling; Markdown parser available if needed |
| Git integration | Can invoke or wrap Git behavior? |
| Secure storage integration | Can interact with platform key stores through adapters? |
| API boundary | Can expose coarse operations to UI shells? |
| Safety/reliability | Can be written robustly with clear ownership/error handling? |
| Complexity | Build, packaging, binding, and dependency complexity |
| Long-term maintainability | Fit for future signing, encryption, indexing, and collaboration |
| Minimum code | Can it implement the MVP without speculative infrastructure? |

---

## 6. Candidate Runtime Approaches

This updated study considers:

```text
A. Swift-first backend
B. Rust shared core
C. C++24 shared core
D. Kotlin Multiplatform core
E. TypeScript/JavaScript core
F. Dart/Flutter-oriented core
G. Platform-specific backends following shared specs
H. Swift Apple backend + C++ non-Apple backend
```

---

# 7. Option A — Swift-First Backend

## Description

Implement the backend initially in Swift, optimized for Apple platforms.

## Strengths

```text
Excellent Apple integration
Good Foundation file APIs
Strong Codable/JSON support
Natural Keychain integration
Fast path to macOS/iPadOS/iPhone/visionOS MVP
High productivity for Apple UI integration
```

## Weaknesses

```text
Does not solve Windows/Linux/Android backend sharing
Risks Apple-first backend assumptions
Would likely require later shared-core migration
Could duplicate behavior on non-Apple platforms
```

## Fit

Good if Scrivi were Apple-first.

Less appropriate now that the decision is to build the shared backend core from the start.

## Status

Not selected.

---

# 8. Option B — Rust Shared Core

## Description

Implement the shared backend core in Rust and wrap it for each platform.

## Strengths

```text
Strong cross-platform systems language
Memory safety without garbage collection
Excellent error-handling culture
Good JSON/filesystem/crypto ecosystem
Good fit for future signing/encryption/indexing
```

## Weaknesses

```text
Project lead does not know Rust
FFI and build complexity remain
Would slow early implementation
Would require learning curve before core behavior is validated
```

## Fit

Rust remains technically attractive, but project lead fluency changes the trade.

## Status

Not selected.

## Reason

Scrivi should not base its core on a language the project lead cannot currently maintain fluently.

---

# 9. Option C — C++24 Shared Core

## Description

Implement the Scrivi backend core in modern C++24 from the start.

The C++ core exposes coarse operations to native UI shells through thin platform wrappers.

## Strengths

```text
Project lead is fluent in C++24
Strong portability across Apple, Windows, Linux, and Android
Mature systems programming ecosystem
Good fit for filesystem-heavy backend work
Excellent control over package layout, file I/O, and atomic writes
Can expose C ABI or platform-specific wrappers
Can support future indexing, signing, encryption, and Git integration
Avoids future Swift-to-shared-core rewrite
```

## Weaknesses

```text
More initial wrapper work than pure Swift
Requires disciplined ownership/error handling
Requires careful dependency selection
Secure storage still needs platform-specific adapters
Apple integration is not as effortless as pure Swift
Android requires JNI/NDK boundary
```

## Fit

Strongest overall fit given:

```text
multi-platform product requirement
project lead C++24 fluency
need for platform-neutral backend
desire to avoid speculative rewrites
```

## Status

Selected.

---

# 10. Option D — Kotlin Multiplatform Core

## Description

Use Kotlin Multiplatform for shared backend logic.

## Strengths

```text
Excellent Android fit
Good shared app logic model
Good JSON support
Good productivity
Desktop JVM path available
```

## Weaknesses

```text
Project lead fluency not established
Apple/visionOS native integration less direct
Backend may drift toward Kotlin UI architecture
Git/secure storage still require platform adapters
```

## Fit

Could be viable for Android-heavy strategy, but not the best match for the current team/skill context.

## Status

Not selected.

---

# 11. Option E — TypeScript/JavaScript Core

## Description

Implement backend logic in TypeScript/JavaScript, likely paired with Electron, Tauri, or another web-adjacent shell.

## Strengths

```text
Excellent JSON and Markdown ecosystem
High productivity
Good desktop prototyping path
Easy schema iteration
```

## Weaknesses

```text
Risks moving Scrivi toward web-stack architecture
Less natural for premium Apple/visionOS native shells
Runtime packaging concerns
Secure storage and filesystem behavior depend on host shell
Not ideal as the long-term native shared backend core
```

## Fit

Good for prototypes or tools.

Not selected for Scrivi's main backend core.

---

# 12. Option F — Dart / Flutter-Oriented Core

## Description

Use Dart as shared app/backend language, typically with Flutter UI.

## Strengths

```text
Broad cross-platform UI story
Good mobile/desktop coverage
Single language for UI and much app logic
Good JSON support
```

## Weaknesses

```text
Backend becomes coupled to Flutter-oriented app architecture
visionOS/native Apple polish may be weaker
Git and secure storage depend on plugins/platform channels
Dart is less compelling as a backend-only shared systems core
```

## Fit

Viable if Scrivi chooses Flutter for the entire app.

Not selected because the approved direction is native/platform-appropriate UI shells with a shared backend core.

---

# 13. Option G — Platform-Specific Backends Following Shared Specs

## Description

Implement separate backends per platform using the shared documents as compatibility specs.

## Strengths

```text
Best native integration per platform
No FFI boundary
Each platform can use its own secure storage APIs directly
```

## Weaknesses

```text
Duplicated behavior
High risk of schema drift
More tests required
More maintenance
Harder for a small team
Repair behavior may diverge
Git behavior may diverge
```

## Fit

Not appropriate as the main strategy.

## Status

Rejected as primary strategy.

---

# 14. Option H — Swift Apple Backend + C++ Non-Apple Backend

## Description

Use Swift backend on Apple platforms and C++ backend elsewhere.

## Strengths

```text
Fast Apple integration
C++ path for Windows/Linux/Android
Avoids some Swift/C++ wrapper complexity on Apple at first
```

## Weaknesses

```text
Two backend implementations
Behavior drift risk
Duplicate test burden
Different repair/Git behavior across platforms
Harder to guarantee project compatibility
```

## Fit

Possible fallback if wrapper complexity becomes unacceptable, but not the preferred architecture.

## Status

Not selected.

---

## 15. Updated Scoring Matrix

Scores are 1–5, where 5 is strongest.

| Criterion | Swift | Rust | C++24 | Kotlin MP | TypeScript | Dart/Flutter | Per-platform | Swift+C++ Split |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Project lead fluency | 4 | 1 | 5 | 2 | 3 | 2 | 3 | 4 |
| Apple integration | 5 | 3 | 4 | 2 | 2 | 2 | 5 | 5 |
| Windows/Linux viability | 2 | 5 | 5 | 4 | 4 | 4 | 5 | 5 |
| Android viability | 2 | 4 | 4 | 5 | 4 | 5 | 5 | 4 |
| visionOS fit | 5 | 3 | 4 | 2 | 1 | 2 | 5 | 5 |
| File/backend suitability | 4 | 5 | 5 | 4 | 4 | 3 | 4 | 5 |
| JSON/schema support | 5 | 5 | 4 | 5 | 5 | 5 | 4 | 4 |
| Git integration potential | 3 | 4 | 4 | 3 | 4 | 3 | 3 | 4 |
| Secure storage path | 5 | 3 | 3 | 3 | 2 | 3 | 5 | 4 |
| Wrapper/binding simplicity | 5 | 2 | 3 | 3 | 3 | 3 | 5 | 4 |
| MVP productivity | 5 | 2 | 5 | 3 | 4 | 3 | 2 | 4 |
| Long-term shared core | 2 | 5 | 5 | 4 | 3 | 3 | 2 | 3 |
| Minimum code for MVP | 5 | 2 | 4 | 3 | 4 | 3 | 2 | 3 |
| Risk of platform lock-in | 2 | 5 | 5 | 4 | 3 | 3 | 5 | 3 |
| Overall MVP fit | 4 | 3 | 5 | 3 | 3 | 3 | 2 | 4 |
| Overall long-term fit | 3 | 4 | 5 | 4 | 3 | 3 | 2 | 3 |

---

## 16. Selected Runtime Direction

Scrivi will use:

```text
C++24 shared backend core from the start
```

with:

```text
thin Swift wrapper for Apple platforms
thin native wrappers for Windows/Linux
JNI/NDK wrapper for Android
native or platform-appropriate UI shells
```

This decision is approved.

---

## 17. Required Guardrails

The C++24 backend must follow strict guardrails.

---

### 17.1 Coarse backend API boundary

The C++ core should expose coarse operations.

Good:

```text
createProject()
openProject()
saveScene()
scanForExternalChanges()
enableGitSnapshots()
createSnapshot()
```

Bad:

```text
hundreds of tiny UI-driven getters/setters across the wrapper boundary
```

---

### 17.2 No UI types in backend

The C++ core must not know about:

```text
SwiftUI
AppKit
UIKit
RealityKit
WinUI
Qt widgets
GTK widgets
Android Views
Jetpack Compose
```

The backend returns data structures and repair states. The UI decides presentation.

---

### 17.3 Platform services are injected

Platform-specific services should be abstracted behind interfaces.

Examples:

```text
SecureStore
FileSystem
Clock
GitProvider
UUIDProvider
Logger
```

The core should not hardcode Keychain, Windows Credential Manager, Android Keystore, or Linux keyrings.

---

### 17.4 Canonical formats stay boring

The MVP must keep canonical formats simple:

```text
Markdown
JSON
native asset files
```

No custom binary project format.

No platform-only canonical storage.

No SwiftData/Core Data/SQLite as canonical source.

---

### 17.5 No speculative object framework

The first C++ core should prove the writing loop only.

Do not implement:

```text
complete character model
timeline engine
relationship graph
query language
plugin system
custom object database
real-time collaboration
```

---

### 17.6 Test fixtures are platform-neutral

A `.scrivi` project fixture created by one platform must be readable by the core on another platform.

Fixtures should live outside platform UI tests and be usable by command-line tests.

---

### 17.7 Git is abstracted

The core should hide the Git implementation.

Possible providers:

```text
SystemGitProvider
LibGitProvider
MockGitProvider
NoGitProvider
```

MVP can start with system Git if that is the smallest useful implementation, but the API must not leak that decision.

---

### 17.8 Secure storage does not leak into project format

Private keys stay outside `.scrivi` packages.

Project files may contain:

```text
identity IDs
public keys
persona IDs
display names
roles
```

They must not contain private signing/encryption keys.

---

### 17.9 Error model must be explicit

The C++ core must not rely on ambiguous exceptions across the wrapper boundary.

Errors should map to structured backend results.

Example categories:

```text
ok
validationWarning
repairRequired
unsupportedVersion
ioError
parseError
permissionDenied
gitError
internalError
```

---

### 17.10 Memory ownership must be wrapper-safe

The public API boundary must avoid unclear ownership.

Preferred:

```text
C++ facade API for native C++ callers
C ABI or generated binding layer for Swift/Android/etc.
serialized JSON result payloads where appropriate
opaque handles only where necessary
```

---

## 18. Architectural Shape

Recommended layering:

```text
ScriviCore/
  PublicAPI/
  Domain/
  Schemas/
  ProjectPackage/
  Manuscript/
  Identity/
  Workspace/
  Repair/
  Git/
  Platform/
  Tests/
```

Wrappers:

```text
ScriviApple/
  Swift wrapper around ScriviCore

ScriviAndroid/
  JNI/NDK wrapper around ScriviCore

ScriviDesktop/
  wrapper/adapters for Windows/Linux shells
```

---

## 19. MVP Runtime Acceptance Criteria

The runtime decision is successful if:

1. The C++24 core can create the minimum Scrivi project package.
2. The same core can open that project on another supported platform.
3. The core can save Markdown and update metadata.
4. The core can restore workspace state through app-local paths.
5. The core can classify simple external changes.
6. The core can support optional Git snapshots through an abstraction.
7. The Apple UI can call the core through a thin Swift wrapper.
8. The C++ core contains no UI framework dependencies.
9. The project package remains platform-neutral.
10. Future Windows/Linux/Android implementations do not require rewriting backend behavior.

---

## 20. Implementation Consequences

Choosing C++24 from the start means the next implementation design must address:

```text
build system
dependency management
JSON library
UUID v7 generation
filesystem abstraction
atomic writes
error/result model
C++ public API
Swift binding strategy
Git provider strategy
secure storage abstraction
test fixture strategy
continuous integration targets
```

These belong in the next document:

```text
Scrivi C++24 Backend Core Plan v0.1
```

---

## 21. Final Decision Statement

> Scrivi will implement its backend MVP as a C++24 shared backend core from the start. The core will expose coarse project operations to platform-specific UI shells through thin wrappers. The project package format remains platform-neutral, text-based, Git-friendly, and independent of any UI framework. Platform-specific services such as secure storage and Git execution will be abstracted and injected. The initial implementation will prove only the approved MVP writing loop and will not implement speculative future systems.

# Task - Index

This is the main index for all Scrivi Tasks. Tasks track planned improvements, new features, and requirement changes to the system.

> **Note:** For bugs and unintended behavior, see [Issues (I)](../Issues/Issue-Documentation.md)
> **Related:** [Epics](../Epics/Epic-Documentation.md) | [Sprints](../Sprints/Sprint-Documentation.md)

## Organization

- **Task-backlog.md** — Proposed Tasks not assigned to a Sprint
- **Task-active.md** — In-Progress Tasks assigned to a Sprint
- **Task-unverified.md** — Implemented Tasks awaiting user verification
- **Verified/Task-verified-XXXX.md** — Verified Tasks (one file per Task)

## Backlog Tasks

Currently: **2 backlog Tasks**

| Task   | Title | Epic | Priority | Status |
| ------ | ----- | ---- | -------- | ------ |
| T-0046 | Inbox — `listInbox` / `importFromInbox` Facade Methods | EP-005 | Medium | 🔵 Backlog |
| T-0047 | `ScriviCoreAdapter` — Expose All EP-005 Facade Methods | EP-005 | High | 🔵 Backlog |

See: [Task-backlog.md](Task-backlog.md)

## Active Tasks

Currently: **0 active Tasks**

| Task | Title | Status |
| ---- | ----- | ------ |
| —    | None  | —      |

See: [Task-active.md](Task-active.md)

## Unverified Tasks (Implemented, Awaiting Verification)

Currently: **0 unverified Tasks**

| Task | Title | Status |
| ---- | ----- | ------ |
| —    | None  | —      |

See: [Task-unverified.md](Task-unverified.md)

## Verified Tasks

Currently: **36 verified Tasks** | Next available: **T-0048**

| Task   | Title | File | Status |
| ------ | ----- | ---- | ------ |
| T-0001 | Repository Skeleton | [Task-verified-0001.md](Verified/Task-verified-0001.md) | ✅ Verified |
| T-0002 | Core Value Types | [Task-verified-0002.md](Verified/Task-verified-0002.md) | ✅ Verified |
| T-0003 | Utility Foundation | [Task-verified-0003.md](Verified/Task-verified-0003.md) | ✅ Verified |
| T-0004 | Mock Services | [Task-verified-0004.md](Verified/Task-verified-0004.md) | ✅ Verified |
| T-0005 | Schema Read/Write | [Task-verified-0005.md](Verified/Task-verified-0005.md) | ✅ Verified |
| T-0006 | Project Creation | [Task-verified-0006.md](Verified/Task-verified-0006.md) | ✅ Verified |
| T-0007 | Open/Resume | [Task-verified-0007.md](Verified/Task-verified-0007.md) | ✅ Verified |
| T-0008 | Save Scene | [Task-verified-0008.md](Verified/Task-verified-0008.md) | ✅ Verified |
| T-0009 | External Change Scan | [Task-verified-0009.md](Verified/Task-verified-0009.md) | ✅ Verified |
| T-0010 | Git Snapshots | [Task-verified-0010.md](Verified/Task-verified-0010.md) | ✅ Verified |
| T-0011 | Swift Interop Prototype | [Task-verified-0011.md](Verified/Task-verified-0011.md) | ✅ Verified |
| T-0012 | Identity Service and UUID Provider | [Task-verified-0012.md](Verified/Task-verified-0012.md) | ✅ Verified |
| T-0013 | appSupportRoot Directory Bootstrap | [Task-verified-0013.md](Verified/Task-verified-0013.md) | ✅ Verified |
| T-0014 | Introduce `src/domain/` and Relocate Slug and TextStats | [Task-verified-0014.md](Verified/Task-verified-0014.md) | ✅ Verified |
| T-0015 | Move `AppSupportLayout` from `src/util/` to `src/platform/` | [Task-verified-0015.md](Verified/Task-verified-0015.md) | ✅ Verified |
| T-0016 | Merge `MockServicesTests.cpp` into `ResultTests.cpp` | [Task-verified-0016.md](Verified/Task-verified-0016.md) | ✅ Verified |
| T-0017 | Merge `SchemaTests.cpp` into `JsonSchemaTests.cpp` | [Task-verified-0017.md](Verified/Task-verified-0017.md) | ✅ Verified |
| T-0018 | First Green Build and Full Test Suite Verification | [Task-verified-0018.md](Verified/Task-verified-0018.md) | ✅ Verified |
| T-0019 | Add `SnapshotMetadataJson` Schema | [Task-verified-0019.md](Verified/Task-verified-0019.md) | ✅ Verified |
| T-0020 | Add `SnapshotMetadataJson` Test Coverage | [Task-verified-0020.md](Verified/Task-verified-0020.md) | ✅ Verified |
| T-0021 | Milestone 10 Verification — External Change Scan | [Task-verified-0021.md](Verified/Task-verified-0021.md) | ✅ Verified |
| T-0022 | Milestone 11 Verification — Git Snapshots | [Task-verified-0022.md](Verified/Task-verified-0022.md) | ✅ Verified |
| T-0023 | Extract `ScriviError.swift` as a Separate File | [Task-verified-0023.md](Verified/Task-verified-0023.md) | ✅ Verified |
| T-0024 | Add Adapter Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | [Task-verified-0024.md](Verified/Task-verified-0024.md) | ✅ Verified |
| T-0025 | Add Swift Engine Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | [Task-verified-0025.md](Verified/Task-verified-0025.md) | ✅ Verified |
| T-0026 | Add Swift Interop Tests for New Adapter Methods | [Task-verified-0026.md](Verified/Task-verified-0026.md) | ✅ Verified |
| T-0027 | Full MVP Loop Integration Test | [Task-verified-0027.md](Verified/Task-verified-0027.md) | ✅ Verified |
| T-0028 | `RepairIssueJson` Schema Module | [Task-verified-0028.md](Verified/Task-verified-0028.md) | ✅ Verified |
| T-0029 | `applyRepair` Facade Method — Request, Result, and Dispatch | [Task-verified-0029.md](Verified/Task-verified-0029.md) | ✅ Verified |
| T-0030 | Repair Handlers — Manuscript File Operations | [Task-verified-0030.md](Verified/Task-verified-0030.md) | ✅ Verified |
| T-0031 | Automatic Rename Detection in `RepairClassifier` | [Task-verified-0031.md](Verified/Task-verified-0031.md) | ✅ Verified |
| T-0032 | Integration Tests for `applyRepair` | [Task-verified-0032.md](Verified/Task-verified-0032.md) | ✅ Verified |
| T-0033 | Adapter — `applyRepair` + Full `RepairIssue` Serialization | [Task-verified-0033.md](Verified/Task-verified-0033.md) | ✅ Verified |
| T-0034 | Character Object Schema (`ObjectJson`) | [Task-verified-0034.md](Verified/Task-verified-0034.md) | ✅ Verified |
| T-0035 | `createObject` / `openObject` / `saveObject` / `deleteObject` Facade — Characters | [Task-verified-0035.md](Verified/Task-verified-0035.md) | ✅ Verified |
| T-0036 | Integration Tests for Character Object CRUD | [Task-verified-0036.md](Verified/Task-verified-0036.md) | ✅ Verified |
| T-0037 | Remaining Object Types — Location, Item, Rule, Timeline Schemas | [Task-verified-0037.md](Verified/Task-verified-0037.md) | ✅ Verified |
| T-0038 | Generalize Object CRUD Facade to All Object Types | [Task-verified-0038.md](Verified/Task-verified-0038.md) | ✅ Verified |
| T-0039 | Integration Tests for All Object Types CRUD | [Task-verified-0039.md](Verified/Task-verified-0039.md) | ✅ Verified |
| T-0040 | Asset Metadata Sidecar Schema (`AssetMetaJson`) | [Task-verified-0040.md](Verified/Task-verified-0040.md) | ✅ Verified |
| T-0041 | `importAsset` / `listAssets` / `removeAsset` Facade Methods | [Task-verified-0041.md](Verified/Task-verified-0041.md) | ✅ Verified |
| T-0042 | Integration Tests for Asset Operations | [Task-verified-0042.md](Verified/Task-verified-0042.md) | ✅ Verified |
| T-0043 | Comments Schema (`CommentJson`) and Comment Types | [Task-verified-0043.md](Verified/Task-verified-0043.md) | ✅ Verified |
| T-0044 | `addComment` / `listComments` / `resolveComment` Facade Methods | [Task-verified-0044.md](Verified/Task-verified-0044.md) | ✅ Verified |
| T-0045 | Integration Tests for Comment Operations | [Task-verified-0045.md](Verified/Task-verified-0045.md) | ✅ Verified |

## Statistics

- **Total Tasks:** 47
- **Verified:** 45 (96%) ✅
- **Unverified:** 0 (0%) 🟠
- **Active:** 0 (0%) 🟡
- **Backlog:** 2 (4%) 🔵

---

*Last Updated: 2026-05-28 (T-0040–T-0045 verified; SP-012 closed)*

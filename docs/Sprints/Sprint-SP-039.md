# SP-039: Timeline Persistence — ScriviCore Schema and API

**Status:** 🔵 Planning
**Epic:** EP-016 — Timeline Panel Full Implementation
**Goal:** All timeline data has a defined home on disk and every ScriviCore operation that reads and writes timeline data is functional and tested. The Swift layer can call every new API operation. No UI changes in this sprint.

---

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0139 | `TimelineMetaJson` schema — read/write `timeline.meta.json` | 🔵 Backlog |
| T-0140 | `SceneMetaJson` extension — `storyTime` block read/write | 🔵 Backlog |
| T-0141 | `StoryStructureJson` schema — read/write `story-structure.json` | 🔵 Backlog |
| T-0142 | `HistoricalEventJson` schema — read/write `historical-events/<id>.json` | 🔵 Backlog |
| T-0143 | `ExternalTimelineJson` schema — read/write imported timeline files | 🔵 Backlog |
| T-0144 | ScriviCore facade — timeline, story-time, and band-assignment operations | 🔵 Backlog |
| T-0145 | ScriviCore facade — story structure, historical events, imported timelines, export | 🔵 Backlog |
| T-0146 | C API + `ScriviEngine.swift` — expose all timeline operations | 🔵 Backlog |
| T-0147 | Integration tests — timeline persistence round-trip | 🔵 Backlog |
| T-0148 | `createProject` update — write `timeline.meta.json` on new project | 🔵 Backlog |

---

## Sprint Acceptance Criteria

- [ ] `timeline.meta.json` is written when a new project is created.
- [ ] Scene `scene.meta.json` `storyTime` block can be written and read back correctly (all fields: `offsetMs`, `offsetSource`, `inferenceHint`, `inferenceConfidence`, `storyStructure.bandID`, `storyStructure.assignedAt`).
- [ ] `story-structure.json` can be written and read back (`activeStructureID`, `bandLayout` with proportions, `customBands`).
- [ ] Historical event JSON files can be created, read, updated, and deleted in `objects/historical-events/`.
- [ ] External timeline JSON files can be imported (stored in `objects/imported-timelines/`), listed, and removed.
- [ ] `exportProjectTimeline()` produces a valid `scrivi.externalTimeline.v1` JSON string containing project scenes and historical events.
- [ ] All new C API functions are declared in `scrivi.h`, implemented in `scrivi_c_api.cpp`, and callable from `ScriviEngine.swift` with correct Swift result types.
- [ ] Integration tests cover round-trip for all new schema files.
- [ ] `ctest --test-dir build --output-on-failure` fully green — zero failures, no regressions.

---

## Dependency Order

Tasks must be done in this sequence. Each depends on the one before it.

```
T-0139  →  T-0140  →  T-0141  →  T-0142  →  T-0143
                                                  ↓
                              T-0148  ←  T-0144  ←  T-0145
                                                  ↓
                                              T-0146
                                                  ↓
                                              T-0147
```

T-0139 through T-0143 are the schema modules (pure data layer, no facade). T-0144 and T-0145 add facade methods to `ScriviCore` (depends on the schemas and new ID/request/result types). T-0148 modifies `createProject` (depends on T-0139). T-0146 wires the C API and Swift engine (depends on T-0144 and T-0145). T-0147 writes integration tests (depends on T-0146).

---

## Detailed Task Specifications

---

### T-0139: `TimelineMetaJson` Schema — Read/Write `timeline.meta.json`

**Status:** 🔵 Backlog
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-039
**Date Requested:** 2026-06-11

**Rationale:**
`timeline.meta.json` is the project's own timeline anchor. It establishes the epoch label used in all human-readable story-time display. It follows the same schema module pattern as `SceneMetaJson`, `ProjectJson`, etc.

**New files:**
- `ScriviCore/src/schemas/TimelineMetaJson.hpp`
- `ScriviCore/src/schemas/TimelineMetaJson.cpp`

**New ID type** (add to `IDs.hpp`):
```cpp
struct TimelineID { std::string value; };
```

**New struct** (in `TimelineMetaJson.hpp`):
```cpp
namespace scrivi::schemas {
struct TimelineMetaData {
    TimelineID          timelineID;
    ProjectID           projectID;
    ISO8601Timestamp    createdAt;
    std::string         epochLabel;   // default: "Story Open"
    std::string         notes;        // may be empty
};
std::string serializeTimelineMeta(const TimelineMetaData& data);
Result<TimelineMetaData> parseTimelineMeta(std::string_view json);
}
```

**Schema tag:** `"scrivi.timeline.v1"`

**Required fields on parse:** `schema`, `timelineID`, `projectID`, `createdAt`, `epochLabel`

**Serialization:** Follows `serializeSceneMeta` pattern exactly — build a `util::JsonDoc`, `doc.setString(...)` for each field, `doc.dump()`.

**CMakeLists.txt:** Add `TimelineMetaJson.cpp` to the `ScriviCore` target source list.

**Test coverage:** Unit test in `ScriviCore/tests/unit/JsonSchemaTests.cpp` (or a new `TimelineMetaJsonTests.cpp`): round-trip serialize → parse, verify all fields survive. Verify parse fails on wrong schema tag.

---

### T-0140: `SceneMetaJson` Extension — `storyTime` Block Read/Write

**Status:** 🔵 Backlog
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-039
**Date Requested:** 2026-06-11

**Rationale:**
`scene.meta.json` already exists. This task extends `SceneMetaData` with the `storyTime` block defined in the design spec. The block is optional on read (existing scenes without the block parse cleanly with defaults).

**Files modified:**
- `ScriviCore/src/schemas/SceneMetaJson.hpp` — extend `SceneMetaData`
- `ScriviCore/src/schemas/SceneMetaJson.cpp` — extend serialize/parse

**Extension to `SceneMetaData`:**
```cpp
struct SceneStoryTime {
    int64_t     offsetMs           = 0;
    std::string offsetSource       = "default";   // "default" | "manual" | "inferred"
    std::string inferenceHint;                     // empty = null in JSON
    double      inferenceConfidence = -1.0;        // -1.0 = null in JSON
    std::string bandID;                            // empty = null in JSON
    std::string bandAssignedAt;                    // empty = null in JSON
};

// Added to SceneMetaData:
SceneStoryTime storyTime;
```

**Serialize:** Add `storyTime` block to `serializeSceneMeta`. Use `doc.setSubDoc("storyTime", ...)`. Within the sub-doc: `setInt64` for `offsetMs` — note `JsonDoc` currently has `setInt` (int) not `setInt64`. **This task must add `setInt64` / `getInt64` to `JsonDoc`** (or use `setDouble` and cast — prefer `setInt64` for precision. Add `void setInt64(std::string_view key, int64_t value)` and `int64_t getInt64(std::string_view, int64_t defaultValue = 0)` to `Json.hpp` / `Json.cpp`).

**Parse:** On `parseSceneMeta`, read the `storyTime` sub-doc if present. If absent, leave `SceneStoryTime` at defaults (offsetSource = "default", offsetMs = 0). This preserves backward compatibility with existing project files.

**Existing tests:** All existing `SceneMetaJson` round-trip tests must continue to pass with no changes to their expected output (the `storyTime` block is additive).

---

### T-0141: `StoryStructureJson` Schema — Read/Write `story-structure.json`

**Status:** 🔵 Backlog
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-039
**Date Requested:** 2026-06-11

**Rationale:**
`story-structure.json` stores the active Story Structure for the project: which built-in structure is selected, and the band layout (each band's ID, label, color, and proportion). It may be absent if no structure has been applied.

**New files:**
- `ScriviCore/src/schemas/StoryStructureJson.hpp`
- `ScriviCore/src/schemas/StoryStructureJson.cpp`

**Structs:**
```cpp
namespace scrivi::schemas {

struct BandLayout {
    std::string bandID;
    std::string label;
    std::string color;       // hex, e.g. "#5B8DD9"
    double      proportion;  // 0.0 < proportion ≤ 1.0; sum over all bands = 1.0
};

struct StoryStructureData {
    std::string              activeStructureID;  // e.g. "three-act", "custom"
    std::vector<BandLayout>  bandLayout;
    std::vector<BandLayout>  customBands;        // non-empty only when activeStructureID == "custom"
};

std::string serializeStoryStructure(const StoryStructureData& data);
Result<StoryStructureData> parseStoryStructure(std::string_view json);
}
```

**Schema tag:** `"scrivi.storyStructure.v1"`

**Serialization of bandLayout:** Use `doc.appendToArray("bandLayout", bandItem)` for each band. Each band item is a `JsonDoc` with `bandID`, `label`, `color`, `proportion` (double).

**Parse of bandLayout:** Loop `doc.arraySize("bandLayout")` times, `doc.arrayItem("bandLayout", i)` for each.

**Required fields on parse:** `schema`, `activeStructureID`, `bandLayout`

---

### T-0142: `HistoricalEventJson` Schema — Read/Write `historical-events/<id>.json`

**Status:** 🔵 Backlog
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-039
**Date Requested:** 2026-06-11

**Rationale:**
Historical events are worldbuilding objects (not scenes) that appear on the project timeline. Each is stored as a single JSON file in `objects/historical-events/`.

**New files:**
- `ScriviCore/src/schemas/HistoricalEventJson.hpp`
- `ScriviCore/src/schemas/HistoricalEventJson.cpp`

**New ID type** (add to `IDs.hpp`):
```cpp
struct HistoricalEventID { std::string value; };
```

**Structs:**
```cpp
namespace scrivi::schemas {

struct HistoricalEventData {
    HistoricalEventID   eventID;
    std::string         title;
    Slug                slug;
    int64_t             offsetMs     = 0;
    std::string         offsetSource = "manual";
    std::string         description;   // may be empty
    std::vector<std::string> tags;
    ISO8601Timestamp    createdAt;
    ISO8601Timestamp    modifiedAt;
};

std::string serializeHistoricalEvent(const HistoricalEventData& data);
Result<HistoricalEventData> parseHistoricalEvent(std::string_view json);
}
```

**Schema tag:** `"scrivi.historicalEvent.v1"`

**Tags serialization:** Use `doc.appendToArray("tags", ...)` — but `JsonDoc` currently only supports arrays of `JsonDoc` sub-objects, not string arrays. **This task must add `appendStringToArray` and `getStringArray` to `JsonDoc`** (or serialize tags as a JSON string array using `nlohmann::json` in the `.cpp` implementation file — acceptable since nlohmann is hidden behind the `Json` wrapper in `.cpp` files).

**Required fields on parse:** `schema`, `eventID`, `title`, `offsetMs`, `offsetSource`

---

### T-0143: `ExternalTimelineJson` Schema — Read/Write Imported Timeline Files

**Status:** 🔵 Backlog
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-039
**Date Requested:** 2026-06-11

**Rationale:**
This schema serves two purposes: it is the exchange format for exported project timelines (`scrivi.externalTimeline.v1`) and is also the stored format for imported timelines (with the additional `epochOffsetMs`, `visible`, and `assignedGreyShade` fields added at import time).

**New files:**
- `ScriviCore/src/schemas/ExternalTimelineJson.hpp`
- `ScriviCore/src/schemas/ExternalTimelineJson.cpp`

**Structs:**
```cpp
namespace scrivi::schemas {

struct ExternalTimelineEvent {
    std::string eventID;
    std::string title;
    int64_t     offsetMs = 0;
    std::string kind;    // "scene" | "historical"
    std::string notes;   // may be empty
};

struct ExternalTimelineData {
    std::string                        timelineID;
    std::string                        sourceProjectTitle;
    std::string                        sourceProjectID;
    ISO8601Timestamp                   exportedAt;
    std::string                        epochLabel;
    std::vector<ExternalTimelineEvent> events;

    // Import-time additions (present in stored files, absent in fresh exports)
    int64_t     epochOffsetMs     = 0;
    bool        visible           = true;
    std::string assignedGreyShade;  // e.g. "#8A8A8A"; empty if not set
};

std::string serializeExternalTimeline(const ExternalTimelineData& data);
Result<ExternalTimelineData> parseExternalTimeline(std::string_view json);
}
```

**Schema tag:** `"scrivi.externalTimeline.v1"`

**Required fields on parse:** `schema`, `timelineID`, `events`

**Import-time fields** (`epochOffsetMs`, `visible`, `assignedGreyShade`) are optional on parse; they default to 0, true, and "" respectively if absent.

---

### T-0144: ScriviCore Facade — Timeline, Story-Time, and Band-Assignment Operations

**Status:** 🔵 Backlog
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-039
**Date Requested:** 2026-06-11

**Rationale:**
Adds the first batch of new facade methods to `ScriviCore`: operations on the project's own timeline meta, scene story-time offsets, and band assignment. These are the operations the SwiftUI layer will call most frequently during interactive dragging.

**New request/result types** (add to `Requests.hpp` and `Results.hpp`):

```cpp
// Requests
struct GetTimelineRequest    { AbsolutePath projectRootPath; };
struct SetTimelineEpochLabelRequest { AbsolutePath projectRootPath; std::string label; };

struct SetSceneStoryTimeRequest {
    AbsolutePath projectRootPath;
    SceneID      sceneID;
    int64_t      offsetMs;
    std::string  source;   // "manual" | "inferred" | "default"
};
struct GetSceneStoryTimeRequest { AbsolutePath projectRootPath; SceneID sceneID; };
struct ClearSceneStoryTimeRequest { AbsolutePath projectRootPath; SceneID sceneID; };

struct AssignSceneToBandRequest {
    AbsolutePath projectRootPath;
    SceneID      sceneID;
    std::string  bandID;
};
struct UnassignSceneFromBandRequest { AbsolutePath projectRootPath; SceneID sceneID; };

// Results
struct GetTimelineResult {
    std::string  timelineID;
    std::string  epochLabel;
    std::string  projectID;
    std::string  createdAt;
};
struct SetTimelineEpochLabelResult { bool updated = false; };

struct SetSceneStoryTimeResult { SceneID sceneID; bool updated = false; };
struct GetSceneStoryTimeResult {
    SceneID     sceneID;
    int64_t     offsetMs     = 0;
    std::string offsetSource;
    std::string inferenceHint;
    double      inferenceConfidence = -1.0;
    std::string bandID;
    std::string bandAssignedAt;
};
struct ClearSceneStoryTimeResult { SceneID sceneID; bool cleared = false; };
struct AssignSceneToBandResult   { SceneID sceneID; bool assigned = false; };
struct UnassignSceneFromBandResult { SceneID sceneID; bool unassigned = false; };
```

**New methods on `ScriviCore`** (add to `ScriviCore.hpp` and implement in `ScriviCore.cpp`):
```cpp
Result<GetTimelineResult>             getTimeline(const GetTimelineRequest&);
Result<SetTimelineEpochLabelResult>   setTimelineEpochLabel(const SetTimelineEpochLabelRequest&);
Result<SetSceneStoryTimeResult>       setSceneStoryTime(const SetSceneStoryTimeRequest&);
Result<GetSceneStoryTimeResult>       getSceneStoryTime(const GetSceneStoryTimeRequest&);
Result<ClearSceneStoryTimeResult>     clearSceneStoryTime(const ClearSceneStoryTimeRequest&);
Result<AssignSceneToBandResult>       assignSceneToBand(const AssignSceneToBandRequest&);
Result<UnassignSceneFromBandResult>   unassignSceneFromBand(const UnassignSceneFromBandRequest&);
```

**Implementation pattern:** Each method follows the `ObjectStore` / `CommentStore` pattern: construct the file path (e.g. `projectRootPath + "/objects/timelines/timeline.meta.json"`), call the file system to read, parse with the schema module, modify the relevant field(s), re-serialize, and write atomically using the `AtomicWrite` utility.

**`setSceneStoryTime` and band assignment** must locate the correct `scene.meta.json` by traversing the manuscript order (using `ManuscriptOrderResolver`) to find the scene, then reading, updating, and rewriting the metadata file.

---

### T-0145: ScriviCore Facade — Story Structure, Historical Events, Imported Timelines, Export

**Status:** 🔵 Backlog
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-039
**Date Requested:** 2026-06-11

**Rationale:**
Adds the second batch of facade methods: the story structure management operations, historical event CRUD, imported timeline management, and the timeline export function.

**New request/result types** (add to `Requests.hpp` and `Results.hpp`):

```cpp
// Story Structure
struct GetStoryStructureRequest    { AbsolutePath projectRootPath; };
struct SetStoryStructureRequest    { AbsolutePath projectRootPath; std::string structureID; std::string bandLayoutJSON; };
struct UpdateBandLayoutRequest     { AbsolutePath projectRootPath; std::string bandLayoutJSON; };
struct RemoveStoryStructureRequest { AbsolutePath projectRootPath; };

struct GetStoryStructureResult {
    bool        hasStructure = false;
    std::string structureID;
    std::string bandLayoutJSON;  // serialized array of band objects
};
struct SetStoryStructureResult    { bool set = false; };
struct UpdateBandLayoutResult     { bool updated = false; };
struct RemoveStoryStructureResult { bool removed = false; };

// Historical Events
struct CreateHistoricalEventRequest {
    AbsolutePath projectRootPath;
    std::string  title;
    int64_t      offsetMs = 0;
    std::string  description;
    std::vector<std::string> tags;
    AuthorshipRef author;
};
struct UpdateHistoricalEventRequest {
    AbsolutePath        projectRootPath;
    HistoricalEventID   eventID;
    std::string         title;
    int64_t             offsetMs = 0;
    std::string         description;
    std::vector<std::string> tags;
};
struct DeleteHistoricalEventRequest {
    AbsolutePath      projectRootPath;
    HistoricalEventID eventID;
};
struct ListHistoricalEventsRequest { AbsolutePath projectRootPath; };

struct CreateHistoricalEventResult { HistoricalEventID eventID; Slug slug; };
struct UpdateHistoricalEventResult { HistoricalEventID eventID; bool updated = false; };
struct DeleteHistoricalEventResult { HistoricalEventID eventID; bool deleted = false; };
struct ListHistoricalEventsResult  { std::string eventsJSON; int count = 0; };

// Imported Timelines
struct ImportExternalTimelineRequest {
    AbsolutePath projectRootPath;
    std::string  timelineJSON;      // the full scrivi.externalTimeline.v1 JSON
    int64_t      epochOffsetMs = 0;
    std::string  assignedGreyShade; // set by caller (Swift layer assigns shades)
};
struct UpdateImportedTimelineOffsetRequest {
    AbsolutePath projectRootPath;
    std::string  timelineID;
    int64_t      epochOffsetMs;
};
struct SetImportedTimelineVisibleRequest {
    AbsolutePath projectRootPath;
    std::string  timelineID;
    bool         visible;
};
struct ListImportedTimelinesRequest { AbsolutePath projectRootPath; };
struct RemoveImportedTimelineRequest {
    AbsolutePath projectRootPath;
    std::string  timelineID;
};
struct ExportProjectTimelineRequest { AbsolutePath projectRootPath; };

struct ImportExternalTimelineResult { std::string timelineID; bool imported = false; };
struct UpdateImportedTimelineOffsetResult { std::string timelineID; bool updated = false; };
struct SetImportedTimelineVisibleResult   { std::string timelineID; bool updated = false; };
struct ListImportedTimelinesResult  { std::string timelinesJSON; int count = 0; };
struct RemoveImportedTimelineResult { std::string timelineID; bool removed = false; };
struct ExportProjectTimelineResult  { std::string timelineJSON; };
```

**New methods on `ScriviCore`:**
```cpp
Result<GetStoryStructureResult>             getStoryStructure(const GetStoryStructureRequest&);
Result<SetStoryStructureResult>             setStoryStructure(const SetStoryStructureRequest&);
Result<UpdateBandLayoutResult>              updateBandLayout(const UpdateBandLayoutRequest&);
Result<RemoveStoryStructureResult>          removeStoryStructure(const RemoveStoryStructureRequest&);
Result<CreateHistoricalEventResult>         createHistoricalEvent(const CreateHistoricalEventRequest&);
Result<UpdateHistoricalEventResult>         updateHistoricalEvent(const UpdateHistoricalEventRequest&);
Result<DeleteHistoricalEventResult>         deleteHistoricalEvent(const DeleteHistoricalEventRequest&);
Result<ListHistoricalEventsResult>          listHistoricalEvents(const ListHistoricalEventsRequest&);
Result<ImportExternalTimelineResult>        importExternalTimeline(const ImportExternalTimelineRequest&);
Result<UpdateImportedTimelineOffsetResult>  updateImportedTimelineOffset(const UpdateImportedTimelineOffsetRequest&);
Result<SetImportedTimelineVisibleResult>    setImportedTimelineVisible(const SetImportedTimelineVisibleRequest&);
Result<ListImportedTimelinesResult>         listImportedTimelines(const ListImportedTimelinesRequest&);
Result<RemoveImportedTimelineResult>        removeImportedTimeline(const RemoveImportedTimelineRequest&);
Result<ExportProjectTimelineResult>         exportProjectTimeline(const ExportProjectTimelineRequest&);
```

**`exportProjectTimeline` implementation:** Walk the manuscript order to collect all scenes (title, offsetMs, offsetSource). Walk `objects/historical-events/` to collect all historical events. Serialize as `scrivi.externalTimeline.v1` JSON and return it in `ExportProjectTimelineResult.timelineJSON`.

**`setStoryStructure` auto-division:** When `bandLayoutJSON` is an empty string or `"[]"`, the implementation auto-divides the current timeline span equally among the built-in bands for the given `structureID`. The built-in band definitions (IDs, labels, colors) are compiled into a static lookup table in `ScriviCore.cpp`.

---

### T-0146: C API + `ScriviEngine.swift` — Expose All Timeline Operations

**Status:** 🔵 Backlog
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-039
**Date Requested:** 2026-06-11

**Rationale:**
Adds all new timeline operations to `scrivi.h` (declarations) and `scrivi_c_api.cpp` (implementations), then adds matching methods to `ScriviEngine.swift` with Swift result types.

**New C functions** (declare in `scrivi.h`, implement in `scrivi_c_api.cpp`):

```c
// Timeline meta
const char* scrivi_get_timeline(const char* projectRootPath);
const char* scrivi_set_timeline_epoch_label(const char* projectRootPath, const char* label);

// Scene story-time
const char* scrivi_set_scene_story_time(const char* projectRootPath, const char* sceneID,
                                         int64_t offsetMs, const char* source);
const char* scrivi_get_scene_story_time(const char* projectRootPath, const char* sceneID);
const char* scrivi_clear_scene_story_time(const char* projectRootPath, const char* sceneID);

// Band assignment
const char* scrivi_assign_scene_to_band(const char* projectRootPath, const char* sceneID,
                                         const char* bandID);
const char* scrivi_unassign_scene_from_band(const char* projectRootPath, const char* sceneID);

// Story structure
const char* scrivi_get_story_structure(const char* projectRootPath);
const char* scrivi_set_story_structure(const char* projectRootPath, const char* structureID,
                                        const char* bandLayoutJSON);
const char* scrivi_update_band_layout(const char* projectRootPath, const char* bandLayoutJSON);
const char* scrivi_remove_story_structure(const char* projectRootPath);

// Historical events
const char* scrivi_create_historical_event(const char* projectRootPath, const char* title,
                                             int64_t offsetMs, const char* description,
                                             const char* tagsJSON,
                                             const char* identityID, const char* personaID,
                                             const char* displayName);
const char* scrivi_update_historical_event(const char* projectRootPath, const char* eventID,
                                             const char* title, int64_t offsetMs,
                                             const char* description, const char* tagsJSON);
const char* scrivi_delete_historical_event(const char* projectRootPath, const char* eventID);
const char* scrivi_list_historical_events(const char* projectRootPath);

// Imported timelines
const char* scrivi_import_external_timeline(const char* projectRootPath,
                                              const char* timelineJSON,
                                              int64_t epochOffsetMs,
                                              const char* assignedGreyShade);
const char* scrivi_update_imported_timeline_offset(const char* projectRootPath,
                                                     const char* timelineID,
                                                     int64_t epochOffsetMs);
const char* scrivi_set_imported_timeline_visible(const char* projectRootPath,
                                                   const char* timelineID,
                                                   int visible);
const char* scrivi_list_imported_timelines(const char* projectRootPath);
const char* scrivi_remove_imported_timeline(const char* projectRootPath, const char* timelineID);
const char* scrivi_export_project_timeline(const char* projectRootPath);
```

**Note on `int64_t` in the C API:** The plain C header uses `int64_t` for `offsetMs`. Include `<stdint.h>` in `scrivi.h`.

**Swift result types** (add to `ScriviEngine.swift`):
```swift
public struct GetTimelineResult: Decodable, Sendable {
    public let timelineID: String
    public let epochLabel: String
    public let projectID:  String
    public let createdAt:  String
}

public struct SceneStoryTimeResult: Decodable, Sendable {
    public let sceneID:              String
    public let offsetMs:             Int64
    public let offsetSource:         String
    public let inferenceHint:        String
    public let inferenceConfidence:  Double
    public let bandID:               String
    public let bandAssignedAt:       String
}

public struct StoryStructureResult: Decodable, Sendable {
    public let hasStructure:   Bool
    public let structureID:    String
    public let bandLayoutJSON: String
}

public struct HistoricalEventResult: Decodable, Sendable {
    public let eventID: String
    public let slug:    String
}

public struct HistoricalEventsListResult: Decodable, Sendable {
    public let count:      Int
    public let eventsJSON: String
}

public struct ImportedTimelinesListResult: Decodable, Sendable {
    public let count:          Int
    public let timelinesJSON:  String
}

public struct ExportTimelineResult: Decodable, Sendable {
    public let timelineJSON: String
}

public struct TimelineBoolResult: Decodable, Sendable {
    public let updated:  Bool
}
// (reused for set/update/remove operations that return only a success flag)
```

**`ScriviEngine.swift` methods:** Each method follows the existing pattern: `withCString` nesting for each `const char*` parameter, `decodeC(raw)` to decode the result.

**`int64_t` in Swift:** The `offsetMs` field decodes as `Int64` in the Swift `Decodable` struct. The C function takes `int64_t`, which Swift's C interop maps to `Int64` directly.

---

### T-0147: Integration Tests — Timeline Persistence Round-Trip

**Status:** 🔵 Backlog
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-039
**Date Requested:** 2026-06-11

**Rationale:**
Verifies all new ScriviCore API operations against real temporary directories, following the established pattern of `CreateProjectTests.cpp`, `ObjectCrudTests.cpp`, etc.

**New file:** `ScriviCore/tests/integration/TimelineTests.cpp`

**Test cases required:**

1. **Timeline meta created on new project** — `createProject` → read `objects/timelines/timeline.meta.json` directly from disk → verify schema, `epochLabel = "Story Open"`, `projectID` matches.

2. **Set and get scene story-time** — `createProject` → `setSceneStoryTime(sceneID, 3600000, "manual")` → `getSceneStoryTime(sceneID)` → verify `offsetMs = 3600000`, `offsetSource = "manual"`. Read `scene.meta.json` from disk directly and verify JSON contains the `storyTime` block.

3. **Clear scene story-time** — Set → Clear → `getSceneStoryTime` → verify `offsetMs = 0`, `offsetSource = "default"`.

4. **Band assignment** — `assignSceneToBand(sceneID, "act-ii")` → `getSceneStoryTime` → verify `bandID = "act-ii"`, `bandAssignedAt` is non-empty. `unassignSceneFromBand` → verify `bandID` is empty.

5. **Story structure round-trip** — `setStoryStructure("three-act", "")` → verify `story-structure.json` written → `getStoryStructure` → verify `hasStructure = true`, `structureID = "three-act"`, band layout has 3 bands. `removeStoryStructure` → verify file removed or `hasStructure = false`.

6. **Historical event CRUD** — `createHistoricalEvent(...)` → verify file exists in `objects/historical-events/` → `listHistoricalEvents` → verify count = 1 → `updateHistoricalEvent(...)` → read back → verify title changed → `deleteHistoricalEvent` → verify file removed.

7. **External timeline import/list/remove** — Build a minimal `scrivi.externalTimeline.v1` JSON string in the test → `importExternalTimeline(..., epochOffsetMs=0, ...)` → verify file written in `objects/imported-timelines/` → `listImportedTimelines` → verify count = 1 → `removeImportedTimeline` → verify file removed.

8. **Export project timeline** — `createProject` + `createScene` (2 scenes) + set story-time on both → `exportProjectTimeline` → parse result JSON → verify `schema = "scrivi.externalTimeline.v1"`, 2 events present with correct offsetMs.

9. **Epoch offset filter** — Import a timeline with `epochOffsetMs` set such that one event falls inside and one falls outside the project's current span. Verify `listImportedTimelines` returns the file; the filtering logic is rendered by the UI, not ScriviCore, so this test just verifies storage. (Rendering filter is a SP-040 concern.)

---

### T-0148: `createProject` Update — Write `timeline.meta.json` on New Project

**Status:** 🔵 Backlog
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-039
**Date Requested:** 2026-06-11

**Rationale:**
Every new project must have a `timeline.meta.json` from creation. Without it, the Swift layer has nothing to load when the timeline panel is first shown. This task adds a single write call to `ProjectCreator::create()`.

**File modified:** `ScriviCore/src/project_package/ProjectCreator.cpp`

**Change:** After writing `project.json` and before or after writing `manuscript/manuscript.meta.json`, write `objects/timelines/timeline.meta.json`:

```cpp
// In ProjectCreator::create(), after the objects/ folder is created:
schemas::TimelineMetaData tlMeta;
tlMeta.timelineID.value = services_.uuidProvider->generate("timeline_");
tlMeta.projectID.value  = result.project.projectID.value;
tlMeta.createdAt        = services_.clock->nowUTC();
tlMeta.epochLabel       = "Story Open";
tlMeta.notes            = "";
auto tlJson = schemas::serializeTimelineMeta(tlMeta);
auto tlPath = request.projectRootPath + "/objects/timelines/timeline.meta.json";
// Ensure the timelines/ subfolder exists (create_directories equivalent)
if (auto r = services_.fileSystem->createDirectory(
        request.projectRootPath + "/objects/timelines"); !r.ok()) {
    return Result<CreateProjectResult>::failure(r.error());
}
if (auto r = util::atomicWrite(tlPath, tlJson, *services_.fileSystem); !r.ok()) {
    return Result<CreateProjectResult>::failure(r.error());
}
```

**Existing tests:** `CreateProjectTests.cpp` should be extended with one assertion: after `createProject`, verify `objects/timelines/timeline.meta.json` exists and is valid. No other existing tests need to change.

---

## Implementation Notes for SP-039

### What already exists and must not change

- `scrivi.h` — append only. Existing function signatures are untouched.
- `Requests.hpp` / `Results.hpp` — append only.
- `ScriviCore.hpp` — append only.
- `SceneMetaJson.hpp/.cpp` — the existing `SceneMetaData` struct is extended. Existing fields and their serialization are unchanged.
- `IDs.hpp` — `TimelineID` and `HistoricalEventID` are added. No existing ID types change.

### `JsonDoc` additions required

Two `JsonDoc` additions are needed and are the responsibility of the tasks that need them:

1. **T-0140** needs `setInt64` / `getInt64` on `JsonDoc` (`Json.hpp` + `Json.cpp`).
2. **T-0142** needs string array support — either `appendStringToArray` / `getStringArray` on `JsonDoc`, or a direct nlohmann approach inside `HistoricalEventJson.cpp`. Prefer the `JsonDoc` extension for consistency.

### File and folder paths

| Schema | File path relative to project root |
|---|---|
| Timeline meta | `objects/timelines/timeline.meta.json` |
| Story structure | `objects/story-structures/story-structure.json` |
| Historical event | `objects/historical-events/<id>-<slug>.json` |
| Imported timeline | `objects/imported-timelines/<timelineID>-<sourceSlug>.scrivi-timeline.json` |

Folder creation follows the existing pattern used in `ObjectStore.cpp`: check if the directory exists, create if not, then write.

### CMakeLists.txt

All new `.cpp` files must be added to the `ScriviCore` target's source list in `CMakeLists.txt` before any build is attempted. Per the CLAUDE.md rule: new files must be registered before building.

---

## Test Steps for Sprint Verification

1. `cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON`
2. `cmake --build build --parallel`
3. `ctest --test-dir build --output-on-failure`
4. Confirm zero test failures and that `TimelineTests` appears in the test list.
5. Confirm no existing tests regressed (all prior counts preserved).

---

*Last Updated: 2026-06-11 (SP-039 created)*

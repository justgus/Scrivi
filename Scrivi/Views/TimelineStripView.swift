import SwiftUI

// MARK: — Story Structure definitions (§8 of Timeline Panel Design v0.3)

struct StoryBand: Identifiable, Codable, Equatable {
    var bandID: String
    var label: String
    var color: String        // hex, e.g. "#5B8DD9"
    var proportion: Double   // 0 < proportion < 1; sum across all bands == 1.0
    var id: String { bandID }

    var swiftUIColor: Color { Color(hex: color) ?? .accentColor }
}

struct StoryStructureDefinition {
    let structureID: String
    let name: String
    let bands: [StoryBand]
}

enum BuiltInStructure: String, CaseIterable, Identifiable {
    case threeAct        = "three-act"
    case fiveAct         = "five-act"
    case heroesJourney   = "heroes-journey"
    case saveTheCat      = "save-the-cat"
    case freytag         = "freytag"
    case kishotenketsu   = "kishotenketsu"
    case inMediasRes     = "in-medias-res"
    case custom          = "custom"

    var id: String { rawValue }

    var displayName: String {
        switch self {
        case .threeAct:      return "Three Act Structure"
        case .fiveAct:       return "Five Act Structure"
        case .heroesJourney: return "Hero's Journey"
        case .saveTheCat:    return "Save the Cat"
        case .freytag:       return "Freytag's Pyramid"
        case .kishotenketsu: return "Kishōtenketsu"
        case .inMediasRes:   return "In Medias Res"
        case .custom:        return "Custom…"
        }
    }

    var definition: StoryStructureDefinition {
        StoryStructureDefinition(structureID: rawValue, name: displayName,
                                 bands: Self.bandTable[rawValue] ?? [])
    }

    // Band definitions with equal initial proportions applied in TimelineViewModel.
    private static let bandTable: [String: [StoryBand]] = [
        "three-act": [
            StoryBand(bandID: "act-i",   label: "Act I",   color: "#5B8DD9", proportion: 1/3),
            StoryBand(bandID: "act-ii",  label: "Act II",  color: "#D9A05B", proportion: 1/3),
            StoryBand(bandID: "act-iii", label: "Act III", color: "#5BD98D", proportion: 1/3),
        ],
        "five-act": [
            StoryBand(bandID: "exposition",    label: "Exposition",    color: "#5B8DD9", proportion: 0.2),
            StoryBand(bandID: "rising-action", label: "Rising Action", color: "#8D5BD9", proportion: 0.2),
            StoryBand(bandID: "climax",        label: "Climax",        color: "#D95B5B", proportion: 0.2),
            StoryBand(bandID: "falling-action",label: "Falling Action",color: "#D9A05B", proportion: 0.2),
            StoryBand(bandID: "denouement",    label: "Denouement",    color: "#5BD98D", proportion: 0.2),
        ],
        "heroes-journey": [
            StoryBand(bandID: "ordinary-world",       label: "Ordinary World",       color: "#5B8DD9", proportion: 1/12),
            StoryBand(bandID: "call-to-adventure",    label: "Call to Adventure",    color: "#7B8DD9", proportion: 1/12),
            StoryBand(bandID: "refusal",              label: "Refusal of the Call",  color: "#9B8DD9", proportion: 1/12),
            StoryBand(bandID: "meeting-mentor",       label: "Meeting the Mentor",   color: "#BD8DD9", proportion: 1/12),
            StoryBand(bandID: "crossing-threshold",   label: "Crossing the Threshold",color: "#D98DBD",proportion: 1/12),
            StoryBand(bandID: "tests-allies-enemies", label: "Tests, Allies, Enemies",color: "#D98D9B",proportion: 1/12),
            StoryBand(bandID: "approach",             label: "Approach",             color: "#D9A05B", proportion: 1/12),
            StoryBand(bandID: "ordeal",               label: "Ordeal",               color: "#D9735B", proportion: 1/12),
            StoryBand(bandID: "reward",               label: "Reward",               color: "#D95B5B", proportion: 1/12),
            StoryBand(bandID: "road-back",            label: "The Road Back",        color: "#A05BD9", proportion: 1/12),
            StoryBand(bandID: "resurrection",         label: "Resurrection",         color: "#7B5BD9", proportion: 1/12),
            StoryBand(bandID: "return",               label: "Return with the Elixir",color: "#5B8DD9",proportion: 1/12),
        ],
        "save-the-cat": [
            StoryBand(bandID: "opening-image",    label: "Opening Image",     color: "#5B8DD9", proportion: 1/15),
            StoryBand(bandID: "theme-stated",     label: "Theme Stated",      color: "#6B8DD9", proportion: 1/15),
            StoryBand(bandID: "set-up",           label: "Set-Up",            color: "#7B8DD9", proportion: 1/15),
            StoryBand(bandID: "catalyst",         label: "Catalyst",          color: "#8B8DD9", proportion: 1/15),
            StoryBand(bandID: "debate",           label: "Debate",            color: "#9B8DD9", proportion: 1/15),
            StoryBand(bandID: "break-into-two",   label: "Break Into Two",    color: "#AB8DD9", proportion: 1/15),
            StoryBand(bandID: "b-story",          label: "B Story",           color: "#BD8DD9", proportion: 1/15),
            StoryBand(bandID: "fun-and-games",    label: "Fun and Games",     color: "#D98DBD", proportion: 1/15),
            StoryBand(bandID: "midpoint",         label: "Midpoint",          color: "#D98D9B", proportion: 1/15),
            StoryBand(bandID: "bad-guys-close-in",label: "Bad Guys Close In", color: "#D9735B", proportion: 1/15),
            StoryBand(bandID: "all-is-lost",      label: "All Is Lost",       color: "#D95B5B", proportion: 1/15),
            StoryBand(bandID: "dark-night",       label: "Dark Night of the Soul",color: "#C05B5B",proportion: 1/15),
            StoryBand(bandID: "break-into-three", label: "Break Into Three",  color: "#A05BD9", proportion: 1/15),
            StoryBand(bandID: "finale",           label: "Finale",            color: "#7B5BD9", proportion: 1/15),
            StoryBand(bandID: "final-image",      label: "Final Image",       color: "#5B8DD9", proportion: 1/15),
        ],
        "freytag": [
            StoryBand(bandID: "introduction",  label: "Introduction",  color: "#5B8DD9", proportion: 0.2),
            StoryBand(bandID: "rising-action", label: "Rising Action", color: "#8D5BD9", proportion: 0.2),
            StoryBand(bandID: "climax",        label: "Climax",        color: "#D95B5B", proportion: 0.2),
            StoryBand(bandID: "falling-action",label: "Falling Action",color: "#D9A05B", proportion: 0.2),
            StoryBand(bandID: "catastrophe",   label: "Catastrophe",   color: "#5BD98D", proportion: 0.2),
        ],
        "kishotenketsu": [
            StoryBand(bandID: "ki",   label: "Ki — Introduction", color: "#5B8DD9", proportion: 0.25),
            StoryBand(bandID: "sho",  label: "Shō — Development", color: "#8D5BD9", proportion: 0.25),
            StoryBand(bandID: "ten",  label: "Ten — Twist",       color: "#D95B5B", proportion: 0.25),
            StoryBand(bandID: "ketsu",label: "Ketsu — Conclusion",color: "#5BD98D", proportion: 0.25),
        ],
        "in-medias-res": [
            StoryBand(bandID: "action-open",   label: "Action Open",   color: "#D95B5B", proportion: 0.25),
            StoryBand(bandID: "backstory",     label: "Backstory",     color: "#5B8DD9", proportion: 0.25),
            StoryBand(bandID: "rising-action", label: "Rising Action", color: "#8D5BD9", proportion: 0.25),
            StoryBand(bandID: "resolution",    label: "Resolution",    color: "#5BD98D", proportion: 0.25),
        ],
    ]
}

// MARK: — SP-042 data models

struct HistoricalEventDot: Identifiable {
    let id: String          // == eventID
    let eventID: String
    var title: String
    var offsetMs: Int64
    var offsetSource: String
    var description: String
}

struct ImportedTimelineRow: Identifiable {
    let id: String          // == timelineID
    let timelineID: String
    let sourceName: String
    let epochLabel: String
    var epochOffsetMs: Int64
    var visible: Bool
    var greyShade: String   // hex
    var events: [ImportedEventDot]

    var swiftUIColor: Color { Color(hex: greyShade) ?? .gray }
}

struct ImportedEventDot: Identifiable {
    let id: String          // == eventID
    let eventID: String
    let title: String
    let sourceOffsetMs: Int64   // offset within source timeline
    let kind: String            // "scene" | "historical"
    var projectOffsetMs: Int64  // sourceOffsetMs + epochOffsetMs
}

// Decodable shape of a stored .scrivi-timeline.json file (import format).
// SP-129 / T-0502 — the shape `scrivi_list_imported_timelines` returns.
//
// ⚠️ **THIS REPLACED A DIRECT FILESYSTEM READ.** Until SP-129 this file decoded the
// STORED `.scrivi-timeline.json` files itself — `FileManager.contentsOfDirectory`
// plus `Data(contentsOf:)` plus `JSONDecoder`, per file, on the timeline load path.
// That is backend logic in the UI layer, which CLAUDE.md forbids outright ("Swift is
// responsible for UI only"), and it re-parsed files the core had ALREADY parsed.
//
// ✅ The core projection now carries `events` with `projectOffsetMs` PRE-RESOLVED,
// so this decodes one envelope instead of walking a directory.
private struct ImportedTimelinesPayload: Decodable {
    struct Timeline: Decodable {
        struct Event: Decodable {
            let eventID: String
            let title: String
            let offsetMs: Int64
            let projectOffsetMs: Int64
            let kind: String

            private enum CodingKeys: String, CodingKey {
                case eventID, title, offsetMs, projectOffsetMs, kind
            }
            init(from decoder: Decoder) throws {
                let c = try decoder.container(keyedBy: CodingKeys.self)
                eventID         = (try? c.decode(String.self, forKey: .eventID)) ?? ""
                title           = (try? c.decode(String.self, forKey: .title)) ?? ""
                offsetMs        = (try? c.decode(Int64.self,  forKey: .offsetMs)) ?? 0
                projectOffsetMs = (try? c.decode(Int64.self,  forKey: .projectOffsetMs)) ?? offsetMs
                kind            = (try? c.decode(String.self, forKey: .kind)) ?? ""
            }
        }
        let timelineID: String
        let sourceProjectTitle: String
        let epochLabel: String
        let epochOffsetMs: Int64
        let visible: Bool
        let assignedGreyShade: String
        let events: [Event]

        private enum CodingKeys: String, CodingKey {
            case timelineID, sourceProjectTitle, epochLabel, epochOffsetMs, visible,
                 assignedGreyShade, events
        }
        init(from decoder: Decoder) throws {
            let c = try decoder.container(keyedBy: CodingKeys.self)
            timelineID         = try c.decode(String.self, forKey: .timelineID)
            sourceProjectTitle = (try? c.decode(String.self, forKey: .sourceProjectTitle)) ?? ""
            epochLabel         = (try? c.decode(String.self, forKey: .epochLabel)) ?? "Story Open"
            epochOffsetMs      = (try? c.decode(Int64.self,  forKey: .epochOffsetMs)) ?? 0
            visible            = (try? c.decode(Bool.self,   forKey: .visible)) ?? true
            assignedGreyShade  = (try? c.decode(String.self, forKey: .assignedGreyShade)) ?? ""
            // ⚠️ Absent `events` is an EMPTY LIST, not a failure — same trap AC4 hit:
            // the JSON writer omits an empty array entirely.
            events             = (try? c.decode([Event].self, forKey: .events)) ?? []
        }
    }
    let timelines: [Timeline]

    private enum CodingKeys: String, CodingKey { case timelines }
    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        timelines = (try? c.decode([Timeline].self, forKey: .timelines)) ?? []
    }
}

// MARK: — TimelineViewModel

@Observable @MainActor final class TimelineViewModel {

    struct SceneDot: Identifiable {
        let id: String              // == sceneID
        let sceneID: String
        var title: String
        var chapterTitle: String
        var offsetMs: Int64         // derived — computed from the gap chain, never stored as canonical
        var offsetSource: String    // "default" | "manual" | "inferred"
        var gapMs: Int64            // canonical: gap from previousSceneEnd to this scene's start
                                    // 0 for "default" scenes; set by picker for "manual" scenes
        var durationMs: Int64
        var durationSource: String
        var bandID: String
    }

    // I-0204: `dots` and `historicalEvents` feed four derived values that the timeline's
    // layout path reads thousands of times per frame (`minOffsetMs`, `maxEndMs`, `spanMs`,
    // and the view's `smallestMainRowGapMs()`). Recomputing them per access cost ~33 s of
    // main thread per layout pass on a 1,156-scene project. They are now computed ONCE per
    // mutation via `didSet` and read from storage.
    //
    // ⚠️ `didSet` is the correct hook and not merely convenient: both arrays are
    // `private(set)`, so EVERY mutation — whole-array assignment, `append`, `removeAll`, and
    // in-place element writes like `historicalEvents[idx].offsetMs = …` — happens inside this
    // class and fires it. A cache keyed off `load()` alone would go stale on the in-place
    // element writes (`updateHistoricalEventOffset`), which move a dot's offset.
    private(set) var dots: [SceneDot] = [] {
        didSet { recomputeDerivedBounds() }
    }
    private(set) var epochLabel: String = "Story Open"
    private(set) var defaultSceneDurationMs: Int64 = 3_600_000

    // Hovered IDs — set by child dot views, read by TimelineStripView to render tooltips
    // in the panel-level overlay outside .clipped() so they are never cut off.
    var hoveredDotID: String? = nil
    var hoveredHistoricalEventID: String? = nil
    var hoveredImportedEventKey: String? = nil   // "timelineID:eventID"

    // Historical events
    private(set) var historicalEvents: [HistoricalEventDot] = [] {
        didSet { recomputeDerivedBounds() }
    }

    // Imported timelines
    // ⚠️ I-0211 — imported timelines are FRAMING REFERENCES for the main timeline.
    //
    // `didSet` so the derived bounds recompute when rows are loaded, shown, hidden or
    // removed — the same discipline `dots` and `historicalEvents` already use. Without
    // it the span ignores imports entirely and every imported event lands off-panel.
    /// [I-0214] Imported-timeline files the CORE could not read or parse.
    ///
    /// ⚠️ **WHY THIS EXISTS.** These files were discarded in silence: the panel simply
    /// drew nothing, and a writer could not tell "I never imported that" from "my
    /// import is broken". ✅ **The core now names each file and why it was rejected.**
    ///
    /// ⛔ **This is NOT an error state for the panel.** Good timelines still draw; this
    /// reports only what was left out (same discipline as `WorldWarningModel`).
    private(set) var rejectedImports: [ImportedTimelineRejection] = []

    private(set) var importedTimelines: [ImportedTimelineRow] = [] {
        didSet { recomputeDerivedBounds() }
    }

    // Story structure state — loaded from disk via ScriviEngine
    private(set) var activeBands: [StoryBand] = []          // empty == no structure active
    private(set) var activeStructureID: String = ""

    // I-0204: read from the cache; recomputed only on mutation. The bodies below are the
    // ORIGINAL expressions, moved verbatim into `recomputeDerivedBounds()` — the values are
    // unchanged, only their frequency is.
    private(set) var minOffsetMs: Int64 = 0
    private(set) var maxEndMs: Int64 = 1
    private(set) var spanMs: Int64 = 1

    // I-0204: smallest non-zero gap between consecutive main-row item offsets (scenes +
    // historical events). Lives on the model rather than the view because it depends only on
    // model data; the view's `smallestMainRowGapMs()` now reads this.
    private(set) var smallestMainRowGapMs: Int64 = 0

    private func recomputeDerivedBounds() {
        let sceneMin = dots.map(\.offsetMs).min() ?? 0
        let heMin    = historicalEvents.map(\.offsetMs).min() ?? sceneMin
        minOffsetMs = min(sceneMin, heMin)

        let sceneMax = dots.map { $0.offsetMs + $0.durationMs }.max() ?? 1
        let heMax    = historicalEvents.map(\.offsetMs).max() ?? sceneMax
        maxEndMs = max(sceneMax, heMax)

        // ⚠️ I-0211 — IMPORTED TIMELINES FRAME THE MAIN TIMELINE (user ruling 2026-09-14).
        //
        // ⚠️ THE DEFECT THIS FIXES: the span was computed from scene dots and historical
        // events ONLY. A manuscript whose scenes all carry the DEFAULT story time spans
        // `sceneCount × 1h` — on the 1,174-scene Dumas fixture that is **48.9 days**,
        // while its imported historical events sit 77x to 1,799x beyond the right edge.
        // The row filter (`x >= 16 && x <= panelW - 16`) then discarded EVERY event, so
        // all four imported rows rendered as bare lines and looked EMPTY.
        //
        // ✅ THE RULE: once a timeline is imported it becomes a FRAMING REFERENCE —
        // the earliest moment across all VISIBLE imported timelines is the main
        // timeline's start, and the latest is its end. The manuscript's scenes are then
        // placed WITHIN that frame rather than defining it.
        //
        // ⚠️ VISIBLE ROWS ONLY: hiding a row must actually narrow the frame, or the
        // writer cannot use visibility to focus on one period.
        let importedOffsets = importedTimelines
            .filter(\.visible)
            .flatMap { $0.events.map(\.projectOffsetMs) }
        if let impMin = importedOffsets.min(), let impMax = importedOffsets.max() {
            minOffsetMs = min(minOffsetMs, impMin)
            maxEndMs    = max(maxEndMs, impMax)
        }

        spanMs = max(maxEndMs - minOffsetMs, 1)

        // Verbatim from the view's former `smallestMainRowGapMs()`; 0 means "no non-zero gap",
        // which the view translates to its own floor (it owns `minVisibleSpanFloorMs`).
        var offsets: [Int64] = dots.map(\.offsetMs) + historicalEvents.map(\.offsetMs)
        offsets.sort()
        var smallest: Int64 = .max
        for i in 1..<max(offsets.count, 1) where i < offsets.count {
            let gap = offsets[i] - offsets[i - 1]
            if gap > 0 { smallest = min(smallest, gap) }
        }
        smallestMainRowGapMs = (smallest == .max) ? 0 : smallest
    }

    // I-0048: Story Structure bands span the story from the FIRST scene to the LAST scene in
    // manuscript order (dots[0] → dots.last), NOT min/max offset. A flashback scene in the
    // middle of the order (e.g. with a far-past offset) therefore falls OUTSIDE the band
    // region rather than stretching it. Returns nil when there are fewer than 2 scenes.
    var structureRange: (startMs: Int64, endMs: Int64)? {
        guard let first = dots.first, let last = dots.last, dots.count >= 2 else { return nil }
        let start = first.offsetMs
        let end   = last.offsetMs + last.durationMs
        guard end > start else { return nil }
        return (start, end)
    }

    func load(engine: ScriviEngine, projectRootPath: String, scenes: [SceneInfo]) {
        epochLabel = (try? engine.getTimeline(projectRootPath: projectRootPath))?.epochLabel ?? "Story Open"

        // ✅ EP-039 AC4 ADOPTED (I-0213) — ONE bulk crossing instead of one per scene.
        // This path cost `TimelineViewModel.load = 113 ms` at open on 1,174 scenes.
        // ⚠️ Empty is normal, not failure — see `explicitStoryTimes`.
        let explicitByScene = Self.explicitStoryTimes(engine: engine,
                                                      projectRootPath: projectRootPath)

        var raw: [SceneDot] = scenes.enumerated().map { idx, info in
            let st = explicitByScene[info.sceneID]
            let dur = st?.durationMs ?? defaultSceneDurationMs
            return SceneDot(
                id: info.sceneID,
                sceneID: info.sceneID,
                title: info.title.isEmpty ? "Scene \(idx + 1)" : info.title,
                chapterTitle: info.chapterTitle,
                offsetMs: 0,
                offsetSource: st?.offsetSource ?? "default",
                gapMs: st?.gapMs ?? 0,
                durationMs: dur > 0 ? dur : defaultSceneDurationMs,
                durationSource: st?.durationSource ?? "default",
                bandID: st?.bandID ?? ""
            )
        }
        recomputeAllOffsets(in: &raw)
        dots = raw

        loadStoryStructure(engine: engine, projectRootPath: projectRootPath)
        loadHistoricalEvents(engine: engine, projectRootPath: projectRootPath)
        loadImportedTimelines(projectRootPath: projectRootPath, engine: engine)
    }

    // Reload only the scene dots from an updated scene list.
    // Used after scene/chapter creation, split, or merge so the timeline stays in sync
    // without resetting historical events, imported timelines, or story structure.
    func reloadSceneDots(engine: ScriviEngine, projectRootPath: String, scenes: [SceneInfo]) {
        // ✅ EP-039 AC4 ADOPTED (I-0213). This made ONE `getSceneStoryTime` C ABI call
        // PER SCENE — 1,174 synchronous crossings on the main actor, MEASURED at
        // ~950 ms inside a chapter create. `scrivi_list_story_times` shipped in SP-131
        // to replace exactly this, but was never bound in Swift; it is now.
        //
        // ⚠️ EMPTY IS NORMAL AND IS NOT FAILURE. On a default-chain manuscript NO scene
        // has an explicit story time, so the sparse list is legitimately empty and every
        // dot takes its defaults. A throw is the only failure signal — and on a throw we
        // keep going with defaults rather than drawing a blank timeline.
        let explicitByScene = Self.explicitStoryTimes(engine: engine,
                                                      projectRootPath: projectRootPath)

        // ⚠️ `dots.first(where:)` INSIDE the map was a second O(N²). One map instead.
        var bandByScene: [String: String] = [:]
        bandByScene.reserveCapacity(dots.count)
        for d in dots where !d.bandID.isEmpty { bandByScene[d.sceneID] = d.bandID }

        var raw: [SceneDot] = scenes.enumerated().map { idx, info in
            let st = explicitByScene[info.sceneID]
            let existingBandID = bandByScene[info.sceneID] ?? ""
            let dur = st?.durationMs ?? defaultSceneDurationMs
            return SceneDot(
                id: info.sceneID,
                sceneID: info.sceneID,
                title: info.title.isEmpty ? "Scene \(idx + 1)" : info.title,
                chapterTitle: info.chapterTitle,
                offsetMs: 0,
                offsetSource: st?.offsetSource ?? "default",
                gapMs: st?.gapMs ?? 0,
                durationMs: dur > 0 ? dur : defaultSceneDurationMs,
                durationSource: st?.durationSource ?? "default",
                bandID: st?.bandID.isEmpty == false ? st!.bandID : existingBandID
            )
        }
        recomputeAllOffsets(in: &raw)
        dots = raw
    }

    /// EP-039 AC4 — one bulk crossing, keyed by sceneID for O(1) lookup.
    ///
    /// ⚠️ **THE EMPTY-ARRAY TRAP IS HANDLED HERE, ONCE.** The C ABI omits `storyTimes`
    /// entirely when nothing is set, and that is the COMMON case — so an empty result
    /// means "every scene is on the default chain", NOT "the call failed".
    /// ⛔ Only a THROW is failure, and even then the caller proceeds with defaults:
    /// drawing a blank timeline because a read failed would hide the writer's story.
    static func explicitStoryTimes(engine: ScriviEngine,
                                   projectRootPath: String) -> [String: SceneStoryTimeEntry] {
        guard let result = try? engine.listStoryTimes(projectRootPath: projectRootPath) else {
            return [:]
        }
        var byScene: [String: SceneStoryTimeEntry] = [:]
        byScene.reserveCapacity(result.storyTimes.count)
        for st in result.storyTimes { byScene[st.sceneID] = st }
        return byScene
    }

    // Patch dot titles to match the Scene Navigator's display logic exactly:
    //   1. Explicit engine title (info.title non-empty) → use it unchanged.
    //   2. No explicit title → use liveTitles first-line text if available.
    //   3. Neither → keep the existing "Scene N" ordinal fallback on the dot.
    // `allScenes` is passed so we can check info.title without a separate lookup path.
    func updateDotTitles(liveTitles: [String: String], allScenes: [SceneInfo]) {
        // ⚠️ I-0213 — THIS WAS THE REMAINING ~745 ms, NOT `reloadSceneDots`.
        //
        // ✅ MEASURED: after AC4's bulk read landed, `load()` — which builds the SAME dots
        // — fell to 3.5 ms, while the block timed as "reloadSceneDots" stayed at ~745 ms.
        // The difference is THIS function, which `load()` never calls. My timer spanned
        // both, so the label was misleading.
        //
        // ⚠️ IT WAS DOUBLY O(N²):
        //   1. `allScenes.first(where:)` — a 1,177-element LINEAR SCAN **per dot**,
        //      ~1.4 M comparisons;
        //   2. `dots[i].title = …` — an `@Observable` write **per dot**, so up to 1,177
        //      notifications, each able to drive its own SwiftUI update pass.
        //
        // ✅ One `sceneID → SceneInfo` map (O(N)), and one assignment to `dots` at the end.
        var infoByScene: [String: SceneInfo] = [:]
        infoByScene.reserveCapacity(allScenes.count)
        for info in allScenes { infoByScene[info.sceneID] = info }

        var updated = dots
        var changed = false
        for i in updated.indices {
            let sceneID = updated[i].sceneID
            guard let info = infoByScene[sceneID] else { continue }
            let newTitle: String?
            if !info.title.trimmingCharacters(in: .whitespaces).isEmpty {
                newTitle = info.title                      // Explicit title always wins.
            } else if let live = liveTitles[sceneID],
                      !live.trimmingCharacters(in: .whitespaces).isEmpty {
                newTitle = live                            // Else the first-line live text.
            } else {
                newTitle = nil                             // Else keep "Scene N".
            }
            if let newTitle, updated[i].title != newTitle {
                updated[i].title = newTitle
                changed = true
            }
        }
        // ⚠️ Skip the write entirely when nothing changed — a no-op reload should not
        // post an observation notification at all.
        if changed { dots = updated }
    }

    // MARK: Historical events

    func loadHistoricalEvents(engine: ScriviEngine, projectRootPath: String) {
        guard let result = try? engine.listHistoricalEvents(projectRootPath: projectRootPath),
              !result.eventsJSON.isEmpty,
              let data = result.eventsJSON.data(using: .utf8) else {
            historicalEvents = []
            return
        }
        struct Wrapper: Decodable {
            struct Item: Decodable {
                let eventID: String
                let title: String
                let offsetMs: Int64
                let offsetSource: String
                let description: String
            }
            let events: [Item]
        }
        guard let w = try? JSONDecoder().decode(Wrapper.self, from: data) else {
            historicalEvents = []
            return
        }
        historicalEvents = w.events.map {
            HistoricalEventDot(id: $0.eventID, eventID: $0.eventID,
                               title: $0.title.isEmpty ? "Historical Event" : $0.title,
                               offsetMs: $0.offsetMs, offsetSource: $0.offsetSource,
                               description: $0.description)
        }
    }

    func createHistoricalEvent(at offsetMs: Int64, engine: ScriviEngine,
                                projectRootPath: String, authorshipRef: AuthorshipRef) {
        guard let result = try? engine.createHistoricalEvent(
            projectRootPath: projectRootPath, title: "New Historical Event",
            offsetMs: offsetMs, authorshipRef: authorshipRef) else { return }
        let dot = HistoricalEventDot(id: result.eventID, eventID: result.eventID,
                                     title: "New Historical Event",
                                     offsetMs: offsetMs, offsetSource: "manual", description: "")
        historicalEvents.append(dot)
    }

    func updateHistoricalEventOffset(eventID: String, offsetMs: Int64,
                                      engine: ScriviEngine, projectRootPath: String) {
        guard let idx = historicalEvents.firstIndex(where: { $0.eventID == eventID }) else { return }
        historicalEvents[idx].offsetMs = offsetMs
        _ = try? engine.updateHistoricalEvent(
            projectRootPath: projectRootPath, eventID: eventID,
            title: historicalEvents[idx].title, offsetMs: offsetMs,
            description: historicalEvents[idx].description)
    }

    func updateHistoricalEvent(eventID: String, title: String, description: String,
                                engine: ScriviEngine, projectRootPath: String) {
        guard let idx = historicalEvents.firstIndex(where: { $0.eventID == eventID }) else { return }
        historicalEvents[idx].title = title
        historicalEvents[idx].description = description
        _ = try? engine.updateHistoricalEvent(
            projectRootPath: projectRootPath, eventID: eventID,
            title: title, offsetMs: historicalEvents[idx].offsetMs, description: description)
    }

    func deleteHistoricalEvent(eventID: String, engine: ScriviEngine, projectRootPath: String) {
        historicalEvents.removeAll { $0.eventID == eventID }
        _ = try? engine.deleteHistoricalEvent(projectRootPath: projectRootPath, eventID: eventID)
    }

    // MARK: Imported timelines

    /// SP-129 / T-0502 — imported timelines, THROUGH THE CORE.
    ///
    /// ⚠️ **WHAT THIS REPLACED.** This function used to walk
    /// `objects/imported-timelines/` with `FileManager.contentsOfDirectory`, then
    /// `Data(contentsOf:)` + `JSONDecoder` per file — ⚠️ **on the timeline load path,
    /// the same path measured at `251 s` in [I-0196]**, and in plain violation of the
    /// standing rule that no backend logic is reimplemented in Swift.
    ///
    /// ⚠️ **WHY IT COULD NOT SIMPLY BE SWAPPED.** `listImportedTimelines` projected
    /// `eventCount` but NOT the events, so the endpoint alone could not feed the dots —
    /// which is exactly why Linux reads the files too. ✅ **SP-129 extended the CORE
    /// projection to carry events with `projectOffsetMs` pre-resolved**, so this is now
    /// one crossing and the offset arithmetic lives in one place instead of two.
    ///
    /// ⛔ A THROW leaves the existing rows ALONE rather than blanking them: a failed
    /// read must not look to a writer like "your imported timelines are gone".
    func loadImportedTimelines(projectRootPath: String, engine: ScriviEngine) {
        guard let result = try? engine.listImportedTimelines(projectRootPath: projectRootPath) else {
            return
        }
        // [I-0214] Record what the core refused, so the panel can say so.
        rejectedImports = result.rejected
        guard let data = result.timelinesJSON.data(using: .utf8),
              let payload = try? JSONDecoder().decode(ImportedTimelinesPayload.self, from: data) else {
            // ⚠️ `timelinesJSON` is "{}" when there are no imported timelines — the
            // empty-array trap again. Decoding yields zero rows, which IS the answer.
            importedTimelines = []
            return
        }

        importedTimelines = payload.timelines.map { tl in
            ImportedTimelineRow(
                id: tl.timelineID, timelineID: tl.timelineID,
                sourceName: tl.sourceProjectTitle.isEmpty ? "Imported Timeline" : tl.sourceProjectTitle,
                epochLabel: tl.epochLabel, epochOffsetMs: tl.epochOffsetMs,
                visible: tl.visible,
                greyShade: tl.assignedGreyShade.isEmpty ? "#8A8A8A" : tl.assignedGreyShade,
                events: tl.events.map { ev in
                    ImportedEventDot(
                        id: ev.eventID, eventID: ev.eventID, title: ev.title,
                        sourceOffsetMs: ev.offsetMs, kind: ev.kind,
                        // ✅ Pre-resolved by the core; NOT recomputed here.
                        projectOffsetMs: ev.projectOffsetMs)
                })
        }
    }

    func setImportedTimelineVisible(timelineID: String, visible: Bool,
                                     engine: ScriviEngine, projectRootPath: String) {
        guard let idx = importedTimelines.firstIndex(where: { $0.timelineID == timelineID }) else { return }
        importedTimelines[idx].visible = visible
        _ = try? engine.setImportedTimelineVisible(projectRootPath: projectRootPath,
                                                    timelineID: timelineID, visible: visible)
    }

    func removeImportedTimeline(timelineID: String, engine: ScriviEngine, projectRootPath: String) {
        importedTimelines.removeAll { $0.timelineID == timelineID }
        _ = try? engine.removeImportedTimeline(projectRootPath: projectRootPath, timelineID: timelineID)
    }

    // Assigns distinct grey shades to imported timelines that have none.
    static func greyShade(for index: Int) -> String {
        let shades = ["#8A8A8A", "#6A6A6A", "#AAAAAA", "#555555", "#BBBBBB"]
        return shades[index % shades.count]
    }

    // Load the active story structure from disk.
    func loadStoryStructure(engine: ScriviEngine, projectRootPath: String) {
        guard let ss = try? engine.getStoryStructure(projectRootPath: projectRootPath),
              ss.hasStructure else {
            activeBands = []
            activeStructureID = ""
            return
        }
        activeStructureID = ss.structureID
        // bandLayoutJSON is a JSON array string — decode it.
        let parsed = decodeBandLayout(ss.bandLayoutJSON)
        activeBands = parsed.isEmpty
            ? (BuiltInStructure(rawValue: ss.structureID)?.definition.bands ?? [])
            : parsed
    }

    // bandLayoutJSON from the C backend is {"bands":[{bandID,label,color,proportion},...]}
    private func decodeBandLayout(_ json: String) -> [StoryBand] {
        guard !json.isEmpty, let data = json.data(using: .utf8) else { return [] }
        struct Wrapper: Decodable { let bands: [StoryBand] }
        if let w = try? JSONDecoder().decode(Wrapper.self, from: data) { return w.bands }
        // Fallback: try plain array (future-proof against format change)
        return (try? JSONDecoder().decode([StoryBand].self, from: data)) ?? []
    }

    // Apply a built-in structure. Passes empty bandLayoutJSON so the C++ backend
    // populates bands from its own built-in table, then refreshes in-memory state.
    func applyStructure(_ structureID: String, engine: ScriviEngine, projectRootPath: String) {
        _ = try? engine.setStoryStructure(projectRootPath: projectRootPath,
                                           structureID: structureID,
                                           bandLayoutJSON: "")
        loadStoryStructure(engine: engine, projectRootPath: projectRootPath)
    }

    // Remove the active structure. Clears bands from display; preserves scene bandIDs on disk.
    func removeStructure(engine: ScriviEngine, projectRootPath: String) {
        _ = try? engine.removeStoryStructure(projectRootPath: projectRootPath)
        activeBands = []
        activeStructureID = ""
    }

    // Persist updated band proportions after a border drag.
    func updateBandLayout(_ bands: [StoryBand], engine: ScriviEngine, projectRootPath: String) {
        activeBands = bands
        let layoutJSON = encodeBandLayout(bands)
        _ = try? engine.updateBandLayout(projectRootPath: projectRootPath,
                                          bandLayoutJSON: layoutJSON)
    }

    // Assign a scene to a band.
    func assignToBand(sceneID: String, bandID: String,
                      engine: ScriviEngine, projectRootPath: String) {
        guard let idx = dots.firstIndex(where: { $0.sceneID == sceneID }) else { return }
        dots[idx].bandID = bandID
        _ = try? engine.assignSceneToBand(projectRootPath: projectRootPath,
                                           sceneID: sceneID, bandID: bandID)
    }

    // Unassign a scene from its band.
    func unassignFromBand(sceneID: String, engine: ScriviEngine, projectRootPath: String) {
        guard let idx = dots.firstIndex(where: { $0.sceneID == sceneID }) else { return }
        dots[idx].bandID = ""
        _ = try? engine.unassignSceneFromBand(projectRootPath: projectRootPath, sceneID: sceneID)
    }

    // Commit a position change.
    func setStoryTime(sceneID: String, offsetMs: Int64, offsetSource: String,
                      gapMs: Int64, durationMs: Int64, durationSource: String,
                      engine: ScriviEngine, projectRootPath: String) {
        guard let idx = dots.firstIndex(where: { $0.sceneID == sceneID }) else { return }
        dots[idx].offsetMs       = offsetMs
        dots[idx].offsetSource   = offsetSource
        dots[idx].gapMs          = gapMs
        dots[idx].durationMs     = durationMs
        dots[idx].durationSource = durationSource
        _ = try? engine.setSceneStoryTime(
            projectRootPath: projectRootPath, sceneID: sceneID,
            offsetMs: offsetMs, source: offsetSource,
            gapMs: gapMs, durationMs: durationMs, durationSource: durationSource)
        recomputeAndPersistFrom(idx + 1, engine: engine, projectRootPath: projectRootPath)
    }

    func resetToDefault(sceneID: String, engine: ScriviEngine, projectRootPath: String) {
        guard let idx = dots.firstIndex(where: { $0.sceneID == sceneID }) else { return }
        dots[idx].offsetSource   = "default"
        dots[idx].gapMs          = 0
        dots[idx].durationMs     = defaultSceneDurationMs
        dots[idx].durationSource = "default"
        _ = try? engine.clearSceneStoryTime(projectRootPath: projectRootPath, sceneID: sceneID)
        recomputeAndPersistFrom(idx, engine: engine, projectRootPath: projectRootPath)
    }

    // MARK: Chain helpers

    private func recomputeAllOffsets(in dots: inout [SceneDot]) {
        for i in dots.indices {
            if i == 0 {
                dots[i].offsetMs = dots[i].gapMs
            } else {
                let prevEnd = dots[i - 1].offsetMs + dots[i - 1].durationMs
                dots[i].offsetMs = prevEnd + dots[i].gapMs
            }
        }
    }

    private func recomputeAndPersistFrom(_ startIdx: Int,
                                          engine: ScriviEngine,
                                          projectRootPath: String) {
        guard startIdx < dots.count else { return }
        for i in startIdx..<dots.count {
            let prevEnd: Int64 = i == 0 ? 0 : dots[i - 1].offsetMs + dots[i - 1].durationMs
            dots[i].offsetMs = prevEnd + dots[i].gapMs
            _ = try? engine.setSceneStoryTime(
                projectRootPath: projectRootPath,
                sceneID: dots[i].sceneID,
                offsetMs: dots[i].offsetMs,
                source: dots[i].offsetSource,
                gapMs: dots[i].gapMs,
                durationMs: dots[i].durationMs,
                durationSource: dots[i].durationSource)
        }
    }

    // Encode band layout as the {"bands":[...]} JSON object the C backend expects.
    private func encodeBandLayout(_ bands: [StoryBand]) -> String {
        let items = bands.map { b in
            "{\"bandID\":\"\(b.bandID)\",\"label\":\"\(b.label)\",\"color\":\"\(b.color)\",\"proportion\":\(b.proportion)}"
        }.joined(separator: ",")
        return "{\"bands\":[\(items)]}"
    }
}

// MARK: — TimelineStripView

struct TimelineStripView: View {

    var model: TimelineViewModel
    var engine: ScriviEngine
    var projectRootPath: String
    var authorshipRef: AuthorshipRef?
    // T-0173: shared selection state. `loader.viewportSceneID` is the single source of truth
    // for the currently-selected scene across the Navigator, manuscript, and this timeline.
    var loader: ViewportSceneLoader?
    // T-0173: invoked when a scene dot is clicked — drives manuscript navigation (which also
    // sets viewportSceneID, so the Navigator highlights automatically).
    var onSelectScene: ((String) -> Void)? = nil

    @State private var showEpochOffsetDialog = false
    // SP-129/T-0503 — editing an ALREADY-IMPORTED timeline's epoch offset. Non-nil
    // means the dialog is in EDIT mode for that timelineID; nil means import mode.
    // ⚠️ Mirrors Linux's `onEditImportedOffsetRequested`, which is the finished surface.
    @State private var editingOffsetTimelineID: String?
    @State private var pendingImportJSON: String = ""
    @State private var pendingImportName: String = ""
    @State private var pendingImportEpochLabel: String = ""
    @State private var showHistoricalEventEditor = false
    @State private var editingEventID: String? = nil
    @State private var editingEventTitle: String = ""
    @State private var editingEventDescription: String = ""
    // Tracks the right-click X position (in usable coords) for "New Historical Event Here"
    @State private var contextMenuClickX: CGFloat = 0

    @State private var panelHeight: CGFloat = 120
    private let minPanelHeightBase: CGFloat = 64
    private let labelRowHeight: CGFloat = 24
    // I-0045: scene + main-row historical dots halved (was 7) to cut cluster clipping/overlap.
    private let dotRadius: CGFloat = 3.5
    // Imported-row dots were already small; keep them near their prior absolute size,
    // slightly smaller than scene dots. (Previously dotRadius * 0.7 = 4.9.)
    private let importedDotRadius: CGFloat = 3.0
    private let lineThickness: CGFloat = 2

    private let importedRowHeight: CGFloat = 32

    // MARK: Zoom & pan state (FR-009, FR-032)
    // zoomFactor 1.0 = full span visible; higher values shrink the visible window so dots spread apart.
    // magnifyGestureScale accumulates during a pinch gesture and is folded into zoomFactor on end.
    @State private var zoomFactor: CGFloat = 1.0
    @GestureState private var magnifyGestureScale: CGFloat = 1.0
    // scrollOffsetFraction 0.0 = leftmost edge of full span; 1.0 = rightmost.
    @State private var scrollOffsetFraction: CGFloat = 0.0

    // I-0045: max zoom is data-driven, not a fixed 50. For year-spanning timelines a cap of
    // 50 leaves a 24-hour cluster unresolvable (visible span stays ~weeks wide). We compute
    // the zoom needed to shrink the visible span down to the smallest scene gap so any
    // co-located cluster can always be pulled apart, whatever the total span.
    private let minVisibleSpanFloorMs: Int64 = 60 * 60 * 1000   // 1 hour — finest useful window
    private let maxZoomCeiling: CGFloat = 100_000               // safety bound

    private var maxZoom: CGFloat {
        let span = model.spanMs
        guard span > 0 else { return 1.0 }
        // Smallest gap between adjacent scene/historical offsets on the main row.
        let target = max(minVisibleSpanFloorMs, smallestMainRowGapMs())
        let z = CGFloat(span) / CGFloat(max(target, 1))
        return max(1.0, min(maxZoomCeiling, z))
    }

    // Smallest non-zero gap between consecutive main-row item offsets (scenes + historical
    // events), so max zoom can always separate the tightest pair by more than one diameter.
    //
    // I-0204: this used to BUILD AND SORT a 1,156-element array on every call, and `maxZoom`
    // calls it — so it ran thousands of times per layout pass. The sort now happens once per
    // model mutation (`TimelineViewModel.recomputeDerivedBounds`); this reads the result.
    // The model reports 0 for "no non-zero gap"; the floor stays here because the view owns it.
    private func smallestMainRowGapMs() -> Int64 {
        let gap = model.smallestMainRowGapMs
        return gap == 0 ? minVisibleSpanFloorMs : gap
    }

    // Effective zoom including any in-progress pinch gesture.
    private var effectiveZoom: CGFloat {
        max(1.0, min(maxZoom, zoomFactor * magnifyGestureScale))
    }
    // Visible span in milliseconds at the current zoom level.
    private func visibleSpanMs() -> Int64 {
        max(Int64(CGFloat(model.spanMs) / effectiveZoom), 1)
    }
    // Leftmost visible offset in milliseconds.
    private func visibleMinMs() -> Int64 {
        let slack = CGFloat(model.spanMs) - CGFloat(visibleSpanMs())
        return model.minOffsetMs + Int64(scrollOffsetFraction * slack)
    }


    // I-0045: the vertical room (px above the timeline line) the tallest current cluster
    // needs. Recomputed inside the GeometryReader as clusters are built; drives auto-grow
    // of the panel so ring stacks don't clip into neighbouring rows or off the top edge.
    @State private var requiredClusterHeight: CGFloat = 0

    // T-0174: aggregate-dot popover. A co-located group of ≥2 collapses to one dot; hovering
    // it opens a popover with a circle-of-dots. The popover stays open (hover is just the
    // opener) and dismisses on outside-click or member selection. Keyed by centerItemID.
    @State private var openAggregateID: String? = nil
    /// I-0212 — title of the member hovered inside the open aggregate popover.
    @State private var hoveredMemberTitle: String? = nil
    // Imported-row aggregate popover, keyed by "timelineID:centerEventID".
    @State private var openImportedAggregateID: String? = nil
    // Aggregate core dot is notably larger than a regular dot (dotRadius 3.5) so the count
    // number fits and it reads clearly as a group, distinct from placement / band rings.
    private let aggregateDotRadius: CGFloat = 11
    // I-0208 — visual separation between adjacent aggregate dots. With
    // `aggregateDotRadius`, this sets the bucket pitch and therefore how many distinct
    // aggregates fit across the width (design §3.1; gap value is design Q2, not yet ruled).
    private let clusterBucketGap: CGFloat = 4
    /// I-0210 — the full hit frame of `AggregateDotView`, which is what must not overlap.
    /// Kept in step with that view's own `.frame(width: ringDiameter + 8, …)`, where
    /// `ringDiameter = radius * 2 + 12`. Slice pitch derives from THIS, never from the
    /// visual dot diameter, or adjacent aggregates occlude one another's hit areas.
    private var aggregateHitFrame: CGFloat {
        AggregateDotView<EmptyView>.hitFrame(radius: aggregateDotRadius)
    }

    private var minPanelHeight: CGFloat {
        let bandExtra: CGFloat = model.activeBands.isEmpty ? 0 : labelRowHeight
        let importedExtra = CGFloat(model.importedTimelines.filter(\.visible).count) * importedRowHeight
        // I-0045: grow to fit the tallest cluster's ring stack (plus the badge cap above it).
        let clusterExtra = requiredClusterHeight > 0 ? requiredClusterHeight + 12 : 0
        return minPanelHeightBase + bandExtra + importedExtra + clusterExtra
    }


    /// [I-0214] Says what the core REFUSED to load, instead of drawing nothing.
    ///
    /// ⚠️ **PASSIVE AND NON-BLOCKING, like `WorldWarningView`** — never a sheet, never an
    /// alert, and it takes no focus from the manuscript. ⛔ **Nothing destructive lives
    /// here:** the files are the writer's, they may be hand-authored or from another
    /// tool, and this surface must not offer to delete what it merely failed to read.
    ///
    /// ⚠️ It names the FILE, because "an import failed" is unactionable — the writer
    /// needs to know WHICH one.
    @ViewBuilder
    private var rejectedImportsBanner: some View {
        if !model.rejectedImports.isEmpty {
            VStack(alignment: .leading, spacing: 2) {
                HStack(spacing: 6) {
                    Image(systemName: "exclamationmark.triangle.fill")
                        .foregroundStyle(.orange)
                        .accessibilityHidden(true)
                    Text(model.rejectedImports.count == 1
                         ? "1 imported timeline could not be loaded"
                         : "\(model.rejectedImports.count) imported timelines could not be loaded")
                        .font(.caption).fontWeight(.medium)
                    Spacer(minLength: 8)
                }
                ForEach(model.rejectedImports, id: \.path) { rej in
                    Text("\((rej.path as NSString).lastPathComponent) — \(rej.reason)")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                        .lineLimit(2)
                        .fixedSize(horizontal: false, vertical: true)
                }
            }
            .padding(.horizontal, 10)
            .padding(.vertical, 6)
            .frame(maxWidth: .infinity, alignment: .leading)
            .background(.orange.opacity(0.10))
            .overlay(alignment: .bottom) { Divider() }
            .accessibilityElement(children: .contain)
        }
    }

    var body: some View {
        VStack(spacing: 0) {
            topEdgeHandle
            Divider()
            panelHeader
            rejectedImportsBanner
            GeometryReader { geo in
                let panelW = geo.size.width
                let usable = panelW - 32
                // If bands are active, reserve the top for the label row.
                let contentTop: CGFloat = model.activeBands.isEmpty ? 0 : labelRowHeight
                let contentH = geo.size.height - contentTop
                // When imported timelines are visible the project row shifts up so the
                // entire row group (project + imported) is centred in contentH.
                let visibleImportedCount = model.importedTimelines.filter(\.visible).count
                let rowSpacing: CGFloat = 32
                let lineY = contentTop + (contentH - CGFloat(visibleImportedCount) * rowSpacing) / 2

                // I-0045: tallest ring stack across the main row and all imported rows at the
                // current zoom. Drives auto-grow (requiredClusterHeight) so clusters never clip.
                let tallestStack = tallestClusterStack(usable: usable, panelW: panelW)

                ZStack(alignment: .topLeading) {
                    // I-0036: hit-testable background so the ZStack-level context menu fires
                    // on empty space. Must be first (bottom layer) so all child views on top
                    // of it win hit-testing when clicked directly.
                    Color.clear.contentShape(Rectangle())

                    // Band background — drawn behind everything.
                    // I-0048: the band region spans the FIRST→LAST scene in story time, mapped
                    // through eventX so it zooms and pans with the timeline. Hidden when there
                    // is no usable scene range (0–1 scene).
                    if !model.activeBands.isEmpty, let range = model.structureRange {
                        let regionX = eventX(offsetMs: range.startMs, usable: usable, panelW: panelW)
                        let regionEndX = eventX(offsetMs: range.endMs, usable: usable, panelW: panelW)
                        let _ = NSLog("BANDDIAG P1-parent regionX=\(regionX) regionEndX=\(regionEndX) zoom=\(effectiveZoom) scroll=\(scrollOffsetFraction)")
                        BandOverlayView(
                            bands: model.activeBands,
                            regionX: regionX,
                            regionWidth: max(regionEndX - regionX, 1),
                            panelWidth: panelW,
                            panelHeight: geo.size.height,
                            labelRowHeight: labelRowHeight,
                            // I-0048: convert an absolute panel-X back to a fraction within the
                            // band region so border-drag edits map through story time.
                            fractionForPanelX: { px in
                                let f = (px - regionX) / max(regionEndX - regionX, 1)
                                return Double(min(max(f, 0), 1))
                            },
                            onBorderDragged: { updatedBands in
                                model.updateBandLayout(updatedBands, engine: engine,
                                                        projectRootPath: projectRootPath)
                            }
                        )
                    }

                    // Timeline line
                    Rectangle()
                        .fill(Color.secondary.opacity(0.35))
                        .frame(width: max(usable, 1), height: lineThickness)
                        .position(x: panelW / 2, y: lineY)

                    // Empty state
                    if model.dots.isEmpty {
                        Text("Scenes will appear here as you write.")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                            .position(x: panelW / 2, y: lineY)
                    }

                    // Main-row dots — scene and historical event dots clustered together.
                    // T-0174: a co-located group of ≥2 renders as a single AggregateDotView
                    // whose hover opens a popover containing a circle-of-dots. A group of 1
                    // renders as the normal individual dot.
                    let clusters = buildClusters(usable: usable, panelW: panelW)
                    ForEach(clusters, id: \.centerItemID) { cluster in
                        // I-0208 — place the aggregate at its SLICE anchor, not at its
                        // first member. A cluster is a fixed partition of the story, so
                        // its dot must hold position as membership varies; anchoring to
                        // a member would make it jitter whenever the set changed.
                        // A single-member cluster still draws at the member's own x, so
                        // a lone dot sits exactly on its scene's story time.
                        let centerX = cluster.members.count == 1
                            ? itemX(cluster.members[0], usable: usable, panelW: panelW)
                            : eventX(offsetMs: cluster.anchorOffsetMs, usable: usable, panelW: panelW)
                        if cluster.members.count == 1 {
                            memberDot(cluster.members[0], at: CGPoint(x: centerX, y: lineY),
                                      usable: usable, panelW: panelW)
                        } else {
                            AggregateDotView(
                                members: cluster.members,
                                radius: aggregateDotRadius,
                                selectedSceneID: loader?.viewportSceneID,
                                isPopoverOpen: openAggregateID == cluster.centerItemID,
                                onHover: {
                                    if ScriviDiag.timingEnabled {
                                        NSLog("[SCRIVI-TL] hover aggregate id=%@ x=%.1f members=%d open=%@",
                                              cluster.centerItemID as NSString,
                                              Double(centerX), cluster.members.count,
                                              (openAggregateID ?? "nil") as NSString)
                                    }
                                    // ⚠️ I-0210 — DISMISS THE OPEN POPOVER BEFORE PRESENTING THE NEXT.
                                    //
                                    // `openAggregateID` is a SINGLE SLOT. Overwriting it directly
                                    // moved presentation to the new dot while the old popover was
                                    // still tearing down — SwiftUI will not present a second
                                    // popover mid-dismissal, so the new one silently never
                                    // appeared. Whether it worked depended on timing, which is
                                    // the ALTERNATING failure the user reported: hover A (opens),
                                    // hover B (swallowed), hover C (opens)…
                                    //
                                    // ⚠️ The direct write also BYPASSED `onPopoverDismiss`, which
                                    // fires only from the binding's setter — so the old dot never
                                    // learned it had been closed.
                                    //
                                    // Clearing first, then setting on the next runloop turn, gives
                                    // the dismissal a chance to complete.
                                    if openAggregateID != nil,
                                       openAggregateID != cluster.centerItemID {
                                        openAggregateID = nil
                                        DispatchQueue.main.async {
                                            openAggregateID = cluster.centerItemID
                                        }
                                    } else {
                                        openAggregateID = cluster.centerItemID
                                    }
                                },
                                popoverContent: {
                                    AggregateMembersPopover(
                                        members: cluster.members,
                                        selectedSceneID: loader?.viewportSceneID,
                                        dotRadius: dotRadius,
                                        memberContent: { item, isSelected in
                                            popoverMemberDot(item, isSelected: isSelected,
                                                             onSelect: {
                                                                 selectMember(item)
                                                                 openAggregateID = nil
                                                                 hoveredMemberTitle = nil
                                                             },
                                                             onHoverChanged: { hovered in
                                                                 hoveredMemberTitle =
                                                                     hovered ? memberLabel(item) : nil
                                                             })
                                        },
                                        hoveredTitle: hoveredMemberTitle
                                    )
                                },
                                onPopoverDismiss: {
                                    openAggregateID = nil
                                    hoveredMemberTitle = nil
                                }
                            )
                            .position(x: centerX, y: lineY)
                        }
                    }

                    // Imported timeline rows — centred as a group with the project row.
                    // lineY already accounts for the group offset; each imported row sits
                    // one rowSpacing step below the previous, starting one step below lineY.
                    let visibleImported = model.importedTimelines.filter(\.visible)
                    ForEach(Array(visibleImported.enumerated()),
                            id: \.element.id) { rowIdx, timeline in
                        let rowY = lineY + CGFloat(rowIdx + 1) * rowSpacing
                        // Row label
                        Text(timeline.sourceName)
                            .font(.system(size: 9))
                            .foregroundStyle(timeline.swiftUIColor)
                            .lineLimit(1)
                            .position(x: 50, y: rowY)
                            // SP-129/T-0503 — the imported row's own actions. Until now
                            // the epoch offset could only be set AT IMPORT, so a writer
                            // who misjudged it had to remove and re-import.
                            .contextMenu {
                                Button("Adjust Epoch Offset…") {
                                    pendingImportName       = timeline.sourceName
                                    pendingImportEpochLabel = timeline.epochLabel
                                    editingOffsetTimelineID = timeline.timelineID
                                    showEpochOffsetDialog   = true
                                }
                                Divider()
                                Button("Hide This Timeline") {
                                    model.setImportedTimelineVisible(
                                        timelineID: timeline.timelineID, visible: false,
                                        engine: engine, projectRootPath: projectRootPath)
                                }
                                Button("Remove Imported Timeline") {
                                    model.removeImportedTimeline(
                                        timelineID: timeline.timelineID,
                                        engine: engine, projectRootPath: projectRootPath)
                                }
                            }
                        // Dividing line
                        Rectangle()
                            .fill(Color.secondary.opacity(0.2))
                            .frame(width: max(usable, 1), height: 1)
                            .position(x: panelW / 2, y: rowY)
                        // Event dots — clustered per row so co-located events don't overlap.
                        let importedRadius = importedDotRadius
                        let visibleEvents = timeline.events.filter {
                            let x = eventX(offsetMs: $0.projectOffsetMs, usable: usable, panelW: panelW)
                            return x >= 16 && x <= panelW - 16
                        }
                        let importedClusters = buildImportedRowClusters(
                            events: visibleEvents, usable: usable, panelW: panelW,
                            radius: importedRadius)
                        // T-0174: ≥2 co-located imported events render as one aggregate dot
                        // whose hover opens a circle-of-dots popover (no selection segment —
                        // imported events aren't selectable scenes).
                        ForEach(importedClusters, id: \.centerEventID) { iCluster in
                            let iCenterX = eventX(offsetMs: iCluster.members[0].projectOffsetMs,
                                                  usable: usable, panelW: panelW)
                            let aggKey = "\(timeline.timelineID):\(iCluster.centerEventID)"
                            if iCluster.members.count == 1 {
                                let ev  = iCluster.members[0]
                                let key = "\(timeline.timelineID):\(ev.eventID)"
                                ImportedEventDotView(
                                    ev: ev, color: timeline.swiftUIColor, radius: importedRadius,
                                    onHoverChanged: { hovered in
                                        model.hoveredImportedEventKey = hovered ? key : nil
                                    }
                                )
                                .position(x: iCenterX, y: rowY)
                            } else {
                                ImportedAggregateDotView(
                                    members: iCluster.members,
                                    count: iCluster.members.count,
                                    color: timeline.swiftUIColor,
                                    radius: importedRadius + 4,
                                    dotRadius: importedRadius,
                                    isPopoverOpen: openImportedAggregateID == aggKey,
                                    // I-0210 — same single-slot handoff as the main row above:
                                    // dismiss the open popover before presenting the next, or
                                    // SwiftUI swallows the second presentation.
                                    onHover: {
                                        if openImportedAggregateID != nil,
                                           openImportedAggregateID != aggKey {
                                            openImportedAggregateID = nil
                                            DispatchQueue.main.async {
                                                openImportedAggregateID = aggKey
                                            }
                                        } else {
                                            openImportedAggregateID = aggKey
                                        }
                                    },
                                    onPopoverDismiss: { openImportedAggregateID = nil }
                                )
                                .position(x: iCenterX, y: rowY)
                            }
                        }
                    }
                }
                // I-0036: context menu on the ZStack itself — child views' own context menus
                // take priority when the right-click lands on them; this menu fires only on
                // empty space. No Color.clear overlay needed (and it must never be used —
                // a full-panel Color.clear.contentShape blocks all child hit-testing).
                .onContinuousHover { phase in
                    if case .active(let loc) = phase {
                        contextMenuClickX = loc.x
                    }
                }
                .contextMenu {
                    Button("New Historical Event Here") {
                        guard let ref = authorshipRef else { return }
                        let clickMs = offsetMs(fromPanelX: contextMenuClickX, usable: usable)
                        model.createHistoricalEvent(at: clickMs, engine: engine,
                                                    projectRootPath: projectRootPath,
                                                    authorshipRef: ref)
                        // Open editor immediately so user can name the new event
                        if let newest = model.historicalEvents.last {
                            editingEventID          = newest.eventID
                            editingEventTitle       = newest.title
                            editingEventDescription = newest.description
                            showHistoricalEventEditor = true
                        }
                    }
                    .disabled(authorshipRef == nil)
                    Divider()
                    Button("Import Timeline…") { triggerImport() }
                    Button("Export Timeline…") { triggerExport() }
                    Divider()
                    Button("Set Epoch Label…") { }
                        .disabled(true)
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .clipped()
                // All tooltips rendered outside .clipped() so they are never cut off.
                .overlay(alignment: .topLeading) {
                    // Scene dot tooltip
                    if let dotID = model.hoveredDotID,
                       let dot = model.dots.first(where: { $0.sceneID == dotID }) {
                        let startX    = dotX(for: dot, usable: usable, panelW: panelW)
                        let bandCol   = bandColor(for: dot)
                        let prevTitle = previousSceneTitle(for: dot)
                        let prevEnd   = previousSceneEndMs(for: dot)
                        DotTooltipView(
                            dot: dot,
                            startX: startX,
                            lineY: lineY,
                            panelWidth: panelW,
                            bands: model.activeBands,
                            bandRingColor: bandCol,
                            previousSceneTitle: prevTitle,
                            previousSceneEndMs: prevEnd,
                            epochLabel: model.epochLabel
                        )
                        .allowsHitTesting(false)
                        .transition(.opacity.animation(.easeOut(duration: 0.1)))
                    }
                    // Historical event dot tooltip
                    if let eid = model.hoveredHistoricalEventID,
                       let event = model.historicalEvents.first(where: { $0.eventID == eid }) {
                        let ex = eventX(offsetMs: event.offsetMs, usable: usable, panelW: panelW)
                        SimpleTooltipView(
                            title: event.title,
                            subtitle: humanReadableDuration(ms: event.offsetMs,
                                                            epochLabel: model.epochLabel),
                            anchorX: ex,
                            lineY: lineY,
                            panelWidth: panelW
                        )
                        .allowsHitTesting(false)
                        .transition(.opacity.animation(.easeOut(duration: 0.1)))
                    }
                    // Imported event dot tooltip
                    if let key = model.hoveredImportedEventKey {
                        let parts = key.split(separator: ":", maxSplits: 1).map(String.init)
                        if parts.count == 2,
                           let tl = model.importedTimelines.first(where: { $0.timelineID == parts[0] }),
                           let ev = tl.events.first(where: { $0.eventID == parts[1] }) {
                            let ex = eventX(offsetMs: ev.projectOffsetMs, usable: usable, panelW: panelW)
                            let visIdx = model.importedTimelines.filter(\.visible)
                                .firstIndex(where: { $0.timelineID == tl.timelineID }) ?? 0
                            let rowY = lineY + CGFloat(visIdx + 1) * rowSpacing
                            SimpleTooltipView(
                                title: ev.title,
                                subtitle: tl.sourceName,
                                anchorX: ex,
                                lineY: rowY,
                                panelWidth: panelW
                            )
                            .allowsHitTesting(false)
                            .transition(.opacity.animation(.easeOut(duration: 0.1)))
                        }
                    }
                }
                // I-0045: feed the tallest current ring stack to the auto-grow state.
                //
                // ⚠️ T-0546 — THIS WRITE CLOSES A LAYOUT CYCLE AND MUST BE DEFERRED.
                //
                // The loop: GeometryReader reads the width → `tallestClusterStack(usable:)`
                // re-clusters → this writes `requiredClusterHeight` → `minPanelHeight` grows
                // → the strip is a `safeAreaBar` (SP-135), so the content inset changes → the
                // Inspector is a real `.inspector` column (SP-136), so
                // `SplitViewChildController` reports a new min/max → layout invalidates →
                // the width is re-read. AppKit counts the passes and throws:
                //
                //   NSGenericException: … more Update Constraints in Window passes than
                //   there are views in the window
                //   … SplitViewChildController.hostingView(_:didUpdateMinSize:maxSize:)
                //
                // ⚠️ NEITHER SPRINT CAUSED THIS ALONE. SP-135 made the strip a bar; SP-136
                // made the Inspector a column. Each was fine; together they closed the ring.
                // ⛔ The log tells the story: `usable` oscillated 222 → 264 → 232 → 167 and
                // repeated, three stable states, forever.
                //
                // ✅ `Task { @MainActor }` moves the write OUT of the current layout pass, so
                // the height settles on the next runloop turn instead of re-entering this one.
                // ⚠️ The guard is what makes it terminate: once the height stops changing the
                // chain stops, so the deferral costs one extra pass, not a permanent tick.
                // ⛔ Do not "simplify" this back to a direct assignment.
                .onChange(of: tallestStack, initial: true) { _, newValue in
                    guard requiredClusterHeight != newValue else { return }
                    Task { @MainActor in
                        guard requiredClusterHeight != newValue else { return }
                        requiredClusterHeight = newValue
                    }
                }
                // T-0173: when the selected scene changes elsewhere (Navigator click, manuscript
                // cursor), pan the timeline so the matching dot is visible. The highlight itself
                // is driven by isSelected on each dot.
                .onChange(of: loader?.viewportSceneID) { _, newID in
                    if let id = newID { revealScene(id) }
                }
                // Pinch-to-zoom (FR-009): MagnifyGesture adjusts zoomFactor.
                // magnifyGestureScale live-tracks during pinch; zoomFactor is committed on end.
                .simultaneousGesture(
                    MagnifyGesture()
                        .updating($magnifyGestureScale) { value, state, _ in
                            state = value.magnification
                        }
                        .onEnded { value in
                            let newZoom = max(1.0, min(maxZoom, zoomFactor * value.magnification))
                            // Recentre the visible window so the current visible centre stays fixed.
                            let oldSpan = CGFloat(model.spanMs) / effectiveZoom
                            let newSpan = CGFloat(model.spanMs) / newZoom
                            let centreFraction = scrollOffsetFraction + 0.5 * oldSpan / CGFloat(model.spanMs)
                            let newStart = centreFraction - 0.5 * newSpan / CGFloat(model.spanMs)
                            scrollOffsetFraction = max(0, min(1, newStart))
                            zoomFactor = newZoom
                        }
                )
                // Scroll-wheel capture: pan (horizontal) when zoomed, zoom (vertical/⌘) via trackpad (FR-009).
                .background {
                    #if os(macOS)
                    BandPointerProbeView()   // BANDDIAG: passive raw-pointer observer
                    TimelineScrollCaptureView(
                        onScroll: { dx, dy in
                            if NSEvent.modifierFlags.contains(.command) || abs(dy) > abs(dx) {
                                let factor = 1.0 - dy * 0.04
                                let newZoom = max(1.0, min(maxZoom, zoomFactor * factor))
                                let oldSpan = CGFloat(model.spanMs) / effectiveZoom
                                let newSpan = CGFloat(model.spanMs) / newZoom
                                let centre  = scrollOffsetFraction + 0.5 * oldSpan / CGFloat(model.spanMs)
                                let newStart = centre - 0.5 * newSpan / CGFloat(model.spanMs)
                                scrollOffsetFraction = max(0, min(1, newStart))
                                zoomFactor = newZoom
                            } else if zoomFactor > 1.0 {
                                // I-0047: scrollOffsetFraction spans the FULL timeline, but the
                                // visible window is only 1/zoom of it — so a raw dx/usable moves
                                // the content by dx*zoom px (far too fast when zoomed in). Divide
                                // by effectiveZoom so dx pixels of finger travel pans dx pixels.
                                let usable = geo.size.width - 32
                                let panFraction = -dx / usable / effectiveZoom
                                scrollOffsetFraction = max(0, min(1, scrollOffsetFraction + panFraction))
                            }
                        }
                    )
                    #endif
                }
            }
            .frame(height: max(minPanelHeight, panelHeight))
        }
    }

    // MARK: Panel header — "Timeline" label + Structure selector + Timeline menu

    private var panelHeader: some View {
        HStack {
            Text("Timeline")
                .font(.caption)
                .foregroundStyle(.secondary)
                .padding(.leading, 12)
            Spacer()
            timelineMenu
                .padding(.trailing, 4)
            structureMenu
                .padding(.trailing, 8)
        }
        .frame(height: 28)
        #if os(macOS)
        .background(Color(nsColor: .windowBackgroundColor).opacity(0.95))
        #else
        .background(Color(.systemBackground).opacity(0.95))
        #endif
        .sheet(isPresented: $showEpochOffsetDialog) {
            EpochOffsetDialog(
                sourceName: pendingImportName,
                sourceEpochLabel: pendingImportEpochLabel,
                onCommit: { offsetMs, greyShade in
                    // SP-129/T-0503: the SAME dialog serves import and edit. ⚠️ In edit
                    // mode the grey shade is NOT reassigned — it is how the writer
                    // recognises the row, and changing it on an offset edit would be a
                    // silent second change she did not ask for.
                    if let editID = editingOffsetTimelineID {
                        _ = try? engine.updateImportedTimelineOffset(
                            projectRootPath: projectRootPath,
                            timelineID: editID,
                            epochOffsetMs: offsetMs)
                    } else {
                        _ = try? engine.importExternalTimeline(
                            projectRootPath: projectRootPath,
                            timelineJSON: pendingImportJSON,
                            epochOffsetMs: offsetMs,
                            assignedGreyShade: greyShade)
                    }
                    model.loadImportedTimelines(projectRootPath: projectRootPath, engine: engine)
                    editingOffsetTimelineID = nil
                    showEpochOffsetDialog = false
                },
                onCancel: {
                    editingOffsetTimelineID = nil
                    showEpochOffsetDialog = false
                }
            )
        }
        .sheet(isPresented: $showHistoricalEventEditor) {
            HistoricalEventEditorSheet(
                title: $editingEventTitle,
                description: $editingEventDescription,
                onSave: {
                    if let eid = editingEventID {
                        model.updateHistoricalEvent(
                            eventID: eid, title: editingEventTitle,
                            description: editingEventDescription,
                            engine: engine, projectRootPath: projectRootPath)
                    }
                    showHistoricalEventEditor = false
                },
                onCancel: { showHistoricalEventEditor = false }
            )
        }
    }

    @ViewBuilder
    private var timelineMenu: some View {
        Menu {
            Button("Import Timeline…") { triggerImport() }
            Button("Export Timeline…") { triggerExport() }
            if !model.importedTimelines.isEmpty {
                Divider()
                // Toggle inside a Menu renders as a native macOS checkmark menu item.
                ForEach(model.importedTimelines) { tl in
                    Toggle(tl.sourceName, isOn: Binding(
                        get: { tl.visible },
                        set: { newValue in
                            model.setImportedTimelineVisible(
                                timelineID: tl.timelineID, visible: newValue,
                                engine: engine, projectRootPath: projectRootPath)
                        }
                    ))
                }
                Divider()
                ForEach(model.importedTimelines) { tl in
                    Button(role: .destructive) {
                        model.removeImportedTimeline(timelineID: tl.timelineID,
                                                     engine: engine,
                                                     projectRootPath: projectRootPath)
                    } label: {
                        Text("Remove \(tl.sourceName)")
                    }
                }
            }
        } label: {
            Image(systemName: "ellipsis.circle")
                .font(.system(size: 12))
                .foregroundStyle(.secondary)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }

    @ViewBuilder
    private var structureMenu: some View {
        Menu {
            // None / remove
            Button("None") {
                model.removeStructure(engine: engine, projectRootPath: projectRootPath)
            }
            Divider()
            // Built-in structures (exclude custom for now — Custom… is a future feature)
            ForEach(BuiltInStructure.allCases.filter { $0 != .custom }) { s in
                Button(s.displayName) {
                    model.applyStructure(s.rawValue, engine: engine,
                                         projectRootPath: projectRootPath)
                }
            }
            Divider()
            Button("Custom…") { }
                .disabled(true)
        } label: {
            HStack(spacing: 3) {
                Text(structureMenuLabel)
                    .font(.caption)
                Image(systemName: "chevron.down")
                    .font(.system(size: 9, weight: .semibold))
            }
            .foregroundStyle(.secondary)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }

    private var structureMenuLabel: String {
        if model.activeStructureID.isEmpty { return "Structure" }
        return BuiltInStructure(rawValue: model.activeStructureID)?.displayName ?? "Structure"
    }

    // MARK: Resize handle

    private var topEdgeHandle: some View {
        Color.clear
            .frame(height: 6)
            .contentShape(Rectangle())
            #if os(macOS)
            .cursor(.resizeUpDown)
            #endif
            .gesture(
                DragGesture(minimumDistance: 1)
                    .onChanged { v in
                        panelHeight = max(minPanelHeight, panelHeight - v.translation.height)
                    }
            )
    }

    // I-0048: band region (panel coords) from the first→last scene story-time range, mapped
    // through eventX. 0 when there's no usable range (then dots fall back to full-panel logic).
    private func bandRegionX(usable: CGFloat, panelW: CGFloat) -> CGFloat {
        guard let r = model.structureRange else { return 0 }
        return eventX(offsetMs: r.startMs, usable: usable, panelW: panelW)
    }
    private func bandRegionWidth(usable: CGFloat, panelW: CGFloat) -> CGFloat {
        guard let r = model.structureRange else { return 0 }
        let x0 = eventX(offsetMs: r.startMs, usable: usable, panelW: panelW)
        let x1 = eventX(offsetMs: r.endMs, usable: usable, panelW: panelW)
        return max(x1 - x0, 1)
    }

    // MARK: T-0174 — member dot rendering (shared by resolved singles and fan-out)

    // Renders one main-row member (scene or historical event) as the normal individual dot
    // at the given panel position. Used both for size-1 clusters and for fanned-out members.
    @ViewBuilder
    private func memberDot(_ item: MainRowItem, at point: CGPoint,
                           usable: CGFloat, panelW: CGFloat) -> some View {
        let baseX = itemX(item, usable: usable, panelW: panelW)
        switch item {
        case .scene(let dot):
            let prevEndMs = previousSceneEndMs(for: dot)
            let prevTitle = previousSceneTitle(for: dot)
            let bandColor = bandColor(for: dot)
            SceneDotView(
                dot: dot,
                radius: dotRadius,
                epochLabel: model.epochLabel,
                startX: baseX,
                panelWidth: panelW,
                lineY: point.y,
                labelRowHeight: model.activeBands.isEmpty ? 0 : labelRowHeight,
                bands: model.activeBands,
                bandRingColor: bandColor,
                bandRegionX: bandRegionX(usable: usable, panelW: panelW),
                bandRegionWidth: bandRegionWidth(usable: usable, panelW: panelW),
                previousSceneEndMs: prevEndMs,
                previousSceneTitle: prevTitle,
                defaultDurationMs: model.defaultSceneDurationMs,
                computeOffsetMs: { finalPanelX in
                    offsetMs(fromPanelX: finalPanelX, usable: usable)
                },
                onCommit: { result, durationMs in
                    applyPickerResult(result, for: dot, pickerDurationMs: durationMs)
                },
                onAssignToBand: { bandID in
                    model.assignToBand(sceneID: dot.sceneID, bandID: bandID,
                                       engine: engine, projectRootPath: projectRootPath)
                },
                onUnassign: {
                    model.unassignFromBand(sceneID: dot.sceneID, engine: engine,
                                            projectRootPath: projectRootPath)
                },
                onHoverChanged: { hovered in
                    model.hoveredDotID = hovered ? dot.sceneID : nil
                },
                isSelected: loader?.viewportSceneID == dot.sceneID,
                onSelect: { onSelectScene?(dot.sceneID) },
                aggregateJoinOffsetMs: { finalX in
                    aggregateJoinOffsetMs(finalPanelX: finalX, excludingSceneID: dot.sceneID,
                                          usable: usable, panelW: panelW)
                }
            )
            .position(x: point.x, y: point.y)
        case .historical(let event):
            HistoricalEventDotView(
                event: event,
                radius: dotRadius,
                startX: baseX,
                panelWidth: panelW,
                lineY: point.y,
                epochLabel: model.epochLabel,
                onDragEnd: { newOffsetMs in
                    model.updateHistoricalEventOffset(
                        eventID: event.eventID, offsetMs: newOffsetMs,
                        engine: engine, projectRootPath: projectRootPath)
                },
                onEdit: {
                    editingEventID = event.eventID
                    editingEventTitle = event.title
                    editingEventDescription = event.description
                    showHistoricalEventEditor = true
                },
                onDelete: {
                    model.deleteHistoricalEvent(
                        eventID: event.eventID, engine: engine,
                        projectRootPath: projectRootPath)
                },
                computeOffsetMs: { finalX in
                    offsetMs(fromPanelX: finalX, usable: usable)
                },
                onHoverChanged: { hovered in
                    model.hoveredHistoricalEventID = hovered ? event.eventID : nil
                }
            )
            .position(x: point.x, y: point.y)
        }
    }

    // T-0174: one non-draggable member dot for the aggregate popover. Looks like the timeline
    // dot (band ring, placement ring, selection highlight); grows on hover with a native
    // tooltip; click selects and dismisses. No drag, no context menu, no picker.
    /// I-0212 — the human-readable label for a popover member, used by the caption.
    /// Scenes carry their chapter so a repeated scene title is still locatable;
    /// historical events say what they are, since they have no chapter.
    private func memberLabel(_ item: MainRowItem) -> String {
        switch item {
        case .scene(let dot):
            let title = dot.title.isEmpty ? "Untitled scene" : dot.title
            return dot.chapterTitle.isEmpty ? title : "\(title) — \(dot.chapterTitle)"
        case .historical(let event):
            let title = event.title.isEmpty ? "Historical event" : event.title
            return "\(title) — historical event"
        }
    }

    @ViewBuilder
    private func popoverMemberDot(_ item: MainRowItem, isSelected: Bool,
                                  onSelect: @escaping () -> Void,
                                  onHoverChanged: @escaping (Bool) -> Void = { _ in }) -> some View {
        switch item {
        case .scene(let dot):
            PopoverMemberDotView(
                radius: dotRadius,
                fill: Color.accentColor,
                isSelected: isSelected,
                bandRingColor: bandColor(for: dot),
                placementSource: dot.offsetSource,
                tooltip: dot.title.isEmpty ? "Untitled scene" : dot.title,
                subtitle: dot.chapterTitle,
                onSelect: onSelect,
                onHoverChanged: onHoverChanged
            )
        case .historical(let event):
            PopoverMemberDotView(
                radius: dotRadius,
                fill: Color(hex: "#C8A97A") ?? .orange,
                isSelected: false,
                bandRingColor: nil,
                placementSource: "manual",
                tooltip: event.title.isEmpty ? "Historical event" : event.title,
                subtitle: "Historical event",
                onSelect: {},                      // historical events aren't selectable scenes
                onHoverChanged: onHoverChanged
            )
        }
    }

    // MARK: Coordinate math

    private func dotX(for dot: TimelineViewModel.SceneDot,
                      usable: CGFloat, panelW: CGFloat) -> CGFloat {
        let vSpan = visibleSpanMs()
        let vMin  = visibleMinMs()
        guard vSpan > 0 else { return 16 + dotRadius }
        let fraction = CGFloat(dot.offsetMs - vMin) / CGFloat(vSpan)
        return 16 + fraction * usable
    }

    private func eventX(offsetMs: Int64, usable: CGFloat, panelW: CGFloat) -> CGFloat {
        let vSpan = visibleSpanMs()
        let vMin  = visibleMinMs()
        guard vSpan > 0 else { return 16 + dotRadius }
        let fraction = CGFloat(offsetMs - vMin) / CGFloat(vSpan)
        return 16 + fraction * usable
    }

    // T-0174: select a member from the aggregate popover. Scenes navigate (driving the
    // bidirectional selection); historical events are not selectable scenes — no-op for now.
    private func selectMember(_ item: MainRowItem) {
        if case .scene(let dot) = item { onSelectScene?(dot.sceneID) }
    }

    // T-0174: if `finalPanelX` lands on an existing aggregate (cluster of ≥2) that does not
    // already include `excludingSceneID`, return that aggregate's offsetMs (its snap target).
    private func aggregateJoinOffsetMs(finalPanelX: CGFloat, excludingSceneID: String,
                                       usable: CGFloat, panelW: CGFloat) -> Int64? {
        let snapRadius = aggregateDotRadius + dotRadius   // generous hit zone
        for cluster in buildClusters(usable: usable, panelW: panelW) where cluster.members.count > 1 {
            let alreadyMember = cluster.members.contains {
                if case .scene(let d) = $0 { return d.sceneID == excludingSceneID }
                return false
            }
            if alreadyMember { continue }
            // I-0208 — hit-test against the cluster's DRAWN position (its slice anchor),
            // not its first member. They diverge now that a cluster is a fixed partition,
            // and testing the member would snap to a point the dot does not occupy.
            let cx = eventX(offsetMs: cluster.anchorOffsetMs, usable: usable, panelW: panelW)
            if abs(cx - finalPanelX) <= snapRadius {
                return cluster.anchorOffsetMs
            }
        }
        return nil
    }

    // T-0173: pan the visible window so the given scene's dot is on-screen. If it already
    // falls inside the current visible span, do nothing (avoid yanking the view around).
    // Only meaningful when zoomed in; at zoom 1 the whole span is visible.
    private func revealScene(_ sceneID: String) {
        guard let dot = model.dots.first(where: { $0.sceneID == sceneID }) else { return }
        let vSpan = visibleSpanMs()
        let vMin  = visibleMinMs()
        if dot.offsetMs >= vMin && dot.offsetMs <= vMin + vSpan { return }
        // Centre the dot in the visible window.
        let desiredMin = dot.offsetMs - vSpan / 2
        let slack = CGFloat(model.spanMs) - CGFloat(vSpan)
        guard slack > 0 else { return }
        let fraction = CGFloat(desiredMin - model.minOffsetMs) / slack
        scrollOffsetFraction = max(0, min(1, fraction))
    }

    private func offsetMs(fromPanelX x: CGFloat, usable: CGFloat) -> Int64 {
        let fraction = (x - 16) / usable
        return visibleMinMs() + Int64(fraction * CGFloat(visibleSpanMs()))
    }

    // MARK: Clustering (FR-030–FR-035)

    // A single item on the main timeline row — either a scene dot or a historical event dot.
    enum MainRowItem {
        case scene(TimelineViewModel.SceneDot)
        case historical(HistoricalEventDot)

        var id: String {
            switch self {
            case .scene(let d): return "s:\(d.id)"
            case .historical(let e): return "h:\(e.id)"
            }
        }

        var offsetMs: Int64 {
            switch self {
            case .scene(let d): return d.offsetMs
            case .historical(let e): return e.offsetMs
            }
        }
    }

    struct DotCluster {
        let centerItemID: String
        let members: [MainRowItem]
        /// I-0208 — the story-time centre of this cluster's SLICE, not of its members.
        /// A cluster is a fixed partition of the story, so its anchor must not drift as
        /// membership varies; the view maps this through `eventX` to place the dot.
        let anchorOffsetMs: Int64
        var ringCount: Int { members.count <= 1 ? 0 : members.count <= 7 ? 1 : 2 }
    }

    // Groups all main-row items (scene dots + historical event dots) into clusters, sorted by
    // X, using a contiguous-window pass so grouping is transitive. T-0174: the absorption
    // threshold is size-aware — once the running cluster has ≥2 members it renders as a large
    // aggregate dot, so the next item must be within the AGGREGATE footprint (not just one
    // regular diameter) to be merged. This stops adjacent aggregates from overlapping when
    // their gap exceeds a regular diameter but is smaller than an aggregate's width.
    private func buildClusters(usable: CGFloat, panelW: CGFloat) -> [DotCluster] {
        // I-0204: decorate-sort-undecorate. `itemX` was previously called INSIDE the sort
        // comparator, so an n=1,156 sort made ~11,600 comparisons × 2 calls ≈ 23,000 `itemX`
        // calls — and each one re-derived the zoom by sorting the full offset array. Computing
        // x ONCE per item and sorting on the stored value makes it 1,156 calls.
        //
        // ⚠️ The x values are only valid for THIS (usable, panelW, zoom, scroll) combination,
        // which is exactly the scope of one call — they are deliberately local, not cached
        // across calls, because a pan or zoom changes every one of them.
        let allItems: [MainRowItem] = model.dots.map { .scene($0) }
            + model.historicalEvents.map { .historical($0) }
        var positioned: [(item: MainRowItem, x: CGFloat)] = allItems.map {
            ($0, itemX($0, usable: usable, panelW: panelW))
        }
        positioned.sort { $0.x < $1.x }

        // I-0208 ROUND 3 — BUCKETS PARTITION THE STORY, NOT THE VIEWPORT.
        //
        // ⚠️ TWO EARLIER ATTEMPTS WERE WRONG, BOTH FALSIFIED BY THE USER ON THE RIG:
        //   1. Proximity chaining was TRANSITIVE (A merges B, B merges C, unbounded),
        //      so 1,158 items at sub-pixel spacing became ONE dot for the whole story.
        //   2. Screen-space bucketing fixed the count but made buckets STATIONARY
        //      SCREEN SLOTS — the dots flowed THROUGH them as you panned, so a bucket
        //      meant "whatever is under these pixels right now", a viewport artifact
        //      rather than a fact about the story. The user put it exactly:
        //      *"The buckets stay clamped in place and the dots flow through them."*
        //
        // ✅ THE RULED MODEL (user, 2026-09-14): buckets are EQUAL STORY-TIME SLICES
        // across the WHOLE story. Zoom decides how many slices exist and where they sit;
        // that assignment is then FIXED. Panning slides the viewport across a stable
        // layout — it NEVER re-forms the buckets.
        //
        // ⚠️ THE KEY CONSEQUENCE: the partition is computed in STORY-TIME space, so it
        // is INDEPENDENT OF SCROLL BY CONSTRUCTION. Nothing here reads `scrollOffsetFraction`.
        // Panning changes only WHICH slices are visible, never their membership.
        //
        // ⚠️ Equal TIME slices, not equal scene COUNTS (ruled): dots are positioned by
        // `offsetMs`, so a time-sliced bucket always sits where its members actually are.
        // Counts therefore VARY — a dense month yields a fat bucket, a quiet year a thin
        // one — and that is the truthful reading. Manually placed dots stay consistent
        // with their bucket, which equal-count partitioning could not guarantee.
        // ⚠️ I-0210 — PITCH MUST BE THE HIT FRAME, NOT THE VISUAL DIAMETER.
        //
        // This read `aggregateDotRadius * 2 + gap` = 26pt. But `AggregateDotView` frames
        // itself at `ringDiameter + 8` = `(radius*2 + 12) + 8` = 42pt, and that frame
        // carries the `.contentShape` — so the HIT AREA is 42pt while slices were only
        // 26pt apart: a 16pt overlap between EVERY adjacent pair, 8pt each side.
        //
        // Later siblings in a `ForEach` paint (and hit-test) above earlier ones, so each
        // dot was partly covered by its right-hand neighbour. Whether a dot kept any
        // exposed area depended on how the overlaps stacked — which is exactly the
        // ALTERNATING failure the user observed: *"every other popup will show and the
        // others will not"*, worst when clusters hold 4-5 members.
        //
        // ✅ Deriving pitch from the frame guarantees slices are never closer than a dot
        // is wide, so no aggregate can ever occlude its neighbour's hit area.
        let pitch = aggregateHitFrame + clusterBucketGap

        // Slice count is set by how many aggregates fit across the FULL zoomed extent —
        // i.e. the viewport width times the zoom — so zooming in yields more, finer
        // slices (R3) while the visible count stays bounded by what fits (R2).
        let fullExtent = usable * effectiveZoom
        let sliceCount = max(1, Int((fullExtent / pitch).rounded(.down)))

        let storySpan = max(model.spanMs, 1)
        let storyMin  = model.minOffsetMs
        let sliceMs   = max(storySpan / Int64(sliceCount), 1)

        var buckets: [Int: [MainRowItem]] = [:]
        var bucketOrder: [Int] = []
        for p in positioned {
            let rel = p.item.offsetMs - storyMin
            let idx = min(max(Int(rel / sliceMs), 0), sliceCount - 1)
            if buckets[idx] == nil {
                buckets[idx] = []
                bucketOrder.append(idx)
            }
            buckets[idx]?.append(p.item)
        }
        // ⚠️ The clamp above is SAFE here, unlike round 2: `rel` is bounded by the STORY
        // span (every item lies within it by definition of `minOffsetMs`/`maxEndMs`), so
        // it only catches the final boundary item, never off-viewport accumulation.

        // Empty slices produce NO cluster (ruled) — the timeline shows only where story is.
        // `bucketOrder` is ascending because `positioned` is sorted by x.
        let allClusters: [DotCluster] = bucketOrder.compactMap { idx in
            guard let members = buckets[idx], let first = members.first else { return nil }
            // Anchor at the SLICE centre so the dot does not drift as membership varies.
            let anchor = storyMin + Int64(idx) * sliceMs + sliceMs / 2
            return DotCluster(centerItemID: first.id,
                              members: members,
                              anchorOffsetMs: anchor)
        }

        // Drawing is viewport-limited, but MEMBERSHIP is not: a cluster whose anchor lies
        // outside the visible span is simply not drawn, and is reached by panning.
        let vMin = visibleMinMs()
        let vMax = vMin + visibleSpanMs()
        let clusters = allClusters.filter { $0.anchorOffsetMs >= vMin && $0.anchorOffsetMs <= vMax }

        if ScriviDiag.timingEnabled {
            let maxMembers = clusters.map(\.members.count).max() ?? 0
            let shown = clusters.reduce(0) { $0 + $1.members.count }
            NSLog("[SCRIVI-TL] visible=%d slices=%d maxMembers=%d shown=%d items=%d zoom=%.2f usable=%.0f",
                  clusters.count, sliceCount, maxMembers, shown, positioned.count,
                  Double(effectiveZoom), Double(usable))
        }
        return clusters
    }

    // Centre-to-centre distance within which the next item joins the running cluster: the
    // radius of what the cluster currently renders as, plus one regular dot radius and a small
    // margin. Size 1 → regular dot; size ≥2 → aggregate dot footprint.
    private func mergeThreshold(currentSize: Int) -> CGFloat {
        let currentRenderedRadius = currentSize >= 2 ? aggregateDotRadius : dotRadius
        return currentRenderedRadius + dotRadius + 2
    }

    private func itemX(_ item: MainRowItem, usable: CGFloat, panelW: CGFloat) -> CGFloat {
        eventX(offsetMs: item.offsetMs, usable: usable, panelW: panelW)
    }

    // T-0174: co-located groups now render as compact aggregate dots and their members live in
    // a floating popover (its own window), so the panel never needs to grow for clustering.
    // The aggregate dot itself is only ~one regular ring taller than a normal dot. (This
    // supersedes the I-0045 ring-stack auto-grow, which existed for the old in-panel rings.)
    private func tallestClusterStack(usable: CGFloat, panelW: CGFloat) -> CGFloat {
        let hasAggregate = buildClusters(usable: usable, panelW: panelW)
            .contains { $0.members.count > 1 }
        return hasAggregate ? aggregateDotRadius : 0
    }

    // Clusters imported event dots within a single row by X proximity.
    // Events from different rows are never mixed — call once per visible row.
    struct ImportedRowCluster {
        let centerEventID: String
        let members: [ImportedEventDot]
        var ringCount: Int { members.count <= 1 ? 0 : members.count <= 7 ? 1 : 2 }
    }

    private func buildImportedRowClusters(events: [ImportedEventDot],
                                          usable: CGFloat, panelW: CGFloat,
                                          radius: CGFloat) -> [ImportedRowCluster] {
        // ⚠️ I-0210 — THE THRESHOLD MUST BE THE HIT FRAME, NOT THE DOT GEOMETRY.
        //
        // This computed `(aggRadius) + radius + 2` = 12pt from dot radii, and its comment
        // claimed that stopped adjacent aggregates overlapping. It did not.
        // `ImportedAggregateDotView` is rendered with `radius: importedRadius + 4` and
        // frames itself at `(radius*2 + 8) + 8` — a **30pt** hit area. Two aggregates
        // could therefore sit 12pt apart while each claimed 30pt: an 18pt mutual overlap.
        //
        // Later siblings in a `ForEach` hit-test above earlier ones, so a covered dot
        // never receives hover and its popover silently never opens — the same failure
        // the user reported on the MAIN row, where the numbers were 42pt vs 26pt.
        //
        // ✅ Deriving the multi-member threshold from the view's own `hitFrame` keeps the
        // two in step by construction. The single-dot case keeps its original geometry:
        // a lone imported dot really is only `radius` wide.
        let aggHitFrame = ImportedAggregateDotView.hitFrame(radius: radius + 4)
        func threshold(currentSize: Int) -> CGFloat {
            currentSize >= 2 ? aggHitFrame : (radius + radius + 2)
        }
        let sorted = events.sorted {
            eventX(offsetMs: $0.projectOffsetMs, usable: usable, panelW: panelW)
            < eventX(offsetMs: $1.projectOffsetMs, usable: usable, panelW: panelW)
        }
        var clusters: [ImportedRowCluster] = []
        var i = 0
        while i < sorted.count {
            let anchor = sorted[i]
            var members = [anchor]
            var clusterMaxX = eventX(offsetMs: anchor.projectOffsetMs, usable: usable, panelW: panelW)
            var j = i + 1
            while j < sorted.count {
                let ox = eventX(offsetMs: sorted[j].projectOffsetMs, usable: usable, panelW: panelW)
                if ox - clusterMaxX <= threshold(currentSize: members.count) {
                    members.append(sorted[j])
                    clusterMaxX = ox
                    j += 1
                } else {
                    break
                }
            }
            clusters.append(ImportedRowCluster(centerEventID: anchor.eventID, members: members))
            i = j
        }
        return clusters
    }

    // Returns the x/y offset for cluster member at index `i` within a cluster of `clusterSize`.
    // i=0 always returns .zero — the anchor dot stays on the timeline line.
    // i=1+ are placed in the ring: 12 o'clock, 2 o'clock, 4 o'clock, 6 o'clock, 8 o'clock, 10 o'clock…
    // height is positive-upward; callers use `lineY - offset.height`.
    private func clusterOffset(position: Int, clusterSize: Int, radius: CGFloat) -> CGSize {
        guard clusterSize > 1, position > 0 else { return .zero }
        let spacing = radius * 2 + 3
        // Ring members start at position 1. Map to ring/slot (ring 1 has 6 slots, ring 2 has 12).
        let ringPosition = position - 1
        let (ring, slot): (Int, Int)
        if ringPosition < 6       { (ring, slot) = (1, ringPosition) }
        else if ringPosition < 18 { (ring, slot) = (2, ringPosition - 6) }
        else                      { (ring, slot) = (3, ringPosition - 18) }
        let count = ring * 6
        // Start at 90° (12 o'clock) and step clockwise at 60° increments.
        let angleDeg = 90.0 - (Double(slot) / Double(count)) * 360.0
        let angleRad = angleDeg * .pi / 180.0
        let r = CGFloat(ring) * spacing
        return CGSize(width: r * cos(angleRad), height: r * sin(angleRad))
    }

    // MARK: Import / Export

    private func triggerImport() {
        #if os(macOS)
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.init(filenameExtension: "json")!]
        panel.message = "Select a .scrivi-timeline.json file to import"
        panel.begin { response in
            guard response == .OK, let url = panel.url,
                  let jsonStr = try? String(contentsOf: url, encoding: .utf8) else { return }   // boundary-ok: NSPanel, writer-chosen path outside any package
            // Parse source name and epoch label for the dialog
            struct Peek: Decodable {
                let sourceProjectTitle: String?
                let epochLabel: String?
            }
            if let data = jsonStr.data(using: .utf8),
               let peek = try? JSONDecoder().decode(Peek.self, from: data) {
                DispatchQueue.main.async {
                    pendingImportJSON = jsonStr
                    pendingImportName = peek.sourceProjectTitle ?? "Imported Timeline"
                    pendingImportEpochLabel = peek.epochLabel ?? "Story Open"
                    showEpochOffsetDialog = true
                }
            }
        }
        #endif
    }

    private func triggerExport() {
        #if os(macOS)
        guard let result = try? engine.exportProjectTimeline(projectRootPath: projectRootPath) else { return }
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.init(filenameExtension: "json")!]
        panel.nameFieldStringValue = "timeline.scrivi-timeline.json"
        panel.begin { response in
            guard response == .OK, let url = panel.url else { return }
            try? result.timelineJSON.write(to: url, atomically: true, encoding: .utf8)   // boundary-ok: NSPanel, writer-chosen path outside any package
        }
        #endif
    }

    // Returns the band color for a dot's assigned band, or nil if unassigned / no structure.
    private func bandColor(for dot: TimelineViewModel.SceneDot) -> Color? {
        guard !dot.bandID.isEmpty, !model.activeBands.isEmpty else { return nil }
        return model.activeBands.first(where: { $0.bandID == dot.bandID })?.swiftUIColor
    }

    private func applyPickerResult(_ result: TimeDeltaPickerResult,
                                   for dot: TimelineViewModel.SceneDot,
                                   pickerDurationMs: Int64) {
        let dur = pickerDurationMs > 0 ? pickerDurationMs : model.defaultSceneDurationMs
        let prevEnd = previousSceneEndMs(for: dot)
        switch result {
        case .spinner(let ms), .anchor(let ms), .keepPosition(let ms):
            let gap = ms - prevEnd
            model.setStoryTime(sceneID: dot.sceneID, offsetMs: ms, offsetSource: "manual",
                               gapMs: gap, durationMs: dur, durationSource: "manual",
                               engine: engine, projectRootPath: projectRootPath)
        case .resetDefault:
            model.resetToDefault(sceneID: dot.sceneID, engine: engine,
                                 projectRootPath: projectRootPath)
        }
    }

    private func previousSceneEndMs(for dot: TimelineViewModel.SceneDot) -> Int64 {
        guard let idx = model.dots.firstIndex(where: { $0.id == dot.id }), idx > 0 else {
            return 0
        }
        let prev = model.dots[idx - 1]
        return prev.offsetMs + prev.durationMs
    }

    private func previousSceneTitle(for dot: TimelineViewModel.SceneDot) -> String {
        guard let idx = model.dots.firstIndex(where: { $0.id == dot.id }), idx > 0 else {
            return ""
        }
        return model.dots[idx - 1].title
    }
}

// MARK: — BandOverlayView
//
// Draws the colored band rectangles behind the timeline and the label row at the top.
// Band borders are draggable (T-0157). The view owns dragging state internally and
// calls onBorderDragged with the updated band array on each drag update and on release.

private struct BandOverlayView: View {

    var bands: [StoryBand]
    // I-0048: the band region in panel coordinates — spans first→last scene in story time and
    // moves/scales with the timeline (the parent computes these via eventX). All band geometry
    // is relative to [regionX, regionX + regionWidth] instead of the full panel width.
    var regionX: CGFloat
    var regionWidth: CGFloat
    var panelWidth: CGFloat
    var panelHeight: CGFloat
    var labelRowHeight: CGFloat
    // I-0048: maps an absolute panel-X to a [0,1] fraction within the band region (through the
    // timeline transform) for border-drag editing.
    var fractionForPanelX: (CGFloat) -> Double
    var onBorderDragged: ([StoryBand]) -> Void

    // Track which border is being dragged: index == right edge of bands[index]
    @State private var draggingBorderIndex: Int? = nil
    @State private var workingBands: [StoryBand] = []

    var body: some View {
        Canvas { ctx, size in
            drawBands(ctx: &ctx, size: size)
        }
        .allowsHitTesting(false)   // I-0033: canvas must not intercept events; gesture layer only
        .frame(width: panelWidth, height: panelHeight)
        // I-0033 fix: allowsHitTesting(false) on the Canvas also disables any gesture attached
        // to the same view, so the band-border drag never fired. Host the gesture on a separate
        // hit-testable surface (contentShape) layered over the non-hittable canvas instead.
        .overlay {
            Color.clear
                .contentShape(Rectangle())
                .frame(width: panelWidth, height: panelHeight)
                .simultaneousGesture(bandBorderDragGesture)   // coexist with dot gestures
                // I-0032: change cursor to horizontal resize when near a band border.
                // Hover lives on the same hittable surface as the gesture so loc.x is in
                // the same (panel) coordinate space the canvas was drawn in.
                .onContinuousHover { phase in
                    #if os(macOS)
                    switch phase {
                    case .active(let loc):
                        if nearestBorderIndex(at: loc.x) != nil {
                            NSCursor.resizeLeftRight.push()
                        } else {
                            NSCursor.pop()
                        }
                    case .ended:
                        NSCursor.pop()
                    }
                    #endif
                }
        }
        .onAppear { workingBands = bands }
        .onChange(of: bands) { _, new in workingBands = new }
        // Label row — rendered on top of the canvas
        .overlay(alignment: .topLeading) {
            bandLabelRow
        }
    }

    // Draw the band rectangles into the canvas, within the band region [regionX, +regionWidth].
    private func drawBands(ctx: inout GraphicsContext, size: CGSize) {
        let effectiveBands = workingBands.isEmpty ? bands : workingBands
        var x: CGFloat = regionX
        for band in effectiveBands {
            let w = CGFloat(band.proportion) * regionWidth
            let rect = CGRect(x: x, y: labelRowHeight,
                              width: w, height: size.height - labelRowHeight)
            ctx.fill(Path(rect), with: .color((Color(hex: band.color) ?? .accentColor).opacity(0.18)))
            x += w
        }
    }

    // Render band labels in the label row, offset to the band region and clipped to it.
    private var bandLabelRow: some View {
        HStack(spacing: 0) {
            let effectiveBands = workingBands.isEmpty ? bands : workingBands
            ForEach(effectiveBands) { band in
                Text(band.label)
                    .font(.system(size: 10, weight: .medium))
                    .foregroundStyle((Color(hex: band.color) ?? .accentColor).opacity(0.85))
                    .lineLimit(1)
                    .truncationMode(.tail)
                    .padding(.leading, 5)
                    .frame(width: CGFloat(band.proportion) * regionWidth,
                           height: labelRowHeight, alignment: .leading)
            }
        }
        .frame(width: regionWidth, alignment: .leading)
        .clipped()
        .offset(x: regionX)
    }

    // Find the nearest band border within a hit area, then redistribute proportions.
    private var bandBorderDragGesture: some Gesture {
        DragGesture(minimumDistance: 2)
            .onChanged { v in
                let x = v.location.x
                if draggingBorderIndex == nil {
                    draggingBorderIndex = nearestBorderIndex(at: x)
                    if draggingBorderIndex != nil { workingBands = bands }
                }
                // P2: region as the gesture/draw/hit-test see it (props on this instance).
                // P3: region implied by the closure (back-derive from two sample points).
                let f0 = fractionForPanelX(regionX)            // should be 0 if P3==P2
                let f1 = fractionForPanelX(regionX + regionWidth) // should be 1 if P3==P2
                NSLog("BANDDIAG P2-gesture regionX=\(regionX) regionW=\(regionWidth) | P3-closure f(regionX)=\(f0) f(regionX+W)=\(f1) | x=\(x) idx=\(String(describing: draggingBorderIndex))")
                guard let borderIdx = draggingBorderIndex else {
                    NSLog("BANDDIAG  -> no border latched (hit-test failed at drag start)")
                    return
                }
                let before = workingBands[borderIdx].proportion
                let frac = fractionForPanelX(x)
                let updated = redistributeProportions(
                    draggedBorderIndex: borderIdx,
                    draggedFraction: frac,
                    bands: workingBands
                )
                NSLog("BANDDIAG  -> frac=\(frac) before=\(before) after=\(updated[borderIdx].proportion) changed=\(updated[borderIdx].proportion != before)")
                workingBands = updated
                onBorderDragged(workingBands)
            }
            .onEnded { _ in
                if let _ = draggingBorderIndex {
                    onBorderDragged(workingBands)
                }
                draggingBorderIndex = nil
            }
    }

    // Returns the index of the band whose right border is within 8pt of `x`, or nil.
    // Borders are measured within the band region [regionX, regionX + regionWidth].
    private func nearestBorderIndex(at x: CGFloat) -> Int? {
        guard regionWidth > 0 else { return nil }
        var accumulated: CGFloat = regionX
        let effectiveBands = workingBands.isEmpty ? bands : workingBands
        for i in 0..<effectiveBands.count - 1 {   // last band has no right-border to drag
            accumulated += CGFloat(effectiveBands[i].proportion) * regionWidth
            if abs(accumulated - x) < 8 { return i }
        }
        return nil
    }

    // Move border at `draggedBorderIndex` to `draggedFraction` (a [0,1] position within the
    // band region, computed through the timeline transform). Clamp so neither adjacent band
    // goes below 4%.
    private func redistributeProportions(draggedBorderIndex: Int, draggedFraction: Double,
                                          bands: [StoryBand]) -> [StoryBand] {
        var result = bands
        let minProportion = 0.04

        // Sum of all bands strictly before the dragged border = the dragged band's left edge.
        var sumBefore: Double = 0
        for i in 0..<draggedBorderIndex { sumBefore += result[i].proportion }

        // Clamp the new border position so both adjacent bands keep ≥ minProportion.
        let pairTotal = result[draggedBorderIndex].proportion
            + result[draggedBorderIndex + 1].proportion
        let newBorder = min(max(draggedFraction, sumBefore + minProportion),
                            sumBefore + pairTotal - minProportion)

        let leftBandNewProp  = newBorder - sumBefore
        let rightBandNewProp = pairTotal - leftBandNewProp

        result[draggedBorderIndex].proportion = leftBandNewProp
        result[draggedBorderIndex + 1].proportion = rightBandNewProp
        return result
    }
}

// MARK: — AggregateDotView (T-0174)

// A co-located group of ≥2 members renders as one large dot with a count and a thick arc
// ring whose lit segment marks the selected member (its angular position = which member).
// Hovering the dot opens a popover containing a circle-of-dots (see AggregateMembersPopover).
private struct AggregateDotView<Popover: View>: View {

    let members: [TimelineStripView.MainRowItem]
    let radius: CGFloat
    let selectedSceneID: String?
    let isPopoverOpen: Bool
    let onHover: () -> Void
    @ViewBuilder let popoverContent: () -> Popover
    let onPopoverDismiss: () -> Void

    @State private var isHovered = false

    // Below this per-segment angle, dividers are sub-pixel; draw the base ring continuous and
    // overlay only the selected member's slot.
    private let minSegmentDegrees: Double = 6
    private var count: Int { members.count }

    /// I-0210 — the hit frame this view claims, and the SINGLE SOURCE for it.
    /// `TimelineStripView` derives its slice pitch from this; if the two ever drift,
    /// adjacent aggregates overlap one another's hit areas and popovers silently stop
    /// opening for some dots (the alternating failure the user reported 2026-09-14).
    static func hitFrame(radius: CGFloat) -> CGFloat { radius * 2 + 12 + 8 }

    private var selectedIndex: Int? {
        guard let sid = selectedSceneID else { return nil }
        return members.firstIndex {
            if case .scene(let d) = $0 { return d.sceneID == sid }
            return false
        }
    }

    var body: some View {
        let ringDiameter = radius * 2 + 12
        ZStack {
            arcRing
                .frame(width: ringDiameter, height: ringDiameter)
            Circle()
                .fill(Color.accentColor)
                .frame(width: radius * 2, height: radius * 2)
            Text("\(count)")
                .font(.system(size: 11, weight: .bold))
                .foregroundStyle(.white)
                .minimumScaleFactor(0.5)
                .frame(width: radius * 2 - 2, height: radius * 2 - 2)
        }
        .scaleEffect(isHovered ? 1.15 : 1.0)               // grow on hover like scene dots
        .animation(.easeOut(duration: 0.1), value: isHovered)
        .frame(width: Self.hitFrame(radius: radius), height: Self.hitFrame(radius: radius))
        .contentShape(Circle())
        .onHover { hovered in
            isHovered = hovered
            if hovered { onHover() }
        }
        .popover(isPresented: Binding(get: { isPopoverOpen },
                                      set: { if !$0 { onPopoverDismiss() } })) {
            popoverContent()
        }
        .help("\(count) scenes here — hover to open")
    }

    @ViewBuilder
    private var arcRing: some View {
        let perSegment = 360.0 / Double(count)
        GeometryReader { geo in
            let rect = CGRect(origin: .zero, size: geo.size)
            let centre = CGPoint(x: rect.midX, y: rect.midY)
            let r = min(rect.width, rect.height) / 2 - 2
            ZStack {
                if perSegment >= minSegmentDegrees {
                    ForEach(0..<count, id: \.self) { i in
                        segmentPath(centre: centre, r: r, index: i, perSegment: perSegment,
                                    gapDegrees: 2)
                            .stroke(segmentColor(i),
                                    style: StrokeStyle(lineWidth: i == selectedIndex ? 5 : 4,
                                                       lineCap: .butt))
                    }
                } else {
                    Circle().stroke(Color.secondary.opacity(0.5), lineWidth: 4).padding(2)
                    if let sel = selectedIndex {
                        segmentPath(centre: centre, r: r, index: sel, perSegment: perSegment,
                                    gapDegrees: 0)
                            .stroke(Color.orange, lineWidth: 5)
                    }
                }
            }
        }
    }

    // Segment 0 starts at 12 o'clock and proceeds clockwise.
    private func segmentPath(centre: CGPoint, r: CGFloat, index: Int,
                             perSegment: Double, gapDegrees: Double) -> Path {
        let start = -90.0 + Double(index) * perSegment + gapDegrees / 2
        let end   = -90.0 + Double(index + 1) * perSegment - gapDegrees / 2
        var p = Path()
        p.addArc(center: centre, radius: r,
                 startAngle: .degrees(start), endAngle: .degrees(end), clockwise: false)
        return p
    }

    private func segmentColor(_ i: Int) -> Color {
        if i == selectedIndex { return .orange }
        switch members[i] {
        case .scene:      return Color.accentColor.opacity(0.6)
        case .historical: return (Color(hex: "#C8A97A") ?? .orange).opacity(0.85)
        }
    }
}

// MARK: — AggregateMembersPopover (T-0174)

// The popover content: members laid out as a circle of dots around the centre, the radius
// scaling with member count. Each member is rendered by the caller's `memberContent` builder
// (so they look exactly like timeline dots). Display-only positions — dots are not draggable.
// I-0208 — bounds for the aggregate popover. File-scope rather than static members
// because `AggregateMembersPopover` is generic, and generic types cannot hold static
// stored properties.
private let kAggregatePopoverMaxRingMembers = 12
private let kAggregatePopoverMaxGridHeight: CGFloat = 260

private struct AggregateMembersPopover<Member: View>: View {

    let members: [TimelineStripView.MainRowItem]
    let selectedSceneID: String?
    let dotRadius: CGFloat
    @ViewBuilder let memberContent: (TimelineStripView.MainRowItem, Bool) -> Member
    /// I-0212 — title of the member currently hovered, supplied by the host so the
    /// popover can show it as READABLE TEXT. `.help()` is a system tooltip and macOS
    /// does not reliably present those inside a popover.
    var hoveredTitle: String? = nil

    private var count: Int { members.count }

    /// A always-present caption line. Reserving the row keeps the popover from
    /// resizing as the pointer moves between members, which would chase the cursor.
    @ViewBuilder
    var captionLine: some View {
        Text(hoveredTitle ?? "\(count) items — hover to read, click to go")
            .font(.caption)
            .foregroundStyle(hoveredTitle == nil ? .secondary : .primary)
            .lineLimit(2)
            .multilineTextAlignment(.center)
            .frame(maxWidth: 260)
            .fixedSize(horizontal: false, vertical: true)
    }

    // I-0208 defect (2) — THE RING MUST BE BOUNDED.
    //
    // ⚠️ `ringR = count * spacing / 2π` is LINEAR IN MEMBER COUNT and was unbounded.
    // On `dumas-prose` one cluster held 1,158 members: ringR ≈ 3,300pt, canvas ≈ 6,600pt
    // — a panel larger than any screen, with its dots on a ring almost entirely
    // off-screen. It read as "a giant blank semi-translucent panel", and because it
    // exceeded the screen there was no reachable outside-click target, so it could only
    // be dismissed by resigning app focus.
    //
    // ⚠️ THIS BOUND IS REQUIRED EVEN WITH CAPACITY BUCKETING (§3.3 of the design):
    // bucketing makes a huge cluster unlikely, NOT impossible — N scenes sharing one
    // story-time offset land in one bucket at ANY zoom.
    //
    // Beyond `maxRingMembers` the ring is abandoned for a scrollable grid, which is
    // bounded by construction.

    var body: some View {
        // I-0212 — the caption is OUTSIDE the two layouts so both get it, and so the
        // popover's size does not change as the pointer moves between members.
        VStack(spacing: 8) {
            if count <= kAggregatePopoverMaxRingMembers {
                ringBody
            } else {
                gridBody
            }
            captionLine
        }
        .padding(12)
    }

    // The original circle-of-dots, now only for counts it actually suits.
    private var ringBody: some View {
        let spacing = dotRadius * 2 + 8
        let ringR = max(spacing, CGFloat(count) * spacing / (2 * .pi))
        let canvas = (ringR + spacing) * 2
        return ZStack {
            ForEach(Array(members.enumerated()), id: \.element.id) { i, item in
                let angle = -90.0 + (Double(i) / Double(count)) * 360.0
                let rad   = angle * .pi / 180.0
                let x = canvas / 2 + ringR * CGFloat(cos(rad))
                let y = canvas / 2 + ringR * CGFloat(sin(rad))
                memberContent(item, isSelected(item))
                    .position(x: x, y: y)
            }
        }
        .frame(width: canvas, height: canvas)
    }

    // Overflow presentation: a bounded, SCROLLABLE grid. The member count is shown
    // because at this size the writer cannot count the dots, and a dense cluster is
    // itself information ("these 300 scenes share a story time").
    private var gridBody: some View {
        let cell = dotRadius * 2 + 10
        return VStack(alignment: .leading, spacing: 6) {
            ScrollView {
                LazyVGrid(columns: [GridItem(.adaptive(minimum: cell), spacing: 6)],
                          spacing: 6) {
                    ForEach(members, id: \.id) { item in
                        memberContent(item, isSelected(item))
                            .frame(width: cell, height: cell)
                    }
                }
            }
            .frame(maxHeight: kAggregatePopoverMaxGridHeight)
        }
        .frame(width: 300)
    }

    private func isSelected(_ item: TimelineStripView.MainRowItem) -> Bool {
        guard let sid = selectedSceneID else { return false }
        if case .scene(let d) = item { return d.sceneID == sid }
        return false
    }
}

// MARK: — PopoverMemberDotView (T-0174)

// A non-draggable dot used inside the aggregate popover. Mirrors the timeline dot's look
// (placement ring, band ring, selection highlight), grows on hover with a native tooltip,
// and selects on click.
private struct PopoverMemberDotView: View {

    let radius: CGFloat
    let fill: Color
    let isSelected: Bool
    let bandRingColor: Color?
    let placementSource: String          // "manual" | "inferred" | "default"
    let tooltip: String
    let subtitle: String
    let onSelect: () -> Void
    /// I-0212 — report hover so the popover can show a READABLE label.
    /// `.help()` alone is a SYSTEM tooltip and macOS does not reliably present those
    /// for views inside a popover, which is exactly this case.
    var onHoverChanged: ((Bool) -> Void)? = nil

    @State private var isHovered = false

    var body: some View {
        ZStack {
            if let bc = bandRingColor {
                Circle().strokeBorder(bc, lineWidth: 3)
                    .frame(width: radius * 2 + 10, height: radius * 2 + 10)
            }
            if placementSource == "manual" {
                Circle().strokeBorder(Color.accentColor, lineWidth: 2)
                    .frame(width: radius * 2 + 6, height: radius * 2 + 6)
            } else if placementSource == "inferred" {
                Circle().strokeBorder(Color.accentColor.opacity(0.6), lineWidth: 1.5)
                    .frame(width: radius * 2 + 6, height: radius * 2 + 6)
            }
            if isSelected {
                Circle().strokeBorder(Color.yellow, lineWidth: 2)
                    .frame(width: radius * 2 + 4, height: radius * 2 + 4)
            }
            Circle()
                .fill(isSelected ? Color.orange : fill)
                .frame(width: radius * 2, height: radius * 2)
                .scaleEffect(isHovered ? 1.25 : (isSelected ? 1.15 : 1.0))
                .animation(.easeOut(duration: 0.1), value: isHovered)
        }
        .frame(width: radius * 2 + 14, height: radius * 2 + 14)
        .contentShape(Circle())
        .onHover { hovered in
            isHovered = hovered
            onHoverChanged?(hovered)
        }
        .onTapGesture { onSelect() }
        .help("\(tooltip)\n\(subtitle)")
    }
}

// MARK: — ImportedAggregateDotView (T-0174)

// Aggregate dot for a co-located group of imported-timeline events. Larger dot + count + a
// uniform ring in the row colour (no selection segment — imported events aren't selectable
// scenes). Hover opens a circle-of-dots popover of the member events for their tooltips.
private struct ImportedAggregateDotView: View {

    let members: [ImportedEventDot]
    let count: Int
    let color: Color
    let radius: CGFloat
    let dotRadius: CGFloat
    let isPopoverOpen: Bool
    let onHover: () -> Void
    let onPopoverDismiss: () -> Void

    @State private var isHovered = false
    /// I-0212 — title of the member hovered inside this popover, shown as readable text.
    /// `.help()` is a system tooltip and macOS does not reliably present those inside a
    /// popover — which made imported timelines unreadable, since an imported event has
    /// no click-through that would reveal what it is.
    @State private var hoveredTitle: String? = nil

    /// I-0210 — the hit frame this view claims, and the SINGLE SOURCE for it.
    /// `buildImportedRowClusters` must space aggregates by at least this, or adjacent
    /// dots occlude one another's hit areas and popovers stop opening for some of them.
    /// Mirrors `AggregateDotView.hitFrame` but with this view's OWN ring (+8, not +12).
    static func hitFrame(radius: CGFloat) -> CGFloat { radius * 2 + 8 + 8 }

    var body: some View {
        let ringDiameter = radius * 2 + 8
        ZStack {
            Circle().stroke(color.opacity(0.6), lineWidth: 3)
                .frame(width: ringDiameter, height: ringDiameter)
            Circle().fill(color)
                .frame(width: radius * 2, height: radius * 2)
            Text("\(count)")
                .font(.system(size: 9, weight: .bold))
                .foregroundStyle(.white)
                .minimumScaleFactor(0.5)
                .frame(width: radius * 2 - 2, height: radius * 2 - 2)
        }
        .scaleEffect(isHovered ? 1.15 : 1.0)
        .animation(.easeOut(duration: 0.1), value: isHovered)
        .frame(width: Self.hitFrame(radius: radius), height: Self.hitFrame(radius: radius))
        .contentShape(Circle())
        .onHover { hovered in
            isHovered = hovered
            if hovered { onHover() }
        }
        .popover(isPresented: Binding(get: { isPopoverOpen },
                                      set: { if !$0 { onPopoverDismiss() } })) {
            popover
        }
        .help("\(count) events here — hover to open")
    }

    // I-0208 defect (2), imported row — THE RING MUST BE BOUNDED.
    //
    // `ringR = count * spacing / 2π` is LINEAR IN MEMBER COUNT. On the main row this
    // produced a 6,600pt panel that exceeded the screen and could only be dismissed by
    // resigning app focus. An imported timeline can carry just as many co-located events,
    // so the same bound applies here.
    private var popover: some View {
        VStack(spacing: 8) {
            if count <= kAggregatePopoverMaxRingMembers {
                AnyView(ringPopover)
            } else {
                AnyView(gridPopover)
            }
            captionLine
        }
        .padding(12)
    }

    /// I-0212 — always-present caption. Reserving the row stops the popover resizing
    /// as the pointer moves between events, which would chase the cursor.
    private var captionLine: some View {
        Text(hoveredTitle ?? "\(count) events — hover to read")
            .font(.caption)
            .foregroundStyle(hoveredTitle == nil ? .secondary : .primary)
            .lineLimit(2)
            .multilineTextAlignment(.center)
            .frame(maxWidth: 260)
            .fixedSize(horizontal: false, vertical: true)
    }

    private func label(_ ev: ImportedEventDot) -> String {
        ev.title.isEmpty ? "Event" : ev.title
    }

    private var ringPopover: some View {
        let spacing = dotRadius * 2 + 8
        let ringR = max(spacing, CGFloat(count) * spacing / (2 * .pi))
        let canvas = (ringR + spacing) * 2
        return ZStack {
            ForEach(Array(members.enumerated()), id: \.element.id) { i, ev in
                let angle = -90.0 + (Double(i) / Double(count)) * 360.0
                let rad = angle * .pi / 180.0
                let x = canvas / 2 + ringR * CGFloat(cos(rad))
                let y = canvas / 2 + ringR * CGFloat(sin(rad))
                ImportedEventDotView(ev: ev, color: color, radius: dotRadius,
                                     onHoverChanged: { hovered in
                                         hoveredTitle = hovered ? label(ev) : nil
                                     })
                    .help(label(ev))
                    .position(x: x, y: y)
            }
        }
        .frame(width: canvas, height: canvas)
    }

    private var gridPopover: some View {
        let cell = dotRadius * 2 + 10
        return VStack(alignment: .leading, spacing: 6) {
            Text("\(count) events")
                .font(.caption)
                .foregroundStyle(.secondary)
            ScrollView {
                LazyVGrid(columns: [GridItem(.adaptive(minimum: cell), spacing: 6)],
                          spacing: 6) {
                    ForEach(members, id: \.id) { ev in
                        ImportedEventDotView(ev: ev, color: color, radius: dotRadius,
                                             onHoverChanged: { hovered in
                                                 hoveredTitle = hovered ? label(ev) : nil
                                             })
                            .help(label(ev))
                            .frame(width: cell, height: cell)
                    }
                }
            }
            .frame(maxHeight: kAggregatePopoverMaxGridHeight)
        }
        .frame(width: 300)
    }
}

// MARK: — SceneDotView

private struct SceneDotView: View {

    let dot: TimelineViewModel.SceneDot
    let radius: CGFloat
    let epochLabel: String
    let startX: CGFloat
    let panelWidth: CGFloat
    let lineY: CGFloat                   // Y of timeline line in panel coords
    let labelRowHeight: CGFloat          // 0 if no structure, else height of label row
    let bands: [StoryBand]               // active bands — needed for assign-by-drag
    let bandRingColor: Color?            // ring color when assigned to a band
    // I-0048: band region in panel coords, so drag-up-to-assign maps to the same band extents
    // the bands are drawn at. Defaults make the dot fall back to full-panel behaviour.
    var bandRegionX: CGFloat = 0
    var bandRegionWidth: CGFloat = 0
    let previousSceneEndMs: Int64
    let previousSceneTitle: String
    let defaultDurationMs: Int64
    let computeOffsetMs: (CGFloat) -> Int64
    let onCommit: (TimeDeltaPickerResult, Int64) -> Void
    let onAssignToBand: (String) -> Void
    let onUnassign: () -> Void
    let onHoverChanged: (Bool) -> Void
    // T-0173: selection — highlight the dot when it is the selected scene; click to select.
    var isSelected: Bool = false
    var onSelect: () -> Void = {}
    // T-0174: drag-to-join. Given the final panel X of a horizontal drag, returns the
    // offsetMs of an aggregate the dot was released onto (snap target), or nil. When non-nil
    // the dot snaps to that offset (manual) instead of opening the Time Delta Picker.
    var aggregateJoinOffsetMs: (CGFloat) -> Int64? = { _ in nil }

    @State private var dragOffsetX: CGFloat = 0
    @State private var dragOffsetY: CGFloat = 0
    @State private var isDragging = false
    @State private var isDraggingUp = false
    @State private var hoveredBandID: String? = nil
    @State private var showPicker = false
    @State private var pendingOffsetMs: Int64 = 0
    @State private var isHovered = false

    var body: some View {
        ZStack {
            // Band ring — outermost, colored by assigned band
            if let bc = bandRingColor {
                Circle()
                    .strokeBorder(bc, lineWidth: 3)
                    .frame(width: radius * 2 + 10, height: radius * 2 + 10)
            }

            // Offset-source ring — manual (solid) or inferred (dashed)
            if dot.offsetSource == "manual" {
                Circle()
                    .strokeBorder(Color.accentColor, lineWidth: 2)
                    .frame(width: radius * 2 + 6, height: radius * 2 + 6)
            } else if dot.offsetSource == "inferred" {
                Circle()
                    .strokeBorder(Color.accentColor.opacity(0.6), lineWidth: 1.5)
                    .frame(width: radius * 2 + 6, height: radius * 2 + 6)
            }

            // T-0173: selection highlight ring — drawn just outside the core dot.
            if isSelected {
                Circle()
                    .strokeBorder(Color.yellow, lineWidth: 2)
                    .frame(width: radius * 2 + 4, height: radius * 2 + 4)
            }

            // Core dot — scale up on hover, back to normal on press/drag.
            // T-0173: selected dot is tinted to stand out from the cluster.
            Circle()
                .fill(isSelected ? Color.orange : Color.accentColor)
                .frame(width: radius * 2, height: radius * 2)
                .scaleEffect(isDragging ? 1.3 : (isHovered ? 1.2 : (isSelected ? 1.15 : 1.0)))
                .animation(.easeOut(duration: 0.15), value: isDragging)
                .animation(.easeOut(duration: 0.1), value: isHovered)
                .animation(.easeOut(duration: 0.1), value: isSelected)

            // Hover label when dragging upward into label row
            if isDraggingUp, let hid = hoveredBandID,
               let band = bands.first(where: { $0.bandID == hid }) {
                Text(band.label)
                    .font(.system(size: 9, weight: .medium))
                    .foregroundStyle(.white)
                    .padding(.horizontal, 4)
                    .padding(.vertical, 2)
                    .background(Capsule().fill(band.swiftUIColor))
                    .offset(y: -(radius * 2 + 14))
            }
        }
        .frame(width: radius * 2 + 12, height: radius * 2 + 12)
        .contentShape(Circle().size(CGSize(width: radius * 2 + 12, height: radius * 2 + 12)))
        .popover(isPresented: $showPicker, arrowEdge: .top) {
            TimeDeltaPicker(
                referenceName: previousSceneTitle,
                rawOffsetMs: pendingOffsetMs,
                previousSceneEndMs: previousSceneEndMs,
                currentDurationMs: dot.durationMs,
                defaultDurationMs: defaultDurationMs,
                epochLabel: epochLabel,
                onResult: { result, chosenDurationMs in
                    showPicker = false
                    onCommit(result, chosenDurationMs)
                }
            )
            .padding()
            .frame(width: 340)
        }
        .contextMenu {
            // Assign to band submenu — I-0035: Picker gives native macOS checkmarks
            if !bands.isEmpty {
                Picker(selection: Binding(
                    get: { dot.bandID },
                    set: { newID in
                        if newID.isEmpty { onUnassign() } else { onAssignToBand(newID) }
                    }
                ), label: Text("Assign to Act…")) {
                    ForEach(bands) { band in
                        Text(band.label).tag(band.bandID)
                    }
                }
                .pickerStyle(.inline)
                Button("Unassign from Act") {
                    onUnassign()
                }
                .disabled(dot.bandID.isEmpty)
                Divider()
            }
            Button("Set Time Delta…") {
                pendingOffsetMs = computeOffsetMs(startX)
                showPicker = true
            }
            Button("Set Story Time…") { }
                .disabled(true)
            // T-0173: jump to this scene in the manuscript + Navigator.
            Button("Go to Scene") { onSelect() }
            Divider()
            Button("Copy Story-Time Position") { }
                .disabled(true)
            Button("Paste Story-Time Position") { }
                .disabled(true)
        }
        .onHover { over in
            isHovered = over
            onHoverChanged(over)
        }
        .offset(x: dragOffsetX, y: dragOffsetY)
        // T-0173: single click selects the scene (navigates manuscript + highlights in the
        // Navigator). The reposition drag uses minimumDistance 4, so a stationary click here
        // does not conflict with drag-to-reposition.
        .onTapGesture { onSelect() }
        .simultaneousGesture(
            DragGesture(minimumDistance: 0)
                .onChanged { _ in
                    isHovered = false
                    onHoverChanged(false)
                }
        )
        .simultaneousGesture(combinedDragGesture)
    }

    // Single gesture that handles both horizontal (story-time) and vertical-up (band assignment).
    private var combinedDragGesture: some Gesture {
        DragGesture(minimumDistance: 4)
            .onChanged { v in
                if !isDragging { isDragging = true }
                let dx = v.translation.width
                let dy = v.translation.height

                // Decide gesture axis: upward motion past the label row = band assignment.
                if dy < -16 && labelRowHeight > 0 {
                    // Vertical-up: band assignment gesture
                    isDraggingUp = true
                    dragOffsetX = 0
                    dragOffsetY = dy
                    hoveredBandID = bandAtLabelRow(dotPanelX: startX, dragY: dy)
                } else {
                    // Horizontal: story-time drag
                    isDraggingUp = false
                    dragOffsetX = dx
                    dragOffsetY = 0
                    hoveredBandID = nil
                }
            }
            .onEnded { v in
                isDragging = false
                dragOffsetX = 0
                dragOffsetY = 0

                if isDraggingUp && !dot.bandID.isEmpty {
                    // I-0034: drag-up on an assigned dot always unassigns — toggle behaviour.
                    // Release position (over a label or not) is irrelevant.
                    onUnassign()
                } else if isDraggingUp, let hid = hoveredBandID {
                    // Drag-up on an unassigned dot released over a label — assign.
                    onAssignToBand(hid)
                } else {
                    // Horizontal drag completed.
                    let finalPanelX = startX + v.translation.width
                    if let joinMs = aggregateJoinOffsetMs(finalPanelX) {
                        // T-0174: released over an aggregate — snap to its offset (manual)
                        // so this scene joins the co-located group. No picker.
                        onCommit(.keepPosition(joinMs), dot.durationMs)
                    } else {
                        // Otherwise show the Time Delta Picker for the dropped position.
                        pendingOffsetMs = computeOffsetMs(finalPanelX)
                        showPicker = true
                    }
                }

                isDraggingUp = false
                hoveredBandID = nil
            }
    }

    // Determine which band the dot is hovering over in the label row.
    // We use the dot's panel-X position and find which band's horizontal span it falls in.
    private func bandAtLabelRow(dotPanelX: CGFloat, dragY: CGFloat) -> String? {
        // Only highlight when the dot has been dragged high enough to be in the label row.
        // The label row sits at the very top; lineY is the center of the timeline content area.
        // dragY is negative (dragging upward). labelRowHeight is from top of the panel.
        let dotPanelY = lineY + dragY
        guard dotPanelY < labelRowHeight else { return nil }
        // I-0048: bands occupy the region [bandRegionX, +bandRegionWidth]; map within it.
        let width = bandRegionWidth > 0 ? bandRegionWidth : panelWidth
        let origin = bandRegionWidth > 0 ? bandRegionX : 0
        var accumulated: CGFloat = origin
        for band in bands {
            accumulated += CGFloat(band.proportion) * width
            if dotPanelX <= accumulated { return band.bandID }
        }
        return bands.last?.bandID
    }

}

// MARK: — HistoricalEventDotView

private struct HistoricalEventDotView: View {

    let event: HistoricalEventDot
    let radius: CGFloat
    let startX: CGFloat
    let panelWidth: CGFloat
    let lineY: CGFloat
    let epochLabel: String
    let onDragEnd: (Int64) -> Void
    let onEdit: () -> Void
    let onDelete: () -> Void
    let computeOffsetMs: (CGFloat) -> Int64
    let onHoverChanged: (Bool) -> Void

    private let dotColor = Color(hex: "#C8A97A") ?? .orange

    @State private var dragOffsetX: CGFloat = 0
    @State private var isDragging = false
    @State private var isHovered = false

    var body: some View {
        ZStack {
            Circle()
                .fill(dotColor)
                .frame(width: radius * 2, height: radius * 2)
                .scaleEffect(isDragging ? 1.3 : (isHovered ? 1.15 : 1.0))
                .animation(.easeOut(duration: 0.12), value: isDragging)
        }
        .frame(width: radius * 2 + 8, height: radius * 2 + 8)
        .contentShape(Circle())
        .contextMenu {
            Button("Edit Historical Event…") { onEdit() }
            Button("Set Story Time…") { }
                .disabled(true)
            Divider()
            Button("Delete Historical Event") { onDelete() }
        }
        .onHover { hovered in
            isHovered = hovered
            onHoverChanged(hovered)
        }
        .offset(x: dragOffsetX)
        .simultaneousGesture(
            DragGesture(minimumDistance: 4)
                .onChanged { v in
                    isDragging = true
                    dragOffsetX = v.translation.width
                }
                .onEnded { v in
                    isDragging = false
                    dragOffsetX = 0
                    onDragEnd(computeOffsetMs(startX + v.translation.width))
                }
        )
    }
}

// MARK: — ImportedEventDotView

private struct ImportedEventDotView: View {

    let ev: ImportedEventDot
    let color: Color
    let radius: CGFloat
    let onHoverChanged: (Bool) -> Void

    var body: some View {
        Circle()
            .fill(color)
            .frame(width: radius * 2, height: radius * 2)
            .onHover { onHoverChanged($0) }
    }
}

// MARK: — EpochOffsetDialog

private struct EpochOffsetDialog: View {

    let sourceName: String
    let sourceEpochLabel: String
    let onCommit: (Int64, String) -> Void
    let onCancel: () -> Void

    @State private var amount: Int = 0
    @State private var unit: OffsetUnit = .years
    @State private var direction: OffsetDirection = .later

    enum OffsetUnit: String, CaseIterable, Identifiable {
        case years = "Years"; case months = "Months"; case days = "Days"
        case hours = "Hours"; case minutes = "Minutes"
        var id: String { rawValue }
        var ms: Int64 {
            switch self {
            case .years: return 31_536_000_000
            case .months: return 2_592_000_000
            case .days: return 86_400_000
            case .hours: return 3_600_000
            case .minutes: return 60_000
            }
        }
    }

    enum OffsetDirection: String, CaseIterable, Identifiable {
        case later = "Later"; case before = "Before"
        var id: String { rawValue }
    }

    private var greyShade: String {
        // Assign a shade based on a hash of the source name for consistency
        let shades = ["#8A8A8A", "#6A6A6A", "#AAAAAA", "#555555", "#BBBBBB"]
        let idx = abs(sourceName.hashValue) % shades.count
        return shades[idx]
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            Text("Import Timeline")
                .font(.headline)
            VStack(alignment: .leading, spacing: 4) {
                Text("Source: \(sourceName)").font(.subheadline)
                Text("Epoch: \(sourceEpochLabel)").font(.caption).foregroundStyle(.secondary)
            }
            Divider()
            Text("Epoch offset from this project's story open:")
                .font(.caption).foregroundStyle(.secondary)
            HStack(spacing: 8) {
                TextField("", value: $amount, format: .number)
                    .frame(width: 60).textFieldStyle(.roundedBorder)
                    .multilineTextAlignment(.center)
                Picker("", selection: $unit) {
                    ForEach(OffsetUnit.allCases) { Text($0.rawValue).tag($0) }
                }.pickerStyle(.menu).frame(width: 100)
                Picker("", selection: $direction) {
                    ForEach(OffsetDirection.allCases) { Text($0.rawValue).tag($0) }
                }.pickerStyle(.menu).frame(width: 80)
            }
            Text("Events from \"\(sourceName)\" will be offset by this amount relative to your project's story timeline.")
                .font(.caption).foregroundStyle(.secondary)
            Divider()
            HStack {
                Button("Cancel") { onCancel() }.keyboardShortcut(.cancelAction)
                Spacer()
                Button("Import") {
                    let ms = Int64(max(amount, 0)) * unit.ms
                    let signed = direction == .later ? ms : -ms
                    onCommit(signed, greyShade)
                }.keyboardShortcut(.defaultAction).buttonStyle(.borderedProminent)
            }
        }
        .padding()
        .frame(width: 380)
    }
}

// MARK: — HistoricalEventEditorSheet

private struct HistoricalEventEditorSheet: View {

    @Binding var title: String
    @Binding var description: String
    let onSave: () -> Void
    let onCancel: () -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Edit Historical Event").font(.headline)
            TextField("Title", text: $title).textFieldStyle(.roundedBorder)
            TextField("Description (optional)", text: $description).textFieldStyle(.roundedBorder)
            Divider()
            HStack {
                Button("Cancel") { onCancel() }.keyboardShortcut(.cancelAction)
                Spacer()
                Button("Save") { onSave() }
                    .keyboardShortcut(.defaultAction).buttonStyle(.borderedProminent)
                    .disabled(title.trimmingCharacters(in: .whitespaces).isEmpty)
            }
        }
        .padding()
        .frame(width: 320)
    }
}

// MARK: — SimpleTooltipView
//
// Lightweight tooltip for historical event and imported event dots.
// Always renders above the anchor row; no fallback-to-below logic.

private struct SimpleTooltipView: View {

    let title: String
    let subtitle: String
    let anchorX: CGFloat
    let lineY: CGFloat
    let panelWidth: CGFloat

    private let tipWidth: CGFloat = 200
    private let tipHeight: CGFloat = 60
    private let margin: CGFloat = 8

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(title)
                .font(.headline)
                .lineLimit(2)
            Text(subtitle)
                .font(.caption)
                .foregroundStyle(.secondary)
                .lineLimit(2)
        }
        .padding(10)
        .frame(width: tipWidth)
        .background(
            RoundedRectangle(cornerRadius: 8)
                .fill(.regularMaterial)
                .shadow(color: .black.opacity(0.18), radius: 6, x: 0, y: 2)
        )
        .position(x: clampedX, y: lineY - tipHeight / 2 - 16)
    }

    private var clampedX: CGFloat {
        min(max(anchorX, tipWidth / 2 + margin), panelWidth - tipWidth / 2 - margin)
    }
}

// MARK: — DotTooltipView
//
// Rendered as an overlay on the GeometryReader (outside the clipped ZStack) so it is
// never cut off by panel boundaries. allowsHitTesting(false) is set by the caller.

private struct DotTooltipView: View {

    let dot: TimelineViewModel.SceneDot
    let startX: CGFloat
    let lineY: CGFloat
    let panelWidth: CGFloat
    let bands: [StoryBand]
    let bandRingColor: Color?
    let previousSceneTitle: String
    let previousSceneEndMs: Int64
    let epochLabel: String

    private let tipWidth: CGFloat = 200
    private let tipHeight: CGFloat = 90   // approximate — enough to decide above/below
    private let margin: CGFloat = 8

    var body: some View {
        tooltipContent
            .frame(width: tipWidth)
            .background(
                RoundedRectangle(cornerRadius: 8)
                    .fill(.regularMaterial)
                    .shadow(color: .black.opacity(0.18), radius: 6, x: 0, y: 2)
            )
            .position(x: clampedX, y: tooltipY)
    }

    // Horizontal centre clamped so tooltip stays within panel.
    private var clampedX: CGFloat {
        min(max(startX, tipWidth / 2 + margin), panelWidth - tipWidth / 2 - margin)
    }

    private var tooltipY: CGFloat {
        lineY - tipHeight / 2 - 16
    }

    private var tooltipContent: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(dot.title)
                .font(.headline)
            Text(dot.chapterTitle)
                .font(.caption)
                .foregroundStyle(.secondary)
            if let bc = bandRingColor, !dot.bandID.isEmpty,
               let band = bands.first(where: { $0.bandID == dot.bandID }) {
                HStack(spacing: 4) {
                    Circle().fill(bc).frame(width: 8, height: 8)
                    Text(band.label).font(.caption).foregroundStyle(.secondary)
                }
            }
            Divider()
            Text(storyTimeDescription)
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .padding(10)
    }

    private var storyTimeDescription: String {
        if dot.offsetSource == "default" {
            return "Immediately after \(previousSceneTitle.isEmpty ? "previous scene" : previousSceneTitle)"
        }
        let gap = dot.offsetMs - previousSceneEndMs
        let ref = previousSceneTitle.isEmpty ? epochLabel : "\(previousSceneTitle) ends"
        if gap == 0 { return "Immediately after \(ref)" }
        return humanReadableDuration(ms: gap, epochLabel: ref)
    }
}

// MARK: — ExpandTimelinePopover

private struct ExpandTimelinePopover: View {

    enum Direction { case forward, backward }

    let direction: Direction
    let onExpand: (Int64) -> Void

    @State private var amount: String = "1"
    @State private var unit: DurationUnit = .days

    enum DurationUnit: String, CaseIterable, Identifiable {
        case minutes = "Minutes"
        case hours   = "Hours"
        case days    = "Days"
        case weeks   = "Weeks"
        case months  = "Months"
        case years   = "Years"
        var id: String { rawValue }

        var milliseconds: Int64 {
            switch self {
            case .minutes: return 60_000
            case .hours:   return 3_600_000
            case .days:    return 86_400_000
            case .weeks:   return 604_800_000
            case .months:  return 2_592_000_000
            case .years:   return 31_536_000_000
            }
        }
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text(direction == .forward
                 ? "Expand timeline forward by:"
                 : "Expand timeline backward by:")
                .font(.headline)
            HStack {
                TextField("", text: $amount)
                    .frame(width: 60)
                    .textFieldStyle(.roundedBorder)
                Picker("", selection: $unit) {
                    ForEach(DurationUnit.allCases) { u in Text(u.rawValue).tag(u) }
                }
                .pickerStyle(.menu)
                .frame(width: 100)
            }
            HStack {
                Button("Cancel") { onExpand(0) }
                    .keyboardShortcut(.cancelAction)
                Spacer()
                Button("Expand") {
                    let n = Int64(amount) ?? 1
                    let ms = n * unit.milliseconds
                    onExpand(direction == .forward ? ms : -ms)
                }
                .keyboardShortcut(.defaultAction)
            }
        }
        .padding()
        .frame(width: 280)
    }
}

// MARK: — TimeDeltaPicker

enum TimeDeltaPickerResult {
    case spinner(Int64)
    case anchor(Int64)
    case keepPosition(Int64)
    case resetDefault
}

struct TimeDeltaPicker: View {

    let referenceName: String
    let rawOffsetMs: Int64
    let previousSceneEndMs: Int64
    let currentDurationMs: Int64
    let defaultDurationMs: Int64
    let epochLabel: String
    let onResult: (TimeDeltaPickerResult, Int64) -> Void

    @State private var amount: Int = 1
    @State private var unit: DeltaUnit = .hours
    @State private var direction: DeltaDirection = .later
    @State private var durAmount: Int = 1
    @State private var durUnit: DeltaUnit = .hours

    enum DeltaUnit: String, CaseIterable, Identifiable {
        case minutes = "Minutes"
        case hours   = "Hours"
        case days    = "Days"
        case weeks   = "Weeks"
        case months  = "Months"
        case years   = "Years"
        var id: String { rawValue }

        var milliseconds: Int64 {
            switch self {
            case .minutes: return 60_000
            case .hours:   return 3_600_000
            case .days:    return 86_400_000
            case .weeks:   return 604_800_000
            case .months:  return 2_592_000_000
            case .years:   return 31_536_000_000
            }
        }
    }

    enum DeltaDirection: String, CaseIterable, Identifiable {
        case later  = "Later"
        case before = "Before"
        var id: String { rawValue }
    }

    private struct Anchor: Identifiable {
        let id = UUID()
        let label: String
        let deltaMs: Int64
    }

    private let anchors: [Anchor] = [
        Anchor(label: "that morning",     deltaMs:  21_600_000),
        Anchor(label: "that evening",     deltaMs:  64_800_000),
        Anchor(label: "that night",       deltaMs:  79_200_000),
        Anchor(label: "before dawn",      deltaMs: 100_800_000),
        Anchor(label: "around 3am",       deltaMs:  97_200_000),
        Anchor(label: "the next morning", deltaMs: 108_000_000),
    ]

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {

            Text(referenceName.isEmpty ? "Time from story open:" : "Time after \"\(referenceName)\" ends:")
                .font(.subheadline)
                .foregroundStyle(.secondary)

            HStack(spacing: 8) {
                TextField("", value: $amount, format: .number)
                    .frame(width: 52)
                    .textFieldStyle(.roundedBorder)
                    .multilineTextAlignment(.center)
                Picker("", selection: $unit) {
                    ForEach(DeltaUnit.allCases) { u in Text(u.rawValue).tag(u) }
                }
                .pickerStyle(.menu)
                .frame(width: 90)
                Picker("", selection: $direction) {
                    ForEach(DeltaDirection.allCases) { d in Text(d.rawValue).tag(d) }
                }
                .pickerStyle(.menu)
                .frame(width: 80)
                Button("Set") { onResult(.spinner(spinnerMs), chosenDurationMs) }
                    .keyboardShortcut(.defaultAction)
                    .buttonStyle(.borderedProminent)
            }

            HStack(spacing: 8) {
                Text("Scene duration:")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                TextField("", value: $durAmount, format: .number)
                    .frame(width: 44)
                    .textFieldStyle(.roundedBorder)
                    .multilineTextAlignment(.center)
                Picker("", selection: $durUnit) {
                    ForEach(DeltaUnit.allCases) { u in Text(u.rawValue).tag(u) }
                }
                .pickerStyle(.menu)
                .frame(width: 90)
            }
            .font(.caption)

            Divider()

            Text("Or jump to:")
                .font(.caption)
                .foregroundStyle(.secondary)

            LazyVGrid(columns: [GridItem(.flexible()), GridItem(.flexible()),
                                GridItem(.flexible())], spacing: 6) {
                ForEach(anchors) { anchor in
                    Button {
                        onResult(.anchor(previousSceneEndMs + anchor.deltaMs), chosenDurationMs)
                    } label: {
                        Text(anchor.label)
                            .font(.caption)
                            .multilineTextAlignment(.center)
                            .frame(maxWidth: .infinity, minHeight: 30)
                    }
                    .buttonStyle(.bordered)
                }
            }

            Divider()

            HStack(spacing: 12) {
                Button("Immediately after") { onResult(.resetDefault, defaultDurationMs) }
                    .buttonStyle(.borderless)
                    .font(.caption)
                Button("Keep position") { onResult(.keepPosition(rawOffsetMs), chosenDurationMs) }
                    .buttonStyle(.borderless)
                    .font(.caption)
                Spacer()
                Button("Reset") { onResult(.resetDefault, defaultDurationMs) }
                    .buttonStyle(.borderless)
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
        }
        .onAppear { initialiseSpinner() }
    }

    private var spinnerMs: Int64 {
        let delta = Int64(max(amount, 0)) * unit.milliseconds
        return direction == .later
            ? previousSceneEndMs + delta
            : previousSceneEndMs - delta
    }

    private var chosenDurationMs: Int64 {
        max(1, Int64(max(durAmount, 0)) * durUnit.milliseconds)
    }

    private func initialiseSpinner() {
        let delta = abs(rawOffsetMs - previousSceneEndMs)
        direction = rawOffsetMs >= previousSceneEndMs ? .later : .before
        (unit, amount) = bestFit(ms: delta)
        let dur = currentDurationMs > 0 ? currentDurationMs : defaultDurationMs
        (durUnit, durAmount) = bestFit(ms: dur)
    }

    private func bestFit(ms: Int64) -> (DeltaUnit, Int) {
        if ms >= DeltaUnit.years.milliseconds {
            return (.years,   max(1, Int(ms / DeltaUnit.years.milliseconds)))
        } else if ms >= DeltaUnit.months.milliseconds {
            return (.months,  max(1, Int(ms / DeltaUnit.months.milliseconds)))
        } else if ms >= DeltaUnit.weeks.milliseconds {
            return (.weeks,   max(1, Int(ms / DeltaUnit.weeks.milliseconds)))
        } else if ms >= DeltaUnit.days.milliseconds {
            return (.days,    max(1, Int(ms / DeltaUnit.days.milliseconds)))
        } else if ms >= DeltaUnit.hours.milliseconds {
            return (.hours,   max(1, Int(ms / DeltaUnit.hours.milliseconds)))
        } else {
            return (.minutes, max(1, Int(ms / max(DeltaUnit.minutes.milliseconds, 1))))
        }
    }
}

// MARK: — Scroll capture (macOS) for timeline pan and zoom (FR-009)

#if os(macOS)
/// Wraps the scroll-event monitor lifecycle. Attach via .background so the SwiftUI content
/// layer receives all hit-testing while this view intercepts scroll-wheel events.
struct TimelineScrollCaptureView: NSViewRepresentable {
    var onScroll: (_ dx: CGFloat, _ dy: CGFloat) -> Void

    func makeNSView(context: Context) -> _TimelineScrollNSView {
        let v = _TimelineScrollNSView()
        v.onScroll = onScroll
        return v
    }

    func updateNSView(_ nsView: _TimelineScrollNSView, context: Context) {
        nsView.onScroll = onScroll
    }
}

// BANDDIAG: passive raw-pointer observer. Logs left mouse down/drag in panel-local coords
// WITHOUT consuming events, so we can tell whether SwiftUI is even delivering pointer events
// to the band gesture (gesture logs absent + these present == routing failure).
struct BandPointerProbeView: NSViewRepresentable {
    func makeNSView(context: Context) -> _BandPointerProbeNSView { _BandPointerProbeNSView() }
    func updateNSView(_ nsView: _BandPointerProbeNSView, context: Context) {}
}

@MainActor final class _BandPointerProbeNSView: NSView {
    private var monitor: Any?
    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        monitor.map { NSEvent.removeMonitor($0) }
        guard window != nil else { monitor = nil; return }
        monitor = NSEvent.addLocalMonitorForEvents(matching: [.leftMouseDown, .leftMouseDragged]) { [weak self] event in
            guard let self else { return event }
            let eventWindow = event.window
            let loc = event.locationInWindow
            let kind = event.type == .leftMouseDown ? "DOWN" : "DRAG"
            MainActor.assumeIsolated {
                guard let win = self.window, eventWindow === win else { return }
                let pt = self.convert(loc, from: nil)
                if self.bounds.contains(pt) {
                    NSLog("BANDDIAG RAW-\(kind) panelLocalX=\(pt.x) y=\(pt.y) bounds=\(self.bounds)")
                }
            }
            return event   // never consume — pure observation
        }
    }
    override func viewWillMove(toWindow newWindow: NSWindow?) {
        if newWindow == nil { monitor.map { NSEvent.removeMonitor($0) }; monitor = nil }
        super.viewWillMove(toWindow: newWindow)
    }
}

@MainActor final class _TimelineScrollNSView: NSView {
    var onScroll: ((_ dx: CGFloat, _ dy: CGFloat) -> Void)?
    private var monitor: Any?

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        monitor.map { NSEvent.removeMonitor($0) }
        guard window != nil else { monitor = nil; return }
        monitor = NSEvent.addLocalMonitorForEvents(matching: .scrollWheel) { [weak self] event in
            guard let self else { return event }
            // Extract the values we need from the (non-Sendable) NSEvent here so the
            // event itself never crosses into the main-actor closure below.
            let eventWindow    = event.window
            let momentumPhase  = event.momentumPhase
            let phase          = event.phase
            let locationInWindow = event.locationInWindow
            let deltaX         = event.scrollingDeltaX
            let deltaY         = event.scrollingDeltaY
            let consume = MainActor.assumeIsolated { () -> Bool in
                guard let win = self.window, eventWindow === win else { return false }
                // Only consume trackpad scroll/momentum phases.
                if !momentumPhase.isEmpty || !phase.isEmpty {
                    let pt = self.convert(locationInWindow, from: nil)
                    if self.bounds.contains(pt) {
                        self.onScroll?(deltaX, deltaY)
                        return true
                    }
                }
                return false
            }
            return consume ? nil : event
        }
    }

    override func viewWillMove(toWindow newWindow: NSWindow?) {
        if newWindow == nil {
            monitor.map { NSEvent.removeMonitor($0) }
            monitor = nil
        }
        super.viewWillMove(toWindow: newWindow)
    }
}
#endif

// MARK: — Human-readable duration

func humanReadableDuration(ms: Int64, epochLabel: String) -> String {
    if ms == 0 { return epochLabel }
    let absMs     = abs(ms)
    let direction = ms >= 0 ? "after" : "before"

    let years   = absMs / 31_536_000_000
    let rem1    = absMs % 31_536_000_000
    let months  = rem1  / 2_592_000_000
    let rem2    = rem1  % 2_592_000_000
    let days    = rem2  / 86_400_000
    let rem3    = rem2  % 86_400_000
    let hours   = rem3  / 3_600_000
    let minutes = (rem3 % 3_600_000) / 60_000

    var parts: [String] = []
    if years   > 0 { parts.append("\(years) year\(years == 1 ? "" : "s")") }
    if months  > 0 { parts.append("\(months) month\(months == 1 ? "" : "s")") }
    if days    > 0 { parts.append("\(days) day\(days == 1 ? "" : "s")") }
    if hours   > 0 { parts.append("\(hours) hour\(hours == 1 ? "" : "s")") }
    if minutes > 0 { parts.append("\(minutes) minute\(minutes == 1 ? "" : "s")") }

    if parts.isEmpty { parts = ["< 1 minute"] }
    return parts.prefix(2).joined(separator: ", ") + " \(direction) \(epochLabel)"
}

// MARK: — Cursor helper (macOS only)

#if os(macOS)
extension View {
    @ViewBuilder
    func cursor(_ cursor: NSCursor) -> some View {
        self.onHover { inside in
            if inside { cursor.push() } else { NSCursor.pop() }
        }
    }
}
#endif

// MARK: — Color hex initialiser

extension Color {
    init?(hex: String) {
        let h = hex.hasPrefix("#") ? String(hex.dropFirst()) : hex
        guard h.count == 6, let value = UInt64(h, radix: 16) else { return nil }
        let r = Double((value >> 16) & 0xFF) / 255
        let g = Double((value >>  8) & 0xFF) / 255
        let b = Double( value        & 0xFF) / 255
        self.init(red: r, green: g, blue: b)
    }
}

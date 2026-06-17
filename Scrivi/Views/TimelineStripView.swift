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
private struct ImportedTimelineFile: Decodable {
    struct Event: Decodable {
        let eventID: String
        let title: String
        let offsetMs: Int64
        let kind: String
        var notes: String?
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
        events             = (try? c.decode([Event].self, forKey: .events)) ?? []
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

    private(set) var dots: [SceneDot] = []
    private(set) var epochLabel: String = "Story Open"
    private(set) var defaultSceneDurationMs: Int64 = 3_600_000

    // Hovered IDs — set by child dot views, read by TimelineStripView to render tooltips
    // in the panel-level overlay outside .clipped() so they are never cut off.
    var hoveredDotID: String? = nil
    var hoveredHistoricalEventID: String? = nil
    var hoveredImportedEventKey: String? = nil   // "timelineID:eventID"

    // Historical events
    private(set) var historicalEvents: [HistoricalEventDot] = []

    // Imported timelines
    private(set) var importedTimelines: [ImportedTimelineRow] = []

    // Story structure state — loaded from disk via ScriviEngine
    private(set) var activeBands: [StoryBand] = []          // empty == no structure active
    private(set) var activeStructureID: String = ""

    var minOffsetMs: Int64 {
        let sceneMin = dots.map(\.offsetMs).min() ?? 0
        let heMin    = historicalEvents.map(\.offsetMs).min() ?? sceneMin
        return min(sceneMin, heMin)
    }
    var maxEndMs: Int64 {
        let sceneMax = dots.map { $0.offsetMs + $0.durationMs }.max() ?? 1
        let heMax    = historicalEvents.map(\.offsetMs).max() ?? sceneMax
        return max(sceneMax, heMax)
    }
    var spanMs: Int64 { max(maxEndMs - minOffsetMs, 1) }

    func load(engine: ScriviEngine, projectRootPath: String, scenes: [SceneInfo]) {
        epochLabel = (try? engine.getTimeline(projectRootPath: projectRootPath))?.epochLabel ?? "Story Open"

        var raw: [SceneDot] = scenes.enumerated().map { idx, info in
            let st = try? engine.getSceneStoryTime(
                projectRootPath: projectRootPath, sceneID: info.sceneID)
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
        loadImportedTimelines(projectRootPath: projectRootPath)
    }

    // Reload only the scene dots from an updated scene list.
    // Used after scene/chapter creation, split, or merge so the timeline stays in sync
    // without resetting historical events, imported timelines, or story structure.
    func reloadSceneDots(engine: ScriviEngine, projectRootPath: String, scenes: [SceneInfo]) {
        var raw: [SceneDot] = scenes.enumerated().map { idx, info in
            // Preserve any existing dot's bandID so band assignments survive the reload.
            let existingBandID = dots.first(where: { $0.sceneID == info.sceneID })?.bandID ?? ""
            let st = try? engine.getSceneStoryTime(
                projectRootPath: projectRootPath, sceneID: info.sceneID)
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

    // Patch dot titles to match the Scene Navigator's display logic exactly:
    //   1. Explicit engine title (info.title non-empty) → use it unchanged.
    //   2. No explicit title → use liveTitles first-line text if available.
    //   3. Neither → keep the existing "Scene N" ordinal fallback on the dot.
    // `allScenes` is passed so we can check info.title without a separate lookup path.
    func updateDotTitles(liveTitles: [String: String], allScenes: [SceneInfo]) {
        for i in dots.indices {
            let sceneID = dots[i].sceneID
            // Find the canonical title from allScenes.
            if let info = allScenes.first(where: { $0.sceneID == sceneID }) {
                if !info.title.trimmingCharacters(in: .whitespaces).isEmpty {
                    // Explicit title always wins.
                    dots[i].title = info.title
                } else if let live = liveTitles[sceneID],
                          !live.trimmingCharacters(in: .whitespaces).isEmpty {
                    // No explicit title — use first-line live text.
                    dots[i].title = live
                }
                // Otherwise leave the "Scene N" fallback already on the dot.
            }
        }
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

    // Reads imported timeline metadata from engine, then reads full event arrays from disk
    // (the stored files include events; listImportedTimelines returns metadata only).
    func loadImportedTimelines(projectRootPath: String) {
        let dir = projectRootPath + "/objects/imported-timelines"
        guard let entries = try? FileManager.default.contentsOfDirectory(atPath: dir) else {
            importedTimelines = []
            return
        }
        var rows: [ImportedTimelineRow] = []
        for filename in entries where filename.hasSuffix(".scrivi-timeline.json") {
            let path = dir + "/" + filename
            guard let data = try? Data(contentsOf: URL(fileURLWithPath: path)),
                  let json = try? JSONDecoder().decode(ImportedTimelineFile.self, from: data) else {
                continue
            }
            let events = json.events.map { ev in
                ImportedEventDot(
                    id: ev.eventID, eventID: ev.eventID, title: ev.title,
                    sourceOffsetMs: ev.offsetMs, kind: ev.kind,
                    projectOffsetMs: ev.offsetMs + json.epochOffsetMs)
            }
            rows.append(ImportedTimelineRow(
                id: json.timelineID, timelineID: json.timelineID,
                sourceName: json.sourceProjectTitle.isEmpty ? "Imported Timeline" : json.sourceProjectTitle,
                epochLabel: json.epochLabel, epochOffsetMs: json.epochOffsetMs,
                visible: json.visible, greyShade: json.assignedGreyShade.isEmpty ? "#8A8A8A" : json.assignedGreyShade,
                events: events))
        }
        importedTimelines = rows
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

    @State private var showEpochOffsetDialog = false
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
    private let dotRadius: CGFloat = 7
    private let lineThickness: CGFloat = 2

    private let importedRowHeight: CGFloat = 32

    // MARK: Zoom & pan state (FR-009, FR-032)
    // zoomFactor 1.0 = full span visible; higher values shrink the visible window so dots spread apart.
    // magnifyGestureScale accumulates during a pinch gesture and is folded into zoomFactor on end.
    @State private var zoomFactor: CGFloat = 1.0
    @GestureState private var magnifyGestureScale: CGFloat = 1.0
    // scrollOffsetFraction 0.0 = leftmost edge of full span; 1.0 = rightmost.
    @State private var scrollOffsetFraction: CGFloat = 0.0
    private let maxZoom: CGFloat = 50.0

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


    private var minPanelHeight: CGFloat {
        let bandExtra: CGFloat = model.activeBands.isEmpty ? 0 : labelRowHeight
        let importedExtra = CGFloat(model.importedTimelines.filter(\.visible).count) * importedRowHeight
        return minPanelHeightBase + bandExtra + importedExtra
    }

    var body: some View {
        VStack(spacing: 0) {
            topEdgeHandle
            Divider()
            panelHeader
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

                ZStack(alignment: .topLeading) {
                    // I-0036: hit-testable background so the ZStack-level context menu fires
                    // on empty space. Must be first (bottom layer) so all child views on top
                    // of it win hit-testing when clicked directly.
                    Color.clear.contentShape(Rectangle())

                    // Band background — drawn behind everything
                    if !model.activeBands.isEmpty {
                        BandOverlayView(
                            bands: model.activeBands,
                            panelWidth: panelW,
                            panelHeight: geo.size.height,
                            labelRowHeight: labelRowHeight,
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
                    let clusters = buildClusters(usable: usable, panelW: panelW)
                    ForEach(clusters, id: \.centerItemID) { cluster in
                        // All ring offsets are relative to the cluster center X (first member
                        // after X-sort). Each dot's own baseX is still passed as startX to the
                        // dot view so drag/story-time calculations remain correct.
                        let centerX = itemX(cluster.members[0], usable: usable, panelW: panelW)
                        ForEach(Array(cluster.members.enumerated()), id: \.element.id) { i, item in
                            let baseX         = itemX(item, usable: usable, panelW: panelW)
                            let offset        = clusterOffset(position: i, clusterSize: cluster.members.count, radius: dotRadius)
                            let posX          = centerX + offset.width
                            let posY          = lineY - offset.height
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
                                    lineY: posY,
                                    labelRowHeight: model.activeBands.isEmpty ? 0 : labelRowHeight,
                                    bands: model.activeBands,
                                    bandRingColor: bandColor,
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
                                    }
                                )
                                .position(x: posX, y: posY)
                            case .historical(let event):
                                HistoricalEventDotView(
                                    event: event,
                                    radius: dotRadius,
                                    startX: baseX,
                                    panelWidth: panelW,
                                    lineY: posY,
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
                                .position(x: posX, y: posY)
                            }
                        }
                        // Count badge when cluster is too tall for the panel
                        if cluster.members.count > 1 {
                            let tallest = CGFloat(cluster.ringCount) * (dotRadius * 2 + 4)
                            if tallest > lineY - (model.activeBands.isEmpty ? 0 : labelRowHeight) {
                                let cx = itemX(cluster.members[0], usable: usable, panelW: panelW)
                                Text("\(cluster.members.count)")
                                    .font(.system(size: 8, weight: .bold))
                                    .foregroundStyle(.white)
                                    .padding(.horizontal, 4).padding(.vertical, 2)
                                    .background(Capsule().fill(Color.accentColor))
                                    .position(x: cx, y: lineY - tallest - 8)
                                    .allowsHitTesting(false)
                            }
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
                        // Dividing line
                        Rectangle()
                            .fill(Color.secondary.opacity(0.2))
                            .frame(width: max(usable, 1), height: 1)
                            .position(x: panelW / 2, y: rowY)
                        // Event dots — clustered per row so co-located events don't overlap.
                        let importedRadius = dotRadius * 0.7
                        let visibleEvents = timeline.events.filter {
                            let x = eventX(offsetMs: $0.projectOffsetMs, usable: usable, panelW: panelW)
                            return x >= 16 && x <= panelW - 16
                        }
                        let importedClusters = buildImportedRowClusters(
                            events: visibleEvents, usable: usable, panelW: panelW,
                            radius: importedRadius)
                        ForEach(importedClusters, id: \.centerEventID) { iCluster in
                            let iCenterX = eventX(offsetMs: iCluster.members[0].projectOffsetMs,
                                                  usable: usable, panelW: panelW)
                            ForEach(Array(iCluster.members.enumerated()), id: \.element.id) { i, ev in
                                let offset = clusterOffset(position: i, clusterSize: iCluster.members.count, radius: importedRadius)
                                let key    = "\(timeline.timelineID):\(ev.eventID)"
                                ImportedEventDotView(
                                    ev: ev,
                                    color: timeline.swiftUIColor,
                                    radius: importedRadius,
                                    onHoverChanged: { hovered in
                                        model.hoveredImportedEventKey = hovered ? key : nil
                                    }
                                )
                                .position(x: iCenterX + offset.width, y: rowY - offset.height)
                            }
                            if iCluster.members.count > 1 {
                                let tallest = CGFloat(iCluster.ringCount) * (importedRadius * 2 + 4)
                                if tallest > rowSpacing / 2 {
                                    let cx = eventX(offsetMs: iCluster.members[0].projectOffsetMs,
                                                    usable: usable, panelW: panelW)
                                    Text("\(iCluster.members.count)")
                                        .font(.system(size: 7, weight: .bold))
                                        .foregroundStyle(.white)
                                        .padding(.horizontal, 3).padding(.vertical, 1)
                                        .background(Capsule().fill(Color.secondary))
                                        .position(x: cx, y: rowY - tallest - 6)
                                        .allowsHitTesting(false)
                                }
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
                                let usable = geo.size.width - 32
                                let panFraction = -dx / usable
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
                    _ = try? engine.importExternalTimeline(
                        projectRootPath: projectRootPath,
                        timelineJSON: pendingImportJSON,
                        epochOffsetMs: offsetMs,
                        assignedGreyShade: greyShade)
                    model.loadImportedTimelines(projectRootPath: projectRootPath)
                    showEpochOffsetDialog = false
                },
                onCancel: { showEpochOffsetDialog = false }
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
        var ringCount: Int { members.count <= 1 ? 0 : members.count <= 7 ? 1 : 2 }
    }

    // Groups all main-row items (scene dots + historical event dots) within one dot-diameter
    // of each other into clusters. Items are sorted by X first, then grouped using a
    // contiguous-window pass so grouping is transitive: any item within one diameter of the
    // rightmost current cluster member joins, not just items near the first member.
    private func buildClusters(usable: CGFloat, panelW: CGFloat) -> [DotCluster] {
        let diameter = dotRadius * 2
        var allItems: [MainRowItem] = model.dots.map { .scene($0) }
            + model.historicalEvents.map { .historical($0) }
        allItems.sort { itemX($0, usable: usable, panelW: panelW) < itemX($1, usable: usable, panelW: panelW) }

        var clusters: [DotCluster] = []
        var i = 0
        while i < allItems.count {
            let anchor = allItems[i]
            var members = [anchor]
            var clusterMaxX = itemX(anchor, usable: usable, panelW: panelW)
            var j = i + 1
            while j < allItems.count {
                let ox = itemX(allItems[j], usable: usable, panelW: panelW)
                if ox - clusterMaxX <= diameter {
                    members.append(allItems[j])
                    clusterMaxX = ox
                    j += 1
                } else {
                    break
                }
            }
            clusters.append(DotCluster(centerItemID: anchor.id, members: members))
            i = j
        }
        return clusters
    }

    private func itemX(_ item: MainRowItem, usable: CGFloat, panelW: CGFloat) -> CGFloat {
        eventX(offsetMs: item.offsetMs, usable: usable, panelW: panelW)
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
        let diameter = radius * 2
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
                if ox - clusterMaxX <= diameter {
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
                  let jsonStr = try? String(contentsOf: url, encoding: .utf8) else { return }
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
            try? result.timelineJSON.write(to: url, atomically: true, encoding: .utf8)
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
    var panelWidth: CGFloat
    var panelHeight: CGFloat
    var labelRowHeight: CGFloat
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
        .simultaneousGesture(bandBorderDragGesture)   // I-0033: coexist with dot gestures
        .onAppear { workingBands = bands }
        .onChange(of: bands) { _, new in workingBands = new }
        // I-0032: change cursor to horizontal resize when near a band border
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
        // Label row — rendered on top of the canvas
        .overlay(alignment: .topLeading) {
            bandLabelRow
        }
    }

    // Draw the band rectangles into the canvas. The label row height is reserved at top.
    private func drawBands(ctx: inout GraphicsContext, size: CGSize) {
        let effectiveBands = workingBands.isEmpty ? bands : workingBands
        var x: CGFloat = 0
        for band in effectiveBands {
            let w = CGFloat(band.proportion) * size.width
            let rect = CGRect(x: x, y: labelRowHeight,
                              width: w, height: size.height - labelRowHeight)
            ctx.fill(Path(rect), with: .color((Color(hex: band.color) ?? .accentColor).opacity(0.18)))
            x += w
        }
    }

    // Render band labels in the label row. Labels truncate with ellipsis.
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
                    .frame(width: CGFloat(band.proportion) * panelWidth,
                           height: labelRowHeight, alignment: .leading)
            }
        }
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
                guard let borderIdx = draggingBorderIndex else { return }
                workingBands = redistributeProportions(
                    draggedBorderIndex: borderIdx,
                    newX: x,
                    bands: workingBands,
                    totalWidth: panelWidth
                )
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
    private func nearestBorderIndex(at x: CGFloat) -> Int? {
        var accumulated: CGFloat = 0
        let effectiveBands = workingBands.isEmpty ? bands : workingBands
        for i in 0..<effectiveBands.count - 1 {   // last band has no right-border to drag
            accumulated += CGFloat(effectiveBands[i].proportion) * panelWidth
            if abs(accumulated - x) < 8 { return i }
        }
        return nil
    }

    // Move border at `draggedBorderIndex`: clamp so neither adjacent band goes below 4%.
    private func redistributeProportions(draggedBorderIndex: Int, newX: CGFloat,
                                          bands: [StoryBand], totalWidth: CGFloat) -> [StoryBand] {
        guard totalWidth > 0 else { return bands }
        var result = bands
        let minProportion = 0.04

        // Compute current left edge of the dragged border
        var leftEdge: CGFloat = 0
        for i in 0...draggedBorderIndex {
            leftEdge += (i < draggedBorderIndex)
                ? CGFloat(result[i].proportion) * totalWidth
                : 0
        }

        let clampedX = max(leftEdge + CGFloat(minProportion) * totalWidth,
                           min(newX, totalWidth - CGFloat(minProportion) * totalWidth))
        let newLeftProp = Double(clampedX / totalWidth)

        // Sum of all bands up to and including the dragged band's left
        var sumBefore: Double = 0
        for i in 0..<draggedBorderIndex { sumBefore += result[i].proportion }

        let leftBandNewProp = max(minProportion, newLeftProp - sumBefore)
        let rightBandNewProp = max(minProportion,
                                   result[draggedBorderIndex].proportion
                                   + result[draggedBorderIndex + 1].proportion
                                   - leftBandNewProp)

        result[draggedBorderIndex].proportion = leftBandNewProp
        result[draggedBorderIndex + 1].proportion = rightBandNewProp
        return result
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
    let previousSceneEndMs: Int64
    let previousSceneTitle: String
    let defaultDurationMs: Int64
    let computeOffsetMs: (CGFloat) -> Int64
    let onCommit: (TimeDeltaPickerResult, Int64) -> Void
    let onAssignToBand: (String) -> Void
    let onUnassign: () -> Void
    let onHoverChanged: (Bool) -> Void

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

            // Core dot — scale up on hover, back to normal on press/drag
            Circle()
                .fill(Color.accentColor)
                .frame(width: radius * 2, height: radius * 2)
                .scaleEffect(isDragging ? 1.3 : (isHovered ? 1.2 : 1.0))
                .animation(.easeOut(duration: 0.15), value: isDragging)
                .animation(.easeOut(duration: 0.1), value: isHovered)

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
            Button("View Scene") { }
                .disabled(true)
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
                    // Horizontal drag completed — show Time Delta Picker
                    let finalPanelX = startX + v.translation.width
                    pendingOffsetMs = computeOffsetMs(finalPanelX)
                    showPicker = true
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
        var accumulated: CGFloat = 0
        for band in bands {
            accumulated += CGFloat(band.proportion) * panelWidth
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

import SwiftUI

// MARK: — TimelineViewModel

@Observable @MainActor final class TimelineViewModel {

    struct SceneDot: Identifiable {
        let id: String              // == sceneID
        let sceneID: String
        let title: String
        let chapterTitle: String
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
    private(set) var defaultSceneDurationMs: Int64 = 3_600_000  // project default, 1 hour

    var minOffsetMs: Int64 { dots.map(\.offsetMs).min() ?? 0 }
    var maxOffsetMs: Int64 { dots.map(\.offsetMs).max() ?? 1 }
    // Max end = last dot's offset + its duration
    var maxEndMs: Int64 { dots.map { $0.offsetMs + $0.durationMs }.max() ?? 1 }
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
                offsetMs: 0,                                        // computed below
                offsetSource: st?.offsetSource ?? "default",
                gapMs: st?.gapMs ?? 0,
                durationMs: dur > 0 ? dur : defaultSceneDurationMs,
                durationSource: st?.durationSource ?? "default",
                bandID: st?.bandID ?? ""
            )
        }
        recomputeAllOffsets(in: &raw)
        dots = raw
    }

    // Commit a position change. The picker gives us the absolute target offsetMs and the
    // previousSceneEndMs it used — from these we compute and store gapMs.
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
        // Recompute all subsequent scenes from their stored gapMs values.
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

    // Compute offsetMs for every dot from its gapMs and its predecessor's end.
    // "default" scenes have gapMs == 0 (start immediately when predecessor ends).
    // "manual" scenes have gapMs == the gap the writer specified.
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

    // Recompute and persist from `startIdx` to end of list.
    // Stops at nothing — all scenes recompute because every scene's
    // absolute position depends on its predecessor's absolute position.
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
}

// MARK: — TimelineStripView

struct TimelineStripView: View {

    var model: TimelineViewModel
    var engine: ScriviEngine
    var projectRootPath: String

    @State private var panelHeight: CGFloat = 100
    private let minPanelHeight: CGFloat = 64
    private let dotRadius: CGFloat = 7
    private let lineThickness: CGFloat = 2

    var body: some View {
        VStack(spacing: 0) {
            topEdgeHandle
            Divider()
            GeometryReader { geo in
                let lineY    = geo.size.height / 2
                let usable   = geo.size.width - 32
                let panelW   = geo.size.width

                ZStack {
                    // Timeline line
                    Rectangle()
                        .fill(Color.secondary.opacity(0.35))
                        .frame(width: max(usable, 1), height: lineThickness)
                        .position(x: panelW / 2, y: lineY)

                    if model.dots.isEmpty {
                        Text("Scenes will appear here as you write.")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                            .position(x: panelW / 2, y: lineY)
                    }

                    // Each dot knows its own panel-X and the full panel width —
                    // no coordinate conversion needed inside the dot.
                    ForEach(model.dots) { dot in
                        let startX    = dotX(for: dot, usable: usable, panelW: panelW)
                        // previousSceneEndMs is the base for all "time since previous scene"
                        // arithmetic: previous scene's start + its duration.
                        let prevEndMs = previousSceneEndMs(for: dot)
                        let prevTitle = previousSceneTitle(for: dot)
                        SceneDotView(
                            dot: dot,
                            radius: dotRadius,
                            epochLabel: model.epochLabel,
                            startX: startX,
                            panelWidth: panelW,
                            previousSceneEndMs: prevEndMs,
                            previousSceneTitle: prevTitle,
                            defaultDurationMs: model.defaultSceneDurationMs,
                            computeOffsetMs: { finalPanelX in
                                offsetMs(fromPanelX: finalPanelX, usable: usable)
                            },
                            onCommit: { result, durationMs in
                                applyPickerResult(result, for: dot, pickerDurationMs: durationMs)
                            }
                        )
                        .position(x: startX, y: lineY)
                    }
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .clipped()
            }
            .frame(height: panelHeight)
        }
    }

    // MARK: Resize handle

    private var topEdgeHandle: some View {
        Color.clear
            .frame(height: 6)
            .contentShape(Rectangle())
            .cursor(.resizeUpDown)
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
        guard model.dots.count > 1, model.spanMs > 0 else { return 16 + dotRadius }
        let fraction = CGFloat(dot.offsetMs - model.minOffsetMs) / CGFloat(model.spanMs)
        return 16 + fraction * usable
    }

    private func offsetMs(fromPanelX x: CGFloat, usable: CGFloat) -> Int64 {
        guard model.dots.count > 1 else { return 0 }
        let fraction = (x - 16) / usable
        return model.minOffsetMs + Int64(fraction * CGFloat(model.spanMs))
    }

    private func applyPickerResult(_ result: TimeDeltaPickerResult,
                                   for dot: TimelineViewModel.SceneDot,
                                   pickerDurationMs: Int64) {
        let dur = pickerDurationMs > 0 ? pickerDurationMs : model.defaultSceneDurationMs
        let prevEnd = previousSceneEndMs(for: dot)
        switch result {
        case .spinner(let ms), .anchor(let ms), .keepPosition(let ms):
            let gap = ms - prevEnd   // gap from previous scene's end to this scene's start
            model.setStoryTime(sceneID: dot.sceneID, offsetMs: ms, offsetSource: "manual",
                               gapMs: gap, durationMs: dur, durationSource: "manual",
                               engine: engine, projectRootPath: projectRootPath)
        case .resetDefault:
            model.resetToDefault(sceneID: dot.sceneID, engine: engine,
                                 projectRootPath: projectRootPath)
        }
    }

    // Returns the story-time end of the scene immediately before `dot` in manuscript order:
    // previousScene.offsetMs + previousScene.durationMs.
    // This is the reference point for all "time since previous scene" arithmetic.
    // Returns 0 for the first scene (no predecessor).
    private func previousSceneEndMs(for dot: TimelineViewModel.SceneDot) -> Int64 {
        guard let idx = model.dots.firstIndex(where: { $0.id == dot.id }), idx > 0 else {
            return 0
        }
        let prev = model.dots[idx - 1]
        return prev.offsetMs + prev.durationMs
    }

    // Returns the title of the scene immediately before `dot` in manuscript order,
    // used in picker and tooltip reference text.
    private func previousSceneTitle(for dot: TimelineViewModel.SceneDot) -> String {
        guard let idx = model.dots.firstIndex(where: { $0.id == dot.id }), idx > 0 else {
            return ""
        }
        return model.dots[idx - 1].title
    }
}

// MARK: — SceneDotView
//
// Fix for I-0025 and I-0026:
// • No GeometryReader inside this view. Parent passes startX (panel coords) and panelWidth
//   directly, eliminating all three coordinate-space bugs.
// • The dot owns its own picker state and presents TimeDeltaPicker as a popover on
//   the dot circle itself — concrete anchor, no optional-unwrapping race.

private struct SceneDotView: View {

    let dot: TimelineViewModel.SceneDot
    let radius: CGFloat
    let epochLabel: String
    let startX: CGFloat
    let panelWidth: CGFloat
    let previousSceneEndMs: Int64       // previousScene.offsetMs + previousScene.durationMs
    let previousSceneTitle: String      // for tooltip and picker header
    let defaultDurationMs: Int64
    let computeOffsetMs: (CGFloat) -> Int64
    let onCommit: (TimeDeltaPickerResult, Int64) -> Void

    @State private var dragOffsetX: CGFloat = 0
    @State private var isDragging = false
    @State private var showTooltip = false
    @State private var showPicker = false
    @State private var pendingOffsetMs: Int64 = 0

    var body: some View {
        ZStack {
            // Outer ring — manual / inferred indicator
            if dot.offsetSource == "manual" {
                Circle()
                    .strokeBorder(Color.accentColor, lineWidth: 2)
                    .frame(width: radius * 2 + 6, height: radius * 2 + 6)
            } else if dot.offsetSource == "inferred" {
                Circle()
                    .strokeBorder(Color.accentColor.opacity(0.6), lineWidth: 1.5)
                    .frame(width: radius * 2 + 6, height: radius * 2 + 6)
            }

            Circle()
                .fill(Color.accentColor)
                .frame(width: radius * 2, height: radius * 2)
                .scaleEffect(isDragging ? 1.3 : 1.0)
                .animation(.easeOut(duration: 0.15), value: isDragging)
        }
        .frame(width: radius * 2 + 8, height: radius * 2 + 8)
        .contentShape(Circle().size(CGSize(width: radius * 2 + 8, height: radius * 2 + 8)))
        // Popovers on the ZStack, not the inner Circle — avoids the hit-test
        // interceptor that was blocking the drag gesture (I-0028 fix).
        .popover(isPresented: $showTooltip, arrowEdge: .top) {
            dotTooltip
        }
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
        // Context menu — I-0029 fix
        .contextMenu {
            Button("Set Time Delta…") {
                pendingOffsetMs = computeOffsetMs(startX)   // current position as base
                showPicker = true
            }
            Divider()
            Button("View Scene") { }
                .disabled(true)
            Divider()
            Button("Assign to Act…") { }
                .disabled(true)
            Button("Unassign from Act") { }
                .disabled(true)
        }
        .onHover { over in if !isDragging { showTooltip = over } }
        .offset(x: dragOffsetX)
        // I-0028 fix: .simultaneousGesture lets the drag coexist with popover
        // hit-testing instead of being blocked by it.
        .simultaneousGesture(
            DragGesture(minimumDistance: 4)
                .onChanged { v in
                    if !isDragging { isDragging = true; showTooltip = false }
                    dragOffsetX = v.translation.width
                }
                .onEnded { v in
                    isDragging = false
                    dragOffsetX = 0
                    let finalPanelX = startX + v.translation.width
                    pendingOffsetMs = computeOffsetMs(finalPanelX)
                    showPicker = true
                }
        )
    }

    // MARK: Tooltip

    private var dotTooltip: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(dot.title)
                .font(.headline)
            Text(dot.chapterTitle)
                .font(.caption)
                .foregroundStyle(.secondary)
            Divider()
            Text(storyTimeDescription)
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .padding(10)
        .frame(minWidth: 180)
    }

    private var storyTimeDescription: String {
        if dot.offsetSource == "default" {
            return "Immediately after \(previousSceneTitle.isEmpty ? "previous scene" : previousSceneTitle)"
        }
        // Gap between the previous scene's end and this scene's start.
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
    case spinner(Int64)          // result of amount + unit + direction
    case anchor(Int64)           // result of a named time-of-day anchor
    case keepPosition(Int64)     // raw drag position accepted as-is
    case resetDefault
}

struct TimeDeltaPicker: View {

    let referenceName: String           // title of the previous scene (for header text)
    let rawOffsetMs: Int64             // drag-computed absolute position (for Keep position)
    let previousSceneEndMs: Int64      // previous scene's offsetMs + durationMs — the base
    let currentDurationMs: Int64       // the scene's current duration
    let defaultDurationMs: Int64       // project default
    let epochLabel: String
    let onResult: (TimeDeltaPickerResult, Int64) -> Void

    // Position spinner state — initialised from drag delta
    @State private var amount: Int = 1
    @State private var unit: DeltaUnit = .hours
    @State private var direction: DeltaDirection = .later

    // Duration spinner state — initialised from currentDurationMs
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

    // Named time-of-day anchors, relative to previousSceneEndMs.
    // Deltas represent a named moment in the story day after the previous scene finishes.
    private struct Anchor: Identifiable {
        let id = UUID()
        let label: String
        let deltaMs: Int64   // added to previousSceneEndMs
    }

    private let anchors: [Anchor] = [
        Anchor(label: "that morning",     deltaMs:  21_600_000),   // +6h
        Anchor(label: "that evening",     deltaMs:  64_800_000),   // +18h
        Anchor(label: "that night",       deltaMs:  79_200_000),   // +22h
        Anchor(label: "before dawn",      deltaMs: 100_800_000),   // +28h  (4am next day)
        Anchor(label: "around 3am",       deltaMs:  97_200_000),   // +27h  (3am next day)
        Anchor(label: "the next morning", deltaMs: 108_000_000),   // +30h  (6am next day)
    ]

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {

            Text(referenceName.isEmpty ? "Time from story open:" : "Time after \"\(referenceName)\" ends:")
                .font(.subheadline)
                .foregroundStyle(.secondary)

            // MARK: Position spinner row
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

            // MARK: Duration row (visually secondary)
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

            // MARK: Named anchors
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

            // MARK: Footer
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

    // MARK: Helpers

    private var spinnerMs: Int64 {
        let delta = Int64(max(amount, 0)) * unit.milliseconds
        return direction == .later
            ? previousSceneEndMs + delta
            : previousSceneEndMs - delta
    }

    private var chosenDurationMs: Int64 {
        max(1, Int64(max(durAmount, 0)) * durUnit.milliseconds)
    }

    // Seed spinners from the gap between the drag position and the previous scene's end.
    private func initialiseSpinner() {
        let delta = abs(rawOffsetMs - previousSceneEndMs)
        direction = rawOffsetMs >= previousSceneEndMs ? .later : .before
        (unit, amount) = bestFit(ms: delta)

        // Duration spinner from current scene duration
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

extension View {
    @ViewBuilder
    func cursor(_ cursor: NSCursor) -> some View {
        #if os(macOS)
        self.onHover { inside in
            if inside { cursor.push() } else { NSCursor.pop() }
        }
        #else
        self
        #endif
    }
}

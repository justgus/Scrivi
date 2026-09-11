import Foundation

// ScriviDiag — PHASE TIMING for the open path (I-0196).
//
// ⚠️ WHY THIS EXISTS. I-0196's first diagnosis was WRONG because it was an
// EXTRAPOLATION, not a measurement: the cost of opening ONE scene (the cheapest,
// first one) was multiplied by the scene count, projecting 263 s. The user
// refuted it with an observation no projection could survive — the app was still
// frozen after an HOUR.
//
// ⚠️ A single total is not enough either. "The load took N seconds" does not say
// WHICH LAYER owns the time, and that is the only question that leads to a fix.
// So this records CUMULATIVE TIME PER NAMED PHASE, plus a call count, and prints
// one table at the end.
//
// ⚠️ It measures WALL TIME on whatever thread the phase runs on. That is
// deliberate: the defect is a frozen UI, so the number that matters is how long
// the main thread was unavailable, not CPU time.
//
// Enabled by the SCRIVI_DIAG_TIMING environment variable so a normal run pays
// nothing. Set it in the Xcode scheme (Product ▸ Scheme ▸ Edit Scheme ▸ Run ▸
// Arguments ▸ Environment Variables) or on the command line.
enum ScriviDiag {

    /// Whether timing is being collected. Read once — an env lookup per scene
    /// would itself distort a per-scene measurement.
    static let timingEnabled: Bool =
        ProcessInfo.processInfo.environment["SCRIVI_DIAG_TIMING"] != nil

    private struct Phase {
        var totalSeconds: Double = 0
        var calls: Int = 0
        var maxSeconds: Double = 0
    }

    // ⚠️ Not thread-safe by design, and that is a deliberate trade: a lock here
    // would serialise the very work being measured. Phases are recorded from the
    // main thread during the open path. If a phase is ever timed off-main, that
    // must be revisited rather than assumed harmless.
    nonisolated(unsafe) private static var phases: [String: Phase] = [:]
    nonisolated(unsafe) private static var order: [String] = []
    nonisolated(unsafe) private static var runStart: Date?

    /// Times `body`, attributing the elapsed wall time to `phase`.
    @discardableResult
    static func measure<T>(_ phase: String, _ body: () throws -> T) rethrows -> T {
        guard timingEnabled else { return try body() }
        if runStart == nil { runStart = Date() }
        let t0 = Date()
        defer {
            let dt = Date().timeIntervalSince(t0)
            var p = phases[phase] ?? Phase()
            if p.calls == 0 { order.append(phase) }
            p.totalSeconds += dt
            p.calls += 1
            p.maxSeconds = max(p.maxSeconds, dt)
            phases[phase] = p
        }
        return try body()
    }

    /// Records a phase whose duration is already known.
    static func record(_ phase: String, seconds: Double) {
        guard timingEnabled else { return }
        if runStart == nil { runStart = Date() }
        var p = phases[phase] ?? Phase()
        if p.calls == 0 { order.append(phase) }
        p.totalSeconds += seconds
        p.calls += 1
        p.maxSeconds = max(p.maxSeconds, seconds)
        phases[phase] = p
    }

    /// A progress breadcrumb during a long loop, so a run that never finishes
    /// still tells us HOW FAR it got and whether the rate is degrading.
    ///
    /// ⚠️ This is the line that would have caught the bad 263 s projection: a
    /// rising per-item cost is visible here and invisible in a total.
    static func tick(_ phase: String, _ index: Int, of total: Int, every: Int = 100) {
        guard timingEnabled, index % every == 0 else { return }
        let elapsed = runStart.map { Date().timeIntervalSince($0) } ?? 0
        let per = index > 0 ? elapsed / Double(index) : 0
        NSLog(String(format: "[SCRIVI-TIMING] %@ %d/%d  elapsed=%.1fs  avg=%.1fms/item  projected=%.0fs",
                     phase, index, total, elapsed, per * 1000, per * Double(total)))
    }

    /// Prints the table. Call at the end of the operation being measured.
    static func report(_ label: String) {
        guard timingEnabled else { return }
        let wall = runStart.map { Date().timeIntervalSince($0) } ?? 0
        NSLog("[SCRIVI-TIMING] ===== \(label) =====")
        NSLog(String(format: "[SCRIVI-TIMING] %-34@ %10@ %8@ %10@ %8@",
                     "phase" as NSString, "total(s)" as NSString,
                     "calls" as NSString, "avg(ms)" as NSString, "max(ms)" as NSString))
        // ⚠️ NESTED PHASES ARE NOT SUMMED. A phase whose name is indented (two
        // leading spaces) runs INSIDE another phase, so adding it to the total
        // would double-count -- which is exactly what produced a nonsensical
        // `unaccounted = -139.06` on the first instrumented run.
        var accounted = 0.0
        for name in order {
            guard let p = phases[name] else { continue }
            if !name.hasPrefix("  ") { accounted += p.totalSeconds }
            NSLog(String(format: "[SCRIVI-TIMING] %-34@ %10.2f %8d %10.2f %8.1f",
                         name as NSString, p.totalSeconds, p.calls,
                         p.calls > 0 ? p.totalSeconds / Double(p.calls) * 1000 : 0,
                         p.maxSeconds * 1000))
        }
        // ⚠️ THE MOST IMPORTANT LINE. If wall time greatly exceeds the sum of the
        // named phases, the cost is somewhere NOT YET INSTRUMENTED — which is
        // exactly the state I-0196 is in, and saying so beats guessing.
        NSLog(String(format: "[SCRIVI-TIMING] %-34@ %10.2f", "WALL CLOCK" as NSString, wall))
        NSLog(String(format: "[SCRIVI-TIMING] %-34@ %10.2f", "sum of named phases" as NSString, accounted))
        NSLog(String(format: "[SCRIVI-TIMING] %-34@ %10.2f  <-- UNINSTRUMENTED if large",
                     "unaccounted (top-level only)" as NSString, wall - accounted))
        NSLog("[SCRIVI-TIMING] (indented phases run INSIDE others and are not summed)")
    }

    /// Clears state so a second open in the same session measures itself only.
    static func reset() {
        phases.removeAll()
        order.removeAll()
        runStart = nil
    }
}

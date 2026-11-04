import Foundation
import Synchronization

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
    // ⚠️ Per-LOOP start times. EP-039 — THE PROBE USED TO LIE, AND DID SO AGAIN.
    //
    // ⚠️ `elapsed` was measured from `runStart` (the start of the whole OPEN), not from the
    // start of the loop being ticked. ✅ Correct for `loadAll`, which begins at open;
    // ⚠️ pure fiction for any loop that starts later.
    // ⚠️ OBSERVED 2026-09-13, on a scene split: `rebuildStorage` printed `elapsed=328.2s` on
    // EVERY line and `projected=3788s`, while its own closing line said `took 0.0 s`.
    // ⚠️ THIS FIX WAS WRITTEN ONCE IN SP-132, LOST IN A BISECT, AND NEVER REAPPLIED —
    // ✅ which is why the misleading numbers came back.
    nonisolated(unsafe) private static var loopStarts: [String: Date] = [:]

    static func tick(_ phase: String, _ index: Int, of total: Int, every: Int = 100) {
        guard timingEnabled else { return }
        if index == 0 || loopStarts[phase] == nil { loopStarts[phase] = Date() }
        guard index % every == 0 else { return }
        let elapsed = Date().timeIntervalSince(loopStarts[phase] ?? Date())
        let per = index > 0 ? elapsed / Double(index) : 0
        NSLog(String(format: "[SCRIVI-TIMING] %@ %d/%d  elapsed=%.1fs  avg=%.1fms/item  projected=%.0fs",
                     phase, index, total, elapsed, per * 1000, per * Double(total)))
        if index >= total - every { loopStarts[phase] = nil }   // ready for a re-run
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
        loopStarts.removeAll()
        runStart = nil
    }
}

// MARK: — [SP-188] AC11: spans and the main-thread watchdog
//
// The Mac's half of Linux's `DiagnosticLog` (platforms/linux/src/DiagnosticLog.hpp): a span logs one `[SCRIVI-SPAN]` line
// when it took ≥ 50 ms (or a begin and an end line when `always`), and the watchdog logs `[SCRIVI-STALL]` when the main
// thread has not answered for 250 ms, with the main-thread spans running then. Same switch as the timing above.
// ⛔ Never the writer's text: names and counts only.
extension ScriviDiag {
    static let slowSpanMs = 50.0
    static let stallMs = 250.0

    private struct Watch {
        var running: [(name: String, since: Date)] = []   // the main thread's open spans, outermost first
        var pingSent: Date?
        var stallReported = false
    }
    private static let watch = Mutex(Watch())

    /// Times `body` on whatever thread it runs; on the main thread it is also what a stall reports as running.
    @discardableResult
    static func span<T>(_ name: String, always: Bool = false, _ body: () throws -> T) rethrows -> T {
        let token = begin(name, always: always)
        defer { end(token) }
        return try body()
    }

    struct SpanToken { let name: String; let since: Date; let onMain: Bool; let always: Bool }

    /// `let span = ScriviDiag.begin("…"); defer { ScriviDiag.end(span) }` — a span over a whole function.
    static func begin(_ name: String, always: Bool = false) -> SpanToken? {
        let token = SpanToken(name: name, since: Date(), onMain: Thread.isMainThread, always: always)
        if always { note("\(name) started") }
        if token.onMain { watch.withLock { $0.running.append((name, token.since)) } }
        return token
    }

    static func end(_ token: SpanToken?) {
        guard let token else { return }
        if token.onMain { watch.withLock { _ = $0.running.popLast() } }
        let ms = Date().timeIntervalSince(token.since) * 1000
        if token.always || ms >= slowSpanMs { note(String(format: "%@ took %.0f ms", token.name, ms)) }
    }

    /// One line, from any thread — into the diagnostic log (always), and the console with `SCRIVI_DIAG_TIMING`.
    static func note(_ text: String) {
        let thread = Thread.isMainThread ? "main" : "worker"
        DiagnosticLog.write(thread: thread, text)
        if timingEnabled { NSLog("[SCRIVI-SPAN] [%@] %@", thread, text) }
    }

    /// Starts the diagnostic log and the watchdog thread (once, at launch).
    static func startWatchdog() {
        DiagnosticLog.start()
        let thread = Thread {
            while true {
                Thread.sleep(forTimeInterval: 0.1)
                let now = Date()
                let stalled: String? = watch.withLock { w in
                    guard let sent = w.pingSent else {
                        w.pingSent = now
                        DispatchQueue.main.async { answered() }
                        return nil
                    }
                    let ms = now.timeIntervalSince(sent) * 1000
                    guard ms >= stallMs, !w.stallReported else { return nil }
                    w.stallReported = true
                    let running = w.running.map { String(format: "%@ (%.0f ms)", $0.name, now.timeIntervalSince($0.since) * 1000) }
                    return String(format: "⚠ the window is not answering (%.0f ms so far); running: %@", ms,
                                  running.isEmpty ? "nothing named" : running.joined(separator: " > "))
                }
                if let stalled { stall(stalled) }
            }
        }
        thread.name = "ScriviDiag watchdog"
        thread.start()
    }

    private static func answered() {
        let now = Date()
        let after: Double? = watch.withLock { w in
            defer { w.pingSent = nil; w.stallReported = false }
            guard w.stallReported, let sent = w.pingSent else { return nil }
            return now.timeIntervalSince(sent) * 1000
        }
        if let after { stall(String(format: "the window is answering again, after %.0f ms", after)) }
    }

    private static func stall(_ text: String) {
        DiagnosticLog.write(thread: "watchdog", text)   // as Linux's: the watchdog thread writes it, about the main thread
        NSLog("[SCRIVI-STALL] %@", text)
    }
}

/// EP-053 S5 ([SP-190] Q1): THE MAC'S DIAGNOSTIC LOG — the structural steps, the verification, the main thread's core calls and
/// its stalls, in Linux's format (`DiagnosticLog.cpp`): `yyyy-MM-dd HH:mm:ss.SSS [thread] text`, one file,
/// `~/Library/Logs/Scrivi/scrivi.log` (in the app's container), always on; the previous session kept as `scrivi.log.1`.
/// Help ▸ Show Log Folder opens the folder. ⛔ NEVER THE WRITER'S TEXT. ⚠️ Every line is one `write()` to an O_APPEND
/// descriptor — unbuffered, so what was written survives a crash.
enum DiagnosticLog {
    private static let fd = Mutex<Int32>(-1)

    static var folder: URL {
        FileManager.default.urls(for: .libraryDirectory, in: .userDomainMask)[0].appendingPathComponent("Logs/Scrivi")
    }
    static var file: URL { folder.appendingPathComponent("scrivi.log") }

    static func start() {
        fd.withLock { fd in
            guard fd < 0 else { return }
            try? FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
            let path = file.path(percentEncoded: false)
            rename(path, path + ".1")
            fd = open(path, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0o644)
        }
        let info = Bundle.main.infoDictionary ?? [:]
        write(thread: "main", "==== Scrivi log — Scrivi (macOS) \(info["CFBundleShortVersionString"] as? String ?? "—") "
                              + "(\(info["ScriviCommit"] as? String ?? "—")) — pid \(getpid())")
    }

    static func write(thread: String, _ text: String) {
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd HH:mm:ss.SSS"
        let line = Array("\(formatter.string(from: Date())) [\(thread)] \(text)\n".utf8)
        fd.withLock { fd in
            guard fd >= 0 else { return }
            _ = line.withUnsafeBytes { Darwin.write(fd, $0.baseAddress, $0.count) }
        }
    }
}

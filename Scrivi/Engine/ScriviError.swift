import Foundation

// ScriviError — the single error type thrown by ScriviEngine.
// Carries the numeric error code and message from the C++ ScriviCore backend.

public struct ScriviError: Error, LocalizedError, Sendable {
    public let code:    Int
    public let message: String

    /// Machine-readable discriminator, when the code alone is not specific enough
    /// (`scrivi_c_api.cpp:164`). The case that matters today is
    /// `"worldUnavailable:<status>"` — a world-scoped read or write that cannot
    /// reach its package, and the graph refusing to modify an edge toward it
    /// (`worlds::worldUnavailableDetail`).
    ///
    /// ⚠️ EP-031 SP-099 / T-0407: this was previously **dropped at the boundary**.
    /// The C ABI has always emitted it; `ErrorPayload` simply never decoded it, so
    /// Swift could not tell a frozen-graph refusal from any other failure. Pending
    /// presentation (SP-102, AC23) is unbuildable without it.
    public let detail: String?

    /// Filesystem path the error concerns, when the backend names one.
    public let path: String?

    public init(code: Int, message: String, detail: String? = nil, path: String? = nil) {
        self.code    = code
        self.message = message
        self.detail  = detail
        self.path    = path
    }

    /// What `error.localizedDescription` returns (I-0222).
    ///
    /// ⚠️ **WITHOUT `LocalizedError` CONFORMANCE THIS TYPE RENDERED AS
    /// `"The operation couldn't be completed. ScriviApp.ScriviError 1"`.** Foundation
    /// falls back to `<Module>.<Type> <code>` for a plain `Error`, so the `message`
    /// the C ABI had already composed — *"world '…' is unmounted"* — was built,
    /// carried across the boundary, decoded, and then thrown away at the last step.
    ///
    /// ⚠️ **It is not one screen's problem.** ~15 call sites across `WorldsView`,
    /// `ObjectCard`, `WritingToolCards`, `ObjectPickerView` and `WorldWarningView`
    /// display `error.localizedDescription` directly, so EVERY error this app has
    /// shown a writer has been rendered this way.
    ///
    /// ⚠️ The `path` is appended only when the backend named one AND it is not
    /// already quoted in the message: the core's messages frequently embed the
    /// identifier themselves, and repeating it reads like two separate faults.
    public var errorDescription: String? {
        guard let path, !path.isEmpty, !message.contains(path) else { return message }
        return "\(message) (\(path))"
    }
}

// MARK: — Unreachable-world discrimination (Doc 3 §4.6)
//
// ⚠️ **THERE WAS A SECOND TRIO HERE — `pendingPrefix` / `isWorldPending` /
// `pendingWorldStatus` — MATCHING `"worldPending:"`. It was RETIRED 2026-09-17
// (I-0222) on the user's ruling: "there will be no ambiguity in this message."**
//
// ⚠️ **Both trios worked.** The core emitted both spellings and Swift decoded
// both correctly; neither was broken. The defect was that ONE writer-visible
// condition — *"the world is away, reconnect it"* — was expressed as TWO
// concepts, so a view had to know which store its error came from in order to
// ask the right question. ⛔ **The card list asked neither, which is how a
// perfectly decodable error reached the writer as `ScriviError 1`.**

public extension ScriviError {
    /// The `worldUnavailable:` detail prefix the OBJECT endpoints use when a
    /// world-scoped read or write cannot reach its package (`ObjectStore.cpp`).
    private static let unavailablePrefix = "worldUnavailable:"

    /// True when this error is *"the world is away"* rather than a real failure.
    ///
    /// ⚠️ **This is the SINGLE discriminator for an unreachable world** (I-0222).
    /// It covers both a graph write refused toward a frozen edge and an object
    /// read that cannot reach its package at all — `openObject` on an ejected
    /// drive is the everyday case. ✅ The two were separate spellings until
    /// 2026-09-17; the writer's remedy was always identical.
    ///
    /// ⚠️ **It must never be reported as breakage** (R9). The writer's object is
    /// not damaged and not missing; it is temporarily out of reach, and the UI
    /// owes her that sentence instead of a raw error code (I-0166).
    var isWorldUnavailable: Bool {
        detail?.hasPrefix(Self.unavailablePrefix) ?? false
    }

    /// The world's status when `isWorldUnavailable` — nil for any other error.
    ///
    /// Diagnostic, not behavioral (Doc 2 §7.2.1): the behaviour is the same for
    /// every status, but the REMEDY differs — reconnect a drive, find a moved
    /// package — so the sentence must not guess.
    var unavailableWorldStatus: WorldStatus? {
        guard let detail, detail.hasPrefix(Self.unavailablePrefix) else { return nil }
        return WorldStatus(rawValue: String(detail.dropFirst(Self.unavailablePrefix.count)))
            ?? .unavailable
    }
}

/// Why a world cannot be reached, as reported by ScriviCore
/// (`worlds::worldStatusName`). Mirrors the C++ enum by raw value.
///
/// ⚠️ `offline` and `unmounted` are **declared in the core but never produced by
/// it** (`WorldTypes.hpp:68` — "platform-layer refinement"); ScriviCore emits only
/// `missing` or `unavailable`. Doc 3 §4.4.1 forbids a platform-specific *model*,
/// so refining them is Apple-layer work feeding this neutral enum — scoped to
/// SP-102 / T-0389 (EP-031 AC24). They are represented here so that layer has
/// somewhere to report into, and so a status string from a newer core still
/// decodes.
public enum WorldStatus: String, Codable, Sendable, CaseIterable {
    case available
    case offline
    case unmounted
    case missing
    case unavailable

    /// True for every status except `available` — i.e. the world's objects cannot
    /// be resolved and its edges are held pending.
    public var isUnavailable: Bool { self != .available }

    /// What the writer should actually be told. Never guesses: `unavailable` says
    /// so plainly rather than inviting a destructive remedy. A wrong "missing" is
    /// worse than an honest "unavailable" (Doc 2 §7.2.1).
    public var writerDescription: String {
        switch self {
        case .available:   return "available"
        case .offline:     return "offline"
        case .unmounted:   return "on a disconnected volume"
        case .missing:     return "missing"
        case .unavailable: return "unavailable"
        }
    }
}

// MARK: — JSON envelope decoding (internal)

struct Envelope<T: Decodable>: Decodable {
    let ok:     Bool
    let result: T?
    let error:  ErrorPayload?
}

struct ErrorPayload: Decodable {
    let code:    Int
    let message: String
    // Both are omitted by the C ABI when empty, so both must be optional.
    let detail:  String?
    let path:    String?
}

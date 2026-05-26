import Foundation

// ScriviError — the single error type thrown by ScriviEngine.
// Carries the numeric error code and message from the C++ ScriviCore backend.

public struct ScriviError: Error, Sendable {
    public let code:    Int
    public let message: String
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
}

func decode<T: Decodable>(_ cxxString: std.string) throws -> T {
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

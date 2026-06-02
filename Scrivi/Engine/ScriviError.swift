import Foundation

// ScriviError — the single error type thrown by ScriviEngine.
// Carries the numeric error code and message from the C++ ScriviCore backend.

public struct ScriviError: Error, Sendable {
    public let code:    Int
    public let message: String

    public init(code: Int, message: String) {
        self.code    = code
        self.message = message
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
}


# Task-verified-0012

## T-0012: Identity Service and UUID Provider

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — identity layer
**Priority:** High
**Epic:** EP-003: Identity and First Launch
**Date Requested:** 2026-05-20
**Date Implemented:** 2026-05-20
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-003

**Rationale:**
`ensureLocalIdentity()` was a stub. This task implemented the full `IdentityService` backed by `SecureStore` and a real `SystemUUIDProvider` generating UUID v7-style IDs, enabling the Swift interop prototype to exercise a genuine end-to-end path.

**Implementation Details:**
- `src/identity/IdentityService.hpp/.cpp` — checks `SecureStore` for existing identity; creates new `IdentityID`/`PersonaID` via `UUIDProvider` on first call; stores JSON bundle as `SecretBytes`
- `src/platform/SystemUUIDProvider.hpp/.cpp` — UUID v7-style with 48-bit Unix ms timestamp, random low bits, RFC 9562 version/variant nibbles, type prefix
- `tests/integration/IdentityTests.cpp` — 6 tests (tests 60–65 in final suite): new identity, prefixed IDs, idempotency, SecureStore failure, no plaintext in identity dir, createProject with real AuthorshipRef

---

*Last Updated: 2026-05-26*

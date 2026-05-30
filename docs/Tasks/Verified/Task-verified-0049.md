# T-0049: `KeychainSecureStore` — macOS Keychain Implementation

**Status:** ✅ Verified
**Component:** ScriviCore / platform / KeychainSecureStore (Apple)
**Priority:** Critical
**Date Requested:** 2026-05-28
**Date Implemented:** 2026-05-28
**Date Verified:** 2026-05-29
**Sprint Assigned:** SP-014

**Rationale:**
`PrototypeSecureStore` was an in-memory `std::unordered_map` that did not survive process restart, losing identity (private key material, identityID) on every relaunch. Architecture v0.3 §8 and Open Question 3 flagged this as a pre-user-testing blocker. EP-006 requires working persistent identity before UI work can be verified.

**Implementation Details:**
- `KeychainSecureStore.hpp`: Declares `KeychainSecureStore` implementing `scrivi::SecureStore` (`store`, `load`, `remove`)
- `KeychainSecureStore.cpp`: Implements all three methods using macOS Security framework (`SecItemAdd`, `SecItemCopyMatching`, `SecItemUpdate`, `SecItemDelete`); `kSecClass = kSecClassGenericPassword`, `kSecAttrService = "com.scrivi.core"`, `kSecAttrAccount = key`; `store` upserts (update if exists, add otherwise); `load` returns `std::optional<std::string>` (nullopt if not found); `remove` silently succeeds on `errSecItemNotFound`
- `ScriviCoreAdapter.cpp`: `Impl` now wires `KeychainSecureStore` in place of `PrototypeSecureStore`; `PrototypeSecureStore` definition retained in the `.cpp` for test isolation
- `Package.swift`: `-framework Security` linker flag added to `ScriviCoreAdapter` target
- `ScriviCore.xcodeproj/project.pbxproj`: Both new files (`KeychainSecureStore.hpp`, `KeychainSecureStore.cpp`) added

**Components Affected:**
- `platforms/apple/Sources/ScriviCoreAdapter/KeychainSecureStore.hpp` — new file
- `platforms/apple/Sources/ScriviCoreAdapter/KeychainSecureStore.cpp` — new file
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp` — wired `KeychainSecureStore`
- `platforms/apple/Package.swift` — `-framework Security` linker flag
- `ScriviCore.xcodeproj/project.pbxproj` — new files registered

---

*Verified: 2026-05-29 (user approved)*

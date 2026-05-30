# T-0050: macOS App Target — Xcode Project Setup

**Status:** ✅ Verified
**Component:** ScriviCore.xcodeproj / platforms/apple/Sources/ScriviApp/
**Priority:** Critical
**Date Requested:** 2026-05-29
**Date Implemented:** 2026-05-29
**Date Verified:** 2026-05-29
**Sprint Assigned:** SP-015

**Implementation Details:**
- `PBXNativeTarget` (T003) added to `ScriviCore.xcodeproj`; `PRODUCT_NAME = ScriviApp` (avoids `.swiftmodule` collision with the `Scrivi` SPM library product), `MACOSX_DEPLOYMENT_TARGET = 26.0`, `SWIFT_VERSION = 6.0`, `SDKROOT = macosx`, `SWIFT_OBJC_INTEROP_MODE = objcxx`
- `XCLocalSwiftPackageReference` (SPM_LOCAL_REF) and `XCSwiftPackageProductDependency` (SPM_PRODUCT) registered; `Scrivi` library product linked in Frameworks phase
- `CODE_SIGN_IDENTITY = "-"` (ad-hoc) with `CODE_SIGN_STYLE = Manual` — allows local launch without a provisioning profile
- `platforms/apple/Sources/ScriviApp/ScriviApp.swift` — `@main App` stub
- `platforms/apple/Sources/ScriviApp/Info.plist` — bundle ID `com.caposoft.scrivi`; UTI `com.caposoft.scrivi.project` conforming to `com.apple.package`
- `platforms/apple/Sources/ScriviApp/Scrivi.entitlements` — sandbox disabled, user-selected read-write; `keychain-access-groups` omitted (triggers unwanted password prompt with ad-hoc signing)
- `ScriviCore.xcodeproj/xcshareddata/xcschemes/Scrivi.xcscheme` — new scheme
- Info.plist removed from Resources phase (referenced via `INFOPLIST_FILE` only)

**Issues encountered and resolved:**
- Duplicate `.swiftmodule`: fixed by setting `PRODUCT_NAME = ScriviApp`
- Info.plist in Resources phase: removed from copy phase
- `cplusplus` feature error: fixed by adding `SWIFT_OBJC_INTEROP_MODE = objcxx`
- Run/Test/Profile greyed out: fixed by adding `CODE_SIGN_IDENTITY = "-"`
- Keychain password prompt: fixed by removing `keychain-access-groups` from entitlements

---

*Verified: 2026-05-29 (user approved)*

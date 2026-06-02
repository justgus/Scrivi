# Scrivi — Android Platform

**Status:** Deferred — not yet started.

## Architecture (planned)

- **GUI:** Qt for Android or SKIP (Swift → Jetpack Compose)
- **Backend:** ScriviCore (C++23 static library, built by CMake via Android NDK)
- **Build system:** CMake + Android NDK + Gradle

## Maturity Criteria

Android development begins when **all** of the following are true:

### Apple Platform — Mature
- All features in the active Epic backlog are implemented and verified on Apple platforms
- No open Issues rated higher than "minor"
- The ScriviCore public API has been stable (no breaking changes) for at least two consecutive sprints

### Apple Platform — Solid
- The macOS app has shipped at least one TestFlight build
- The Windows build (Qt/QML) passes CI on every commit
- The Qt/QML Windows project covers feature parity with the Apple app (same screens, same operations, same ScriviCore API calls)

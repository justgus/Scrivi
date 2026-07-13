# Scrivi — Android Platform

**Status:** Deferred — not yet started.

## Architecture (planned)

- **GUI:** Qt for Android or SKIP (Swift → Jetpack Compose)
- **Backend:** ScriviCore (C++23 static library, built by CMake via Android NDK)
- **Build system:** CMake + Android NDK + Gradle

## Maturity Criteria

> **Note (2026-07-13):** The original gate below tied *all* non-Apple GUI work to full Apple-platform
> maturity. That has been **superseded for the Linux (Ubuntu) app by user decision** — an alpha tester
> with no Mac made a Linux build the priority. Linux Qt/QML development is proceeding now under the
> `[Linux]` Epic family (EP-020+). **Android remains deferred** and is still gated, but the gate is
> restated below in Linux-relative terms rather than Apple-relative terms, since Linux is now the
> reference cross-platform GUI.

Android development begins when **all** of the following are true:

### Reference GUI (Linux Qt/QML) — Mature
- The Linux (Ubuntu) Qt/QML app has reached near-parity with the Apple app and passes CI on every commit.
- The ScriviCore public API (`scrivi.h`) has been stable (no breaking changes) for at least two
  consecutive sprints.
- No open Issues rated higher than "minor" against the Linux app.

### Reference GUI (Linux Qt/QML) — Solid
- The Linux app has been exercised by at least one real alpha tester on real Ubuntu.
- The Windows Qt/QML build (which reuses the Linux QML codebase) passes CI on every commit.
- The Qt/QML codebase covers feature parity with the Apple app (same screens, same operations, same
  ScriviCore API calls).

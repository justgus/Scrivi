---
name: T-0080
title: "Auto-save — 1-second debounce Task after last keystroke; scene-exit immediate save; app-resign immediate save + backup placeholder hook"
sprint: SP-023
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

- **1-second debounce**: `Coordinator.textDidChange` cancels and reschedules a `Task.sleep(1s)` on every keystroke; fires `saveCurrentIfDirty` when it runs uninterrupted.
- **Scene-exit save**: `textViewDidChangeSelection` and `textDidChange` detect cursor crossing a `sceneBoundaries` boundary and call `saveAndRelease(at:oldIdx)` immediately.
- **App-resign**: `ScriviApp.swift` subscribes to `NSApplication.willResignActiveNotification` and calls `env.onAppResign()`.
- **Backup placeholder**: `AppEnvironment.onAppResign()` saves current scene then has `// TODO: backup` — backup logic deferred to future Epic.

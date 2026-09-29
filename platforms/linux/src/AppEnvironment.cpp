#include "AppEnvironment.hpp"

#include "ScriviWindow.hpp"

// ⚠️ [SP-146] T-0560 — R7: QUIT FLUSHES EVERY SESSION, NOT ONE.
//
// ⛔ THE OLD HOOK WAS SINGULAR BY CONSTRUCTION, and that is worth stating plainly:
// `main.cpp` constructed ONE `ScriviWindow` BY VALUE and connected
// `QCoreApplication::aboutToQuit` to THAT INSTANCE's `flushEditor`. ⚠️ With N
// windows it would have flushed whichever one `main()` happened to hold and
// SILENTLY DROPPED the edits in every other — ✅ the exact shape of data loss R7
// exists to prevent.
//
// ✅ Now quit asks the WINDOW MANAGER for every open window and flushes each.
// ⚠️ `flushEditor()` is a no-op when a window has no project, so a Landing-only
// window costs nothing.
void AppEnvironment::flushAllWindows()
{
    // ⚠️ Iterate a COPY: `flushEditor()` writes to disk and must not be affected by
    // anything that mutates the map mid-loop. ✅ `allWindows()` already returns a
    // value list, so this is a copy by construction.
    const QList<ScriviWindow*> windows = windows_.allWindows();
    for (ScriviWindow* window : windows) {
        if (window != nullptr) {
            window->flushEditor();
        }
    }
}
